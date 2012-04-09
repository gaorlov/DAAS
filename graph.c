#include "Graphu.h"
#include "Acqcrvu.h"

/* BUGS

950715 - Ver 1.3
- fixed problems with axis labels on graphs

*/

#include <utility.h>
#include <analysis.h>
#include <formatio.h>
#include <ansi_c.h>
#include <userint.h>

#include "util.h"
#include "utilu.h"
#include "list.h"
#include "channel.h"
#include "changen.h"
#include "chanfnc.h"
#include "chanops.h"
#include "acqchan.h"
#include "curve.h"
#include "curveu.h"
#include "acqcrv.h"
#include "acqcrvu.h"
#include "graph.h"
#include "graphu.h"
#include "acquire.h"
#include "acquireu.h"
#include "source.h"

#define TRUE 1
#define FALSE 0

struct graphGStruct graphG;

static struct cursorStruct
{
    double x0, y0, min_x, max_x, min_y, max_y;
    int boxhandle;
    int firstPt;
}   cursor;

/*******************************************************************/

static void axis_Init (axisType *x, axisType *y);
static void axis_InitControl (graphPtr graph, int y_axis);
static void axis_UpdatePanel (int panel, axisType axis);
static void axis_GetGraphArea (graphPtr graph);
static void axis_SetGraphArea (graphPtr graph);

static void graph_ZoomInatMarker (int graph);

void        graphG_Init (void);
void        graphG_Exit(void);
int         ShowGraphsPanelCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void        graphlist_UpdatePanel (void);

double      graph_XTextOffset (void);
double      graph_YTextOffset (void);
graphPtr    graph_Create (char *title);
void		graph_ReplotCurvesWithConv(graphPtr graph);
void        graph_Remove(graphPtr graph);
void        graph_Save (graphPtr graph);
graphPtr    graph_Load (void);

int  ShowCurvesPanelCallback(int panel, int event, void *callbackData, int eventData1, int eventData2);

graphPtr    graphlist_GetItem (int i);
graphPtr    graphlist_GetSelection (void);
void        graphlist_Copy (int panel, int control);
void        graphlist_AddGraph (graphPtr graph);
void        graphlist_RemoveGraph (graphPtr graph);
void        graphlist_AutoSave (void);
void        graphlist_PlotCurves (void);
void        graphlist_PlotReadings (void);
void        graphlist_RemoveReadings (void);


void graphG_Init (void)
{
    util_ChangeInitMessage ("Graph Utilities...");

    cursor.boxhandle = 0;
    cursor.firstPt = FALSE;

    list_Init (&graphG.graphs);
    graphG.p = LoadPanel (utilG.p, "graphu.uir", GRAPHS);
    
    SetPanelPos (graphG.p, 70, 70);
    InstallCtrlCallback (utilG.p, BG_GRAPHS, ShowGraphsPanelCallback, 0);
}

int  PrintGraphCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    char msg[256];
    graphPtr graph;
    int err;
    if (event == EVENT_COMMIT) {
        graph = graphlist_GetSelection();
        Fmt (msg, "Are you sure you desire to print graph: %s", graph->title);
        if (ConfirmPopup ("Print Graph Message", msg)) {
            err = PrintCtrl (graph->p, GRAPH_GRAPH, "", 0, 1);
            if (err < 0) util_MessagePopup ("Print Error Message", "Error printing selected graph");
        }
    }
    return 0;
}

int  LoadGraphCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    graphPtr graph;
    int filestatus;
    char path[260], info[260];

    if (event == EVENT_COMMIT)
    {
        filestatus = FileSelectPopup ("", "*.grf", "*.grf", "Load Graph",
                                      VAL_LOAD_BUTTON, 0, 1, 1, 0, path);
        if (filestatus == VAL_EXISTING_FILE_SELECTED)
        {
            fileHandle.analysis = util_OpenFile(path, FILE_READ, FALSE);
            ReadLine (fileHandle.analysis, info, 255);
            if (CompareBytes (info, 0, "#GRAPHDATA", 0, StringLength ("#GRAPHDATA"), 1))
            {
                util_CloseFile();
                MessagePopup ("Load Graph Error", "Wrong file type");
            }
            else
            {
                graph = graph_Load();
                if (graph) graphlist_AddGraph (graph);
                ReadLine (fileHandle.analysis, info, 255);
                util_CloseFile();
                graphlist_UpdatePanel();
            }
        }
    }
    return 0;
}

int  SaveGraphCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    graphPtr graph;
    int filestatus;
    char path[300];

    if (event == EVENT_COMMIT)
    {
        filestatus = FileSelectPopup ("", "*.grf", "*.grf", "Save Graph",
                                      VAL_SAVE_BUTTON, 0, 1, 1, 1, path);
        if ((filestatus == VAL_NEW_FILE_SELECTED) ||
            (filestatus == VAL_EXISTING_FILE_SELECTED)) {
            fileHandle.analysis = util_OpenFile(path, FILE_WRITE, FALSE);
            FmtFile (fileHandle.analysis, "#GRAPHDATA\n");
            graph_Save (graphlist_GetSelection());
            FmtFile (fileHandle.analysis, "#ENDGRAPHDATA\n");
            util_CloseFile();
        }
    }
    return 0;
}

int  RemoveGraphCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    if (event == EVENT_COMMIT)
    {
        if (ConfirmPopup ("Remove Graph Message",
                      "This procedure cannot be undone.\nAre you sure you want to proceed?"))
        {
            graphlist_RemoveGraph (graphlist_GetSelection());
            graphlist_UpdatePanel ();
        }
    }
    return 0;
}

int  SelectGraphCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    graphPtr graph;
    char info[256], title[256];
    int index, button;

    if (event == EVENT_RIGHT_CLICK)
    {
        graph = graphlist_GetSelection();
        Fmt (info, "Enter new graph title:\n[%s]", graph->title);
        button = GenericMessagePopup ("Edit Graph Title", info, "OK",
                                      "Cancel", 0, title, 255, 1,
                                      VAL_GENERIC_POPUP_BTN1,
                                      VAL_GENERIC_POPUP_BTN1,
                                      VAL_GENERIC_POPUP_BTN2);
        if (button == VAL_GENERIC_POPUP_BTN1)
        {
            Fmt (graph->title, title);
            SetPanelAttribute (graph->p, ATTR_TITLE, graph->title);
            Fmt (info, "curves for [%s]", graph->title);
            SetPanelAttribute (graph->curves.panel, ATTR_TITLE, info);
            index = list_FindItem (graphG.graphs, graph);
            ReplaceListItem (graphG.p, GRAPHS_LIST, index,
                                 graph->title, index);
            return 1;
        }
    }

    if (event == EVENT_LEFT_DOUBLE_CLICK)
    {
        graph = graphlist_GetSelection();
        DisplayPanel (graph->p);
    }
    return 0;
}

int  CreateGraphCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    char title[256];
    graphPtr graph;

    if (event == EVENT_COMMIT)
    {
        Fmt (title, "graph %i", graphG.graphs.nItems);
        graph = graph_Create (title);
        if (graph) graphlist_AddGraph (graph);
        graphlist_UpdatePanel();
    }
    return 0;
}

