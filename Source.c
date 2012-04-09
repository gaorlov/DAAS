/*
	function									description
 	   |											|
	   V											V

source_Init							Installs callbacks for the Acquire panel buttons and menu
exp_DoSourceExp						Runs the functions that set the source, acquire a point and update the graphs
exp_UpdateBiasLevels				Updates the levels for the sources
exp_UpdateMarkedGraphs				Updates the graphs
*exp_SourceBiasNote					Makes a note for the biaslevel
exp_InitSourceExp					Starts the experiment using the sources in the list
exp_InitAcqCurveMarkers				
exp_ResetBiasLevels					Resets the bias levels
AcqCurveSourceMarkerCallback		
updateGraphSource					Updates the sources in the graphs
source_MenuCallback					Callback for the menu on the source panel
RemoveSourceCallback				Callback for the "remove" button on the acquire panel
MoveDownSourceCallback				Callback for the "move down" button on the acquire panel
MoveUpSourceCallback				Callback for the "move up" button on the acquire panel
exp_UpdateSourcePanel				Updates the list of sources on the acquire panel
SelectSourceCallback				Opens the source panel on double click
SourceExpCallback					Callback from the acquire panel menu. Sets the experiment settings for source based experiments
exp_GetSourceSelection				Returns a source from an index passed
sourcelist_ReplaceSource			
sourcelist_RemoveSource				Removes a source from the list of sources
sourcelist_AddSource				Adds a source to the list of sources
sourcelist_InitAcqCurveSourceList	Sets the "source" field in the graph options setup. (Archaic)
sourcelist_GetItem					Finds a source in the source list by index
sourcelist_GetItemByTitle			Finds a source in the source list by name
SaveSourceCallback					Save source function. called from sourceMenu. used to be a button callback
LoadSourceCallback					Load source function. called from sourceMenu. used to be a button callback
PlotSourceCallback					Plot source function. called from sourceMenu. used to be a button callback
SourceControlCallback				Callback for the source part of the source panel
SourceMaxSegCallback				Limits the number of segments available
source_UpdatePanel					Updates the source panel controls
source_InitPanel					Initializes the source panel for a given source
source_EndofScan					Checks for the end of scan
source_CalcBiasLevel				Calculates the output level for the next source set
source_TotalTime					Calculates the total time of the experiment
source_TotalPoints					Calculates the total number of points for the experiment
source_UpdateReading				Updates the output window in the source panel
source_Load							Loads a source from a file (does not include file selection)
source_Save							Saves a source to a file (does not include file selection)
source_Remove						Deletes a source and frees the pointer
range_Create						Creates a range structure
source_Create						Creates a source structure
SegmentControlCallback				Callback for the segment settings
segment_Notify						Displays a panel with the current resolution
segment_Update						Updates the segment part of the source panel
segment_Autorange					Calculates the step, stop and resolution for a scan if the resolution is on autorange
segment_Endof						Checks if the segment is over
segment_Start						Calculates the most accurate start based on the resolution
segment_Stop						Calculates the stop based on the step size and number of points
segment_ErrorCalcPtInc				Calculates the stop adjustment based on increasing the number of steps
segment_ErrorCalcStepCorrecion		Calculates the stop adjustment based on increasing the step size
segment_Step						Calculates the step size based on the start, desired stop and number of points
segment_Time						Calculates the time that the segment will take
segment_Rate						Calculates the rate of the scan based on the time
segment_Delay						Calculates the delay of each point based on the rate
segment_Points						Calculates the number of points based on delta start-stop and step size
segment_Create						Creates a segment structure
GenSourceControlCallback			Callback for the Floating Source experiment panel
gensrc_InitPanel					Initializes the floating source experiment panel
//*/

#include "Acqcrvu.h"

#include <formatio.h>
#include <userint.h>
#include <utility.h>
#include <ansi_c.h>

#include "util.h"
#include "list.h"
#include "channel.h"
#include "changen.h"
#include "chanfnc.h"
#include "chanops.h"
#include "acqchan.h"
#include "acqchanu.h"
#include "curve.h"
#include "acqcrv.h"
#include "acqcrvu.h"
#include "graph.h"
#include "graphu.h"
#include "gpibio.h"
#include "curveop.h"
#include "acquire.h"
#include "acquireu.h"
#include "source.h"
#include "sourceu.h"

#define TRUE 1
#define FALSE 0

static struct sourceLStruct {
    sourcePtr src;
    listType sources;
}   sourceL;
int messagePanel = 0;

static segmentPtr  	segment_Create(int i);
static double      	segment_Time (segmentPtr seg);
static double      	segment_Rate (segmentPtr seg);
static double      	segment_Delay (segmentPtr seg);
static int         	segment_Points (segmentPtr seg);
static double      	segment_Step (segmentPtr seg, int range);
static double		segment_Start (segmentPtr seg, int range);
static double		segment_Stop (segmentPtr seg, int range);
static void			segment_Autorange (segmentPtr seg, rangePtr *range, int panel, int control);
static int         	segment_Endof (segmentPtr seg);
static void        	segment_Update (int panel, sourcePtr src, segmentPtr seg);
static void 		segment_Notify (sourcePtr src, int range);

rangePtr	range_Create  (double maxVal, double minVal, double resolution);
sourcePtr   source_Create (char *label, void *dev, SetLevelPtr SetLevel, GetReadingPtr GetReading);
void        source_Remove (sourcePtr src);
void        source_Save (sourcePtr src);
void        source_Load (void *dev, sourcePtr src);
void        source_UpdateReading (int panel, void *src);
int         source_TotalPoints (sourcePtr src);
double      source_TotalTime(sourcePtr src);
void        source_CalcBiasLevel (sourcePtr src);
int         source_EndofScan (sourcePtr src);

void        gensrc_InitPanel (sourcePtr src);

void        source_InitPanel (sourcePtr src);
void        source_UpdatePanel (int panel, sourcePtr src);

sourcePtr sourcelist_GetItem (int i);
void      sourcelist_InitAcqCurveSourceList (int panel, void *src_marker);
void      sourcelist_AddSource (sourcePtr src);
void      sourcelist_RemoveSource (sourcePtr src);
void      sourcelist_ReplaceSource (sourcePtr src);

