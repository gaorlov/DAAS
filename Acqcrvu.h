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

#define  ACQCURVE                        1
#define  ACQCURVE_AUTOSAVE               2       /* callback function: AcqCurveControlCallback */
#define  ACQCURVE_GRAPHFILE              3
#define  ACQCURVE_MARKER                 4
#define  ACQCURVE_MARKER_SOURCES         5
#define  ACQCURVE_MARKER_TERM            6
#define  ACQCURVE_X0                     7       /* callback function: AcqCurveControlCallback */
#define  ACQCURVE_XCHANNEL               8
#define  ACQCURVE_YCHANNEL               9
#define  ACQCURVE_PTFREQ                 10
#define  ACQCURVE_AUTOCOLOR              11      /* callback function: AcqCurveControlCallback */
#define  ACQCURVE_CLOSE                  12      /* callback function: PanelClose */
#define  ACQCURVE_COLOR                  13
#define  ACQCURVE_NOTE                   14      /* callback function: util_NoteCallback */
#define  ACQCURVE_BOX_1                  15
#define  ACQCURVE_TEXT_1                 16
#define  ACQCURVE_AUTONOTES              17      /* callback function: AcqCurveControlCallback */

#define  AUTONOTES                       2       /* callback function: util_HidePanelCallback */
#define  AUTONOTES_LIST                  2
#define  AUTONOTES_CLOSE                 3       /* callback function: PanelClose */


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */ 

int  CVICALLBACK AcqCurveControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK PanelClose(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK util_HidePanelCallback(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK util_NoteCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