int  ShowGraphsPanelCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    if (event == EVENT_COMMIT)
    {
        graphlist_UpdatePanel();
        DisplayPanel (graphG.p);
    }
    return 0;
}

void graphlist_UpdatePanel (void)
{
    SetInputMode (graphG.p, GRAPHS_LIST, graphG.graphs.nItems);
    SetInputMode (graphG.p, GRAPHS_SAVE, graphG.graphs.nItems);
    SetInputMode (graphG.p, GRAPHS_REMOVE, graphG.graphs.nItems);
    SetInputMode (graphG.p, GRAPHS_PRINT, graphG.graphs.nItems);
}

graphPtr graphlist_GetItem (int i)
{
    nodePtr node;

    node = list_GetNode (graphG.graphs, i);
    return node->item;
}

graphPtr graphlist_GetSelection (void)
{
    int i;
    GetCtrlIndex (graphG.p, GRAPHS_LIST, &i);
    return graphlist_GetItem (i);
}

void graphlist_AddGraph (graphPtr graph)
{
    char label[256];
    if (graph && list_AddItem (&graphG.graphs, graph))
        InsertListItem (graphG.p, GRAPHS_LIST, -1,
                        graph->title, graphG.graphs.nItems - 1);
}

void graphlist_RemoveGraph (graphPtr graph)
{
    int i, index;
    curvePtr curve;
    nodePtr node;

    i = list_FindItem (graphG.graphs, graph);
    if (i != -1)
    {
        node = graph->curves.list.first;
        while (node)
        {
            curve = node->item;
            node = node->next;
            if (node) curve_Hide (graph->p, GRAPH_GRAPH, curve, TRUE);
            else curve_Hide (graph->p, GRAPH_GRAPH, curve, FALSE);
            curvelist_RemoveCurve (&graph->curves, curve);
        }

        if (graph->acqcurve.x0) acqchan_RemoveGraph (graph->acqcurve.x0, &graph->acqcurve);
        if (graph->acqcurve.x) acqchan_RemoveGraph (graph->acqcurve.x, &graph->acqcurve);
        if (graph->acqcurve.y) acqchan_RemoveGraph (graph->acqcurve.y, &graph->acqcurve);

        
        DiscardPanel (graph->curves.panel);

        
        DiscardPanel (graph->p);
		graph->p = 0;

        list_RemoveItem (&graphG.graphs, i, TRUE);
        DeleteListItem (graphG.p, GRAPHS_LIST, i, 1);
    }
}

void graphlist_PlotReadings (void)
{
    int i;
    graphPtr graph;
    for (i = 0; i < graphG.graphs.nItems; i++) {
        graph = graphlist_GetItem (i);
        acqcurve_PlotReading (graph, graph->p, GRAPH_GRAPH, &graph->acqcurve);
    }
}

void graphlist_RemoveReadings (void)
{
    int i;
    graphPtr graph;
    for (i = 0; i < graphG.graphs.nItems; i++) {
        graph = graphlist_GetItem (i);
        acqcurve_Hide(graph->p, GRAPH_GRAPH, &graph->acqcurve);
    }
}

void graphlist_PlotCurves (void)
{
    int i;
    graphPtr graph;
    curvePtr curve;
    for (i = 0; i < graphG.graphs.nItems; i++) {
        graph = graphlist_GetItem (i);
        if (graph->acqcurve.x && graph->acqcurve.y) {
            graph->acqcurve.marker.pts = (utilG.acq.pt - graph->acqcurve.marker.offset);
            if (graph->acqcurve.marker.pts > 1) {
                curve = acqcurve_MakeCurve (graph->curves.list.nItems, graph->title, &graph->acqcurve);
                if (curve){
                    curvelist_AddCurve (&graph->curves, curve);
                    curve_Plot (graph->p, GRAPH_GRAPH, curve, graph);
                }
            }
        }
		
    }
}

void graphlist_AutoSave (void)
{
    int i;
    graphPtr graph;
    for (i = 0; i < graphG.graphs.nItems; i++)
    {
        graph = graphlist_GetItem (i);
        if (graph->acqcurve.autoattr.save)
        {
            fileHandle.analysis = util_OpenFile (graph->acqcurve.path, FILE_WRITE, FALSE);
            FmtFile (fileHandle.analysis, "#GRAPHDATA\n");
            graph_Save (graph);
            FmtFile (fileHandle.analysis, "#ENDGRAPHDATA\n");
            util_CloseFile();
        }
    }
}

void graphlist_Copy (int panel, int control)
{
    int i;
    graphPtr graph;
    ClearListCtrl (panel, control);
    for (i = 0; i < graphG.graphs.nItems; i++)
    {
        graph = graphlist_GetItem (i);
        InsertListItem (panel, control, -1, graph->title, i);
    }
}

void graph_ReplotCurvesWithConv(graphPtr graph)
{
	double *xarray, *yarray;
	int i, limit = graph->curves.list.nItems;
	nodePtr node;
	curvePtr curve;
	limit = (utilG.acq.status != ACQ_BUSY)? limit : limit - 1;
	for(i = 0; i < limit; i++)
	{
		double **xArr, **yArr;
		node = list_GetNode(graph->curves.list, i);
		curve = node->item;
		xArr = acqchan_MeasurementArray (curve->x->readings, graph->acqcurve.x->coeff, graph->x.conversion.val, curve->x->pts);
		yArr = acqchan_MeasurementArray (curve->y->readings, graph->acqcurve.y->coeff, graph->y.conversion.val, curve->y->pts);
		DeleteGraphPlot (graph->p, GRAPH_GRAPH, curve->plothandle, 0);
		curve->plothandle = 
			PlotXY (graph->p, GRAPH_GRAPH, *xArr, *yArr,
			curve->pts, VAL_DOUBLE, VAL_DOUBLE, VAL_THIN_LINE, VAL_NO_POINT, VAL_SOLID,
			graph->acqcurve.ptfreq, graph->acqcurve.color);
		free(xArr);
		free(yArr);
	}
}

void graph_Save (graphPtr graph)
{
    int i;
    curvePtr curve;
    FmtFile (fileHandle.analysis, "Title       : %s;\n", graph->title);
    FmtFile (fileHandle.analysis, "Labels      : %s, %s;\n", graph->x.label, graph->y.label);
    FmtFile (fileHandle.analysis, "Axis Min    : %f, %f\n", graph->x.min, graph->y.min);
    FmtFile (fileHandle.analysis, "Axis Max    : %f, %f\n", graph->x.max, graph->y.max);
    FmtFile (fileHandle.analysis, "Divisions   : %i, %i\n", graph->x.divisions.val, graph->y.divisions.val);
    FmtFile (fileHandle.analysis, "Grid        : %i, %i\n", graph->x.grid.val, graph->y.grid.val);
    FmtFile (fileHandle.analysis, "Log Scale   : %i, %i\n", graph->x.logscale.val, graph->y.logscale.val);
    FmtFile (fileHandle.analysis, "Curves      : %i\n", graph->curves.list.nItems);
    for (i = 0; i < graph->curves.list.nItems; i++)
    {
        FmtFile (fileHandle.analysis, FILE_ITEM_SEP);
        curve = curvelist_GetItem (graph->curves.list, i);
        curve_Save (curve);
    }
}

