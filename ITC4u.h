/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/* Copyright (c) National Instruments 2009. All Rights Reserved.          */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  ITC4_CTRL                        1       /* callback function: itc4PanelCallback */
#define  ITC4_CTRL_SETP                   2       /* callback function: itc4ControlCallback */
#define  ITC4_CTRL_SENS3                  3
#define  ITC4_CTRL_SENS2                  4
#define  ITC4_CTRL_SENS1                  5
#define  ITC4_CTRL_DECORATION_2           6
#define  ITC4_CTRL_DECORATION             7
#define  ITC4_CTRL_CONTROL                8       /* callback function: itc4ControlCallback */
#define  ITC4_CTRL_HEATER_LED             9
#define  ITC4_CTRL_HEATER                 10      /* callback function: itc4ControlCallback */

#define  ITC4_PID                         2       /* callback function: util_HidePanelCallback */
#define  ITC4_PID_D                       2       /* callback function: itc4pidControlCallback */
#define  ITC4_PID_I                       3       /* callback function: itc4pidControlCallback */
#define  ITC4_PID_P                       4       /* callback function: itc4pidControlCallback */
#define  ITC4_PID_ACCEPT                  5       /* callback function: itc4pidControlCallback */
#define  ITC4_PID_DON                     6       /* callback function: itc4pidControlCallback */
#define  ITC4_PID_ION                     7       /* callback function: itc4pidControlCallback */
#define  ITC4_PID_PON                     8       /* callback function: itc4pidControlCallback */

#define  MEASURE                          3       /* callback function: util_HidePanelCallback */
#define  MEASURE_SENS3LABEL               2       /* callback function: itc4SensorControlCallback */
#define  MEASURE_SENS3COEFF               3       /* callback function: itc4SensorControlCallback */
#define  MEASURE_NOTE_3                   4       /* callback function: itc4SensorControlCallback */
#define  MEASURE_SENS3MEAS                5
#define  MEASURE_SENS3ACQ                 6       /* callback function: itc4SensorControlCallback */
#define  MEASURE_SENS2LABEL               7       /* callback function: itc4SensorControlCallback */
#define  MEASURE_NOTE_2                   8       /* callback function: itc4SensorControlCallback */
#define  MEASURE_SENS2COEFF               9       /* callback function: itc4SensorControlCallback */
#define  MEASURE_SENS2MEAS                10
#define  MEASURE_SENS2ACQ                 11      /* callback function: itc4SensorControlCallback */
#define  MEASURE_NOTE_1                   12      /* callback function: itc4SensorControlCallback */
#define  MEASURE_SENS1LABEL               13      /* callback function: itc4SensorControlCallback */
#define  MEASURE_SENS1COEFF               14      /* callback function: itc4SensorControlCallback */
#define  MEASURE_SENS1MEAS                15
#define  MEASURE_SENS1ACQ                 16      /* callback function: itc4SensorControlCallback */
#define  MEASURE_CLOSE                    17      /* callback function: LS340SensorControlCallback */
#define  MEASURE_TEXT1                    18
#define  MEASURE_TEXT2                    19
#define  MEASURE_TEXT3                    20


     /* Menu Bars, Menus, and Menu Items: */

#define  ITC4MENU                         1
#define  ITC4MENU_SOURCE                  2
#define  ITC4MENU_SOURCE_TEMP             3       /* callback function: itc4MenuCallback */
#define  ITC4MENU_MEASURE                 4
#define  ITC4MENU_MEASURE_SENS            5       /* callback function: itc4MenuCallback */
#define  ITC4MENU_SETTINGS                6
#define  ITC4MENU_SETTINGS_PID            7       /* callback function: itc4MenuCallback */


     /* Callback Prototypes: */

int  CVICALLBACK itc4ControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK itc4MenuCallback(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK itc4PanelCallback(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK itc4pidControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK itc4SensorControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK LS340SensorControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK util_HidePanelCallback(int panel, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
