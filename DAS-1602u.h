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

#define  DAS_CTRL                        1       /* callback function: das1602_PanelCallback */
#define  DAS_CTRL_RANGE                  2       /* callback function: das1602_ControlCallback */
#define  DAS_CTRL_ANALOGUE_IN            3
#define  DAS_CTRL_INPUT                  4       /* callback function: das1602_ControlCallback */
#define  DAS_CTRL_ACQ                    5       /* callback function: das1602_ControlCallback */

#define  MEASURE                         2       /* callback function: util_HidePanelCallback */
#define  MEASURE_RANGE                   2       /* callback function: das1602_MeasureControlCallback */
#define  MEASURE_LABEL                   3       /* callback function: das1602_MeasureControlCallback */
#define  MEASURE_COEFF                   4       /* callback function: das1602_MeasureControlCallback */
#define  MEASURE_NOTE                    5       /* callback function: AcqDataNoteCallback */
#define  MEASURE_ACQ                     6       /* callback function: das1602_MeasureControlCallback */


     /* Menu Bars, Menus, and Menu Items: */

#define  DASMENU                         1
#define  DASMENU_SOURCE                  2
#define  DASMENU_SOURCE_DAC1             3       /* callback function: das1602_MenuCallback */
#define  DASMENU_SOURCE_DAC2             4       /* callback function: das1602_MenuCallback */
#define  DASMENU_MEAS                    5
#define  DASMENU_MEAS_IN_0               6       /* callback function: das1602_MenuCallback */
#define  DASMENU_MEAS_IN_1               7       /* callback function: das1602_MenuCallback */
#define  DASMENU_MEAS_IN_2               8       /* callback function: das1602_MenuCallback */
#define  DASMENU_MEAS_IN_3               9       /* callback function: das1602_MenuCallback */
#define  DASMENU_MEAS_IN_4               10      /* callback function: das1602_MenuCallback */
#define  DASMENU_MEAS_IN_5               11      /* callback function: das1602_MenuCallback */
#define  DASMENU_MEAS_IN_6               12      /* callback function: das1602_MenuCallback */
#define  DASMENU_MEAS_IN_7               13      /* callback function: das1602_MenuCallback */


     /* Callback Prototypes: */ 

int  CVICALLBACK AcqDataNoteCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK das1602_ControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK das1602_MeasureControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK das1602_MenuCallback(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK das1602_PanelCallback(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK util_HidePanelCallback(int panel, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