graphPtr graph_Load (void)
{
    int i, curves;
    graphPtr graph;
    curvePtr curve;
    char title[256], itemsep[256], label[25];

    ScanFile (fileHandle.analysis, "%s>Title       : %s[xt59]%s[dw1]", title);
    graph = graph_Create(title);
    if (!graph) return NULL;
    ScanFile (fileHandle.analysis, "%s>Labels      : %s[xt44]%s[xt59]%s[dw1]",
              graph->x.label, graph->y.label);
    ScanFile (fileHandle.analysis, "%s>Axis Min    : %f,%f%s[dw1]", &graph->x.min, &graph->y.min);
    ScanFile (fileHandle.analysis, "%s>Axis Max    : %f,%f%s[dw1]", &graph->x.max, &graph->y.max);
    ScanFile (fileHandle.analysis, "%s>Divisions   : %i,%i%s[dw1]", &graph->x.divisions.val, &graph->y.divisions.val);
    ScanFile (fileHandle.analysis, "%s>Grid        : %i,%i%s[dw1]", &graph->x.grid.val, &graph->y.grid.val);
    ScanFile (fileHandle.analysis, "%s>Log Scale   : %i,%i%s[dw1]", &graph->x.logscale.val, &graph->y.logscale.val);

    axis_SetGraphArea(graph);

    if (StringLength (graph->x.label) > 24) Fmt (label, "%s[w21]...", graph->x.label);
        else Fmt (label, graph->x.label);
    SetCtrlAttribute (graph->p, GRAPH_GRAPH, ATTR_XNAME, label);
    if (StringLength (graph->y.label) > 24) Fmt (label, "%s[w21]...", graph->x.label);
        else Fmt (label, graph->y.label);
    SetCtrlAttribute (graph->p, GRAPH_GRAPH, ATTR_YNAME, label);
    SetCtrlAttribute (graph->p, GRAPH_GRAPH, graph->x.grid.attr, graph->x.grid.val);
    SetCtrlAttribute (graph->p, GRAPH_GRAPH, graph->y.grid.attr, graph->y.grid.val);
    SetCtrlAttribute (graph->p, GRAPH_GRAPH, graph->x.logscale.attr, graph->x.logscale.val);
    SetCtrlAttribute (graph->p, GRAPH_GRAPH, graph->y.logscale.attr, graph->y.logscale.val);
    SetCtrlAttribute (graph->p, GRAPH_GRAPH, graph->x.divisions.attr, graph->x.divisions.val);
    SetCtrlAttribute (graph->p, GRAPH_GRAPH, graph->y.divisions.attr, graph->y.divisions.val);

    ScanFile (fileHandle.analysis, "%s>Curves      : %i%s[dw1]", &curves);
    for (i = 0; i < curves; i++)
    {
        ReadLine (fileHandle.analysis, itemsep, 255);
        curve = curve_Load(graph->title);
        if (curve)
        {
            curvelist_AddCurve (&graph->curves, curve);
            curve_Plot (graph->p, GRAPH_GRAPH, curve, NULL);
        }
        else
        {
            util_OutofMemory ("Load Graph Message");
            break;
        }
    }
    return graph;
}

graphPtr graph_Create (char *title)
{
    graphPtr graph;
    char label[256];

    graph = malloc (sizeof(graphType));
    if (!graph) return NULL;

    Fmt (graph->title, title);

    axis_Init (&graph->x, &graph->y);

    acqcurve_Init (&graph->acqcurve);

    graph->snap = FALSE;
    graph->cursor = CURSOR_NONE;
    graph->textHandle = 0;
	graph->axisP = 0;
	
    graph->p = LoadPanel (utilG.p, "graphu.uir", GRAPH);
    
    SetPanelPos (graph->p, VAL_AUTO_CENTER, VAL_AUTO_CENTER);

    SetPanelAttribute (graph->p, ATTR_TITLE, graph->title);
    SetPanelAttribute(graph->p, ATTR_CALLBACK_DATA, graph);

    SetCtrlAttribute (graph->p, GRAPH_GRAPH, ATTR_CTRL_MODE, VAL_INDICATOR);
    SetCtrlAttribute (graph->p, GRAPH_GRAPH, ATTR_CALLBACK_DATA, graph);
    SetCtrlAttribute (graph->p, GRAPH_GRAPH, ATTR_NUM_CURSORS, 0);
	if(utilG.acq.status != ACQ_NONE)
	{
		SetCtrlAttribute (acqG.p.setup, ACQSETUP_SRC_MOVEUP, ATTR_CALLBACK_DATA, graph);
		SetCtrlAttribute (acqG.p.setup, ACQSETUP_SRC_MOVEDOWN, ATTR_CALLBACK_DATA, graph);
		SetCtrlAttribute (acqG.p.setup, ACQSETUP_SRC_REMOVE, ATTR_CALLBACK_DATA, graph);
	}
    SetCtrlAttribute (graph->p, GRAPH_HOME, ATTR_CALLBACK_DATA, graph);

    SetCtrlAttribute (graph->p, GRAPH_UP, ATTR_CALLBACK_DATA, graph);
    SetCtrlAttribute (graph->p, GRAPH_DOWN, ATTR_CALLBACK_DATA, graph);
    SetCtrlAttribute (graph->p, GRAPH_LEFT, ATTR_CALLBACK_DATA, graph);
    SetCtrlAttribute (graph->p, GRAPH_RIGHT, ATTR_CALLBACK_DATA, graph);

    SetCtrlAttribute (graph->p, GRAPH_ZOOMIN, ATTR_CALLBACK_DATA, graph);
    SetCtrlAttribute (graph->p, GRAPH_ZOOMOUT, ATTR_CALLBACK_DATA, graph);

    SetInputMode (graph->p, GRAPH_ACQCURVE, !(utilG.acq.status == ACQ_NONE));
    SetCtrlAttribute (graph->p, GRAPH_ACQCURVE, ATTR_CALLBACK_DATA, graph);

    SetCtrlAttribute (graph->p, GRAPH_CURSOR, ATTR_CALLBACK_DATA, graph);

    curvelist_Init(&graph->curves);
    Fmt (label, "curves for [%s]", graph->title);
    SetPanelAttribute (graph->curves.panel, ATTR_TITLE, label);
    InstallPanelCallback (graph->curves.panel, ShowCurvesPanelCallback, graph);

    return graph;
}

void graph_Remove(graphPtr graph)
{
    
    DiscardPanel (graph->p);

    
    DiscardPanel (graph->curves.panel);
    list_RemoveAllItems (&graph->curves.list, TRUE);
    free (graph);
}

void graphG_Exit (void)
{
    int i;
    graphPtr graph;

    for (i = 0; i < graphG.graphs.nItems; i ++) {
        graph = graphlist_GetItem (i);
        graph_Remove (graph);
    }
    list_RemoveAllItems (&graphG.graphs, FALSE);
    
    DiscardPanel (graphG.p);

    curve_Exit();
}

int  ShowCurvesPanelCallback(int panel, int event, void *callbackData, int eventData1, int eventData2)
{
    graphPtr graph;

    if (((event == EVENT_KEYPRESS) && (eventData1 == VAL_ESC_VKEY)) || (event == EVENT_RIGHT_DOUBLE_CLICK))
	    HidePanel (panel);

    if (event == EVENT_GOT_FOCUS) {
        graph = callbackData;
        curveG_Init (graph->p, GRAPH_GRAPH, graph->title, &graph->curves);
        curvelist_UpdatePanel(graph->curves);
    }
    return 0;
}

