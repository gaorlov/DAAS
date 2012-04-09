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

#define  HP4156                          1       /* callback function: hp4156_PanelCallback */
#define  HP4156_INTEGR_SLOW              2       /* callback function: hp4156_UnivControlCallback */
#define  HP4156_INTEGR_FAST              3       /* callback function: hp4156_UnivControlCallback */
#define  HP4156_SLI                      4       /* callback function: hp4156_UnivControlCallback */
#define  HP4156_DECORATION               5

#define  HP4156PORT                      2       /* callback function: hp4156_PanelCallback */
#define  HP4156PORT_DISPLAY_2            2       /* callback function: hp4156_ControlCallback */
#define  HP4156PORT_DISPLAY              3       /* callback function: hp4156_ControlCallback */
#define  HP4156PORT_TITLE                4
#define  HP4156PORT_MEASURE              5       /* callback function: hp4156_ControlCallback */
#define  HP4156PORT_SOURCE               6       /* callback function: hp4156_ControlCallback */
#define  HP4156PORT_SELECT               7       /* callback function: hp4156_ControlCallback */
#define  HP4156PORT_MEASRANGE            8       /* callback function: hp4156_ControlCallback */
#define  HP4156PORT_SRCRANGE             9       /* callback function: hp4156_ControlCallback */
#define  HP4156PORT_COMPLIANCE           10      /* callback function: hp4156_ControlCallback */
#define  HP4156PORT_UNITS                11

#define  MEASURE                         3       /* callback function: util_HidePanelCallback */
#define  MEASURE_LABEL                   2       /* callback function: hp4156_MeasureControlCallback */
#define  MEASURE_COEFF                   3       /* callback function: hp4156_MeasureControlCallback */
#define  MEASURE_NOTE                    4       /* callback function: AcqDataNoteCallback */
#define  MEASURE_ACQ                     5       /* callback function: hp4156_MeasureControlCallback */
#define  MEASURE_RANGE                   6       /* callback function: hp4156_MeasureControlCallback */


     /* Menu Bars, Menus, and Menu Items: */

#define  HP4156MENU                      1
#define  HP4156MENU_VSUS                 2
#define  HP4156MENU_VSUS_VSU_1           3       /* callback function: hp4156_MenuCallback */
#define  HP4156MENU_VSUS_VSU_2           4       /* callback function: hp4156_MenuCallback */
#define  HP4156MENU_VMUS                 5
#define  HP4156MENU_VMUS_VMU_1           6       /* callback function: hp4156_MenuCallback */
#define  HP4156MENU_VMUS_VMU_2           7       /* callback function: hp4156_MenuCallback */


     /* Callback Prototypes: */ 

int  CVICALLBACK AcqDataNoteCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK hp4156_ControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK hp4156_MeasureControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK hp4156_MenuCallback(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK hp4156_PanelCallback(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK hp4156_UnivControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK util_HidePanelCallback(int panel, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
