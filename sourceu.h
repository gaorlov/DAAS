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

#define  GENSRC                          1
#define  GENSRC_OUTPUT                   2       /* callback function: GenSourceControlCallback */
#define  GENSRC_LABEL                    3       /* callback function: GenSourceControlCallback */
#define  GENSRC_COEFF                    4       /* callback function: GenSourceControlCallback */
#define  GENSRC_ACQUIRE                  5       /* callback function: GenSourceControlCallback */
#define  GENSRC_NOTE                     6       /* callback function: GenSourceControlCallback */
#define  GENSRC_CLOSE                    7       /* callback function: GenSourceControlCallback */

#define  POPUP_MSG                       2       /* callback function: util_HidePanelOnLoseFocus */
#define  POPUP_MSG_TEXT                  2

#define  SOURCE                          3       /* callback function: util_HidePanelCallback */
#define  SOURCE_OUTPUT                   2       /* callback function: SourceControlCallback */
#define  SOURCE_LABEL                    3       /* callback function: SourceControlCallback */
#define  SOURCE_COEFF                    4       /* callback function: SourceControlCallback */
#define  SOURCE_ACQUIRE                  5       /* callback function: SourceControlCallback */
#define  SOURCE_NOTE                     6       /* callback function: SourceControlCallback */
#define  SOURCE_INLIST                   7       /* callback function: SourceControlCallback */
#define  SOURCE_PTS_TOTAL                8
#define  SOURCE_TIME_TOTAL               9
#define  SOURCE_NSEGMENTS                10      /* callback function: SourceMaxSegCallback */
#define  SOURCE_SEGMENT                  11      /* callback function: SegmentControlCallback */
#define  SOURCE_SEGSEL                   12      /* callback function: SourceControlCallback */
#define  SOURCE_FUNCTION                 13      /* callback function: SegmentControlCallback */
#define  SOURCE_POINTS                   14      /* callback function: SegmentControlCallback */
#define  SOURCE_START                    15      /* callback function: SegmentControlCallback */
#define  SOURCE_STOP                     16      /* callback function: SegmentControlCallback */
#define  SOURCE_STEP                     17      /* callback function: SegmentControlCallback */
#define  SOURCE_DELAY                    18      /* callback function: SegmentControlCallback */
#define  SOURCE_ERROR                    19      /* callback function: SegmentControlCallback */
#define  SOURCE_RATE                     20      /* callback function: SegmentControlCallback */
#define  SOURCE_TIME                     21
#define  SOURCE_CLOSE                    22      /* callback function: SourceControlCallback */
#define  SOURCE_LOGSC                    23      /* callback function: SegmentControlCallback */
#define  SOURCE_ERROR_ON                 24      /* callback function: SegmentControlCallback */
#define  SOURCE_TEXT_1                   25
#define  SOURCE_TEXT_2                   26
#define  SOURCE_DECORATION               27
#define  SOURCE_DECORATION_2             28
#define  SOURCE_OPTIMIZE                 29      /* callback function: SegmentControlCallback */
#define  SOURCE_PRECISION                30      /* callback function: SourceControlCallback */

#define  SOURCEPLOT                      4       /* callback function: util_DiscardPanelCallback */
#define  SOURCEPLOT_CLOSE                2       /* callback function: util_DiscardCallback */
#define  SOURCEPLOT_GRAPH                3


     /* Menu Bars, Menus, and Menu Items: */

#define  SOURCEMENU                      1
#define  SOURCEMENU_SCAN                 2
#define  SOURCEMENU_SCAN_STEP            3
#define  SOURCEMENU_SCAN_STEP_SUBMENU    4
#define  SOURCEMENU_SCAN_STEP_SAVE       5       /* callback function: source_MenuCallback */
#define  SOURCEMENU_SCAN_STEP_LOAD       6       /* callback function: source_MenuCallback */
#define  SOURCEMENU_SCAN_EXTERN          7
#define  SOURCEMENU_SCAN_EXTERN_SUBMENU  8
#define  SOURCEMENU_SCAN_EXTERN_LOAD     9       /* callback function: source_MenuCallback */
#define  SOURCEMENU_SRC_SEG              10
#define  SOURCEMENU_SRC_SEG_PLOT         11      /* callback function: source_MenuCallback */


     /* Callback Prototypes: */ 

int  CVICALLBACK GenSourceControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK SegmentControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK source_MenuCallback(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK SourceControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK SourceMaxSegCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK util_DiscardCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK util_DiscardPanelCallback(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK util_HidePanelCallback(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK util_HidePanelOnLoseFocus(int panel, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