int  SelectCursorCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    graphPtr graph = callbackData;
    int i;

    if (event == EVENT_COMMIT) {
		GetCtrlVal (panel, control, &i);
	    if (i) {
			SetCtrlAttribute (panel, GRAPH_GRAPH, ATTR_NUM_CURSORS, 1);
			SetCtrlAttribute (panel, control, ATTR_MENU_ARROW_COLOR, VAL_RED);
			SetCursorAttribute (panel, GRAPH_GRAPH, 1, ATTR_CURSOR_MODE,
                                VAL_FREE_FORM);
			SetCursorAttribute (panel, GRAPH_GRAPH, 1, ATTR_CURSOR_COLOR, VAL_RED);
			SetCursorAttribute (panel, GRAPH_GRAPH, 1, ATTR_CROSS_HAIR_STYLE,
                            VAL_SHORT_CROSS);
			SetCursorAttribute (panel, GRAPH_GRAPH, 1, ATTR_CURSOR_POINT_STYLE,
                            VAL_EMPTY_CIRCLE);
			SetCtrlAttribute (panel, GRAPH_GRAPH, ATTR_CTRL_MODE, VAL_HOT);
		} else {
			SetCtrlAttribute (panel, GRAPH_GRAPH, ATTR_NUM_CURSORS, 0);
			SetCtrlAttribute (panel, control, ATTR_MENU_ARROW_COLOR, VAL_BLACK);
			SetPanelAttribute (panel, ATTR_MOUSE_CURSOR, VAL_DEFAULT_CURSOR);
			SetCtrlAttribute (panel, GRAPH_GRAPH, ATTR_CTRL_MODE, VAL_INDICATOR);
			graph->cursor = CURSOR_NONE;
			if (graph->textHandle) DeleteGraphPlot (panel, GRAPH_GRAPH, graph->textHandle, VAL_IMMEDIATE_DRAW);
	            graph->textHandle = 0;
		}
		switch (i) {
			case 1:
				graph->cursor = CURSOR_SNAP;
				SetPanelAttribute (panel, ATTR_MOUSE_CURSOR, VAL_CROSS_HAIR_CURSOR);
				SetCursorAttribute (panel, GRAPH_GRAPH, 1, ATTR_CURSOR_MODE,
                                VAL_SNAP_TO_POINT);
				break;
			case 2:
				graph->cursor = CURSOR_PAN;
				SetPanelAttribute (panel, ATTR_MOUSE_CURSOR, VAL_OPEN_HAND_CURSOR);
				break;
			case 3:
				graph->cursor = CURSOR_XY;
				SetPanelAttribute (panel, ATTR_MOUSE_CURSOR, VAL_CROSS_HAIR_CURSOR);
				break;
			case 4:
				graph->cursor = CURSOR_MAGNIFY;
				SetPanelAttribute (panel, ATTR_MOUSE_CURSOR, VAL_BOX_CURSOR);
				break;
		}
    }
    return 0;
}

int  AcquireCurveControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int i;
    graphPtr graph;
	char label[50];
    switch (control) {
        case ACQCURVE_PTFREQ:
        case ACQCURVE_COLOR:
            if (event == EVENT_COMMIT) {
                graph = callbackData;
                GetCtrlVal (panel, ACQCURVE_PTFREQ, &graph->acqcurve.ptfreq);
                GetCtrlVal (panel, ACQCURVE_COLOR, &graph->acqcurve.color);
                acqcurve_Plot (graph, graph->p, GRAPH_GRAPH, &graph->acqcurve);
            }
            break;
        case ACQCURVE_YCHANNEL:
			if (event == EVENT_LEFT_CLICK)
				acqchanlist_Copy (panel, ACQCURVE_YCHANNEL);
            if (event == EVENT_COMMIT) {
                graph = callbackData;
        		if(graph)
					if (graph->acqcurve.y) acqchan_RemoveGraph (graph->acqcurve.y, &graph->acqcurve);
                GetCtrlVal (panel, control, &i);
                if (i == NOT_IN_LIST) graph->acqcurve.y = NULL;
                else {
					GetIndexFromValue (panel, control, &i, i);
					GetLabelFromIndex (panel, control, i, label);
					graph->acqcurve.y = acqchanlist_GetItemByTitle(label);
					if(graph->acqcurve.y)
					{
						//graph->acqcurve.y = acqchanlist_GetItem (i);
                    	acqchan_AddGraph (graph->acqcurve.y, &graph->acqcurve);
                    	acqcurve_Plot (graph, graph->p, GRAPH_GRAPH, &graph->acqcurve);
                    	SetCtrlAttribute (graph->p, GRAPH_GRAPH, ATTR_YNAME,
                                      graph->acqcurve.y->channel->label);
                    	Fmt (graph->y.label, graph->acqcurve.y->channel->label);
					}
                }
            }
            break;
        case ACQCURVE_XCHANNEL:
            if(event == EVENT_LEFT_CLICK)
				acqchanlist_Copy (panel, ACQCURVE_XCHANNEL);
			if (event == EVENT_COMMIT) {
                graph = callbackData;
				if(graph)
					if (graph->acqcurve.x) acqchan_RemoveGraph (graph->acqcurve.x, &graph->acqcurve);
                GetCtrlVal (panel, control, &i);
                if (i == NOT_IN_LIST) graph->acqcurve.x = NULL;
                else {
                    GetIndexFromValue (panel, control, &i, i);
					GetLabelFromIndex (panel, control, i, label);
					graph->acqcurve.x = acqchanlist_GetItemByTitle(label);
					if(graph->acqcurve.x)
					{
						//graph->acqcurve.x = acqchanlist_GetItem (i);
                    	acqchan_AddGraph (graph->acqcurve.x, &graph->acqcurve);
                    	acqcurve_Plot (graph, graph->p, GRAPH_GRAPH, &graph->acqcurve);
                    	SetCtrlAttribute (graph->p, GRAPH_GRAPH, ATTR_XNAME,
                                      graph->acqcurve.x->channel->label);
                    	Fmt (graph->x.label, graph->acqcurve.x->channel->label);
					}
                }
            }
            break;
    }
    return 0;
}

