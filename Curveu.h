/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/* Copyright (c) National Instruments 2012. All Rights Reserved.          */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  CURVEATTR                        1
#define  CURVEATTR_LABEL                  2       /* control type: string, callback function: CurveAttrCallback */
#define  CURVEATTR_X0_LABEL               3       /* control type: string, callback function: CurveAttrCallback */
#define  CURVEATTR_X0_READING             4       /* control type: numeric, callback function: CurveAttrCallback */
#define  CURVEATTR_XCHAN                  5       /* control type: ring, callback function: CurveXChanCallback */
#define  CURVEATTR_XCHANINFO              6       /* control type: command, callback function: CurveAttrCallback */
#define  CURVEATTR_YCHAN                  7       /* control type: ring, callback function: CurveYChanCallback */
#define  CURVEATTR_YCHANINFO              8       /* control type: command, callback function: CurveAttrCallback */
#define  CURVEATTR_POINTSTYLE             9       /* control type: ring, callback function: CurveAttrCallback */
#define  CURVEATTR_PLOTSTYLE              10      /* control type: ring, callback function: CurveAttrCallback */
#define  CURVEATTR_LINESTYLE              11      /* control type: ring, callback function: CurveAttrCallback */
#define  CURVEATTR_PTFREQ                 12      /* control type: numeric, callback function: CurveAttrCallback */
#define  CURVEATTR_HIDDEN                 13      /* control type: toggle, callback function: CurveAttrCallback */
#define  CURVEATTR_COLOR                  14      /* control type: color, callback function: CurveAttrCallback */
#define  CURVEATTR_NOTE                   15      /* control type: textBox, callback function: util_NoteCallback */

#define  CURVEINFO                        2
#define  CURVEINFO_LIST                   2       /* control type: listBox, callback function: (none) */
#define  CURVEINFO_CLOSE                  3       /* control type: toggle, callback function: util_DiscardCallback */
#define  CURVEINFO_TEXT_1                 4       /* control type: textMsg, callback function: (none) */
#define  CURVEINFO_TEXT_2                 5       /* control type: textMsg, callback function: (none) */
#define  CURVEINFO_TEXT_3                 6       /* control type: textMsg, callback function: (none) */
#define  CURVEINFO_TEXT_4                 7       /* control type: textMsg, callback function: (none) */
#define  CURVEINFO_TEXT_5                 8       /* control type: textMsg, callback function: (none) */
#define  CURVEINFO_TEXT_6                 9       /* control type: textMsg, callback function: (none) */
#define  CURVEINFO_TEXT_7                 10      /* control type: textMsg, callback function: (none) */
#define  CURVEINFO_TEXT_8                 11      /* control type: textMsg, callback function: (none) */

#define  CURVES                           3       /* callback function: util_HidePanelCallback */
#define  CURVES_LIST                      2       /* control type: listBox, callback function: CurveSelectCallback */
#define  CURVES_INDEX                     3       /* control type: numeric, callback function: CurveIndexCallback */
#define  CURVES_NCURVES                   4       /* control type: numeric, callback function: (none) */
#define  CURVES_INFO                      5       /* control type: command, callback function: CurveInfoCallback */
#define  CURVES_LOAD                      6       /* control type: command, callback function: LoadCurvesCallback */
#define  CURVES_SAVE                      7       /* control type: command, callback function: InitSaveCurvesCallback */
#define  CURVES_REMOVE                    8       /* control type: command, callback function: InitRemoveCurvesCallback */
#define  CURVES_CREATE                    9       /* control type: command, callback function: CreateCurveCallback */
#define  CURVES_DECORATION                10      /* control type: deco, callback function: (none) */

#define  CURVESAVE                        4
#define  CURVESAVE_LIST                   2       /* control type: listBox, callback function: (none) */
#define  CURVESAVE_ORIGIN                 3       /* control type: command, callback function: SaveOriginCurvesCallback */
#define  CURVESAVE_DTS                    4       /* control type: command, callback function: SaveDTSCurvesCallback */
#define  CURVESAVE_MATLAB                 5       /* control type: command, callback function: SaveMATLABCurvesCallback */
#define  CURVESAVE_IGOR                   6       /* control type: command, callback function: SaveIgorCurvesCallback */
#define  CURVESAVE_DAAS                   7       /* control type: command, callback function: SaveDAASCurvesCallback */
#define  CURVESAVE_ALL                    8       /* control type: toggle, callback function: SaveCurveSelectAllCallback */
#define  CURVESAVE_CLOSE                  9       /* control type: toggle, callback function: util_DiscardCallback */

