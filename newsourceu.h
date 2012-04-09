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

#define  POPUP_MSG                       2       /* callback function: util_HidePanelCallback */
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
#define  SOURCE_PLOT                     29      /* callback function: PlotSourceCallback */
#define  SOURCE_LOAD                     30      /* callback function: LoadSourceCallback */
#define  SOURCE_SAVE                     31      /* callback function: SaveSourceCallback */

#define  SOURCEPLOT                      4
#define  SOURCEPLOT_CLOSE                2       /* callback function: util_DiscardCallback */
#define  SOURCEPLOT_GRAPH                3


     /* Menu Bars, Menus, and Menu Items: */

#define  MENUBAR                         1
#define  MENUBAR_MENU1                   2
#define  MENUBAR_MENU1_LOITEM1           3
#define  MENUBAR_MENU1_LOITEM1_SUBMENU   4
#define  MENUBAR_MENU1_LOITEM1_ITEM3     5       /* callback function: MLoadSourceCallback */
#define  MENUBAR_MENU1_LOITEM1_ITEM4     6       /* callback function: MSaveSourceCallback */
#define  MENUBAR_MENU1_ITEM2             7
#define  MENUBAR_MENU1_ITEM2_SUBMENU     8
#define  MENUBAR_MENU1_ITEM2_ITEM5       9
#define  MENUBAR_MENU1_2                 10      /* callback function: MPlotSourceCallback */


     /* Callback Prototypes: */ 

int  CVICALLBACK GenSourceControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK LoadSourceCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK MLoadSourceCallback(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK MPlotSourceCallback(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK MSaveSourceCallback(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK PlotSourceCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK SaveSourceCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK SegmentControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK SourceControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK SourceMaxSegCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK util_DiscardCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK util_HidePanelCallback(int panel, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