sourcePtr 	exp_GetSourceSelection (void);
int 		SaveSourceCallback (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int			LoadSourceCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int			PlotSourceCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void		source_MenuCallback(int menubar, int menuItem, void *callbackData, int panel);
int			SelectSourceCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int			RemoveSourceCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int			MoveDownSourceCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int			MoveUpSourceCallback (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void		SourceExpCallback(int menubar, int menuItem, void *callbackData, int panel);
void		exp_UpdateSourcePanel (void);
int			AcqCurveSourceMarkerCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
        

void      exp_ResetBiasLevels (int src_index);
void      exp_InitAcqCurveMarkers (void);
void      exp_InitSourceExp (void);
char      *exp_SourceBiasNote (sourcePtr src);
void      exp_UpdateMarkedGraphs (sourcePtr src, int term_seg);
void      exp_UpdateBiasLevels (void);
void      exp_DoSourceExp (void);
int 	  AcqCurvePanelCallback (int panel, int event, void *callbackData, int eventData1, int eventData2);

void updateGraphSource(void);

void source_Init (void)
{
    if (utilG.acq.status != ACQ_NONE) {
        util_ChangeInitMessage ("Source Utilities...");

        InstallMenuCallback (acquire_GetMenuBar(), ACQMENUS_EXP_SOURCE,
                         SourceExpCallback, 0);

        InstallCtrlCallback (acqG.p.setup, ACQSETUP_SRC_LIST,
                             SelectSourceCallback, 0);
        InstallCtrlCallback (acqG.p.setup, ACQSETUP_SRC_MOVEUP,
                             MoveUpSourceCallback, 0);
        InstallCtrlCallback (acqG.p.setup, ACQSETUP_SRC_MOVEDOWN,
                             MoveDownSourceCallback, 0);
        InstallCtrlCallback (acqG.p.setup, ACQSETUP_SRC_REMOVE,
                             RemoveSourceCallback, 0);

        SourceExpCallback (0, 0, NULL, 0);
    }
}

void exp_DoSourceExp (void)
{
    acqchanlist_GetandSaveReadings();
    exp_UpdateBiasLevels();
	graphlist_PlotReadings ();
}

void exp_UpdateBiasLevels (void)
{
    int src_index;
    sourcePtr src;

    for (src_index = 0; src_index <= sourceL.sources.nItems; src_index++) {
        if (src_index == sourceL.sources.nItems) {
            utilG.acq.status = ACQ_TERMINATE;
            break;
        }

        src = sourcelist_GetItem(src_index);
        if (!segment_Endof (src->segments[src->seg])) {
            if((src->segments[src->seg]->error.on && (fabs(src->acqchan->reading - src->biaslevel) < src->segments[src->seg]->error.val)) || !(src->segments[src->seg]->error.on))
			{
				src->segments[src->seg]->pt++;
				exp_ResetBiasLevels (src_index);
			}
			else if(src->segments[src->seg]->error.on)
				utilG.acq.pt--;
			break;
        }
        else {
            if (!source_EndofScan (src)) {
                if((src->segments[src->seg]->error.on && (fabs(src->acqchan->reading - src->biaslevel) < src->segments[src->seg]->error.val)) || !(src->segments[src->seg]->error.on))
				{
					src->seg++;
					exp_UpdateMarkedGraphs(src, TRUE);
                	exp_ResetBiasLevels(src_index);
				}
				else if(src->segments[src->seg]->error.on)
					utilG.acq.pt--;
                break;
            }
            else {
                if((src->segments[src->seg]->error.on && (fabs(src->acqchan->reading - src->biaslevel) < src->segments[src->seg]->error.val)) || !(src->segments[src->seg]->error.on))
				{
					exp_UpdateMarkedGraphs (src, TRUE);
                	exp_UpdateMarkedGraphs (src, FALSE);
				}
				else if(src->segments[src->seg]->error.on)
					utilG.acq.pt--;	
            }
        }
    }
}

void exp_UpdateMarkedGraphs (sourcePtr src, int term_seg)
{
    int g, i, src_index;
    graphPtr graph;
    curvePtr curve;

    for (g = 0; g < graphG.graphs.nItems; g++) {
        graph = graphlist_GetItem (g);
        if (graph && graph->acqcurve.marker.on) {
            if ((src == graph->acqcurve.marker.source) &&
                (term_seg == graph->acqcurve.marker.segment)) {

/*remove*/      acqcurve_Hide(graph->p, GRAPH_GRAPH, &graph->acqcurve);

/*plot*/        if (graph->acqcurve.x && graph->acqcurve.y) {
                    curve = acqcurve_MakeCurve (graph->curves.list.nItems,
                                                graph->title, &graph->acqcurve);
                    if (curve) {
                        Fmt (curve->attr.note, "%s[a]<\n%s", exp_SourceBiasNote(src));
                        curvelist_AddCurve (&graph->curves, curve);
                        curve_Plot (graph->p, GRAPH_GRAPH, curve, graph);
                    }

                    if (graph->acqcurve.autoattr.save)
                    {
                        fileHandle.analysis = util_OpenFile (graph->acqcurve.path, FILE_WRITE, FALSE);
                        FmtFile (fileHandle.analysis, "#GRAPHDATA\n");
                        graph_Save (graph);
                        FmtFile (fileHandle.analysis, "#ENDGRAPHDATA\n");
                        util_CloseFile();
                    }
                }

                graph->acqcurve.marker.offset = utilG.acq.pt + 1;
				src_index = list_FindItem (sourceL.sources, src);
                if (graph->acqcurve.marker.segment) {
                    graph->acqcurve.marker.pts = 1;
                    for (i = 0; i < src_index; i++) {
                        src = sourcelist_GetItem (i);
                        graph->acqcurve.marker.pts*=src->points;
                    }
                    src = sourcelist_GetItem (src_index);
                    graph->acqcurve.marker.pts*=src->segments[src->seg]->points;

                }
            }
        }
    }
}

char *exp_SourceBiasNote (sourcePtr src)
{
    char *note;
    int index, src_index;
    sourcePtr srcX;

    note = "                                                                                       ";
    Fmt (note, "%s: %f[e2p5]\n", src->acqchan->channel->label, src->biaslevel);
    index = list_FindItem (sourceL.sources, src);
    for (src_index = (index+1); src_index < sourceL.sources.nItems; src_index++) {
        srcX = sourcelist_GetItem (src_index);
        Fmt (note, "%s[a]<%s: %f[e2p5]\n", srcX->acqchan->channel->label,
             srcX->biaslevel);
    }
    return note;
}

void exp_InitSourceExp (void)
{
    exp_ResetBiasLevels (sourceL.sources.nItems);
    updateGraphSource();
	exp_InitAcqCurveMarkers();
}

void exp_InitAcqCurveMarkers (void)
{
    int g, i, src_index;
    graphPtr graph;
    sourcePtr src;

    for (g = 0; g < graphG.graphs.nItems; g++) {
        graph = graphlist_GetItem (g);
        graph->acqcurve.marker.offset = 0;
        if (graph && graph->acqcurve.marker.on) {
            src = graph->acqcurve.marker.source;
            src_index = list_FindItem (sourceL.sources, src);
            if(src)
			{
				if (graph->acqcurve.marker.segment)
                	graph->acqcurve.marker.pts = src->segments[0]->points;
            	else
                	graph->acqcurve.marker.pts = src->points;
            	for (i = 0; i < src_index; i++) {
                	src = sourcelist_GetItem (i);
                	graph->acqcurve.marker.pts*=src->points;
				}
            }
        }
        else graph->acqcurve.marker.pts = utilG.acq.nPts;
    }
}

void exp_ResetBiasLevels (int src_index)
{
    int nested_src, seg;
    sourcePtr src;

    for (nested_src = 0; nested_src < src_index; nested_src++) {
        src = sourcelist_GetItem (nested_src);
        src->seg = 0;
        for (seg = 0; seg < src->nSegments; seg++) src->segments[seg]->pt = 0;
        source_CalcBiasLevel (src);
        if (src->SetLevel) src->SetLevel (src);
    }
	if (src_index < sourceL.sources.nItems) {
        src = sourcelist_GetItem (src_index);
        source_CalcBiasLevel (src);
        if (src->SetLevel) src->SetLevel (src);
    }

}

int  AcqCurveSourceMarkerCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    sourcePtr src;
    graphPtr graph;
    int src_index, i, pts;

    if (event == EVENT_COMMIT) {
        graph = callbackData;
        GetCtrlVal (panel, ACQCURVE_MARKER, &graph->acqcurve.marker.on);
        SetInputMode (panel, ACQCURVE_MARKER_SOURCES, graph->acqcurve.marker.on);
        SetInputMode (panel, ACQCURVE_MARKER_TERM, graph->acqcurve.marker.on);

        if (!graph->acqcurve.marker.on) {
            graph->acqcurve.marker.offset = 0;
            graph->acqcurve.marker.pts = utilG.acq.nPts;
        }
        else {
            GetCtrlVal (panel, ACQCURVE_MARKER_SOURCES, &src_index);
            graph->acqcurve.marker.source = sourcelist_GetItem (src_index);
            GetCtrlVal (panel, ACQCURVE_MARKER_TERM, &graph->acqcurve.marker.segment);

            pts = 1;
            for (i = 0; i <= src_index; i++) {
                src = sourcelist_GetItem (i);
                pts *= src->points;
            }
            graph->acqcurve.marker.offset = (utilG.acq.pt + 1) / pts * pts;

            src = sourcelist_GetItem (src_index);
            pts /= src->points;

            if (graph->acqcurve.marker.segment) {
                for (i = 0; i < src->seg; i++)
                    graph->acqcurve.marker.offset+=(pts*src->segments[i]->points);
                graph->acqcurve.marker.pts = pts * src->segments[src->seg]->points;
            }
            else graph->acqcurve.marker.pts = pts * src->points;
			/*
		        FmtOut ("%s: offset: %i, points: %i\n",
                graph->title, graph->acqcurve.marker.offset,
                graph->acqcurve.marker.pts);//*/
        }
        
    }
    return 0;
}

void updateGraphSource()
{
	int i, j, pts;
	char *label;
	sourcePtr src;
	acqcurveGs acqcurveG;
	acqcurveGptr acqptr;
	acqchanPtr x, y, x0;
	graphPtr graph;
	nodePtr node;
	for(i = 0; i < acqcurveL.acqcurves.nItems; i++)
    {
		node = list_GetNode(acqcurveL.acqcurves, i);
		acqptr = node->item;
		graph = acqptr->graph;
		graph->acqcurve.marker.source = sourcelist_GetItem (0);
		if (acqptr->InitSourceMarker && (utilG.exp == EXP_SOURCE))
			acqptr->InitSourceMarker(acqptr->Apanel, graph->acqcurve.marker.source);
		src = graph->acqcurve.marker.source;
		x  = graph->acqcurve.x;
		y  = graph->acqcurve.y;
		x0 = graph->acqcurve.x0;
		if(x) x  = acqchanlist_GetItemByTitle(graph->acqcurve.x->channel->label);
		if(y) y  = acqchanlist_GetItemByTitle(graph->acqcurve.y->channel->label);
		if(x0)x0 = acqchanlist_GetItemByTitle(graph->acqcurve.x0->channel->label);
		graph->acqcurve.x = x;
		graph->acqcurve.y = y;
		graph->acqcurve.x0 = x0;
		label = x? graph->acqcurve.x->channel->label:"X axis";
		if(graph->p)SetCtrlAttribute (graph->p, GRAPH_GRAPH, ATTR_XNAME, label);
		label = y? graph->acqcurve.y->channel->label:"Y axis";
		SetCtrlAttribute (graph->p, GRAPH_GRAPH, ATTR_YNAME, label);
		if(src)
		{
			pts = src->points;
			graph->acqcurve.marker.offset = (utilG.acq.pt + 1) / pts * pts;

			src = graph->acqcurve.marker.source;
			pts /= src->points;

			if (graph->acqcurve.marker.segment) {
				for (j = 0; j < src->seg; j++)
					graph->acqcurve.marker.offset+=(pts*src->segments[j]->points);
				graph->acqcurve.marker.pts = pts * src->segments[src->seg]->points;
			}
			else graph->acqcurve.marker.pts = pts * src->points;
		}
		
	}
}

void source_MenuCallback (int menubar, int menuItem, void *callbackData, int panel)
{
	sourcePtr src = callbackData;
	switch (menuItem)
	{
		case SOURCEMENU_SCAN_STEP_SAVE:
			SaveSourceCallback (panel, 0, EVENT_COMMIT, src, 0, 0);
			break;
		case SOURCEMENU_SCAN_STEP_LOAD:
			LoadSourceCallback (panel, 0, EVENT_COMMIT, src, menuItem, 0);
			break;
		case SOURCEMENU_SCAN_EXTERN_LOAD:
			{
				segmentPtr seg = callbackData;
				LoadSourceCallback (panel, 0, EVENT_COMMIT, seg, menuItem, 0);
			}break;
		case SOURCEMENU_SRC_SEG_PLOT:
			PlotSourceCallback (panel, 0, EVENT_COMMIT, src, 0, 0);
			break;
	}
}
int  RemoveSourceCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int i;
	if (event == EVENT_COMMIT)
	{
		sourcePtr src = exp_GetSourceSelection();
		sourcelist_RemoveSource (src);
		GetNumListItems (panel, ACQSETUP_SRC_LIST, &i);
		if(src->panel)
			source_UpdatePanel(src->panel, src);
		
		if(i)
			updateGraphSource();
	}
	return 0;
}