#define  CURVEVIEW                        5
#define  CURVEVIEW_CURVES                 2       /* control type: ring, callback function: CurveViewSelectCallback */
#define  CURVEVIEW_OFFSET_VAL             3       /* control type: numeric, callback function: (none) */
#define  CURVEVIEW_OFFSET                 4       /* control type: scale, callback function: CurveViewWindowCallback */
#define  CURVEVIEW_TOTALPTS               5       /* control type: numeric, callback function: (none) */
#define  CURVEVIEW_PTS_2                  6       /* control type: ring, callback function: CurveViewWindowCallback */
#define  CURVEVIEW_PTS_1                  7       /* control type: numeric, callback function: CurveViewWindowCallback */
#define  CURVEVIEW_MARKER                 8       /* control type: toggle, callback function: CurveViewMarkerCallback */
#define  CURVEVIEW_M1X                    9       /* control type: numeric, callback function: (none) */
#define  CURVEVIEW_M1Y                    10      /* control type: numeric, callback function: (none) */
#define  CURVEVIEW_AXIS                   11      /* control type: binary, callback function: CurveViewSelectAxisCallback */
#define  CURVEVIEW_LOG                    12      /* control type: toggle, callback function: CurveViewAxisCallback */
#define  CURVEVIEW_GRID                   13      /* control type: toggle, callback function: CurveViewAxisCallback */
#define  CURVEVIEW_DIR                    14      /* control type: binary, callback function: CurveViewDirCallback */
#define  CURVEVIEW_SCATTER                15      /* control type: toggle, callback function: CurveViewWindowCallback */
#define  CURVEVIEW_MASK                   16      /* control type: toggle, callback function: CurveViewMaskCallback */
#define  CURVEVIEW_DONE                   17      /* control type: command, callback function: CurveViewDoneCallback */
#define  CURVEVIEW_BOX_1                  18      /* control type: deco, callback function: (none) */
#define  CURVEVIEW_BOX_2                  19      /* control type: deco, callback function: (none) */
#define  CURVEVIEW_GRAPH                  20      /* control type: graph, callback function: CurveViewGraphCallback */


     /* Control Arrays: */

          /* (no control arrays in the resource file) */


     /* Menu Bars, Menus, and Menu Items: */

#define  CURVEMENUS                       1
#define  CURVEMENUS_PROC                  2
#define  CURVEMENUS_PROC_INTEG            3
#define  CURVEMENUS_PROC_DIFF             4
#define  CURVEMENUS_PROC_FILTER           5
#define  CURVEMENUS_PROC_SMOOTH           6
#define  CURVEMENUS_PROC_OFFSET           7
#define  CURVEMENUS_PROC_REVERSE          8
#define  CURVEMENUS_PROC_SORT             9
#define  CURVEMENUS_PROC_SORT_SUBMENU     10
#define  CURVEMENUS_PROC_SORT_ASC         11
#define  CURVEMENUS_PROC_SORT_DESC        12
#define  CURVEMENUS_MEAS                  13
#define  CURVEMENUS_MEAS_ACDC             14
#define  CURVEMENUS_MEAS_AMPSPEC          15
#define  CURVEMENUS_MEAS_PWRSPEC          16
#define  CURVEMENUS_FIT                   17
#define  CURVEMENUS_FIT_LINEAR            18
#define  CURVEMENUS_FIT_EXP               19
#define  CURVEMENUS_FIT_POLY              20
#define  CURVEMENUS_INTERP                21
#define  CURVEMENUS_INTERP_POLY           22
#define  CURVEMENUS_INTERP_RAT            23
#define  CURVEMENUS_INTERP_SPLINE         24


     /* Callback Prototypes: */

int  CVICALLBACK CreateCurveCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CurveAttrCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CurveIndexCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CurveInfoCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CurveSelectCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CurveViewAxisCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CurveViewDirCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CurveViewDoneCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CurveViewGraphCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CurveViewMarkerCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CurveViewMaskCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CurveViewSelectAxisCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CurveViewSelectCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CurveViewWindowCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CurveXChanCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CurveYChanCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK InitRemoveCurvesCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK InitSaveCurvesCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK LoadCurvesCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK SaveCurveSelectAllCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK SaveDAASCurvesCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK SaveDTSCurvesCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK SaveIgorCurvesCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK SaveMATLABCurvesCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK SaveOriginCurvesCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK util_DiscardCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK util_HidePanelCallback(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK util_NoteCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
