typedef  enum {CURSOR_SNAP, CURSOR_PAN, CURSOR_XY, CURSOR_MAGNIFY, CURSOR_NONE} cursorType;

typedef struct
{
    char label[100];
    int autoscale;
    double min, max;
    struct {int attr, val;} logscale, divisions, grid, conversion;
}   axisType;

typedef struct
{
    char title[260];
    curvelistType curves;
    axisType x, y;
    cursorType cursor;
    int p, snap, textHandle, axisP;
    acqcurveType acqcurve;
}   graphType;

typedef graphType *graphPtr;

extern struct graphGStruct
{
    int p;
    listType graphs;
}   graphG;

extern void graph_ReplotCurvesWithConv(graphPtr graph);
extern void graph_Save (graphPtr graph);
extern void graphG_Init (void);
extern void graphG_Exit(void);
extern void graphlist_Copy (int panel, int control);
extern graphPtr graphlist_GetItem (int i);
extern void graphlist_PlotReadings (void);
extern void graphlist_RemoveReadings (void);
extern void graphlist_PlotCurves (void);
extern void graphlist_AutoSave (void);
