/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/* Copyright (c) National Instruments 2008. All Rights Reserved.          */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  DAS_CTRL                         1       /* callback function: das6036_PanelCallback */
#define  DAS_CTRL_ANALOGUE_IN             2
#define  DAS_CTRL_INPUT                   3       /* callback function: das6036_ControlCallback */
#define  DAS_CTRL_RANGE                   4       /* callback function: das6036_ControlCallback */
#define  DAS_CTRL_ACQ                     5       /* callback function: das6036_ControlCallback */

#define  MEASURE                          2       /* callback function: util_HidePanelCallback */
#define  MEASURE_LABEL                    2       /* callback function: das6036_MeasureControlCallback */
#define  MEASURE_COEFF                    3       /* callback function: das6036_MeasureControlCallback */
#define  MEASURE_RANGE                    4       /* callback function: das6036_MeasureControlCallback */
#define  MEASURE_NOTE                     5       /* callback function: AcqDataNoteCallback */
#define  MEASURE_ACQ                      6       /* callback function: das6036_MeasureControlCallback */

#define  PANEL                            3
#define  PANEL_RANGE                      2       /* callback function: das6036_ControlCallback */


     /* Menu Bars, Menus, and Menu Items: */

#define  DASMENU                          1
#define  DASMENU_FILE                     2
#define  DASMENU_FILE_SAVE                3       /* callback function: das6036_MenuCallback */
#define  DASMENU_FILE_LOAD                4       /* callback function: das6036_MenuCallback */
#define  DASMENU_SOURCE                   5
#define  DASMENU_SOURCE_DAC1              6       /* callback function: das6036_MenuCallback */
#define  DASMENU_SOURCE_DAC2              7       /* callback function: das6036_MenuCallback */
#define  DASMENU_SOURCE_DAC3              8       /* callback function: das6036_MenuCallback */
#define  DASMENU_MEAS                     9
#define  DASMENU_MEAS_IN_0                10      /* callback function: das6036_MenuCallback */
#define  DASMENU_MEAS_IN_1                11      /* callback function: das6036_MenuCallback */
#define  DASMENU_MEAS_IN_2                12      /* callback function: das6036_MenuCallback */
#define  DASMENU_MEAS_IN_3                13      /* callback function: das6036_MenuCallback */
#define  DASMENU_MEAS_IN_4                14      /* callback function: das6036_MenuCallback */
#define  DASMENU_MEAS_IN_5                15      /* callback function: das6036_MenuCallback */
#define  DASMENU_MEAS_IN_6                16      /* callback function: das6036_MenuCallback */
#define  DASMENU_MEAS_IN_7                17      /* callback function: das6036_MenuCallback */


     /* Callback Prototypes: */ 

int  CVICALLBACK AcqDataNoteCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK das6036_ControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK das6036_MeasureControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK das6036_MenuCallback(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK das6036_PanelCallback(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK util_HidePanelCallback(int panel, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
