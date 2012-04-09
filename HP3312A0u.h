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

#define  HP33120A                        1       /* callback function: hp3312aPanelCallback */
#define  HP33120A_FREQ                   2       /* callback function: hp3312aControlCallback */
#define  HP33120A_AMPL                   3       /* callback function: hp3312aControlCallback */
#define  HP33120A_WAVE                   4       /* callback function: hp3312aControlCallback */
#define  HP33120A_DUTY                   5       /* callback function: hp3312aControlCallback */
#define  HP33120A_GPIB                   6


     /* Menu Bars, Menus, and Menu Items: */

#define  HP3312MENU                      1
#define  HP3312MENU_SOURCES              2
#define  HP3312MENU_SOURCES_AMPL         3       /* callback function: hp3312aMenuCallback */
#define  HP3312MENU_SOURCES_FREQ         4       /* callback function: hp3312aMenuCallback */


     /* Callback Prototypes: */ 

int  CVICALLBACK hp3312aControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK hp3312aMenuCallback(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK hp3312aPanelCallback(int panel, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