int  MoveDownSourceCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int i, pts;
    sourcePtr src;
	
	if (event == EVENT_COMMIT) {
        src = exp_GetSourceSelection();
        i = list_FindItem (sourceL.sources, src);
        if (sourceL.sources.last->item != src) {
            DeleteListItem (panel, ACQSETUP_SRC_LIST, i, 1);
            list_MoveDown (sourceL.sources, i);
            InsertListItem (panel, ACQSETUP_SRC_LIST, i+1,
                            src->acqchan->channel->label, i+1);
        }
		updateGraphSource();
	}
    return 0;
}

int  MoveUpSourceCallback (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int i, pts;
    sourcePtr src;
	if (event == EVENT_COMMIT) {
        src = exp_GetSourceSelection();
        i = list_FindItem (sourceL.sources, src);
        if (sourceL.sources.first->item != src) {
            DeleteListItem (panel, ACQSETUP_SRC_LIST, i, 1);
            list_MoveUp (sourceL.sources, i);
            InsertListItem (panel, ACQSETUP_SRC_LIST, i-1, src->acqchan->channel->label, i-1);
        }
		updateGraphSource();
	}
    return 0;
}

void exp_UpdateSourcePanel (void)
{
    int i, points;
    sourcePtr src;
    double time;

    points = 1;
    time = 0;

    SetInputMode (acqG.p.setup, ACQSETUP_SRC_MOVEUP, (sourceL.sources.nItems && !util_TakingData()));
    SetInputMode (acqG.p.setup, ACQSETUP_SRC_MOVEDOWN, (sourceL.sources.nItems && !util_TakingData()));
    SetInputMode (acqG.p.setup, ACQSETUP_SRC_REMOVE, (sourceL.sources.nItems && !util_TakingData()));

    for (i = 0; i < sourceL.sources.nItems ; i++) {
        src = sourcelist_GetItem (i);
        points *= src->points;
        if (i == 0) time = src->time;
        else time = src->points * ((double)src->time/src->points + time);
    }

    SetCtrlVal (acqG.p.setup, ACQSETUP_SRC_POINTS, points);
    SetCtrlVal (acqG.p.setup, ACQSETUP_SRC_TIME, time);
    utilG.acq.nPts = points;
}

int  SelectSourceCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    sourcePtr src;
    if ((event == EVENT_LEFT_DOUBLE_CLICK) && sourceL.sources.nItems) {
        src = exp_GetSourceSelection();
        source_InitPanel(src);
    }
    return 0;
}

void SourceExpCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    int height, i;
    sourcePtr src;

    if (utilG.exp != EXP_SOURCE) {
        utilG.exp = EXP_SOURCE;
        acquire_UpdatePanelExp();
    }

    ClearListCtrl (acqG.p.setup, ACQSETUP_SRC_LIST);
    for (i = 0; i < sourceL.sources.nItems; i++) {
        src = sourcelist_GetItem (i);
        InsertListItem (acqG.p.setup, ACQSETUP_SRC_LIST, -1, src->acqchan->channel->label, i);
    }
	updateGraphSource();
    expG.InitExp = exp_InitSourceExp;
    expG.UpdatePanel = exp_UpdateSourcePanel;
    expG.DoExp = exp_DoSourceExp;
    expG.delay = 0.0;

    exp_UpdateSourcePanel();
}

sourcePtr exp_GetSourceSelection (void)
{
    int i;
    GetCtrlIndex (acqG.p.setup, ACQSETUP_SRC_LIST, &i);
    return sourcelist_GetItem (i);
}

void sourcelist_ReplaceSource (sourcePtr src)
{
    int i;
    if (src->inlist) {
        i = list_FindItem (sourceL.sources, src);
        ReplaceListItem (acqG.p.setup, ACQSETUP_SRC_LIST, i, src->acqchan->channel->label, i);
    }
}

void sourcelist_RemoveSource (sourcePtr src)
{
    int i, g, okRemove = TRUE;
    graphPtr graph;
    char msg[256];
	if (okRemove) {
        i = list_FindItem (sourceL.sources, src);
        if (i != NOT_IN_LIST) {
            list_RemoveItem (&sourceL.sources, i, FALSE);
            DeleteListItem (acqG.p.setup, ACQSETUP_SRC_LIST, i, 1);
            src->inlist = FALSE;
            exp_UpdateSourcePanel();
        }
    }
}

void sourcelist_AddSource (sourcePtr src)
{
    if (list_AddItem(&sourceL.sources, src)){
        InsertListItem (acqG.p.setup, ACQSETUP_SRC_LIST, -1, src->acqchan->channel->label,
                        sourceL.sources.nItems-1);
        exp_UpdateSourcePanel();
    }
}

void sourcelist_InitAcqCurveSourceList (int panel, void *src_marker)
{
    int i=0;
    sourcePtr src;

	ClearListCtrl (panel, ACQCURVE_MARKER_SOURCES);
	src = sourcelist_GetItem (i);
	if(src)
	{
		InsertListItem (panel, ACQCURVE_MARKER_SOURCES, -1,
                        src->acqchan->channel->label, i);
		if (src == src_marker)
			SetCtrlIndex (panel, ACQCURVE_MARKER_SOURCES, i);
	}
}

sourcePtr sourcelist_GetItem (int i)
{
    nodePtr node;
    node = list_GetNode (sourceL.sources, i);
    if(node)
		return node->item;
	else
		return NULL;
}
sourcePtr sourcelist_GetItemByTitle(char *label)
{
	int i = 0;
	nodePtr node;
	sourcePtr src;
	do{
		node = list_GetNode (sourceL.sources, i);
		i++;
		src = node->item;
	}while(node && strcmp(src->acqchan->channel->label, label));
	return node->item;
}
/****************************************************************************/

int  SaveSourceCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int fileselect;
    char path[260];
    sourcePtr src;

    if (event == EVENT_COMMIT) {
        src = callbackData;
        fileselect = FileSelectPopup ("", "*.src", "*.src",
                                      "Save Source Setup", VAL_SAVE_BUTTON, 0,
                                      1, 1, 1, path);
        if (fileselect) {
            fileHandle.analysis = util_OpenFile (path, FILE_WRITE, FALSE);
            FmtFile (fileHandle.analysis, "%s<#SOURCEDATA\n");
            source_Save(src);
            FmtFile (fileHandle.analysis, "%s<#ENDSOURCEDATA\n");
            util_CloseFile();
        }
    }
    return 0;
}

int  LoadSourceCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int fileselect, id, i;
    char path[260], info[260];
    
    if (event == EVENT_COMMIT && eventData1 == SOURCEMENU_SCAN_STEP_LOAD) {
        
		sourcePtr src = callbackData;
        fileselect = FileSelectPopup ("", "*.src", "*.src", "Load Source Setup",
                                      VAL_LOAD_BUTTON, 0, 1, 1, 0, path);
        if (fileselect == VAL_EXISTING_FILE_SELECTED) {
            fileHandle.analysis = util_OpenFile (path, FILE_READ, FALSE);
            ReadLine (fileHandle.analysis, info, 255);
            if (CompareBytes (info, 0, "#SOURCEDATA", 0, 10, 0) == 0) {
				source_Load (src, src);
               	ReadLine (fileHandle.analysis, info, 255);
            }
            else util_MessagePopup ("Load Source Error", "Wrong source type");
			util_CloseFile();
            source_UpdatePanel(panel, src);
            exp_UpdateSourcePanel();
        }
    }
	else if(event == EVENT_COMMIT && eventData1 == SOURCEMENU_SCAN_EXTERN_LOAD)
	{
		segmentPtr seg = callbackData;
		sourcePtr src = seg->source;
		fileselect = FileSelectPopup ("", "*.seg", "*.seg", "Load Segment Curve",
                                      VAL_LOAD_BUTTON, 0, 1, 1, 0, path);
        if (fileselect == VAL_EXISTING_FILE_SELECTED)
		{
            fileHandle.analysis = util_OpenFile (path, FILE_READ, FALSE);
        	ScanFile (fileHandle.analysis, "%s>%i:", &seg->points);
			seg->valArray = malloc(sizeof(double) * seg->points);
			for (i = 0; i < seg->points; i++)
				ScanFile (fileHandle.analysis, "%s>%f[x]", &seg->valArray[i]);
			util_CloseFile();
            segment_Update (src->panel, src, seg); 
            exp_UpdateSourcePanel();
			return 1;
		}
		else
			return 0;
	}
            
	return 0;
}

int *pointArray(int length, int startVal)
{
	int *array = malloc(sizeof(int) * length);
	int i;
	for(i = 0; i < length; i++)
		array[i] = i + startVal;
	return array;
}

