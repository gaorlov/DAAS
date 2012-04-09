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

#define  AXIS                             1       /* callback function: util_HidePanelCallback */
#define  AXIS_LABEL                       2       /* callback function: AxisControlCallback */
#define  AXIS_MIN                         3       /* callback function: AxisControlCallback */
#define  AXIS_MAX                         4       /* callback function: AxisControlCallback */
#define  AXIS_GRID                        5       /* callback function: AxisControlCallback */
#define  AXIS_DIVISIONS                   6       /* callback function: AxisControlCallback */
#define  AXIS_AUTOSCALE                   7       /* callback function: AxisControlCallback */
#define  AXIS_AUTODIV                     8       /* callback function: AxisControlCallback */
#define  AXIS_SELECTION                   9       /* callback function: AxisControlCallback */
#define  AXIS_CLOSE                       10      /* callback function: util_DiscardCallback */
#define  AXIS_SCALE                       11      /* callback function: AxisControlCallback */
#define  AXIS_CONV                        12      /* callback function: AxisControlCallback */
#define  AXIS_DECORATION                  13

#define  GRAPH                            2       /* callback function: GraphPanelCallback */
#define  GRAPH_GRAPH                      2       /* callback function: GraphCallback */
#define  GRAPH_ZOOMOUT                    3       /* callback function: ZoomOutGraphCallback */
#define  GRAPH_ZOOMIN                     4       /* callback function: ZoomInGraphCallback */
#define  GRAPH_ACQCURVE                   5       /* callback function: AcquireCurveCallback */
#define  GRAPH_HOME                       6       /* callback function: HomeGraphCallback */
#define  GRAPH_DOWN                       7       /* callback function: MoveGraphDownCallback */
#define  GRAPH_UP                         8       /* callback function: MoveGraphUpCallback */
#define  GRAPH_RIGHT                      9       /* callback function: MoveGraphRightCallback */
#define  GRAPH_LEFT                       10      /* callback function: MoveGraphLeftCallback */
#define  GRAPH_CURSOR                     11      /* callback function: SelectCursorCallback */
#define  GRAPH_CLOSE                      12      /* callback function: PanelClose */

#define  GRAPHS                           3       /* callback function: util_HidePanelCallback */
#define  GRAPHS_LIST                      2       /* callback function: SelectGraphCallback */
#define  GRAPHS_PRINT                     3       /* callback function: PrintGraphCallback */
#define  GRAPHS_SAVE                      4       /* callback function: SaveGraphCallback */
#define  GRAPHS_LOAD                      5       /* callback function: LoadGraphCallback */
#define  GRAPHS_REMOVE                    6       /* callback function: RemoveGraphCallback */
#define  GRAPHS_CREATE                    7       /* callback function: CreateGraphCallback */
#define  GRAPHS_CLOSE                     8       /* callback function: PanelClose */


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */

int  CVICALLBACK AcquireCurveCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK AxisControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CreateGraphCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK GraphCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK GraphPanelCallback(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK HomeGraphCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK LoadGraphCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK MoveGraphDownCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK MoveGraphLeftCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK MoveGraphRightCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK MoveGraphUpCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK PanelClose(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK PrintGraphCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK RemoveGraphCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK SaveGraphCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK SelectCursorCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK SelectGraphCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK util_DiscardCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK util_HidePanelCallback(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ZoomInGraphCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ZoomOutGraphCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
