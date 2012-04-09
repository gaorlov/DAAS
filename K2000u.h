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

#define  K2000                           1       /* callback function: k2000_ControlPanelCallback */
#define  K2000_MEASURE                   2
#define  K2000_MONITOR                   3       /* callback function: k2000_ControlCallback */
#define  K2000_RATE                      4       /* callback function: k2000_ControlCallback */
#define  K2000_RANGE                     5       /* callback function: k2000_ControlCallback */
#define  K2000_RANGE_AUTO                6       /* callback function: k2000_ControlCallback */
#define  K2000_FILTER_ENABLE             7       /* callback function: k2000_ControlCallback */
#define  K2000_FILTER_TYPE               8       /* callback function: k2000_ControlCallback */
#define  K2000_FILTER_READINGS           9       /* callback function: k2000_ControlCallback */
#define  K2000_MEAS                      10
#define  K2000_TRIG                      11
#define  K2000_IDLE                      12
#define  K2000_OVER                      13
#define  K2000_RANGE_TEXT                14
#define  K2000_INT_TEXT                  15
#define  K2000_DECORATION                16
#define  K2000_OHMS4                     17      /* callback function: k2000_ControlCallback */
#define  K2000_OHMS2                     18      /* callback function: k2000_ControlCallback */
#define  K2000_ACI                       19      /* callback function: k2000_ControlCallback */
#define  K2000_DCI                       20      /* callback function: k2000_ControlCallback */
#define  K2000_ACV                       21      /* callback function: k2000_ControlCallback */
#define  K2000_DCV                       22      /* callback function: k2000_ControlCallback */
#define  K2000_DECORATION_2              23

#define  K2000_CHAN                      2
#define  K2000_CHAN_LABEL                2       /* callback function: k2000_ChannelControlCallback */
#define  K2000_CHAN_COEFF                3       /* callback function: k2000_ChannelControlCallback */
#define  K2000_CHAN_NOTE                 4       /* callback function: AcqDataNoteCallback */
#define  K2000_CHAN_ACQ                  5       /* callback function: k2000_ChannelControlCallback */
#define  K2000_CHAN_CLOSE                6       /* callback function: k2000_ChannelControlCallback */


     /* Menu Bars, Menus, and Menu Items: */

#define  K2000MENUS                      1
#define  K2000MENUS_FILE                 2
#define  K2000MENUS_FILE_LOAD            3       /* callback function: k2000_LoadCallback */
#define  K2000MENUS_FILE_SAVE            4       /* callback function: k2000_SaveCallback */
#define  K2000MENUS_FILE_SEP_1           5
#define  K2000MENUS_FILE_GPIB            6       /* callback function: OperateDevCallback */
#define  K2000MENUS_MEASURE              7
#define  K2000MENUS_MEASURE_DMM          8       /* callback function: k2000_ChannelSetupCallback */
#define  K2000MENUS_MEASURE_MEAS_SEP_1   9
#define  K2000MENUS_MEASURE_CH1          10      /* callback function: k2000_ChannelSetupCallback */
#define  K2000MENUS_MEASURE_CH2          11      /* callback function: k2000_ChannelSetupCallback */
#define  K2000MENUS_MEASURE_CH3          12      /* callback function: k2000_ChannelSetupCallback */
#define  K2000MENUS_MEASURE_CH4          13      /* callback function: k2000_ChannelSetupCallback */
#define  K2000MENUS_MEASURE_CH5          14      /* callback function: k2000_ChannelSetupCallback */
#define  K2000MENUS_MEASURE_CH6          15      /* callback function: k2000_ChannelSetupCallback */
#define  K2000MENUS_MEASURE_CH7          16      /* callback function: k2000_ChannelSetupCallback */
#define  K2000MENUS_MEASURE_CH8          17      /* callback function: k2000_ChannelSetupCallback */
#define  K2000MENUS_MEASURE_CH9          18      /* callback function: k2000_ChannelSetupCallback */
#define  K2000MENUS_MEASURE_CH10         19      /* callback function: k2000_ChannelSetupCallback */


     /* Callback Prototypes: */ 

int  CVICALLBACK AcqDataNoteCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK k2000_ChannelControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK k2000_ChannelSetupCallback(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK k2000_ControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK k2000_ControlPanelCallback(int panel, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK k2000_LoadCallback(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK k2000_SaveCallback(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK OperateDevCallback(int menubar, int menuItem, void *callbackData, int panel);


#ifdef __cplusplus
    }
#endif