int  PlotSourceCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int plotpanel, seg, pt, height, width, *tempArr;
    char msg[256];
    double ymin, ymax, step;
    sourcePtr src;

    if (event == EVENT_COMMIT) {
        src = callbackData;
        plotpanel = LoadPanel (utilG.p, "sourceu.uir", SOURCEPLOT);
        SetPanelPos (plotpanel, VAL_AUTO_CENTER, VAL_AUTO_CENTER);
        util_InitClose (plotpanel, SOURCEPLOT_CLOSE, FALSE);
		ymin = src->segments[0]->start;
        ymax = src->segments[0]->start;
        for (seg = 0; seg < src->nSegments; seg++) {
        	if (src->segments[seg]->output == OUTPUT_EXTERN)
			{
				for(pt =0; pt < src->segments[seg]->points; pt ++)
				{
					if (src->segments[seg]->valArray[pt] < ymin)
                		ymin = src->segments[seg]->valArray[pt];
            		if (src->segments[seg]->valArray[pt] > ymax)
                		ymax = src->segments[seg]->valArray[pt];
				}
			}
			else
			{
				if (src->segments[seg]->start < ymin)
                	ymin = src->segments[seg]->start;
            	if (src->segments[seg]->stop < ymin)
                	ymin = src->segments[seg]->stop;
            	if (src->segments[seg]->start > ymax)
                	ymax = src->segments[seg]->start;
            	if (src->segments[seg]->stop > ymax)
                	ymax = src->segments[seg]->stop;
        	}
		}

       DeleteGraphPlot (plotpanel, SOURCEPLOT_GRAPH, -1, 0);

        if (ymin == ymax)
			SetAxisRange (plotpanel, SOURCEPLOT_GRAPH, VAL_MANUAL, 0.0,
						  source_TotalPoints(src)-src->nSegments, VAL_AUTO, 1.0, 1.0);
        else
			SetAxisRange (plotpanel, SOURCEPLOT_GRAPH, VAL_MANUAL, 0.0,
						  source_TotalPoints(src)-src->nSegments, VAL_MANUAL, ymin, ymax);

        SetCtrlAttribute (plotpanel, SOURCEPLOT_GRAPH, ATTR_YNAME, src->acqchan->channel->label);

        step = (ymax - ymin)/12;
		pt = 0;
        DisplayPanel(plotpanel);
        for (seg = 0; seg < src->nSegments; seg++) {
            if (src->segments[seg]->output == OUTPUT_EXTERN)
			{
				tempArr = pointArray(src->segments[seg]->points, pt);
				PlotXY (plotpanel, SOURCEPLOT_GRAPH, tempArr,
						src->segments[seg]->valArray, src->segments[seg]->points-1,
						VAL_INTEGER, VAL_DOUBLE, VAL_THIN_LINE, VAL_SOLID_SQUARE,
						VAL_SOLID, 1, VAL_YELLOW);
			}
			else
				PlotLine (plotpanel, SOURCEPLOT_GRAPH, pt, src->segments[seg]->start,
                     pt+src->segments[seg]->points-1, src->segments[seg]->stop, VAL_YELLOW);
            pt += src->segments[seg]->points-1;
        }
    }
    return 0;
}

int  SourceControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    sourcePtr src = callbackData;
	int i, i1, i2;
    char title[256];
    double r;

    switch (control) {
        case SOURCE_NOTE:
            AcqDataNoteCallback (panel, control, event, src->acqchan, eventData1, eventData2);
            break;
        case SOURCE_OUTPUT:
            if (event == EVENT_COMMIT) {
                GetCtrlVal (panel, control, &src->biaslevel);
                if (src->SetLevel) src->SetLevel (src);
                if (src->acqchan->GetReading) src->acqchan->GetReading(src->acqchan);
				SetCtrlVal (panel, control, src->acqchan->reading);
            }
            break;
        case SOURCE_LABEL:
            if (event == EVENT_COMMIT) {
                GetCtrlVal (panel, control, src->acqchan->channel->label);
                sourcelist_ReplaceSource (src);
                acqchanlist_ReplaceChannel (src->acqchan);
                Fmt (title, "Source Control: %s", src->acqchan->channel->label);
                SetPanelAttribute (panel, ATTR_TITLE, title);
                acqchan_UpdateReadingPanel(src->acqchan);
            }
            break;
        case SOURCE_COEFF:
            if (event == EVENT_COMMIT) {
                GetCtrlVal (panel, control, &src->acqchan->coeff);
                acqchan_UpdateReadingPanel (src->acqchan);
            }
            break;
        case SOURCE_ACQUIRE:
            if (event == EVENT_VAL_CHANGED) {
                GetCtrlVal (panel, control, &src->acqchan->acquire);
                if (src->acqchan->acquire) acqchanlist_AddChannel(src->acqchan);
                    else acqchanlist_RemoveChannel(src->acqchan);
            }
            break;
        case SOURCE_INLIST:
            if (event == EVENT_VAL_CHANGED) {
                GetCtrlVal (panel, control, &src->inlist);
                if (src->inlist) sourcelist_AddSource(src);
                    else sourcelist_RemoveSource(src);
            }
            break;
        case SOURCE_SEGSEL:
            if (event == EVENT_COMMIT) {
                GetCtrlIndex (panel, control, &i);
                segment_Update (panel, src, src->segments[i]);
            }
            break;
        case SOURCE_CLOSE:
            if (event == EVENT_COMMIT) {
                devPanel_Remove (panel);
                
                DiscardPanel (panel);
            }
            break;
		case SOURCE_PRECISION:
			if(event == EVENT_COMMIT)
				GetCtrlVal (panel, control, &src->precAssist);
			break;
    }
	if(event == EVENT_COMMIT)
	{
		if(src->precAssist)
			src->ranges.range = src->ranges.temprange;
		else
			src->ranges.range = src->ranges.nullrange;
	}
    return 0;
}

int  SourceMaxSegCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int nSegs, sel, dSeg, i;
    sourcePtr src;
    if (event == EVENT_COMMIT) {
        src = callbackData;
        GetCtrlVal (panel, control, &nSegs);
        GetCtrlIndex (panel, SOURCE_SEGSEL, &sel);
        if (nSegs < src->nSegments) {
            if (ConfirmPopup ("Maximum # of Segments Message",
                "Reducing the number of segments will remove some segments.\n"
                "Are you sure you want to do this?")) {
                for (dSeg = (src->nSegments-1); dSeg >= nSegs; dSeg--) {
                    free (src->segments[dSeg]);
                    src->segments[dSeg] = NULL;
                    DeleteListItem (panel, SOURCE_SEGSEL, dSeg, 1);
                }
                if ((nSegs-1) < sel) {
                    SetCtrlIndex (panel, SOURCE_SEGSEL, nSegs-1);
                    segment_Update (panel, src, src->segments[nSegs-1]);
                }
            }
            else {
                SetCtrlVal (panel, SOURCE_NSEGMENTS, src->nSegments);
                return 1;
            }
        }
        else {
            for (i = src->nSegments; i < nSegs; i++) {
                src->segments[i] = segment_Create(i);
				src->segments[i]->source = src;
                InsertListItem (panel, SOURCE_SEGSEL, -1, src->segments[i]->label, i);
            }
            SetCtrlIndex (panel, SOURCE_SEGSEL, sel);
        }
        src->nSegments = nSegs;
        SetCtrlVal (panel, SOURCE_PTS_TOTAL, source_TotalPoints (src));
        SetCtrlVal (panel, SOURCE_TIME_TOTAL, source_TotalTime(src));
        exp_UpdateSourcePanel();
    }
    return 0;
}

void source_UpdatePanel (int panel, sourcePtr src)
{
    int i, mode, bg;
	if (util_TakingData() && src->inlist) {
        mode = VAL_INDICATOR;
        bg = VAL_PANEL_GRAY;
    } else {
        mode = VAL_HOT;
        bg = VAL_WHITE;
    }
	panel = src->panel;
    SetPanelAttribute (panel, ATTR_TITLE, src->acqchan->channel->label);
	SetCtrlAttribute (panel, SOURCE_OUTPUT, ATTR_MIN_VALUE, src->min);
    SetCtrlAttribute (panel, SOURCE_OUTPUT, ATTR_MAX_VALUE, src->max);
    SetCtrlAttribute (panel, SOURCE_OUTPUT, ATTR_CTRL_MODE, mode);
    SetCtrlAttribute (panel, SOURCE_OUTPUT, ATTR_TEXT_BGCOLOR, bg);
    SetInputMode (panel, SOURCE_ACQUIRE, !util_TakingData());
    SetInputMode (panel, SOURCE_INLIST, !util_TakingData());
    SetInputMode (panel, SOURCE_NSEGMENTS, !util_TakingData());

    src->acqchan->GetReading(src->acqchan);
	SetCtrlVal (panel, SOURCE_OUTPUT, src->acqchan->reading);

    SetCtrlVal (panel, SOURCE_LABEL, src->acqchan->channel->label);
    SetCtrlVal (panel, SOURCE_COEFF, src->acqchan->coeff);

    SetCtrlVal (panel, SOURCE_ACQUIRE, src->acqchan->acquire);
    SetCtrlVal (panel, SOURCE_NOTE, src->acqchan->note);
    SetCtrlVal (panel, SOURCE_INLIST, src->inlist);

    SetCtrlVal (panel, SOURCE_PTS_TOTAL, source_TotalPoints (src));
    SetCtrlVal (panel, SOURCE_TIME_TOTAL, source_TotalTime(src));

    SetCtrlVal (panel, SOURCE_NSEGMENTS, src->nSegments);
	SetCtrlVal (panel, SOURCE_PRECISION, src->precAssist);
    ClearListCtrl (panel, SOURCE_SEGSEL);
    for (i = 0; i < src->nSegments; i++)
        InsertListItem (panel, SOURCE_SEGSEL, -1, src->segments[i]->label, i);
    SetCtrlVal (panel, SOURCE_SEGSEL, 0);

    SetCtrlAttribute (panel, SOURCE_OUTPUT, ATTR_CALLBACK_DATA, src);
    SetCtrlAttribute (panel, SOURCE_LABEL, ATTR_CALLBACK_DATA, src);
    SetCtrlAttribute (panel, SOURCE_COEFF, ATTR_CALLBACK_DATA, src);
    SetCtrlAttribute (panel, SOURCE_ACQUIRE, ATTR_CALLBACK_DATA, src);
    SetCtrlAttribute (panel, SOURCE_NOTE, ATTR_CALLBACK_DATA, src);
    SetCtrlAttribute (panel, SOURCE_INLIST, ATTR_CALLBACK_DATA, src);
    SetCtrlAttribute (panel, SOURCE_NSEGMENTS, ATTR_CALLBACK_DATA, src);
    SetCtrlAttribute (panel, SOURCE_SEGSEL, ATTR_CALLBACK_DATA, src);
	SetCtrlAttribute (panel, SOURCE_PRECISION, ATTR_CALLBACK_DATA, src);
	
	SetMenuBarAttribute (src->menu, SOURCEMENU_SCAN_STEP_SAVE, ATTR_CALLBACK_DATA, src);
	SetMenuBarAttribute (src->menu, SOURCEMENU_SCAN_STEP_LOAD, ATTR_CALLBACK_DATA, src);
	SetMenuBarAttribute (src->menu, SOURCEMENU_SRC_SEG_PLOT, ATTR_CALLBACK_DATA, src);

    segment_Update (panel, src, src->segments[0]);
    sourceL.src = src;
}

