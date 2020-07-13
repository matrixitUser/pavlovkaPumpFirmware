/*
  timeout.c
*/

#include <stdlib.h>
#include "common.h"

#include "timeout.h"

#ifndef NUMTIMEOUTS
#error "Es necessari definir NUMTIMEOUTS per que el modul 'timeout' sigui operatiu"
#endif

#ifndef EnterCritical
#warning "EnterCritical() not defined void definition done"
#define EnterCritical() 
#endif

#ifndef ExitCritical
#warning "ExitCritical() not defined void definition done"
#define ExitCritical()
#endif

/**
 * @defgroup TimeOut
 * @ingroup Embedded_Lib
 * @brief TimeOut Object
 * \{
 * \author Isidre Sole Ramon
 * \version 1.0.1
 */
/**
 * @struct tag_timeout
 * @brief TimeOut object.
 * @details Generic object to implement a versatile timeout system if it is used with a timer hardware.
 */
typedef struct tag_timeout{
  /// - Value to use to control ticks of timer for concrete TimeOuts
  uint16_t Counter;
  /// - Flag to indicate if the object had been associated to a system.
  unsigned bAssigned :1;
  /// - Flag to indicat if the object has beed armed to be launched when the TimeOut time had been passed.
  unsigned bArmed : 1;
  unsigned nothing : 6;
  /// - Generic pointer to pass parameters to the callback function that signals the timeout event.
  void *pParams;
  uint32_t uParam;
  /// - Pointer to the callback function that will signal the timeout event.
  SIGNAL_CB pTimeOutFunc;
}TIMEOUT,*PTIMEOUT;

#define TIMEOUTSIZE sizeof(TIMEOUT)

/**
 * @var theTimeOuts[]
 * @brief Set of TimeOut Objects.
 * @details Array of tag_timeout objects that will be used for different objects on the system.
 */
TIMEOUT theTimeOuts[NUMTIMEOUTS];

/**
 * @brief Initialization function.
 * @details Initializes all tag_timeout objects to zero. That function must be called only once at the start up of the system.
 * @param none.
 * @return nothing.
 */
void initTimeOuts(void){
  memset(theTimeOuts,0,NUMTIMEOUTS*TIMEOUTSIZE);
}

/**
 * @brief Assigns a tag_timeout object to the caller object.
 * @details Called for an object (state machine for example) to get control to one of the tag_timeouts of theTimeOuts[] array.
 * @param pFunc Pointer function to be used to signal the TimeOut.
 * @return A handler to the tag_timeout object assigned to the caller. If no objects remain to be assigned then a NULL value is returned.
 */
HTIMEOUT  createTimeOut(SIGNAL_CB pFunc){
  int i;
  if(pFunc == NULL) return NULL;
  for(i =0;i<NUMTIMEOUTS;i++){
    if(theTimeOuts[i].bAssigned == false){
      theTimeOuts[i].bAssigned = true;
      theTimeOuts[i].pTimeOutFunc = pFunc;
      theTimeOuts[i].pParams = NULL;
      theTimeOuts[i].uParam = 0;
      return (HTIMEOUT) &theTimeOuts[i];
    }
  }
  return NULL;
}

/**
 * @brief Enables the timeout object.
 * @details Sets the tag_timeout object handled by hTO in Armed mode and begins to count WaitTime time from now. When the event is signaled (by calling the CallBack function set on the previous createTimeOut function) the value in pParam will be passed to the CallBack function.
 * @param hTO Handler to the tag_timeout object. That value is the one returned by createTimeOut function.
 * @param WaitTime Number of Timer ticks that will be pass before the signaling event. For example : If the TimeOut system is attached to a 16uS timer to signal 1 second  the WaitTime must be 62500d or 0xF424 in hexadecimal.
 * @param pParam Pointer to be passed to the callback funtion when the timeout event signals.
 * @return nothing.
 */
void launchTimeOut(HTIMEOUT hTO,uint16_t WaitTime,void *pParam,uint32_t uParam){
  PTIMEOUT  pTimeOut = (PTIMEOUT) hTO;
  
  if(hTO == NULL || WaitTime == 0 || pTimeOut->bAssigned == false) return;
  EnterCritical();
  pTimeOut->Counter = WaitTime;
  pTimeOut->pParams = pParam;
  pTimeOut->uParam = uParam;
  pTimeOut->bArmed = true;
  ExitCritical();
}

/**
 * @brief Checks the state of the TimeOut object.
 * @details Informs if the object had been armed or not.
 * @param hTO Handler to the TimeOut object.
 * @return True if it is armed, otherwise False.
 */
bool isTimeOutLaunched(HTIMEOUT hTO){
  PTIMEOUT  pTimeOut = (PTIMEOUT) hTO;
  if(hTO == NULL || pTimeOut->bAssigned == false) return false;
  return (bool)(pTimeOut->bArmed);
}

/**
 * @brief Aborts the timeout count.
 * @details In a Armed tag_timeout object it passes to unarmed, and never the signaling event will occur until a new call to LaunchTimeOut function.
 * @param hTO Handler to the TimeOut object.
 * @return true if the object is unarmed, false in case that the object is incorrect or it is not armed.
 */
bool abortTimeOut(HTIMEOUT hTO){
  PTIMEOUT  pTimeOut = (PTIMEOUT) hTO;
  if(hTO == NULL || pTimeOut->bAssigned == false) return false;
  EnterCritical();
  pTimeOut->bArmed = false;
  ExitCritical();
  return true;
}

/**
 * @brief Free the object to be used for other part of the system.
 * @details Free the association of a TimeOut object to the calling system.
 * @param hTO Handler to the TimeOut object.
 * @return true if the object is unsigned, false in case that the object is incorrect or it is not assigned.
 */
bool destroyTimeOut(HTIMEOUT hTO){
  PTIMEOUT  pTimeOut = (PTIMEOUT) hTO;
  if(hTO == NULL || pTimeOut->bAssigned == false) return false;
  EnterCritical();
  pTimeOut->bArmed = false;
  pTimeOut->bAssigned = false;
  ExitCritical();
  return true;
}

/**
 * @brief Timer Associated function.
 * @details This function must be placed to the hardware timer to be executed at each tick. It processes all tag_timeout objects and signals the witch ones that reaches the zero value on the counter.
 * @param none
 * @return nothing
 */
void processTimeOuts(void){
  int i;
  for(i=0;i<NUMTIMEOUTS;i++){
    if(theTimeOuts[i].bArmed && theTimeOuts[i].bAssigned){
      if(theTimeOuts[i].Counter == 0){
        theTimeOuts[i].bArmed = false;
        if(theTimeOuts[i].pTimeOutFunc != NULL)
          theTimeOuts[i].pTimeOutFunc((HTIMEOUT)&theTimeOuts[i],theTimeOuts[i].pParams,theTimeOuts[i].uParam);
      }else theTimeOuts[i].Counter--;
    }
  }
}
/**
 * \}
 */