int  AcquireCurveCallback(int panel, int ctrl, int event, void *callbackData, int eventData1, int eventData2)
{
    graphPtr graph;
    int aPanel, i;

    if (event == EVENT_COMMIT)
    {
        nodePtr node, p;
		acqcurveGptr acqcrv, temp;
		graph = callbackData;
		if(acqcurveL.acqcurves.nItems > 0)
		{
			node = list_GetNode(acqcurveL.acqcurves, 0);
			i = 0;
    		p = acqcurveL.acqcurves.first;
			temp = node->item;
    		while ((p->next) && (temp->graph != graph))
			{
				p = p->next; 
				temp = p->item;
				i++;
			}
			if (temp->graph != graph)
			{
				aPanel = LoadPanel (utilG.p, "acqcrvu.uir", ACQCURVE);
        		acqcrv = malloc(sizeof(acqcurveGs));
				acqcrv->Apanel = aPanel;
				acqcrv->graph = graph;
				list_AddItem(&acqcurveL.acqcurves, acqcrv);
			}
			else
			{
				node = list_GetNode(acqcurveL.acqcurves, i);
				acqcrv = node->item;
				aPanel = acqcrv->Apanel;
			}
		}
		else
		{
			aPanel = LoadPanel (utilG.p, "acqcrvu.uir", ACQCURVE);
        	acqcrv = malloc(sizeof(acqcurveGs));
			acqcrv->Apanel = aPanel;
			acqcrv->graph = graph;
			list_AddItem(&acqcurveL.acqcurves, acqcrv);
		}
		acqcrv->InitSourceMarker = sourcelist_InitAcqCurveSourceList;
        acqcrv->ControlCallback = AcqCurveSourceMarkerCallback;
		acqcrv->PanelCallback = AcqCurvePanelCallback;
        SetPanelPos (aPanel, VAL_AUTO_CENTER, VAL_AUTO_CENTER);

        SetInputMode (acqcrv->Apanel, ACQCURVE_GRAPHFILE, graph->acqcurve.autoattr.save);

        SetInputMode (aPanel, ACQCURVE_MARKER, utilG.exp == EXP_SOURCE);
        SetInputMode (aPanel, ACQCURVE_MARKER_SOURCES, ((utilG.exp == EXP_SOURCE) && graph->acqcurve.marker.on));
        SetInputMode (aPanel, ACQCURVE_MARKER_TERM, ((utilG.exp == EXP_SOURCE) && graph->acqcurve.marker.on));

        acqchanlist_Copy (aPanel, ACQCURVE_X0);
        acqchanlist_Copy (aPanel, ACQCURVE_XCHANNEL);
        acqchanlist_Copy (aPanel, ACQCURVE_YCHANNEL);

        SetCtrlVal (aPanel, ACQCURVE_AUTOSAVE, graph->acqcurve.autoattr.save);
        SetCtrlVal (aPanel, ACQCURVE_GRAPHFILE, graph->acqcurve.path);

        SetCtrlVal (aPanel, ACQCURVE_MARKER, graph->acqcurve.marker.on);
        SetCtrlVal (aPanel, ACQCURVE_MARKER_TERM, graph->acqcurve.marker.segment);
        

        if (graph->acqcurve.x0) i = acqchanlist_FindItem (graph->acqcurve.x0); else i = -1;
        SetCtrlVal (aPanel, ACQCURVE_X0, i);

        if (graph->acqcurve.x) i = acqchanlist_FindItem (graph->acqcurve.x); else i = -1;
        SetCtrlVal (aPanel, ACQCURVE_XCHANNEL, i);

        if (graph->acqcurve.y) i = acqchanlist_FindItem (graph->acqcurve.y); else i = -1;
        SetCtrlVal (aPanel, ACQCURVE_YCHANNEL, i);

        SetCtrlVal (aPanel, ACQCURVE_PTFREQ, graph->acqcurve.ptfreq);
        SetCtrlVal (aPanel, ACQCURVE_COLOR, graph->acqcurve.color);
        SetCtrlVal (aPanel, ACQCURVE_AUTOCOLOR, graph->acqcurve.autoattr.color);
        ResetTextBox (aPanel, ACQCURVE_NOTE, graph->acqcurve.note);

        SetCtrlAttribute (aPanel, ACQCURVE_AUTOSAVE, ATTR_CALLBACK_DATA, &graph->acqcurve);

        SetCtrlAttribute (aPanel, ACQCURVE_X0, ATTR_CALLBACK_DATA, &graph->acqcurve);
        SetCtrlAttribute (aPanel, ACQCURVE_AUTOCOLOR, ATTR_CALLBACK_DATA, &graph->acqcurve);

        SetCtrlAttribute (aPanel, ACQCURVE_NOTE, ATTR_CALLBACK_DATA, graph->acqcurve.note);
		
        InstallCtrlCallback (aPanel, ACQCURVE_MARKER, acqcrv->ControlCallback, graph);
        InstallCtrlCallback (aPanel, ACQCURVE_MARKER_SOURCES, acqcrv->ControlCallback, graph);
        InstallCtrlCallback (aPanel, ACQCURVE_MARKER_TERM, acqcrv->ControlCallback, graph);
		InstallPanelCallback(aPanel, acqcrv->PanelCallback, graph);
		
        InstallCtrlCallback (aPanel, ACQCURVE_XCHANNEL, AcquireCurveControlCallback, graph);
        InstallCtrlCallback (aPanel, ACQCURVE_YCHANNEL, AcquireCurveControlCallback, graph);
        InstallCtrlCallback (aPanel, ACQCURVE_PTFREQ, AcquireCurveControlCallback, graph);
        InstallCtrlCallback (aPanel, ACQCURVE_COLOR, AcquireCurveControlCallback, graph);

		if (acqcrv->InitSourceMarker && (utilG.exp == EXP_SOURCE))
            acqcrv->InitSourceMarker(aPanel, graph->acqcurve.marker.source);

        DisplayPanel (aPanel);
		SetPanelAttribute (aPanel, ATTR_TITLE, graph->title);
    }
    return 0;
}

int  ZoomOutGraphCallback(int panel, int control, int event,
                         void *callbackData, int eventData1, int eventData2)
{
    double xstep, ystep;
    graphPtr graph;

    if (event == EVENT_COMMIT)
    {
        graph = callbackData;
        axis_GetGraphArea(graph);
        if (graph->x.logscale.val) {graph->x.min/=10; graph->x.max*=10;}
        else
        {
            xstep = (graph->x.max - graph->x.min)/2;
            graph->x.min-=xstep; graph->x.max+=xstep;
        }

        if (graph->y.logscale.val) {graph->y.min/=10; graph->y.max*=10;}
        else {
            ystep = (graph->y.max - graph->y.min)/2;
            graph->y.min-=ystep; graph->y.max+=ystep;
        }

        graph->x.autoscale = VAL_MANUAL;
        graph->y.autoscale = VAL_MANUAL;
        axis_SetGraphArea(graph);
        if (graph->cursor != CURSOR_NONE)
            SetGraphCursor (panel, GRAPH_GRAPH, 1,
                        graph->x.min + (graph->x.max - graph->x.min)/2,
                        graph->y.min + (graph->y.max - graph->y.min)/2);
    }
    return 0;
}

int  ZoomInGraphCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    double xstep, ystep;
    graphPtr graph;
    if (event == EVENT_COMMIT)
    {
        graph = callbackData;
        axis_GetGraphArea(graph);
        if (graph->x.logscale.val)
        {
            if ((graph->x.max / graph->x.min) > 100.0)
                {graph->x.min*=10; graph->x.max/=10;}
            else
                MessagePopup ("Zoom In Error", "No more zoomin' on X Axis");
        }
        else
        {
            xstep = (graph->x.max - graph->x.min)/4;
            graph->x.min+=xstep; graph->x.max-=xstep;
        }

        if (graph->y.logscale.val)
        {
            if ((graph->y.max / graph->y.min) > 100.0)
                {graph->y.min*=10; graph->y.max/=10;}
            else
                MessagePopup ("Zoom In Error", "No more zoomin' on Y Axis");
        }
        else
        {
            ystep = (graph->y.max - graph->y.min)/4;
            graph->y.min+=ystep; graph->y.max-=ystep;
        }

        graph->x.autoscale = VAL_MANUAL;
        graph->y.autoscale = VAL_MANUAL;
        axis_SetGraphArea(graph);
        if (graph->cursor != CURSOR_NONE)
            SetGraphCursor (panel, GRAPH_GRAPH, 1,
                        graph->x.min + (graph->x.max - graph->x.min)/2,
                        graph->y.min + (graph->y.max - graph->y.min)/2);
    }
    return 0;
}