void source_InitPanel (sourcePtr src)
{
    char title[256];
    int panel, i;
	
	src->sourceOn = 1;
    panel = src->panel? src->panel : LoadPanel (utilG.p, "sourceu.uir", SOURCE);
	src->panel = panel;
    
	src->menu = GetPanelMenuBar (src->panel);
    SetPanelPos (panel, VAL_AUTO_CENTER, VAL_AUTO_CENTER);
	SetInputMode (panel, GENSRC_ACQUIRE, !util_TakingData());

    util_InitClose (panel, SOURCE_CLOSE, FALSE);
    Fmt (title, "Source Control: %s", src->acqchan->channel->label);
    SetPanelAttribute (panel, ATTR_TITLE, title);
	
	if(src->freq)
	{
		SetCtrlAttribute (panel, SOURCE_FUNCTION, ATTR_CTRL_MODE,
						  VAL_INDICATOR);
		SetCtrlAttribute(panel, SOURCE_START, ATTR_MIN_VALUE, src->min);
		SetCtrlAttribute(panel, SOURCE_START, ATTR_MAX_VALUE, src->max);
		SetCtrlAttribute(panel, SOURCE_STOP, ATTR_MIN_VALUE, src->min);
		SetCtrlAttribute(panel, SOURCE_STOP, ATTR_MAX_VALUE, src->max);
		SetCtrlVal(panel, SOURCE_START, src->min);
		SetCtrlVal(panel, SOURCE_STOP, src->min);
		SetCtrlAttribute(panel, SOURCE_LOGSC, ATTR_DIMMED, 0); 
	}
	
	source_UpdatePanel (panel, src);

    devPanel_Add (panel, src, source_UpdateReading);
    DisplayPanel (panel);
	src->sourceOn = 0;
}

int source_EndofScan (sourcePtr src)
{
    if ((src->seg + 1) < src->nSegments) return FALSE;
    else return TRUE;
}

void source_CalcBiasLevel (sourcePtr src)
{
    segmentPtr seg;
    seg = src->segments[src->seg];
    if (seg->output == OUTPUT_DC) src->biaslevel = seg->start;
	else if (seg->output == OUTPUT_EXTERN) src->biaslevel = seg->valArray[seg->pt];
	else src->biaslevel = seg->start + (seg->step * seg->pt);
	if (src->freq && src->segments[src->seg]->log)
	{
		src->biaslevel = exp(log(seg->start) + seg->pt * (log(seg->stop / seg->start))/(seg->points - 1));
	}
}

double source_TotalTime(sourcePtr src)
{
    int seg;

    src->time = 0;
    for (seg = 0; seg < src->nSegments; seg++)
        src->time+=src->segments[seg]->time;
    return src->time;
}

int source_TotalPoints (sourcePtr src)
{
    int seg;

    src->points = 0;
    for (seg = 0; seg < src->nSegments; seg++)
        src->points+=src->segments[seg]->points;
    return src->points;
}

void source_UpdateReading (int panel, void *src)
{
    sourcePtr my_src = src;
    int control;
    control = GetActiveCtrl (panel);
    if ((control != SOURCE_OUTPUT) || util_TakingData())
        SetCtrlVal (panel, SOURCE_OUTPUT, my_src->acqchan->reading);
}

void source_Load (void *dev, sourcePtr src)
{
    char info[256];
    int seg, func, inlist, acquire, bytes, i;								  

    acqchan_Load (dev, src->acqchan);
    ScanFile (fileHandle.analysis, "%s>In list  : %i%s[w1d]", &inlist);

    for (seg = 0; seg < src->nSegments; seg++)
    { free (src->segments[seg]); src->segments[seg] = NULL;}

    ScanFile (fileHandle.analysis, "%s>Segments : %i%s[w1d]", &src->nSegments);
	ScanFile (fileHandle.analysis, "%s>Frequency: %i%s[w1d]", &src->freq);
    for (seg = 0;seg < src->nSegments; seg++) {
        src->segments[seg] = segment_Create(seg);
		src->segments[seg]->source = src;
        ScanFile (fileHandle.analysis, "%s>%s[xt58]%i,%f[x]", src->segments[seg]->label,
            &func, &src->segments[seg]->des_start);

        if (func == 0) src->segments[seg]->output = OUTPUT_DC;
        else if (func == 1) src->segments[seg]->output = OUTPUT_STEP;
        else if (func == 2) src->segments[seg]->output = OUTPUT_EXTERN;

        if (src->segments[seg]->output == OUTPUT_STEP)
       		ScanFile (fileHandle.analysis, "%s>%f, %i,", &src->segments[seg]->des_stop, &src->segments[seg]->points);
		else if (src->segments[seg]->output == OUTPUT_EXTERN)
		{
			ScanFile (fileHandle.analysis, "%s>%i: ", &src->segments[seg]->points);
			src->segments[seg]->valArray = malloc(sizeof(double) * src->segments[seg]->points);
			for (i = 0; i < src->segments[seg]->points; i++)
				ScanFile (fileHandle.analysis, "%s>%f ", &src->segments[seg]->valArray[i]);
		}
		ScanFile (fileHandle.analysis, "%s>%f, %i, %i, %f%s[w1d]",
                      &src->segments[seg]->delay,
					  &src->segments[seg]->log,
					  &src->segments[seg]->error.on,
					  &src->segments[seg]->error.val);
        segment_Start(src->segments[seg], 0);
		segment_Stop (src->segments[seg], 0);
		segment_Step (src->segments[seg], 0);
        segment_Rate (src->segments[seg]);
        segment_Time (src->segments[seg]);
    }

    source_TotalPoints(src);
    source_TotalTime(src);

    if (dev && (utilG.exp == EXP_SOURCE)) {
        if (!src->inlist) {
            src->inlist = inlist;
            if (src->inlist) sourcelist_AddSource (src);
        }
        else sourcelist_ReplaceSource (src);
    }
}

void source_Save (sourcePtr src)
{
    int seg, i;

    acqchan_Save (src->acqchan);
    FmtFile (fileHandle.analysis, "In list  : %i\n", src->inlist);
    FmtFile (fileHandle.analysis, "Segments : %i\n", src->nSegments);
	FmtFile (fileHandle.analysis, "Frequency: %i\n", src->freq);
    for (seg = 0;seg < src->nSegments; seg++) {
        FmtFile (fileHandle.analysis, "%s<%s: %i, %f[e2p5], ",
                 src->segments[seg]->label, src->segments[seg]->output, src->segments[seg]->des_start);
        if (src->segments[seg]->output == OUTPUT_STEP)
            FmtFile (fileHandle.analysis, "%s<%f[e2p5], %i,", src->segments[seg]->des_stop, src->segments[seg]->points);
		if (src->segments[seg]->output == OUTPUT_EXTERN)
		{
			FmtFile (fileHandle.analysis, "%i: ", src->segments[seg]->points);
			for( i = 0; i < src->segments[seg]->points; i++)
				FmtFile (fileHandle.analysis, "%f ", src->segments[seg]->valArray[i]);
		}
        FmtFile (fileHandle.analysis, "%s<%f, %i, %i, %f\n", src->segments[seg]->delay,
				src->segments[seg]->log, src->segments[seg]->error.on, src->segments[seg]->error.val);
    }
}

void source_Remove (sourcePtr src)
{
    int i;
    sourcelist_RemoveSource(src);
	acqchan_Remove (src->acqchan);
    for (i = 0; i < src->nSegments; i++)
        free (src->segments[i]);
    free (src);
}

