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

#define  DDA_CTRL                        1       /* callback function: util_HidePanelCallback */
#define  DDA_CTRL_RANGE_0                2       /* callback function: dda08_ControlCallback */
#define  DDA_CTRL_ANALOG_OUT_0           3       /* callback function: dda08_ControlCallback */
#define  DDA_CTRL_OUT_ON_0               4       /* callback function: dda08_ControlCallback */
#define  DDA_CTRL_RANGE_7                5       /* callback function: dda08_ControlCallback */
#define  DDA_CTRL_ANALOG_OUT_7           6       /* callback function: dda08_ControlCallback */
#define  DDA_CTRL_OUT_ON_7               7       /* callback function: dda08_ControlCallback */
#define  DDA_CTRL_RANGE_6                8       /* callback function: dda08_ControlCallback */
#define  DDA_CTRL_ANALOG_OUT_6           9       /* callback function: dda08_ControlCallback */
#define  DDA_CTRL_OUT_ON_6               10      /* callback function: dda08_ControlCallback */
#define  DDA_CTRL_RANGE_5                11      /* callback function: dda08_ControlCallback */
#define  DDA_CTRL_ANALOG_OUT_5           12      /* callback function: dda08_ControlCallback */
#define  DDA_CTRL_OUT_ON_5               13      /* callback function: dda08_ControlCallback */
#define  DDA_CTRL_RANGE_4                14      /* callback function: dda08_ControlCallback */
#define  DDA_CTRL_ANALOG_OUT_4           15      /* callback function: dda08_ControlCallback */
#define  DDA_CTRL_OUT_ON_4               16      /* callback function: dda08_ControlCallback */
#define  DDA_CTRL_RANGE_3                17      /* callback function: dda08_ControlCallback */
#define  DDA_CTRL_ANALOG_OUT_3           18      /* callback function: dda08_ControlCallback */
#define  DDA_CTRL_OUT_ON_3               19      /* callback function: dda08_ControlCallback */
#define  DDA_CTRL_RANGE_2                20      /* callback function: dda08_ControlCallback */
#define  DDA_CTRL_ANALOG_OUT_2           21      /* callback function: dda08_ControlCallback */
#define  DDA_CTRL_OUT_ON_2               22      /* callback function: dda08_ControlCallback */
#define  DDA_CTRL_RANGE_1                23      /* callback function: dda08_ControlCallback */
#define  DDA_CTRL_ANALOG_OUT_1           24      /* callback function: dda08_ControlCallback */
#define  DDA_CTRL_OUT_ON_1               25      /* callback function: dda08_ControlCallback */
#define  DDA_CTRL_DECORATION_4           26
#define  DDA_CTRL_DECORATION_5           27
#define  DDA_CTRL_DECORATION_6           28
#define  DDA_CTRL_DECORATION_8           29
#define  DDA_CTRL_DECORATION             30
#define  DDA_CTRL_DECORATION_2           31
#define  DDA_CTRL_DECORATION_7           32
#define  DDA_CTRL_DECORATION_3           33
#define  DDA_CTRL_SOURCE_0               34      /* callback function: dda08_ControlCallback */
#define  DDA_CTRL_SOURCE_7               35      /* callback function: dda08_ControlCallback */
#define  DDA_CTRL_SOURCE_6               36      /* callback function: dda08_ControlCallback */
#define  DDA_CTRL_SOURCE_5               37      /* callback function: dda08_ControlCallback */
#define  DDA_CTRL_SOURCE_4               38      /* callback function: dda08_ControlCallback */
#define  DDA_CTRL_SOURCE_3               39      /* callback function: dda08_ControlCallback */
#define  DDA_CTRL_SOURCE_2               40      /* callback function: dda08_ControlCallback */
#define  DDA_CTRL_SOURCE_1               41      /* callback function: dda08_ControlCallback */


     /* Menu Bars, Menus, and Menu Items: */

#define  DASMENU                         1
#define  DASMENU_SOURCE                  2
#define  DASMENU_SOURCE_DAC1             3       /* callback function: das6036_MenuCallback */
#define  DASMENU_SOURCE_DAC2             4       /* callback function: das6036_MenuCallback */
#define  DASMENU_MEAS                    5
#define  DASMENU_MEAS_IN_0               6       /* callback function: das6036_MenuCallback */
#define  DASMENU_MEAS_IN_1               7       /* callback function: das6036_MenuCallback */
#define  DASMENU_MEAS_IN_2               8       /* callback function: das6036_MenuCallback */
#define  DASMENU_MEAS_IN_3               9       /* callback function: das6036_MenuCallback */
#define  DASMENU_MEAS_IN_4               10      /* callback function: das6036_MenuCallback */
#define  DASMENU_MEAS_IN_5               11      /* callback function: das6036_MenuCallback */
#define  DASMENU_MEAS_IN_6               12      /* callback function: das6036_MenuCallback */
#define  DASMENU_MEAS_IN_7               13      /* callback function: das6036_MenuCallback */


     /* Callback Prototypes: */ 

void CVICALLBACK das6036_MenuCallback(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK dda08_ControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK util_HidePanelCallback(int panel, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