int  MoveGraphRightCallback(int panel, int control, int event,
                      void *callbackData, int eventData1, int eventData2)
{
    double step;
    graphPtr graph;
    if (event == EVENT_COMMIT)
    {
        graph = callbackData;
        axis_GetGraphArea(graph);
        if (graph->x.logscale.val) {graph->x.min/=10; graph->x.max/=10;}
        else
        {
            step = (graph->x.max - graph->x.min)/4;
            graph->x.min-=step; graph->x.max-=step;
        }
        graph->x.autoscale = VAL_MANUAL;
        axis_SetGraphArea(graph);
    }
    return 0;
}

int  MoveGraphLeftCallback(int panel, int control, int event,
                      void *callbackData, int eventData1, int eventData2)
{
    double step;
    graphPtr graph;
    if (event == EVENT_COMMIT)
    {
        graph = callbackData;
        axis_GetGraphArea(graph);
        if (graph->x.logscale.val) {graph->x.min*=10; graph->x.max*=10;}
        else
        {
            step = (graph->x.max - graph->x.min)/4;
            graph->x.min+=step; graph->x.max+=step;
        }
        graph->x.autoscale = VAL_MANUAL;
        axis_SetGraphArea(graph);
    }
    return 0;
}

int  MoveGraphDownCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    double step;
    graphPtr graph;
    if (event == EVENT_COMMIT)
    {
        graph = callbackData;
        axis_GetGraphArea(graph);
        if (graph->y.logscale.val) {graph->y.min*=10; graph->y.max*=10;}
        else
        {
            step = (graph->y.max - graph->y.min)/4;
            graph->y.min+=step; graph->y.max+=step;
        }
        graph->y.autoscale = VAL_MANUAL;
        axis_SetGraphArea(graph);
        return 1;
    }
    return 0;
}

int  MoveGraphUpCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    double step;
    graphPtr graph;
    if (event == EVENT_COMMIT)
    {
        graph = callbackData;
        axis_GetGraphArea(graph);
        if (graph->y.logscale.val) {graph->y.min/=10; graph->y.max/=10;}
        else
        {
            step = (graph->y.max - graph->y.min)/4;
            graph->y.min-=step; graph->y.max-=step;
        }
        graph->y.autoscale = VAL_MANUAL;
        axis_SetGraphArea(graph);
        return 1;
    }
    return 0;
}

int  HomeGraphCallback(int panel, int control, int event,
                      void *callbackData, int eventData1, int eventData2)
{
    graphPtr graph;
    if (event == EVENT_COMMIT)
    {
        graph = callbackData;
        SetAxisRange (panel, GRAPH_GRAPH,
                      VAL_AUTOSCALE, 0.0, 1.0,
                      VAL_AUTOSCALE, 0.0, 1.0);
        axis_GetGraphArea (graph);
    }
    return 0;
}

int  GraphPanelCallback(int panel, int event, void *callbackData, int eventData1, int eventData2)
{
    int height, width;
    graphPtr graph = callbackData;

    if (((event == EVENT_KEYPRESS) && (eventData1 == VAL_ESC_VKEY)) || (event == EVENT_RIGHT_DOUBLE_CLICK))
	    HidePanel (panel);

    if (event == EVENT_PANEL_SIZE) {
        GetPanelAttribute (panel, ATTR_HEIGHT, &height);
        GetPanelAttribute (panel, ATTR_WIDTH, &width);

        SetCtrlAttribute (panel, GRAPH_GRAPH, ATTR_HEIGHT, height-47);
        SetCtrlAttribute (panel, GRAPH_GRAPH, ATTR_WIDTH, width-15);
    }
    return 0;
}

double graph_XTextOffset (void)
{
    return (cursor.max_x-cursor.min_x)/200;
}

double graph_YTextOffset (void)
{
    return (cursor.max_y-cursor.min_y)/200;
}

