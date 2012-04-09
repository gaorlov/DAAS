/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/* Copyright (c) National Instruments 2007. All Rights Reserved.          */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  K213                            1       /* callback function: K213ControlPanelCallback */

#define  K213PORT                        2       /* callback function: K213ControlPanelCallback */
#define  K213PORT_DISPLAY                2       /* callback function: K213ControlCallback */
#define  K213PORT_AUTORANGE              3       /* callback function: K213ControlCallback */
#define  K213PORT_RANGE                  4       /* callback function: K213ControlCallback */
#define  K213PORT_TITLE                  5
#define  K213PORT_SETUP                  6       /* callback function: K213ControlCallback */


     /* Menu Bars, Menus, and Menu Items: */

#define  K213MENUS                       1
#define  K213MENUS_FILE                  2
#define  K213MENUS_FILE_LOAD             3       /* callback function: K213LoadCallback */
#define  K213MENUS_FILE_SAVE             4       /* callback function: K213SaveCallback */
#define  K213MENUS_FILE_SEP_1            5
#define  K213MENUS_FILE_GPIB             6       /* callback function: OperateDevCallback */


     /* Callback Prototypes: */ 

int  CVICALLBACK K213ControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK K213ControlPanelCallback(int panel, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK K213LoadCallback(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK K213SaveCallback(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK OperateDevCallback(int menubar, int menuItem, void *callbackData, int panel);


#ifdef __cplusplus
    }
#endif