rangePtr	range_Create  (double maxVal, double minVal, double resolution)
{
	rangePtr range = malloc(sizeof(rangeType));
	range->maxVal = maxVal;
	range->minVal = minVal;
	range->resolution = resolution;
	return range;
}

sourcePtr   source_Create (char *label, void *dev, SetLevelPtr SetLevel, GetReadingPtr GetReading)
{
    sourcePtr src;
	
    src = malloc (sizeof(sourceType));
    src->ranges.range = malloc(sizeof(rangePtr));
	src->ranges.nullrange = calloc(2, sizeof(rangeType));
	if (!src) {util_OutofMemory("Create Source Error"); return NULL;}

    src->acqchan = acqchan_Create (label, dev, GetReading);
    src->acqchan->conversion = -1;
    src->acqchan->reading = 0;
    src->nSegments = 1;
    src->inlist = FALSE;
    src->time = 0;
    src->points = 2;
    src->panel = 0;
	src->segments[0] = segment_Create(0);
    src->segments[0]->source = src;
	src->seg = 0;
    src->SetLevel = SetLevel;
	src->freq = 0;
	src->sourceOn = 0;
	src->panel = 0;
	src->menuitem_id = 0;
    src->ranges.autoscale = 0;
	src->ranges.range[0] = range_Create(99999999999, -99999999999, 0);
	src->ranges.nullrange[0] = range_Create(99999999999, -99999999999, 0);
	src->ranges.temprange = malloc(sizeof(rangePtr));
	src->messagePanel = 0;
	src->precAssist = 0;
	return src;
}

/****************************************************************************/

int  SegmentControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    segmentPtr seg;
    int i;
    double r;

    switch (control) {
        case SOURCE_RATE:
            if (event == EVENT_VAL_CHANGED) {
                sourcePtr src;
				seg = callbackData;
				src = seg->source;
                GetCtrlVal (panel, control, &r);
                if (r == 0.0) SetCtrlVal (panel, control, segment_Rate (seg));
                else {
                    seg->rate = r;
                    if(!src->ranges.autoscale)
					{
						SetCtrlVal (panel, SOURCE_STOP, segment_Stop (seg, 0));
						SetCtrlVal (panel, SOURCE_STEP, seg->step);
						//segment_Notify (src, 0);
					}
					else
						segment_Autorange (seg, src->ranges.range, panel, control);
					SetCtrlVal (panel, SOURCE_DELAY, segment_Delay(seg));
                    SetCtrlVal (panel, SOURCE_TIME, segment_Time (seg));
                    if (seg->output != OUTPUT_DC)
                        SetCtrlVal (panel, SOURCE_RATE, segment_Rate (seg));
                    SetCtrlVal (panel, SOURCE_TIME_TOTAL, source_TotalTime(sourceL.src));
                    exp_UpdateSourcePanel();
                }
            }
			if(event == EVENT_RIGHT_CLICK)
			{
				sourcePtr src;
				seg = callbackData;
				src = seg->source;
				segment_Notify (src, 0);
			}
            break;
        case SOURCE_DELAY:
            if (event == EVENT_VAL_CHANGED) {
                sourcePtr src;
				seg = callbackData;
				src = seg->source;
                GetCtrlVal (panel, control, &seg->delay);
                SetCtrlVal (panel, SOURCE_TIME, segment_Time (seg));
                if (seg->output != OUTPUT_DC)
                    SetCtrlVal (panel, SOURCE_RATE, segment_Rate (seg));
                SetCtrlVal (panel, SOURCE_TIME_TOTAL, source_TotalTime(sourceL.src));
                exp_UpdateSourcePanel();
            }
            break;
        case SOURCE_POINTS:
            if (event == EVENT_VAL_CHANGED) {
                sourcePtr src;
				seg = callbackData;
				src = seg->source;
                GetCtrlVal (panel, control, &seg->points);
                if(!src->ranges.autoscale)
				{
					SetCtrlVal (panel, SOURCE_STOP, segment_Stop (seg, 0));
					SetCtrlVal (panel, SOURCE_STEP, seg->step);
					//segment_Notify (src, 0);
				}
                else
					segment_Autorange (seg, src->ranges.range, panel, control);
				SetCtrlVal (panel, SOURCE_TIME, segment_Time (seg));
				if (seg->output != OUTPUT_DC)
                    SetCtrlVal (panel, SOURCE_RATE, segment_Rate (seg));
                SetCtrlVal (panel, SOURCE_PTS_TOTAL, source_TotalPoints (sourceL.src));
                SetCtrlVal (panel, SOURCE_TIME_TOTAL, source_TotalTime(sourceL.src));
                exp_UpdateSourcePanel();
            }
            if(event == EVENT_RIGHT_CLICK)
			{
				sourcePtr src;
				seg = callbackData;
				src = seg->source;
				segment_Notify (src, 0);
			}
            break;
        case SOURCE_STEP:
            if (event == EVENT_VAL_CHANGED) {
                sourcePtr src;
				seg = callbackData;
				src = seg->source;
                GetCtrlVal (panel, control, &r);
                if (r != 0.0) {
                    if ((seg->stop - seg->start) < 0) seg->step = -fabs(r);
                    else seg->step = fabs(r);
					
                	if(!src->ranges.autoscale)
					{
						SetCtrlVal (panel, SOURCE_POINTS, seg->points);
                    	SetCtrlVal (panel, control, segment_Step(seg, 0));
						SetCtrlVal (panel, SOURCE_STOP, segment_Stop (seg, 0));
						//segment_Notify (src, 0);
					}
                    else
						segment_Autorange (seg, src->ranges.range, panel, control);
					SetCtrlVal (panel, SOURCE_TIME, segment_Time (seg));
                    if (seg->output != OUTPUT_DC)
                        SetCtrlVal (panel, SOURCE_RATE, segment_Rate (seg));
                    SetCtrlVal (panel, SOURCE_PTS_TOTAL, source_TotalPoints(sourceL.src));
                    SetCtrlVal (panel, SOURCE_TIME_TOTAL, source_TotalTime(sourceL.src));
                    exp_UpdateSourcePanel();
                }
            }
			if(event == EVENT_RIGHT_CLICK)
			{
				sourcePtr src;
				seg = callbackData;
				src = seg->source;
				segment_Notify (src, 0);
			}
            break;
		case SOURCE_STOP:
            if (event == EVENT_VAL_CHANGED) {
                sourcePtr src;
				seg = callbackData;
				src = seg->source;
                GetCtrlVal (panel, control, &seg->stop);
                GetCtrlVal (panel, control, &seg->des_stop);
				if (seg->output != OUTPUT_DC)
                    SetCtrlVal (panel, SOURCE_RATE, segment_Rate (seg));
				if(!src->ranges.autoscale)
				{
					SetCtrlVal (panel, control, segment_Stop (seg, 0));
					SetCtrlVal (panel, SOURCE_STEP, seg->step);
					//segment_Notify (src, 0);
				}
				else
					segment_Autorange (seg, src->ranges.range, panel, control);
				
            }
            if(event == EVENT_RIGHT_CLICK)
			{
				sourcePtr src;
				seg = callbackData;
				src = seg->source;
				segment_Notify (src, 0);
			}
            break;
        case SOURCE_START:
            if (event == EVENT_VAL_CHANGED) {
                sourcePtr src;
				seg = callbackData;
				src = seg->source;
                GetCtrlVal (panel, control, &seg->start);
				GetCtrlVal (panel, control, &seg->des_start);
				if (seg->output != OUTPUT_DC)
                    SetCtrlVal (panel, SOURCE_RATE, segment_Rate (seg));
				if(!src->ranges.autoscale)
				{
					SetCtrlVal (panel, control, segment_Start(seg, 0));
					SetCtrlVal (panel, SOURCE_STOP, segment_Stop (seg, 0));
					SetCtrlVal (panel, SOURCE_STEP, seg->step);
					//segment_Notify (src, 0);
				}
				else
					segment_Autorange (seg, src->ranges.range, panel, control);
            }
            if(event == EVENT_RIGHT_CLICK)
			{
				sourcePtr src;
				seg = callbackData;
				src = seg->source;
				segment_Notify (src, 0);
			}
            break;
        case SOURCE_FUNCTION:
            if (event == EVENT_COMMIT) {
                sourcePtr src;
				seg = callbackData;
				src = seg->source;
                GetCtrlIndex (panel, control, &seg->output);
                SetInputMode (panel, SOURCE_STOP, (seg->output != OUTPUT_DC));
                SetInputMode (panel, SOURCE_STEP, ((seg->output != OUTPUT_DC) && !util_TakingData()));
                SetInputMode (panel, SOURCE_RATE, (seg->output != OUTPUT_DC));
				if(seg->output == OUTPUT_EXTERN)
					if(LoadSourceCallback(panel, 0, EVENT_COMMIT, seg, SOURCEMENU_SCAN_EXTERN_LOAD, 0))
					{
						seg->output = OUTPUT_EXTERN;
						SetCtrlVal (panel, SOURCE_PTS_TOTAL, source_TotalPoints (sourceL.src));
					}
					else
						seg->output = OUTPUT_STEP;
				segment_Update(panel, src, seg);
            }
            break;
        case SOURCE_SEGMENT:
            if (event == EVENT_COMMIT) {
                sourcePtr src;
				seg = callbackData;
				src = seg->source;
                GetCtrlVal (panel, control, seg->label);
                GetCtrlIndex (panel, SOURCE_SEGSEL, &src->seg);
                ReplaceListItem (panel, SOURCE_SEGSEL, src->seg, seg->label, src->seg);
            }
            break;
		case SOURCE_LOGSC:
			if(event == EVENT_COMMIT) {
				seg = callbackData;
				GetCtrlVal (panel, control, &seg->log);
				if (seg->log)
				{
					GetCtrlVal (panel, SOURCE_START, &seg->start);
					GetCtrlVal (panel, SOURCE_STOP, &seg->stop);
				}
				SetCtrlAttribute (panel, SOURCE_STEP, ATTR_DIMMED, seg->log);
				SetCtrlAttribute (panel, SOURCE_RATE, ATTR_DIMMED, seg->log);
			}
			break;
		case SOURCE_ERROR_ON:
			if(event == EVENT_COMMIT)
			{
				seg = callbackData;
				GetCtrlVal(panel, control, &seg->error.on);
				SetCtrlAttribute(panel, SOURCE_ERROR, ATTR_DIMMED, !seg->error.on);
			}
			break;
		case SOURCE_ERROR:
			if(event == EVENT_COMMIT)
			{
				seg = callbackData;
				GetCtrlVal(panel, control, &seg->error.val);
			}
			break;
		case SOURCE_OPTIMIZE:
			if(event == EVENT_COMMIT)
			{
				seg = callbackData;
				GetCtrlVal (panel, control, &seg->optimize);
			}
			break;
    }
	if(event == EVENT_COMMIT)
	{
		sourcePtr src;
		seg = callbackData;
		src = seg->source;
		segment_Update(panel, src, seg);
	}
	return 0;
}
static void segment_Notify (sourcePtr src, int range)
{
	char msg[260];
	int pwidth, width, textX, sLeft, sTop;
	src->messagePanel = src->messagePanel? src->messagePanel : LoadPanel(utilG.p, "sourceu.uir", POPUP_MSG);
				
	Fmt(msg, "the resolution at the current range is %f.", src->ranges.range[range]->resolution);
	GetPanelAttribute(src->panel, ATTR_TOP, &sTop);
	GetPanelAttribute(src->panel, ATTR_LEFT, &sLeft);
	SetCtrlVal(src->messagePanel, POPUP_MSG_TEXT, msg);
	GetCtrlAttribute(src->messagePanel, POPUP_MSG_TEXT, ATTR_WIDTH, &width);
	GetCtrlAttribute(src->messagePanel, POPUP_MSG_TEXT, ATTR_LEFT, &textX);
	pwidth = (width + textX * 2);
	SetPanelAttribute(src->messagePanel, ATTR_WIDTH, pwidth);
		
	SetPanelPos (src->messagePanel, sTop + 300, sLeft + 300);
	DisplayPanel(src->messagePanel);
}

