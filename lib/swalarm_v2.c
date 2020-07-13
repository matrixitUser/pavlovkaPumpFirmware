#include "main.h"

#ifdef __SWALARM_H__

/*
 * ������� ������:
 *
 * � ����� ����� ���� ���������� _Add ��� _AddInterval
 * �������, ������� ��������� ��������� �� ������������ ����� 1 ��� ����� ������� �������
 *
 * � _Add ������� �������� 1 ����� �������, �.� 4 �������
 * ������� ����� ���� ������� � ����� ����� (0~3.99 �������) �������� ������ �������
 * ������������ ������� ����� ��������� �� ������ ���������� ������ �������
 *
 * |XX.XX!.XXXX!XXXX|
 * 	  ^  ^     ^- ��������� ����������� Action
 * 	  |  |------- ������������ Action � ����������� Add
 * 	  |----������ Add (�� main)
 *
 * 1. ADD - ���������� ���������� �� ��������� ����
 * 2. ACTION - ����������
 * 3. NEXT - ����� ���������� ���������� � ��� ��������� - ��������� ����
 * 4.
 *
 */

PRIVATE SWAlarm alarm[SWALARM_TOTAL];
PRIVATE u32 next_div;


//������ hdl ��� ����������� �����������
PRIVATE u8 _alahdl = 0;
PRIVATE u8 handler_Next(void)
{
	++_alahdl;
	if(_alahdl == 0) {
		_alahdl = 1;
	}
	return _alahdl;
}

// ������� ���� ����������� expire
// ������ ����������� ����� ���� (� main)
//
PRIVATE u32 NextDiv(u32 time_div)
{
	u8 i;
	u32 min = 0xFFFFFFFF; 												// ��� �����������
	for(i=0;i<SWALARM_TOTAL;i++)
	{
		SWAlarm *pala = &alarm[i];
		if(pala->state != SWALARM_STATE_OFF)							// ��������� ��������:
		{
			/*
			if(pala->state == SWALARM_STATE_INTERVAL
					|| pala->state == SWALARM_STATE_INTERVAL_ONCE)		// ��������� ������������:
			{
				if(time_div < pala->offset_div)							// ������� ����� ������ ��������
				{
					pala->expire_div = pala->offset_div;				// ������ ������� 0
				}
				else
				{
					u32 time_offset_div = time_div - pala->offset_div;	//�������� �������
					pala->expire_div = time_div + pala->interval_div - (time_offset_div % pala->interval_div); //������ expire
				}
			}
			else														// ��������� ���������
			{
			}
			*/
			min = MIN(pala->expire_div, min);							//����� ��������
			if(min <= time_div)	break;
		}
	}
	return min;
}
//
PRIVATE u32 AlarmGetExpireDiv(u32 time_div, SWAlarm *pala)
{
	u32 time_offset_div = time_div - pala->offset_div;	//�������� �������
	return pala->expire_div = time_div + pala->interval_div - (time_offset_div % pala->interval_div); //������ expire
}

// ���������� ����������
// ���������� 0, ���� �� ������ ���������� ���������
// ���������� handler (1 ~ SWALARM_TOTAL) � ������ ������
PUBLIC u8 SWAlarm_Add(u16 delay_sec, ACTION_CALLBACK(callback))
{
	u16 delay_div = SWALARM_DIV(delay_sec);
	if(callback == NULL) return 0;

	u32 time_div = SWALARM_DIV(Override_RTC_GetCounter());

	SWAlarm *pala = NULL;

	u8 i;
	for(i=0;i<SWALARM_TOTAL;i++)
	{
		pala = &alarm[i];
		if(pala->state == SWALARM_STATE_OFF)						//��������� ��������:
		{
			pala->state = _SWALARM_STATE_TRANSITION;
			//
			pala->handler = handler_Next();
			pala->expire_div = time_div + delay_div;						//��������� ����������

			pala->sCb.cb = callback;
			pala->sCb.hdl = pala->handler;
			pala->sCb.param = SWALARM_UNDIV(pala->expire_div);
			//
			pala->state = SWALARM_STATE_ON;
			break;
		}
	}

	if(i==SWALARM_TOTAL) return 0;									//������!
	RtcAlarm_Reset();

	return pala->handler;
}

