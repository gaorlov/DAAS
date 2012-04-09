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

#define  ACQINFO                          1       /* callback function: RemoveAcqInfoCallback */
#define  ACQINFO_PTS2GO                   2
#define  ACQINFO_TIMEPERPT                3
#define  ACQINFO_TIME2GO                  4
#define  ACQINFO_ELAPSEDTIME              5

#define  ACQSETUP                         2       /* callback function: util_HidePanelCallback */
#define  ACQSETUP_STATUS                  2
#define  ACQSETUP_FILENAME                3       /* callback function: DataFileControlCallback */
#define  ACQSETUP_FILEEXT                 4       /* callback function: DataFileControlCallback */
#define  ACQSETUP_FILEPATH                5
#define  ACQSETUP_GEN_POINTS              6       /* callback function: GenExpControlCallback */
#define  ACQSETUP_GEN_DELAY               7       /* callback function: GenExpControlCallback */
#define  ACQSETUP_GEN_TIME                8
#define  ACQSETUP_SRC_LIST                9
#define  ACQSETUP_SRC_POINTS              10
#define  ACQSETUP_SRC_TIME                11
#define  ACQSETUP_BEEP                    12      /* callback function: BeepCallback */
#define  ACQSETUP_EXPTITLE                13
#define  ACQSETUP_SRC_MOVEUP              14
#define  ACQSETUP_SRC_MOVEDOWN            15
#define  ACQSETUP_SRC_REMOVE              16
#define  ACQSETUP_SAVEAS                  17      /* callback function: DataFileSaveAsCallback */
#define  ACQSETUP_DECORATION              18
#define  ACQSETUP_BOX_1                   19


     /* Menu Bars, Menus, and Menu Items: */

#define  ACQMENUS                         1
#define  ACQMENUS_EXP                     2
#define  ACQMENUS_EXP_SOURCE              3
#define  ACQMENUS_EXP_GENERAL             4       /* callback function: GenExpCallback */
#define  ACQMENUS_EXP_SEP_1               5
#define  ACQMENUS_EXP_BEGIN               6       /* callback function: ExpStatusCallback */
#define  ACQMENUS_EXP_PAUSE               7       /* callback function: ExpStatusCallback */
#define  ACQMENUS_EXP_CONTINUE            8       /* callback function: ExpStatusCallback */
#define  ACQMENUS_EXP_END                 9       /* callback function: ExpStatusCallback */
#define  ACQMENUS_EXP_SEP_2               10
#define  ACQMENUS_EXP_INFO                11      /* callback function: AcqInfoCallback */
#define  ACQMENUS_GPIB                    12
#define  ACQMENUS_GPIB_SEP_1              13
#define  ACQMENUS_GPIB_SETUP              14
#define  ACQMENUS_GPIB_SETUP_SUBMENU      15
#define  ACQMENUS_GPIB_SETUP_SETUP        16
#define  ACQMENUS_GPIB_SETUP_STATUS       17
#define  ACQMENUS_GPIB_SEPARATOR          18
#define  ACQMENUS_GPIB_RS232SETUP         19
#define  ACQMENUS_GPIB_SEP_2              20
#define  ACQMENUS_GPIB_PCIDEV             21
#define  ACQMENUS_GPIB_PCIDEV_SUBMENU     22
#define  ACQMENUS_GPIB_PCIDEV_PCI_SEP     23
#define  ACQMENUS_GPIB_SEP_3              24


     /* Callback Prototypes: */

void CVICALLBACK AcqInfoCallback(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK BeepCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK DataFileControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK DataFileSaveAsCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK ExpStatusCallback(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK GenExpCallback(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK GenExpControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK RemoveAcqInfoCallback(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK util_HidePanelCallback(int panel, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