static void segment_Update (int panel, sourcePtr src, segmentPtr seg)
{
    SetInputMode (panel, SOURCE_FUNCTION, !util_TakingData());
    SetInputMode (panel, SOURCE_STOP, (seg->output != OUTPUT_DC));
    SetInputMode (panel, SOURCE_STEP, ((seg->output != OUTPUT_DC) && !util_TakingData()));
    SetInputMode (panel, SOURCE_POINTS, !util_TakingData());
    SetInputMode (panel, SOURCE_RATE, (seg->output != OUTPUT_DC));

    SetCtrlVal (panel, SOURCE_OPTIMIZE, seg->optimize);
	SetCtrlVal (panel, SOURCE_SEGMENT, seg->label);
    SetCtrlVal (panel, SOURCE_FUNCTION, seg->output);
    SetCtrlVal (panel, SOURCE_START, seg->start);
    SetCtrlVal (panel, SOURCE_STOP, seg->stop);
    SetCtrlVal (panel, SOURCE_STEP, seg->step);
    SetCtrlVal (panel, SOURCE_POINTS, seg->points);
    SetCtrlVal (panel, SOURCE_DELAY, seg->delay);
    SetCtrlVal (panel, SOURCE_RATE, seg->rate);
    SetCtrlVal (panel, SOURCE_TIME, seg->time);
	SetCtrlVal (panel, SOURCE_LOGSC, seg->log);
	SetCtrlVal (panel, SOURCE_ERROR_ON, seg->error.on);
	SetCtrlVal (panel, SOURCE_ERROR, seg->error.val);
	
    SetCtrlAttribute (panel, SOURCE_ERROR, ATTR_DIMMED, (!seg->error.on || (seg->output == OUTPUT_EXTERN)));
	SetCtrlAttribute (panel, SOURCE_STEP, ATTR_DIMMED, seg->log);
	SetCtrlAttribute (panel, SOURCE_RATE, ATTR_DIMMED, seg->log);
	
	SetMenuBarAttribute (src->menu, SOURCEMENU_SCAN_EXTERN, ATTR_DIMMED, !(seg->output == OUTPUT_EXTERN));
	
    SetCtrlAttribute (panel, SOURCE_START, ATTR_DIMMED, (seg->output == OUTPUT_EXTERN));
    SetCtrlAttribute (panel, SOURCE_STOP, ATTR_DIMMED, (seg->output == OUTPUT_EXTERN));
    SetCtrlAttribute (panel, SOURCE_STEP, ATTR_DIMMED, (seg->output == OUTPUT_EXTERN));
    SetCtrlAttribute (panel, SOURCE_POINTS, ATTR_DIMMED, (seg->output == OUTPUT_EXTERN));
    SetCtrlAttribute (panel, SOURCE_TIME, ATTR_DIMMED, (seg->output == OUTPUT_EXTERN));
    SetCtrlAttribute (panel, SOURCE_RATE, ATTR_DIMMED, (seg->output == OUTPUT_EXTERN));
	SetCtrlAttribute (panel, SOURCE_LOGSC, ATTR_DIMMED, (seg->output == OUTPUT_EXTERN));
	SetCtrlAttribute (panel, SOURCE_ERROR_ON, ATTR_DIMMED, (seg->output == OUTPUT_EXTERN));
	
	SetCtrlAttribute (panel, SOURCE_OPTIMIZE, ATTR_CALLBACK_DATA, seg);
	SetCtrlAttribute (panel, SOURCE_SEGMENT, ATTR_CALLBACK_DATA, seg);
    SetCtrlAttribute (panel, SOURCE_FUNCTION, ATTR_CALLBACK_DATA, seg);
    SetCtrlAttribute (panel, SOURCE_START, ATTR_CALLBACK_DATA, seg);
    SetCtrlAttribute (panel, SOURCE_STOP, ATTR_CALLBACK_DATA, seg);
    SetCtrlAttribute (panel, SOURCE_STEP, ATTR_CALLBACK_DATA, seg);
    SetCtrlAttribute (panel, SOURCE_POINTS, ATTR_CALLBACK_DATA, seg);
    SetCtrlAttribute (panel, SOURCE_DELAY, ATTR_CALLBACK_DATA, seg);
    SetCtrlAttribute (panel, SOURCE_RATE, ATTR_CALLBACK_DATA, seg);
	SetCtrlAttribute (panel, SOURCE_LOGSC, ATTR_CALLBACK_DATA, seg);
	SetCtrlAttribute (panel, SOURCE_ERROR, ATTR_CALLBACK_DATA, seg);
	SetCtrlAttribute (panel, SOURCE_ERROR_ON, ATTR_CALLBACK_DATA, seg);
	
	SetMenuBarAttribute (src->menu, SOURCEMENU_SCAN_EXTERN_LOAD, ATTR_CALLBACK_DATA, seg);
}

//range[] has to be declared in ascending order and the last element has to be NULL
static void	segment_Autorange (segmentPtr seg, rangePtr *range, int panel, int control)
{
	int i = 0;
	struct {int startInRange, stopInRange;}inRange;
	sourcePtr src = seg->source;
	rangePtr temp = NULL;
	inRange.startInRange = 0;
	inRange.stopInRange = 0;
	while(range[i])
	{
		temp = range[i];
		if((seg->des_start >= range[i]->minVal && seg->des_start <= range[i]->maxVal) && !inRange.startInRange)
			inRange.startInRange = i+1;
		if((seg->des_stop  >= range[i]->minVal && seg->des_stop  <= range[i]->maxVal) && !inRange.stopInRange)
			inRange.stopInRange =  i+1;
		i++;
	}
	i--;
	if(inRange.startInRange < range[0]->minVal)
	{
		segment_Start (seg, 0);
		inRange.startInRange = 1;
	}
	if(inRange.stopInRange < range[0]->minVal)
	{
		segment_Stop (seg, 0);
		inRange.stopInRange = 1;
	}				
	if(!inRange.stopInRange)
	{
		segment_Stop(seg, i);
		inRange.stopInRange = i + 1;
	}
	
	if(!inRange.startInRange)
	{
		segment_Start(seg, i);
		inRange.startInRange = i + 1;
	}
	i = inRange.startInRange > inRange.stopInRange ? inRange.startInRange : inRange.stopInRange;
	if(control == SOURCE_STEP)
	{
		segment_Points(seg);
	}
	segment_Stop(seg, i-1);
	//segment_Notify (src, i-1);
	segment_Update(panel, src, seg);
}

