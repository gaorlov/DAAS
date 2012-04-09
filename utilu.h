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

#define  BG                               1       /* callback function: BGDoneCallback */
#define  BG_GRAPHS                        2
#define  BG_CHANNELS                      3
#define  BG_ACQSETUP                      4
#define  BG_ACQCHANNELS                   5

#define  ERROR                            2       /* callback function: util_HidePanelCallback */
#define  ERROR_OK                         2       /* callback function: util_ErrorCloseCallback */
#define  ERROR_TEXT                       3

#define  FILESTAT                         3
#define  FILESTAT_TEXT                    2

#define  INIT                             4
#define  INIT_TEXT                        2
#define  INIT_TEXT_2                      3
#define  INIT_TEXT_10                     4
#define  INIT_TEXT_4                      5
#define  INIT_TEXT_3                      6
#define  INIT_EXIT                        7
#define  INIT_ANALYSIS                    8
#define  INIT_CONTROL                     9


     /* Menu Bars, Menus, and Menu Items: */

#define  DAASMENUSX                       1
#define  DAASMENUSX___Experiment          2
#define  DAASMENUSX___Experiment_SOURCE   3
#define  DAASMENUSX___Experiment_GENERAL  4
#define  DAASMENUSX___Experiment_SEP_1    5
#define  DAASMENUSX___Experiment_BEGIN    6
#define  DAASMENUSX___Experiment_PAUSE    7
#define  DAASMENUSX___Experiment_CONTINUE 8
#define  DAASMENUSX___Experiment_END      9
#define  DAASMENUSX___Experiment_SEP_2    10
#define  DAASMENUSX___Experiment_INFO     11
#define  DAASMENUSX___Instruments         12
#define  DAASMENUSX___Instruments_SEP_1   13
#define  DAASMENUSX___Instruments_SETUP   14
#define  DAASMENUSX___Instruments_STATUS  15
#define  DAASMENUSX_DATAFILES             16
#define  DAASMENUSX_DATAFILES_LOAD        17
#define  DAASMENUSX_DATAFILES_SAVE        18
#define  DAASMENUSX_GRAPHS                19
#define  DAASMENUSX_GRAPHS_CREATE         20
#define  DAASMENUSX_GRAPHS_REMOVE         21
#define  DAASMENUSX_GRAPHS_SEP_1          22
#define  DAASMENUSX_GRAPHS_LOAD           23
#define  DAASMENUSX_GRAPHS_SAVE           24
#define  DAASMENUSX_GRAPHS_SEP_2          25
#define  DAASMENUSX_GRAPHS_PRINT          26


     /* Callback Prototypes: */

int  CVICALLBACK BGDoneCallback(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK util_ErrorCloseCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK util_HidePanelCallback(int panel, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