int  GraphCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    double tmp;
    int xscale, yscale, key, height, handle, index, i;
    double x0, x1, y0, y1, dx, dy;
    graphPtr graph = callbackData;
    curvePtr curve;
    char text[100];

    if (event == EVENT_RIGHT_CLICK) {
        DisplayPanel (graph->curves.panel);
        return 1;
    }

    if (event == EVENT_LEFT_DOUBLE_CLICK) {
        switch (graph->cursor) {
            case CURSOR_NONE:
                GetPanelAttribute (panel, ATTR_HEIGHT, &height);
                if (eventData2 < 60) axis_InitControl (graph, TRUE); /* y axis */
                else if ((height - eventData1) < 40) axis_InitControl (graph, FALSE);
                break;
            case CURSOR_MAGNIFY:
                graph_ZoomInatMarker(panel);
                break;
        }
        return 1;
    }

    if ((event == EVENT_LEFT_CLICK) && (graph->cursor != CURSOR_NONE)) {
        GetAxisRange (panel, control, &xscale,
                          &cursor.min_x, &cursor.max_x,
                          &yscale, &cursor.min_y, &cursor.max_y);
        SetAxisRange (panel, control, VAL_MANUAL, cursor.min_x, cursor.max_x,
                                      VAL_MANUAL, cursor.min_y, cursor.max_y);
    }

    if (event == EVENT_VAL_CHANGED) {
        if (!cursor.firstPt) {
            GetGraphCursor (panel, control, 1, &cursor.x0, &cursor.y0);
            cursor.firstPt = TRUE;
            return 1;
        }
        GetGraphCursor (panel, control, 1, &x1, &y1);
        switch (graph->cursor) {
            case CURSOR_MAGNIFY:
                if ((cursor.x0 != x1) && (cursor.y0 != y1)) {
                    if (cursor.boxhandle)
                        DeleteGraphPlot (panel, control, cursor.boxhandle,
                                     VAL_IMMEDIATE_DRAW);
                    cursor.boxhandle =
                    PlotRectangle (panel, control,
                               cursor.x0, cursor.y0, x1, y1,
                               VAL_RED, VAL_TRANSPARENT);
                }
                break;
            case CURSOR_XY:
                if (graph->textHandle)
                    DeleteGraphPlot (panel, control, graph->textHandle, VAL_DELAYED_DRAW);
                Fmt (text, "%s<%f[e2p3], %f[e2p3]", x1, y1);
                graph->textHandle = PlotText (panel, control, x1+graph_XTextOffset(),
                                              y1+graph_YTextOffset(), text,
                                              VAL_DIALOG_META_FONT, VAL_WHITE,
                                              VAL_TRANSPARENT);
                break;
            case CURSOR_PAN:
                dx = x1 - cursor.x0; dy = y1 - cursor.y0;
                cursor.min_x-=dx; cursor.max_x-=dx;
                cursor.min_y-=dy; cursor.max_y-=dy;
                SetAxisRange (panel, control, VAL_MANUAL, cursor.min_x,
                              cursor.max_x, VAL_MANUAL, cursor.min_y,
                              cursor.max_y);
                break;
        }
    }

    if (event == EVENT_COMMIT) {
        cursor.firstPt = FALSE;
        GetGraphCursor (panel, control, 1, &x1, &y1);
        switch (graph->cursor) {
            case CURSOR_MAGNIFY:
                if (cursor.boxhandle) {
                    DeleteGraphPlot (panel, control, cursor.boxhandle, VAL_IMMEDIATE_DRAW);
                    cursor.boxhandle = 0;
                }
                if ((cursor.x0 != x1) && (cursor.y0 != y1)) {
                    if (cursor.x0 > x1) {
                        tmp = x1;
                        x1 = cursor.x0;
                        cursor.x0 = tmp;
                    }

                    if (cursor.y0 > y1) {
                        tmp = y1;
                        y1 = cursor.y0;
                        cursor.y0 = tmp;
                    }

                    SetAxisRange (panel, control,
                          VAL_MANUAL, cursor.x0, x1,
                          VAL_MANUAL, cursor.y0, y1);

                    SetGraphCursor (panel, control, 1,
                            cursor.x0+(x1-cursor.x0)/2,
                            cursor.y0+(y1-cursor.y0)/2);
                }
                break;
            case CURSOR_XY:
                if (graph->textHandle)
                    DeleteGraphPlot (panel, control, graph->textHandle, VAL_DELAYED_DRAW);
                Fmt (text, "%s<%f[e2p3], %f[e2p3]", x1, y1);
                graph->textHandle = PlotText (panel, control, x1+graph_XTextOffset(),
                                              y1+graph_YTextOffset(), text,
                                              VAL_DIALOG_META_FONT, VAL_WHITE,
                                              VAL_TRANSPARENT);
                break;
            case CURSOR_PAN:
                dx = x1 - cursor.x0; dy = y1 - cursor.y0;
                cursor.min_x-=dx; cursor.max_x-=dx;
                cursor.min_y-=dy; cursor.max_y-=dy;
                SetAxisRange (panel, control, VAL_MANUAL, cursor.min_x,
                              cursor.max_x, VAL_MANUAL, cursor.min_y,
                              cursor.max_y);
                break;
            case CURSOR_SNAP:
                if (graph->textHandle) {
                    DeleteGraphPlot (panel, control, graph->textHandle, VAL_DELAYED_DRAW);
                    graph->textHandle = 0;
                }
                GetGraphCursorIndex (panel, control, 1, &handle, &index);
                for (i = 0; i < graph->curves.list.nItems; i++) {
                    curve = curvelist_GetItem (graph->curves.list, i);
                    if (curve->plothandle == handle) {
                        SetCtrlVal (graph->curves.panel, CURVES_LIST, i);
                        graph->textHandle = PlotText (panel, control, x1+graph_XTextOffset(),
                                                      y1+graph_YTextOffset(), curve->attr.label,
                                                      VAL_DIALOG_META_FONT, curve->attr.color,
                                                      VAL_TRANSPARENT);

                    }
                }
                break;
        }
    }
    return 0;
}

static void graph_ZoomInatMarker (int graph)
{
    double graphG_x, graphG_y, x0, x1, y0, y1, xstep, ystep;
    int xautoscale, yautoscale, logscale;

    GetGraphCursor (graph, GRAPH_GRAPH, 1, &graphG_x, &graphG_y);

    GetAxisRange (graph, GRAPH_GRAPH,
                  &xautoscale, &x0, &x1,
                  &yautoscale, &y0, &y1);

    GetCtrlAttribute (graph, GRAPH_GRAPH, ATTR_XMAP_MODE, &logscale);
    if (logscale)
    {
        if ((x1 / x0) > 100.0) {x0*=10; x1/=10;}
        else MessagePopup ("Zoom In Error", "No more zoomin' on X Axis");
    }
    else
    {
        xstep = (x1 - x0)/4;
        x0 = graphG_x - xstep;
        x1 = graphG_x + xstep;
    }

    GetCtrlAttribute (graph, GRAPH_GRAPH, ATTR_YMAP_MODE, &logscale);
    if (logscale)
    {
        if ((y1 / y0) > 100.0) {y0*=10; y1/=10;}
        else MessagePopup ("Zoom In Error", "No more zoomin' on Y Axis");
    }
    else
    {
        ystep = (y1 - y0)/4;
        y0 = graphG_y - ystep;
        y1 = graphG_y + ystep;
    }

    if ((x0 != x1) && (y0 != y1))
    {
        SetAxisRange (graph, GRAPH_GRAPH,
                      VAL_MANUAL, x0, x1,
                      VAL_MANUAL, y0, y1);
    }
}

static void axis_Init (axisType *x, axisType *y)
{
    Fmt (x->label, "X Axis");
    x->autoscale = VAL_AUTOSCALE;
    x->min = -1;
    x->max = 1;
    x->logscale.attr = ATTR_XMAP_MODE;
    x->logscale.val = VAL_LINEAR;
    x->divisions.attr = ATTR_XDIVISIONS;
    x->divisions.val = 10;
    x->grid.attr = ATTR_XGRID_VISIBLE;
    x->grid.val = TRUE;
	x->conversion.val = 0;
	
    Fmt (y->label, "Y Axis");
    y->autoscale = VAL_AUTOSCALE;
    y->min = -1;
    y->max = 1;
    y->logscale.attr = ATTR_YMAP_MODE;
    y->logscale.val = VAL_LINEAR;
    y->divisions.attr = ATTR_YDIVISIONS;
    y->divisions.val = 10;
    y->grid.attr = ATTR_YGRID_VISIBLE;
    y->grid.val = TRUE;
	y->conversion.val = 0;
}

static void axis_InitControl (graphPtr graph, int y_axis)
{
    int x1, y1;
	char name[300];

    graph->axisP = graph->axisP? graph->axisP : LoadPanel (utilG.p, "graphu.uir", AXIS);
    
	Fmt(name, "axis control panel: %s", graph->title);
    SetPanelAttribute (graph->axisP, ATTR_TITLE, name);
	SetPanelAttribute (graph->axisP, ATTR_CALLBACK_DATA, graph);
    util_InitClose (graph->axisP, AXIS_CLOSE, FALSE);

    axis_GetGraphArea (graph);
    if (y_axis) axis_UpdatePanel(graph->axisP, graph->y);
	else axis_UpdatePanel (graph->axisP, graph->x);
    SetCtrlVal (graph->axisP, AXIS_SELECTION, y_axis);

    DisplayPanel (graph->axisP);
}