// ���������� 0, ���� �� ������ ���������� ���������
// ���������� handler (1 ~ SWALARM_TOTAL) � ������ ������
PUBLIC u8 SWAlarm_AddInterval(u32 interval_sec, u16 offset_sec, ACTION_CALLBACK(callback))
{
	u32 interval_div = SWALARM_DIV(interval_sec);
	u16 offset_div = SWALARM_DIV(offset_sec);
	u32 time_div = SWALARM_DIV(Override_RTC_GetCounter());
	
	if(interval_div < offset_div) 		//�������� ������, ��� ��������
	{
		offset_div %= interval_div;
	}

	SWAlarm *pala = NULL;

	u8 i;
	for(i=0;i<SWALARM_TOTAL;i++)
	{
		pala = &alarm[i];
		if(pala->state == SWALARM_STATE_OFF)						//��������� ��������:
		{
			pala->state = _SWALARM_STATE_TRANSITION;
			//
			pala->handler = handler_Next();

			pala->interval_div = interval_div;
			pala->offset_div = offset_div;
			pala->expire_div = (time_div < pala->offset_div)? pala->offset_div : AlarmGetExpireDiv(time_div, pala);

			pala->sCb.cb = callback;
			pala->sCb.hdl = pala->handler;
			pala->sCb.param = SWALARM_UNDIV(pala->expire_div);
			//
			pala->state = SWALARM_STATE_INTERVAL;
			break;
		}
	}

	if(i==SWALARM_TOTAL) return 0;									//������!
	RtcAlarm_Reset();

	return pala->handler;
}

/*
PUBLIC void SWAlarm_Del(u8 hdl)
{
	if(hdl==0 || hdl > SWALARM_TOTAL) return;

	SWAlarm *pala = &alarm[hdl-1];
	pala->state = SWALARM_STATE_OFF;			//�������� ����������
	pala->callback = NULL;
}
*/

PUBLIC void SWAlarm_InitVars(void)
{
	u8 i;
	for(i=0;i<SWALARM_TOTAL;i++)
	{
		alarm[i].state = SWALARM_STATE_OFF;
	}
}

//��������� ������ => ���� ��� ��������� �������� ����������� ���������
//
PUBLIC void SWAlarm_Tick(void)
{
	u32 time_div = next_div;
	u8 updated = FALSE;
	
	u8 i;
	for(i=0;i<SWALARM_TOTAL;i++)
	{
		SWAlarm *pala = &alarm[i];
		if((pala->state == SWALARM_STATE_ON) || (pala->state == SWALARM_STATE_INTERVAL_ONCE))
		{
			if(time_div >= pala->expire_div)					//��������� ������:
			{
				pala->state = _SWALARM_STATE_TRANSITION;
				//
				pala->sCb.param = SWALARM_UNDIV(pala->expire_div);
				Callback_Save(&pala->sCb);
				updated = TRUE;
				//
				pala->state = SWALARM_STATE_OFF;
			}
		}
		else if(pala->state == SWALARM_STATE_INTERVAL)
		{
			if(time_div >= pala->expire_div)					//��������� ������:
			{
				pala->state = _SWALARM_STATE_TRANSITION;
				//
				pala->sCb.param = SWALARM_UNDIV(pala->expire_div);
				Callback_Save(&pala->sCb);
				pala->expire_div = AlarmGetExpireDiv(time_div, pala);	//�������������
				updated = TRUE;
				//
				pala->state = SWALARM_STATE_INTERVAL;
			}
		}
	}

	if(updated == TRUE)
	{
		RtcAlarm_Reset();
	}
}

//PUBLIC void SWAlarm_Next(void)
//{
//	u32 time = RTC_GetCounter();
//	u32 time_div = SWALARM_DIV(time);
//
//	u32 newalarm_div = NextDiv(time_div);
//	if(newalarm_div != 0xFFFFFFFF)
//	{
//		next_div = newalarm_div;
//		u32 newalarm = SWALARM_UNDIV(newalarm_div);
//		RTC_SetAlarm(newalarm);
//		RTC_WaitForLastTask();
//	}
//}

//����������� � RTC Alarm (����� ����)

PUBLIC u32 SWAlarm_Next(u32 time)
{
	u32 time_div = SWALARM_DIV(time);
	u32 newalarm_div = NextDiv(time_div);

	if(newalarm_div == 0xFFFFFFFF) return 0;

	next_div = newalarm_div;

	u32 newalarm = SWALARM_UNDIV(newalarm_div);
	return newalarm;
}

#endif