static int segment_Endof (segmentPtr seg)
{
	sourcePtr src = seg->source;
    if (((seg->pt + 1) < seg->points) || (seg->error.on && (fabs(src->acqchan->reading - src->biaslevel) < seg->error.val))) return FALSE;
    else return TRUE;
}
static double segment_Start(segmentPtr seg, int range)
{
	double start;
	sourcePtr src = seg->source;
	seg->des_start = seg->des_start > src->ranges.range[range]->minVal? seg->des_start : src->ranges.range[range]->minVal;
	seg->des_start = seg->des_start < src->ranges.range[range]->maxVal? seg->des_start : src->ranges.range[range]->maxVal;
	if(src->ranges.range[range]->resolution)
	{
		seg->start = (int)(seg->des_start/src->ranges.range[range]->resolution) * src->ranges.range[range]->resolution;
		start = seg->start;
		while (fabs(seg->des_start - start) > fabs(seg->des_start - (seg->start + src->ranges.range[range]->resolution)))
			seg->start += src->ranges.range[range]->resolution;
		while (fabs(seg->des_start - start) > fabs(seg->des_start - (seg->start - src->ranges.range[range]->resolution)))
			seg->start -= src->ranges.range[range]->resolution;
		while (seg->start > src->ranges.range[range]->maxVal)
			seg->start -= src->ranges.range[range]->resolution;
		while (seg->start < src->ranges.range[range]->minVal)
			seg->start += src->ranges.range[range]->resolution;
	}
	else seg->start = seg->des_start;
	return seg->start;
}

static double segment_Stop (segmentPtr seg, int range)
{
	sourcePtr src = seg->source;
	seg->des_stop = seg->des_stop > src->ranges.range[range]->minVal? seg->des_stop : src->ranges.range[range]->minVal;
	seg->des_stop = seg->des_stop < src->ranges.range[range]->maxVal? seg->des_stop : src->ranges.range[range]->maxVal;
	seg->stop = seg->start + ((double)(seg->points-1) * segment_Step(seg, range));
	return seg->stop;
}

void segment_ErrorCalcPtInc(double start, double des_start, double stop, double des_stop, double step, double res, int points, double returnArr[2])
{
	stop = start + ((double)(points-1) * step); 
	while(fabs(stop - start) < fabs (des_stop - des_start) && step)
	{
		points++;
		stop = start + ((double)(points-1) * step);
	}
	returnArr[0] = stop;
	returnArr[1] = points;
}

void segment_ErrorCalcStepCorrecion(double start, double des_start, double stop, double des_stop, double step, double res, int points, double returnArr[2])
{
	stop = start + ((double)(points-1) * step); 
	while(fabs(stop - start) < fabs (des_stop - des_start))
	{
		if(stop > start)
			step += res;
		else
			step -= res;//
		stop = start + ((double)(points-1) * step);
	}
	returnArr[0] = stop;
	returnArr[1] = step;
}   
	
static double segment_Step (segmentPtr seg, int range)
{
    sourcePtr src = seg->source;
	double errorCheckPt[2], errorCheckInc[2], origError;
	seg->step = (seg->des_stop - seg->des_start)/(seg->points - 1);
	if(src->ranges.range[range]->resolution)
	{
		seg->step = (int)(seg->step/src->ranges.range[range]->resolution);
		seg->step = (seg->step - (int)seg->step) > .5? (int)seg->step + 1: (int)seg->step;
		seg->step *= (double)src->ranges.range[range]->resolution;
		if(seg->optimize)
		{
			segment_ErrorCalcPtInc (seg->start, seg->des_start, seg->stop, seg->des_stop, seg->step, src->ranges.range[range]->resolution, seg->points, errorCheckPt);
			segment_ErrorCalcStepCorrecion (seg->start,seg->des_start, seg->stop, seg->des_stop, seg->step, src->ranges.range[range]->resolution, seg->points, errorCheckInc);
			origError = fabs(seg->des_stop - (seg->start + (double)(seg->points - 1) * seg->step));
			if((fabs(seg->des_stop - errorCheckInc[0]) > fabs(seg->des_stop - errorCheckPt[0])) && (fabs(seg->des_stop - errorCheckPt[0]) < origError))
				seg->points = errorCheckPt[1];
			else if ((fabs(seg->des_stop - errorCheckInc[0]) < fabs(seg->des_stop - errorCheckPt[0])) && (fabs(seg->des_stop - errorCheckInc[0]) < origError))
				seg->step = errorCheckInc[1];
		}
	}
	return seg->step;
}

static double segment_Time (segmentPtr seg)
{
    seg->time = (double)seg->points * seg->delay/60; /* min */
    return seg->time;
}

static double segment_Rate (segmentPtr seg)
{
    seg->rate = fabs (seg->stop-seg->start)/segment_Time (seg);
    return seg->rate;
}

static double segment_Delay (segmentPtr seg)
{
    seg->delay = fabs (seg->stop - seg->start)/seg->rate*60/seg->points; /* sec */
    return seg->delay;
}

static int segment_Points (segmentPtr seg)
{
    seg->points =
        RoundRealToNearestInteger (fabs((seg->stop-seg->start)/seg->step))+1;
    return seg->points;
}

static segmentPtr segment_Create(int i)
{
    segmentPtr newseg;

    newseg = malloc (sizeof(segmentType));
    if (newseg)
    {
        newseg->start = 0; 
		newseg->des_start = 0;
		newseg->stop = 0; 
		newseg->des_stop = 0;
		newseg->step = 0;
        newseg->time = 0;
        newseg->rate = 0;
        newseg->delay = 0.0;
        newseg->output = OUTPUT_STEP;
        newseg->points = 2;
		newseg->log = 0;
		newseg->error.on = 0;
		newseg->error.val = 10;
		newseg->valArray = NULL;
		newseg->optimize = 0;
		Fmt (newseg->label, "segment%i", i);
        return newseg;
    }
    else return NULL;
}

/****************************************************************************/

int  GenSourceControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    sourcePtr src;
    char title[256];

    src = callbackData;
    switch (control) {
        case GENSRC_NOTE:
            AcqDataNoteCallback (panel, control, event, src->acqchan, eventData1, eventData2);
            break;
        case GENSRC_CLOSE:
            if (event == EVENT_COMMIT) {
                
                DiscardPanel (panel);
            }
            break;
        case GENSRC_OUTPUT:
            if (event == EVENT_VAL_CHANGED) {
                GetCtrlVal (panel, control, &src->biaslevel);
                if (src->SetLevel) src->SetLevel(src);
                if (src->acqchan->GetReading) src->acqchan->GetReading(src->acqchan);
                SetCtrlVal (panel, control, src->acqchan->reading);
            }
            break;
        case GENSRC_LABEL:
            if (event == EVENT_COMMIT) {
                GetCtrlVal (panel, control, src->acqchan->channel->label);
                Fmt (title, "Source Control: %s", src->acqchan->channel->label);
                SetPanelAttribute (panel, ATTR_TITLE, title);
                acqchanlist_ReplaceChannel (src->acqchan);
                acqchan_UpdateReadingPanel (src->acqchan);
            }
            break;
        case GENSRC_COEFF:
            if (event == EVENT_COMMIT) {
                GetCtrlVal (panel, control, &src->acqchan->coeff);
                acqchan_UpdateReadingPanel (src->acqchan);
            }
            break;
        case GENSRC_ACQUIRE:
            if (event == EVENT_VAL_CHANGED) {
                GetCtrlVal (panel, control, &src->acqchan->acquire);
                if (src->acqchan->acquire) acqchanlist_AddChannel(src->acqchan);
                    else acqchanlist_RemoveChannel(src->acqchan);
            }
            break;
    }
    return 0;
}

void gensrc_InitPanel (sourcePtr src)
{
    char title[256];
    int panel;

    src->sourceOn = 1;
	panel = LoadPanel (0, "sourceu.uir", GENSRC);
    
    SetPanelPos (panel, VAL_AUTO_CENTER, VAL_AUTO_CENTER);

    util_InitClose (panel, GENSRC_CLOSE, FALSE);
    Fmt (title, "Source Control: %s", src->acqchan->channel->label);
    SetPanelAttribute (panel, ATTR_TITLE, title);
	
    if (src->acqchan->GetReading) src->acqchan->GetReading(src->acqchan);
    SetCtrlVal (panel, GENSRC_OUTPUT, src->acqchan->reading);

    SetCtrlVal (panel, GENSRC_LABEL, src->acqchan->channel->label);
    SetCtrlVal (panel, GENSRC_COEFF, src->acqchan->coeff);
    SetCtrlVal (panel, GENSRC_ACQUIRE, src->acqchan->acquire);
    SetCtrlVal (panel, GENSRC_NOTE, src->acqchan->note);

    SetCtrlAttribute (panel, GENSRC_OUTPUT, ATTR_MIN_VALUE, src->min);
    SetCtrlAttribute (panel, GENSRC_OUTPUT, ATTR_MAX_VALUE, src->max);
    SetCtrlAttribute (panel, GENSRC_OUTPUT, ATTR_CALLBACK_DATA, src);
    SetCtrlAttribute (panel, GENSRC_LABEL, ATTR_CALLBACK_DATA, src);
    SetCtrlAttribute (panel, GENSRC_COEFF, ATTR_CALLBACK_DATA, src);
    SetCtrlAttribute (panel, GENSRC_ACQUIRE, ATTR_CALLBACK_DATA, src);
    SetCtrlAttribute (panel, GENSRC_NOTE, ATTR_CALLBACK_DATA, src);
    InstallPopup (panel);
	src->sourceOn = 0;
}
int AcqCurvePanelCallback (int panel, int event, void *callbackData, int eventData1, int eventData2)
{
	if ((event == EVENT_RIGHT_DOUBLE_CLICK) || ((event == EVENT_KEYPRESS) && (eventData1 == VAL_ESC_VKEY)))
	{
		HidePanel(panel);
		updateGraphSource();
	}	
	return 0;
}