static void axis_UpdatePanel (int panel, axisType axis)
{
    SetCtrlVal (panel, AXIS_LABEL, axis.label);
    SetCtrlVal (panel, AXIS_MIN, axis.min);
    SetCtrlVal (panel, AXIS_MAX, axis.max);

    SetCtrlVal (panel, AXIS_AUTOSCALE, axis.autoscale);
    if (axis.autoscale) {
        SetCtrlAttribute (panel, AXIS_MIN, ATTR_CTRL_MODE, VAL_INDICATOR);
        SetCtrlAttribute (panel, AXIS_MAX, ATTR_CTRL_MODE, VAL_INDICATOR);
    } else {
        SetCtrlAttribute (panel, AXIS_MIN, ATTR_CTRL_MODE, VAL_HOT);
        SetCtrlAttribute (panel, AXIS_MAX, ATTR_CTRL_MODE, VAL_HOT);
    }

    if (axis.divisions.val == VAL_AUTO) {
        SetCtrlVal (panel, AXIS_AUTODIV, TRUE);
        SetInputMode (panel, AXIS_DIVISIONS, FALSE);
    } else {
        SetCtrlVal (panel, AXIS_AUTODIV, FALSE);
        SetInputMode (panel, AXIS_DIVISIONS, TRUE);
        SetCtrlVal (panel, AXIS_DIVISIONS, axis.divisions.val);
    }

    SetCtrlVal (panel, AXIS_SCALE, axis.logscale.val);
    SetInputMode (panel, AXIS_AUTODIV, !axis.logscale.val);
    SetCtrlVal (panel, AXIS_GRID, axis.grid.val);
	SetCtrlVal (panel, AXIS_CONV, axis.conversion.val);
}

int  AxisControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    graphPtr graph;
    void *g;
    int autodiv, y_axis, conv;
    double min, max;
    axisType *myaxis;

    if (event == EVENT_COMMIT)
    {
        GetPanelAttribute (panel, ATTR_CALLBACK_DATA, &g);
        graph = g;
        GetCtrlVal (panel, AXIS_SELECTION, &y_axis);
        if (y_axis) myaxis = &graph->y; else myaxis = &graph->x;
        switch (control)
        {
            case AXIS_SELECTION:
                GetCtrlVal (panel, control, &y_axis);
                if (y_axis) axis_UpdatePanel (panel, graph->y);
                else axis_UpdatePanel (panel, graph->x);
                break;
            case AXIS_LABEL:
                GetCtrlVal (panel, control, myaxis->label);
                SetCtrlAttribute (graph->p, GRAPH_GRAPH,
                                  ATTR_XNAME, graph->x.label);
                SetCtrlAttribute (graph->p, GRAPH_GRAPH,
                                  ATTR_YNAME, graph->y.label);
                break;
            case AXIS_MIN:
                GetCtrlVal (panel, control, &min);
                if (min < myaxis->max)
                {
                    myaxis->min = min;
                    axis_SetGraphArea(graph);
                }
                else
                {
                    MessagePopup ("Minimum Range Error", "Minimum must be less than maximum");
                    SetActiveCtrl (panel, AXIS_MIN);
                }
                axis_UpdatePanel(panel, *myaxis);
                break;
            case AXIS_MAX:
                GetCtrlVal (panel, control, &max);
                if (max > myaxis->min)
                {
                    myaxis->max = max;
                    axis_SetGraphArea(graph);
                }
                else
                {
                    MessagePopup ("Maximum Range Error", "Maximum must be greater than minimum");
                    SetActiveCtrl (panel, AXIS_MAX);
                }
                axis_UpdatePanel(panel, *myaxis);
                break;
            case AXIS_GRID:
                GetCtrlVal (panel, control, &myaxis->grid.val);
                SetCtrlAttribute (graph->p, GRAPH_GRAPH,
                                  myaxis->grid.attr, myaxis->grid.val);
                break;
            case AXIS_SCALE:
                GetCtrlVal (panel, control, &myaxis->logscale.val);
                SetCtrlAttribute (graph->p, GRAPH_GRAPH,
                                  myaxis->logscale.attr,
                                  myaxis->logscale.val);
                if (myaxis->logscale.val)
                {
                    SetCtrlVal (panel, AXIS_AUTODIV, TRUE);
                    myaxis->divisions.val = VAL_AUTO;
                    SetCtrlAttribute (graph->p, GRAPH_GRAPH,
                                      myaxis->divisions.attr,
                                      myaxis->divisions.val);
                }
                axis_UpdatePanel (panel, *myaxis);
                break;
            case AXIS_DIVISIONS:
                GetCtrlVal (panel, control, &myaxis->divisions.val);
                SetCtrlAttribute (graph->p, GRAPH_GRAPH,
                                  myaxis->divisions.attr,
                                  myaxis->divisions.val);
                axis_UpdatePanel(panel, *myaxis);
                break;
            case AXIS_AUTOSCALE:
                GetCtrlVal (panel, control, &myaxis->autoscale);
                axis_SetGraphArea(graph);
                axis_GetGraphArea(graph);
                axis_UpdatePanel(panel, *myaxis);
                break;
            case AXIS_AUTODIV:
                GetCtrlVal (panel, control, &autodiv);
                if (autodiv) myaxis->divisions.val = VAL_AUTO;
                else myaxis->divisions.val = 10;
                SetCtrlAttribute (graph->p, GRAPH_GRAPH,
                                  myaxis->divisions.attr,
                                  myaxis->divisions.val);
                axis_UpdatePanel(panel, *myaxis);
                break;
			case AXIS_CONV:
				GetCtrlVal (panel, control, &myaxis->conversion.val);
				graph_ReplotCurvesWithConv(graph);
				if(myaxis->conversion.val == 3)
				{
					char pathname[260] = "";
					double res, temp, result, arr[274][2];
					int i= 0, file;
					/*
					double **arr;
					arr = malloc(sizeof(double*[274]));
					for(i = 0; i < 274; i++)
						arr[i] = malloc(sizeof(double) * 2);//*/
			
					i = FileSelectPopup ("", "", "*.tbl", "Custom Table", VAL_LOAD_BUTTON,
								 0, 0, 1, 0, pathname);
					if(i)
					{
						file = OpenFile (pathname, VAL_READ_ONLY, VAL_OPEN_AS_IS,
								 VAL_ASCII);
						if(file != -1)
						{
							RUO_USR_TABLEITEMS = 0;
							do{
								result = ScanFile(file, "%f %f", &res, &temp);
								arr[RUO_USR_TABLEITEMS][0] = res;
								arr[RUO_USR_TABLEITEMS][1] = temp;
								RUO_USR_TABLEITEMS++;
							}while(result > 0 && RUO_USR_TABLEITEMS < 274);
							RUO_USR_TABLEITEMS--;
						}
						else
							myaxis->conversion.val = 0;
					}
					else
						myaxis->conversion.val = 0;
				}
				acqcurve_Plot(graph, graph->p, GRAPH_GRAPH, &graph->acqcurve);
				axis_UpdatePanel(panel, *myaxis);
				break;
        }
    }
    return 0;
}

static void axis_GetGraphArea (graphPtr graph)
{
    GetAxisRange (graph->p, GRAPH_GRAPH,
                  &graph->x.autoscale, &graph->x.min, &graph->x.max,
                  &graph->y.autoscale, &graph->y.min, &graph->y.max);
}

static void axis_SetGraphArea (graphPtr graph)
{
    SetAxisRange (graph->p, GRAPH_GRAPH,
                  graph->x.autoscale, graph->x.min, graph->x.max,
                  graph->y.autoscale, graph->y.min, graph->y.max);
}

int CVICALLBACK PanelClose (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	if(event == EVENT_COMMIT)
	{
		SetCtrlVal(panel, control, 0);
		HidePanel(panel);
	}
	return 0;
}
