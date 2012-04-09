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
#include "graph.h"
#include "graphu.h"
#include "curveop.h"
#include "curveopu.h"

#define TRUE 1
#define FALSE 0

#define BUTTERWORTH 0
#define CHEBYSHEV 1
#define INV_CHEBYSHEV 2
#define ELLIPTIC 3
#define FILTER_NONE 4

#define TRI_WIN 1
#define HAN_WIN 2
#define HAM_WIN 3
#define BKMAN_WIN 4
#define KSR_WIN 5
#define BLK_HARRIS_WIN 6
#define COS_TAPERED_WIN 7
#define EX_BKMAN_WIN 8
#define EXP_WIN 9
#define FLAT_TOP_WIN 10
#define FORCE_WIN 11
#define GEN_COS_WIN 12
 int NoErr;
static struct curveopStruct
{
    double interval, freq;
    channelPtr x, y1, y2;
    char label[256], note[256], label2[256];
    void (*UpdatePanel) (curvePtr curve, int input);
    int (*DoBatch) (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
    graphPtr graph;
    struct {int w; WindowConst constants;} win;
    int amp;
}   curveop;

static void curveop_ClearChannel(channelPtr chan);
static void curveop_CalcFreq (curvePtr ccurve, int control);
static void curveop_CalcInterval (curvePtr curve, int control);
static void curveop_InitOpPanel (int panel, int graphs, int pts2, int right);

int  CurveOpSelectCurveCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CurveOpWindowCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);

void curveop_Init (void);

void InitOffsetCallback(int menubar, int menuItem, void *callbackData, int panel);

void UpdateIntegDiffPanel (curvePtr curve, int input);
void InitIntegDiffCallback(int menubar, int menuItem, void *callbackData, int panel);
int  IntegrateCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  DifferenceCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  BatchIntegrateCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  BatchDifferenceCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);

void InitReverseCallback(int menubar, int menuItem, void *callbackData, int panel);
int  ReverseCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);

void InitSortCallback(int menubar, int menuItem, void *callbackData, int panel);
int  AscendingSortCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  DescendingSortCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);

void Filter_Init_Mode (void);
void Filter_Init_Type (void);
void UpdateFilterPanel (curvePtr curve, int input);
void InitFilterCallback(int menubar, int menuItem, void *callbackData, int panel);
int  Filter_Compute (double fs, int pts, double *y, double *filter_y, char *note);
int  BatchFilterCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);

void UpdateSmoothPanel (curvePtr curve, int input);
void InitSmoothCallback(int menubar, int menuItem, void *callbackData, int panel);
int  curveop_SmoothingKernal (int pts, double variance, double *kernal);
int  curveop_Smooth (double *kernal, int k_pts, double *y, int c_pts,
                     int offset, int pts, double *smooth_y);
int  BatchSmoothCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);

void InitACDCCallback(int menubar, int menuItem, void *callbackData, int panel);
void UpdateACDCPanel (curvePtr curve, int input);

void UpdateAmpPhasePanel (curvePtr curve, int input);
void InitAmpPhaseCallback(int menubar, int menuItem, void *callbackData, int panel);
int  Window_Compute_IIR (double *y, int pts, double *win_y, char *note);
int  AmpSpecBatchCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);

void UpdatePowerSpecPanel (curvePtr curve, int input);
void InitPowerSpecCallback(int menubar, int menuItem, void *callbackData, int panel);
int  PowerSpecBatchCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);

void UpdateLinFitPanel (curvePtr curve, int input);
void InitLinFitCallback(int menubar, int menuItem, void *callbackData, int panel);

void UpdateExpFitPanel (curvePtr curve, int input);
void InitExpFitCallback(int menubar, int menuItem, void *callbackData, int panel);

void UpdatePolyFitPanel (curvePtr curve, int input);
void InitPolyFitCallback(int menubar, int menuItem, void *callbackData, int panel);

void UpdateInterpPanel (curvePtr curve, int input);
void InitInterpCallback(int menubar, int menuItem, void *callbackData, int panel);
int  curveop_PolyInterpCurve (double *x, double *y, int offset, int pts, int i_pts, double *i_x, double *i_y);
int  PolyInterpCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  BatchPolyInterpCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);

int  curveop_RatInterpCurve (double *x, double *y, int offset, int pts, int i_pts, double *i_x, double *i_y);
int  RatInterpCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  BatchRatInterpCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);

int  curveop_SplineInterpCurve (double *spline, double *x, double *y, int offset, int pts, int i_pts, double *i_x, double *i_y);
int  SplineInterpCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  BatchSplineInterpCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);

/*
main()
{
    utilG_Init();
    listG_Init();
    fileG_Init();
    channelG_Init();

    changen_Init();
    chanfunc_Init();
    chanops_Init();

    graphG_Init();
    curveop_Init();

    RunUserInterface();
}
*/

void curveop_Init (void)
{
    util_ChangeInitMessage ("Curve Menus...");

    curveG.menuBar = LoadMenuBar (0, "curveu.uir", CURVEMENUS);
    

    InstallMenuCallback (curveG.menuBar, CURVEMENUS_PROC_INTEG, InitIntegDiffCallback, 0);
    InstallMenuCallback (curveG.menuBar, CURVEMENUS_PROC_DIFF, InitIntegDiffCallback, 0);
    InstallMenuCallback (curveG.menuBar, CURVEMENUS_PROC_REVERSE, InitReverseCallback, 0);
    InstallMenuCallback (curveG.menuBar, CURVEMENUS_PROC_SORT_ASC, InitSortCallback, 0);
    InstallMenuCallback (curveG.menuBar, CURVEMENUS_PROC_SORT_DESC, InitSortCallback, 0);
    InstallMenuCallback (curveG.menuBar, CURVEMENUS_PROC_FILTER, InitFilterCallback, 0);
    InstallMenuCallback (curveG.menuBar, CURVEMENUS_PROC_SMOOTH, InitSmoothCallback, 0);
    InstallMenuCallback (curveG.menuBar, CURVEMENUS_PROC_OFFSET, InitOffsetCallback, 0);

    InstallMenuCallback (curveG.menuBar, CURVEMENUS_MEAS_ACDC, InitACDCCallback, 0);
    InstallMenuCallback (curveG.menuBar, CURVEMENUS_MEAS_AMPSPEC, InitAmpPhaseCallback, 0);
    InstallMenuCallback (curveG.menuBar, CURVEMENUS_MEAS_PWRSPEC, InitPowerSpecCallback, 0);

    InstallMenuCallback (curveG.menuBar, CURVEMENUS_FIT_LINEAR, InitLinFitCallback, 0);
    InstallMenuCallback (curveG.menuBar, CURVEMENUS_FIT_EXP, InitExpFitCallback, 0);
    InstallMenuCallback (curveG.menuBar, CURVEMENUS_FIT_POLY, InitPolyFitCallback, 0);

    InstallMenuCallback (curveG.menuBar, CURVEMENUS_INTERP_POLY, InitInterpCallback, 0);
    InstallMenuCallback (curveG.menuBar, CURVEMENUS_INTERP_RAT, InitInterpCallback, 0);
    InstallMenuCallback (curveG.menuBar, CURVEMENUS_INTERP_SPLINE, InitInterpCallback, 0);
}

int  BatchSplineInterpCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int c, checked, n, i, pts, offset;
    channelPtr x = NULL, y1 = NULL;
    curvePtr curve, newcurve;
    double *spline;

    if (event == EVENT_COMMIT)
    {
        GetCtrlVal (curveview.p2, INTERP_PTS, &pts);
        curveop_ClearChannel (x);
        x = channel_Create();
        channel_AllocMem (x, pts);

        GetNumListItems (panel, BATCH_CURVES, &n);
        for (c = 0; c < n; c++)
        {
            IsListItemChecked (panel, BATCH_CURVES, 0, &checked);
            DeleteListItem (panel, BATCH_CURVES, 0, 1);
            if (checked)
            {
                curve = curvelist_GetItem (curveG.curves->list, c);
                if (curve->x->readings[0] >= curve->x->readings[curve->curvepts - 1])
                    MessagePopup ("Spline Interpolation Message", "Only curves w/ ascending x values can be spline interpolated");
                else {

                curveop_ClearChannel (y1);

                offset = curve->offset + curveview.offset;
                Fmt (curveop.note, "");
                y1 = channel_Create();

                spline = (double *)calloc (curve->curvepts, sizeof(double));
                if (Spline (curve->x->readings, curve->y->readings, curve->curvepts,
                            curve->x->readings[0], curve->y->readings[curve->curvepts-1],
                            spline) != NoErr) {
                    MessagePopup ("Spline Interpolation Message", "Error during interpolation calculation");
                    break;
                }

                if (y1 && channel_AllocMem (y1, pts) &&
                    (curveop_SplineInterpCurve (spline, curve->x->readings, curve->y->readings,
                                          offset, curve->curvepts, pts,
                                          x->readings, y1->readings) == NoErr)) {
                    Fmt (y1->label, "Spline Interp(%s)", curve->y->label);
                    Fmt (y1->note, "%s\n%s\n%s\n", y1->label, curve->y->note, curveop.note);

                    newcurve = curve_Create();
                    if (newcurve) {
                        free (spline);
                        curve_CopyAttrs (curve, newcurve);
                        newcurve->curvepts = pts;
                        newcurve->pts = pts;
                        newcurve->offset = 0;

                        newcurve->x0.reading = curve->x0.reading;
                        Fmt (newcurve->x0.label,curve->x0.label);

                        newcurve->x = x;
                        if (!x->curves.nItems) channellist_AddChannel (x);
                        channel_AddCurve (newcurve->x, newcurve->attr.label, curveop.graph->title);

                        Fmt (newcurve->attr.label, "Spline Interp(%s)", curve->attr.label);
                        Fmt (newcurve->attr.note, "%s[a]<%s\n", curveop.note);

                        newcurve->y = y1;
                        if (!y1->curves.nItems) channellist_AddChannel (y1);
                        channel_AddCurve (newcurve->y, newcurve->attr.label, curveop.graph->title);

                        curvelist_AddCurve (&curveop.graph->curves, newcurve);
                        curve_Plot (curveop.graph->p, GRAPH_GRAPH, newcurve, NULL);
                    }
                    else
                    {
                        util_OutofMemory ("Batch Message");
                        break;
                    }
                }
                else
                {
                    util_OutofMemory ("Batch Message");
                    break;
                }
                }
            }
            else
            {
                GetNumCheckedItems (panel, BATCH_CURVES, &checked);
                if (!checked) break;
            }
        }
        curveop_ClearChannel (x);
        curveop_ClearChannel (y1);
        
        DiscardPanel(panel);
    }
    return 0;
}

int  SplineInterpCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int i, offset, n, interpCurve, pts;
    double x, y, err, *spline;
    curvePtr curve;
    char coeff[30];

    if (event == EVENT_COMMIT)
    {
        GetCtrlVal (panel, INTERP_TOGGLE, &interpCurve);
        GetCtrlIndex (curveview.p1, CURVEVIEW_CURVES, &i);
        curve = curvelist_GetItem (curveG.curves->list, i);
        SetCtrlAttribute (panel, INTERP_KEEP, ATTR_CALLBACK_DATA, curve);

        if (curve->x->readings[0] >= curve->x->readings[curve->curvepts - 1]) {
            MessagePopup ("Spline Interpolation Message", "Only curves w/ ascending x values can be spline interpolated");
            return 0;
        }

        offset = curve->offset + curveview.offset;
        Fmt (curveop.note, "");

        spline = (double *)calloc (curve->curvepts, sizeof(double));
        if (Spline (curve->x->readings, curve->y->readings, curve->curvepts,
                curve->x->readings[0], curve->y->readings[curve->curvepts-1],
                spline) != NoErr) {
            MessagePopup ("Spline Interpolation Message", "Error during interpolation calculation");
            return 0;
        }

        if (interpCurve) {
            GetCtrlVal (panel, INTERP_PTS, &pts);
            curveop_ClearChannel (curveop.x);
            curveop_ClearChannel (curveop.y1);

            curveop.x = channel_Create();
            curveop.y1 = channel_Create();

            if (curveop.x && curveop.y1 &&
                channel_AllocMem (curveop.x, pts) &&
                channel_AllocMem (curveop.y1, pts) &&
                (curveop_SplineInterpCurve (spline, curve->x->readings, curve->y->readings,
                                          offset, curve->curvepts, pts,
                                          curveop.x->readings, curveop.y1->readings) == NoErr)) {
                Fmt (curveop.x->label, "Interp(%s)", curve->x->label);
                Fmt (curveop.y1->label, "Spline Interp(%s)", curve->y->label);

                Fmt (curveop.x->note, "%s\n%s\n", curveop.x->label, curve->x->note);
                Fmt (curveop.y1->note, "%s\n%s\n%s\n", curveop.y1->label, curve->y->note, curveop.note);

                Fmt (curveop.label, "Spline Interp(%s)", curve->attr.label);
                UpdateInterpPanel (NULL, TRUE);
                if (curveview.h2)
                    DeleteGraphPlot (curveview.p1, CURVEVIEW_GRAPH, curveview.h2, VAL_DELAYED_DRAW);
                curveview.h2 = PlotXY (curveview.p1, CURVEVIEW_GRAPH,
                               curveop.x->readings, curveop.y1->readings,
                               pts, VAL_DOUBLE, VAL_DOUBLE,
                               VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, 1,
                               VAL_WHITE);
            }
            else util_OutofMemory ("Spline Interpolation Message");
        }
        else {
            GetCtrlVal (panel, INTERP_X, &x);
            if (SpInterp (curve->x->readings, curve->y->readings, spline, curve->curvepts, x, &y) == NoErr)
            {
                SetCtrlVal (panel, INTERP_Y, y);
                //SetCtrlVal (panel, INTERP_ERR, err);
                UpdateInterpPanel(NULL, TRUE);
            }
            else MessagePopup ("Spline Interpolation Message", "Error during interpolation calculation");
        }
        free (spline);
    }
    return 0;
}

int curveop_SplineInterpCurve (double *spline, double *x, double *y, int offset, int pts, int i_pts, double *i_x, double *i_y)
{
    int i, calc_err = FALSE;
    double dx, err;

    dx = (x[pts-1+offset]-x[offset])/(i_pts-1);
    for (i = 0; i < i_pts; i++) {
        i_x[i] = x[offset] + (i*dx);
        if (SpInterp (x, y, spline, pts, i_x[i], &i_y[i]) != NoErr) {
            MessagePopup ("Spline Interpolation Message", "Error during interpolation calculation");
            calc_err = TRUE;
            break;
        }
    }
    return calc_err;
}

int  BatchRatInterpCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int c, checked, n, i, pts, offset;
    channelPtr x = NULL, y1 = NULL;
    curvePtr curve, newcurve;

    if (event == EVENT_COMMIT)
    {
        GetCtrlVal (curveview.p2, INTERP_PTS, &pts);
        curveop_ClearChannel (x);
        x = channel_Create();
        channel_AllocMem (x, pts);

        GetNumListItems (panel, BATCH_CURVES, &n);
        for (c = 0; c < n; c++)
        {
            IsListItemChecked (panel, BATCH_CURVES, 0, &checked);
            DeleteListItem (panel, BATCH_CURVES, 0, 1);
            if (checked)
            {
                curve = curvelist_GetItem (curveG.curves->list, c);
                curveop_ClearChannel (y1);

                offset = curve->offset + curveview.offset;
                Fmt (curveop.note, "");
                y1 = channel_Create();

                if (y1 && channel_AllocMem (y1, pts) &&
                    (curveop_RatInterpCurve (curve->x->readings, curve->y->readings,
                                          offset, curve->curvepts, pts,
                                          x->readings, y1->readings) == NoErr)) {
                    Fmt (y1->label, "Rational Interp(%s)", curve->y->label);
                    Fmt (y1->note, "%s\n%s\n%s\n", y1->label, curve->y->note, curveop.note);

                    newcurve = curve_Create();
                    if (newcurve)
                    {
                        curve_CopyAttrs (curve, newcurve);
                        newcurve->curvepts = pts;
                        newcurve->pts = pts;
                        newcurve->offset = 0;
                        newcurve->x0.reading = curve->x0.reading;
                        Fmt (newcurve->x0.label,curve->x0.label);

                        newcurve->x = x;
                        if (!x->curves.nItems) channellist_AddChannel (x);
                        channel_AddCurve (newcurve->x, newcurve->attr.label, curveop.graph->title);

                        Fmt (newcurve->attr.label, "Rational Interp(%s)", curve->attr.label);
                        Fmt (newcurve->attr.note, "%s[a]<%s\n", curveop.note);

                        newcurve->y = y1;
                        if (!y1->curves.nItems) channellist_AddChannel (y1);
                        channel_AddCurve (newcurve->y, newcurve->attr.label, curveop.graph->title);

                        curvelist_AddCurve (&curveop.graph->curves, newcurve);
                        curve_Plot (curveop.graph->p, GRAPH_GRAPH, newcurve, NULL);
                    }
                    else
                    {
                        util_OutofMemory ("Batch Message");
                        break;
                    }
                }
                else
                {
                    util_OutofMemory ("Batch Message");
                    break;
                }
            }
            else
            {
                GetNumCheckedItems (panel, BATCH_CURVES, &checked);
                if (!checked) break;
            }
        }
        curveop_ClearChannel (x);
        curveop_ClearChannel (y1);
        
        DiscardPanel(panel);
    }
    return 0;
}

int  RatInterpCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int i, offset, n, interpCurve, pts;
    double x, y, err;
    curvePtr curve;
    char coeff[30];

    if (event == EVENT_COMMIT)
    {
        GetCtrlVal (panel, INTERP_TOGGLE, &interpCurve);
        GetCtrlIndex (curveview.p1, CURVEVIEW_CURVES, &i);
        curve = curvelist_GetItem (curveG.curves->list, i);
        SetCtrlAttribute (panel, INTERP_KEEP, ATTR_CALLBACK_DATA, curve);

        offset = curve->offset + curveview.offset;
        Fmt (curveop.note, "");

        if (interpCurve) {
            GetCtrlVal (panel, INTERP_PTS, &pts);
            curveop_ClearChannel (curveop.x);
            curveop_ClearChannel (curveop.y1);

            curveop.x = channel_Create();
            curveop.y1 = channel_Create();

            if (curveop.x && curveop.y1 &&
                channel_AllocMem (curveop.x, pts) &&
                channel_AllocMem (curveop.y1, pts) &&
                (curveop_RatInterpCurve (curve->x->readings, curve->y->readings,
                                          offset, curve->curvepts, pts,
                                          curveop.x->readings, curveop.y1->readings) == NoErr)) {
                Fmt (curveop.x->label, "Interp(%s)", curve->x->label);
                Fmt (curveop.y1->label, "Rational Interp(%s)", curve->y->label);

                Fmt (curveop.x->note, "%s\n%s\n", curveop.x->label, curve->x->note);
                Fmt (curveop.y1->note, "%s\n%s\n%s\n", curveop.y1->label, curve->y->note, curveop.note);

                Fmt (curveop.label, "Rational Interp(%s)", curve->attr.label);
                UpdateInterpPanel (NULL, TRUE);
                if (curveview.h2)
                    DeleteGraphPlot (curveview.p1, CURVEVIEW_GRAPH, curveview.h2, VAL_DELAYED_DRAW);
                curveview.h2 = PlotXY (curveview.p1, CURVEVIEW_GRAPH,
                               curveop.x->readings, curveop.y1->readings,
                               pts, VAL_DOUBLE, VAL_DOUBLE,
                               VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, 1,
                               VAL_WHITE);
            }
            else util_OutofMemory ("Rational Interpolation Message");
        }
        else {
            GetCtrlVal (panel, INTERP_X, &x);
            if (RatInterp (curve->x->readings, curve->y->readings, curve->curvepts, x, &y,
                        &err) == NoErr)
            {
                SetCtrlVal (panel, INTERP_Y, y);
                SetCtrlVal (panel, INTERP_ERR, err);
                UpdateInterpPanel(NULL, TRUE);
            }
            else MessagePopup ("Rational Interpolation Message", "Error during interpolation calculation");
        }
    }
    return 0;
}

int curveop_RatInterpCurve (double *x, double *y, int offset, int pts, int i_pts, double *i_x, double *i_y)
{
    int i, calc_err = FALSE;
    double dx, err;

    dx = (x[pts-1+offset]-x[offset])/(i_pts-1);
    for (i = 0; i < i_pts; i++) {
        i_x[i] = x[offset] + (i*dx);
        if (RatInterp (x, y, pts, i_x[i], &i_y[i], &err) != NoErr) {
            MessagePopup ("Rational Interpolation Message", "Error during interpolation calculation");
            calc_err = TRUE;
            break;
        }
    }
    return calc_err;
}

int  BatchPolyInterpCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int c, checked, n, i, pts, offset;
    channelPtr x = NULL, y1 = NULL;
    curvePtr curve, newcurve;

    if (event == EVENT_COMMIT)
    {
        GetCtrlVal (curveview.p2, INTERP_PTS, &pts);
        curveop_ClearChannel (x);
        x = channel_Create();
        channel_AllocMem (x, pts);

        GetNumListItems (panel, BATCH_CURVES, &n);
        for (c = 0; c < n; c++)
        {
            IsListItemChecked (panel, BATCH_CURVES, 0, &checked);
            DeleteListItem (panel, BATCH_CURVES, 0, 1);
            if (checked)
            {
                curve = curvelist_GetItem (curveG.curves->list, c);
                curveop_ClearChannel (y1);

                offset = curve->offset + curveview.offset;
                Fmt (curveop.note, "");
                y1 = channel_Create();

                if (y1 && channel_AllocMem (y1, pts) &&
                    (curveop_PolyInterpCurve (curve->x->readings, curve->y->readings,
                                          offset, curve->curvepts, pts,
                                          x->readings, y1->readings) == NoErr)) {
                    Fmt (y1->label, "Polynomial Interp(%s)", curve->y->label);
                    Fmt (y1->note, "%s\n%s\n%s\n", y1->label, curve->y->note, curveop.note);

                    newcurve = curve_Create();
                    if (newcurve)
                    {
                        curve_CopyAttrs (curve, newcurve);
                        newcurve->curvepts = pts;
                        newcurve->pts = pts;
                        newcurve->offset = 0;
                        newcurve->x0.reading = curve->x0.reading;
                        Fmt (newcurve->x0.label,curve->x0.label);

                        newcurve->x = x;
                        if (!x->curves.nItems) channellist_AddChannel (x);
                        channel_AddCurve (newcurve->x, newcurve->attr.label, curveop.graph->title);

                        Fmt (newcurve->attr.label, "Polynomial Interp(%s)", curve->attr.label);
                        Fmt (newcurve->attr.note, "%s[a]<%s\n", curveop.note);

                        newcurve->y = y1;
                        if (!y1->curves.nItems) channellist_AddChannel (y1);
                        channel_AddCurve (newcurve->y, newcurve->attr.label, curveop.graph->title);

                        curvelist_AddCurve (&curveop.graph->curves, newcurve);
                        curve_Plot (curveop.graph->p, GRAPH_GRAPH, newcurve, NULL);
                    }
                    else
                    {
                        util_OutofMemory ("Batch Message");
                        break;
                    }
                }
                else
                {
                    util_OutofMemory ("Batch Message");
                    break;
                }
            }
            else
            {
                GetNumCheckedItems (panel, BATCH_CURVES, &checked);
                if (!checked) break;
            }
        }
        curveop_ClearChannel (x);
        curveop_ClearChannel (y1);
        
        DiscardPanel(panel);
    }
    return 0;
}

int  KeepInterpCurveCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    curvePtr curve, oldCurve;
    int pts;

    if (event == EVENT_COMMIT)
    {
        oldCurve = callbackData;
        curve = curve_Create();
        if (curve)
        {
            GetCtrlVal (panel, INTERP_PTS, &pts);
            curve_CopyAttrs (oldCurve, curve);
            Fmt (curve->attr.label, curveop.label);
            curve->curvepts = pts;
            curve->pts = pts;
            curve->offset = 0;
            curve->x0.reading = oldCurve->x0.reading;
            Fmt (curve->x0.label, oldCurve->x0.label);
            Fmt (curve->attr.note, "%s[a]<%s\n", curveop.note);

            curve->x = curveop.x;
            if (!curveop.x->curves.nItems) channellist_AddChannel (curveop.x);
            channel_AddCurve (curve->x, curve->attr.label, curveop.graph->title);

            curve->y = curveop.y1;
            if (!curveop.y1->curves.nItems) channellist_AddChannel (curveop.y1);
            channel_AddCurve (curve->y, curve->attr.label, curveop.graph->title);

            curvelist_AddCurve (&curveop.graph->curves, curve);
            curve_Plot (curveop.graph->p, GRAPH_GRAPH, curve, NULL);
        }
        else util_OutofMemory ("Keep Curve Message");
    }
    return 0;
}

int  PolyInterpCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int i, offset, n, interpCurve, pts;
    double x, y, err;
    curvePtr curve;
    char coeff[30];

    if (event == EVENT_COMMIT)
    {
        GetCtrlVal (panel, INTERP_TOGGLE, &interpCurve);
        GetCtrlIndex (curveview.p1, CURVEVIEW_CURVES, &i);
        curve = curvelist_GetItem (curveG.curves->list, i);
        SetCtrlAttribute (panel, INTERP_KEEP, ATTR_CALLBACK_DATA, curve);

        offset = curve->offset + curveview.offset;
        Fmt (curveop.note, "");

        if (interpCurve) {
            GetCtrlVal (panel, INTERP_PTS, &pts);
            curveop_ClearChannel (curveop.x);
            curveop_ClearChannel (curveop.y1);

            curveop.x = channel_Create();
            curveop.y1 = channel_Create();

            if (curveop.x && curveop.y1 &&
                channel_AllocMem (curveop.x, pts) &&
                channel_AllocMem (curveop.y1, pts) &&
                (curveop_PolyInterpCurve (curve->x->readings, curve->y->readings,
                                          offset, curve->curvepts, pts,
                                          curveop.x->readings, curveop.y1->readings) == NoErr)) {
                Fmt (curveop.x->label, "Interp(%s)", curve->x->label);
                Fmt (curveop.y1->label, "Polynomial Interp(%s)", curve->y->label);

                Fmt (curveop.x->note, "%s\n%s\n", curveop.x->label, curve->x->note);
                Fmt (curveop.y1->note, "%s\n%s\n%s\n", curveop.y1->label, curve->y->note, curveop.note);

                Fmt (curveop.label, "Polynomial Interp(%s)", curve->attr.label);
                UpdateInterpPanel (NULL, TRUE);
                if (curveview.h2)
                    DeleteGraphPlot (curveview.p1, CURVEVIEW_GRAPH, curveview.h2, VAL_DELAYED_DRAW);
                curveview.h2 = PlotXY (curveview.p1, CURVEVIEW_GRAPH,
                               curveop.x->readings, curveop.y1->readings,
                               pts, VAL_DOUBLE, VAL_DOUBLE,
                               VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, 1,
                               VAL_WHITE);
            }
            else util_OutofMemory ("Polynomial Interpolation Message");
        }
        else {
            GetCtrlVal (panel, INTERP_X, &x);
            if (PolyInterp (curve->x->readings, curve->y->readings, curve->curvepts, x, &y,
                        &err) == NoErr)
            {
                SetCtrlVal (panel, INTERP_Y, y);
                SetCtrlVal (panel, INTERP_ERR, err);
                UpdateInterpPanel(NULL, TRUE);
            }
            else MessagePopup ("Polynomial Interpolation Message", "Error during interpolation calculation");
        }
    }
    return 0;
}

int curveop_PolyInterpCurve (double *x, double *y, int offset, int pts, int i_pts, double *i_x, double *i_y)
{
    int i, calc_err = FALSE;
    double dx, err;

    dx = (x[pts-1+offset]-x[offset])/(i_pts-1);
    for (i = 0; i < i_pts; i++) {
        i_x[i] = x[offset] + (i*dx);
        if (PolyInterp (x, y, pts, i_x[i], &i_y[i], &err) != NoErr) {
            MessagePopup ("Polynomial Interpolation Message", "Error during interpolation calculation");
            calc_err = TRUE;
            break;
        }
    }
    return calc_err;
}

int  InterpControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int g;
    switch (control)
    {
        default:
            if (event == EVENT_COMMIT)
                UpdateInterpPanel (NULL, FALSE);
            break;
        case INTERP_GRAPHS:
            if (event == EVENT_COMMIT)
            {
                GetCtrlIndex (panel, control, &g);
                curveop.graph = graphlist_GetItem (g);
            }
            break;
    }
    return 0;
}

void InitInterpCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    curveop.UpdatePanel = UpdateInterpPanel;

    curveop_InitOpPanel (INTERP, INTERP_GRAPHS, FALSE, FALSE);

    SetPanelAttribute (curveview.p1, ATTR_TITLE, "Polynomial Interpolation");

    switch (menuItem) {
        case CURVEMENUS_INTERP_POLY:
            SetPanelAttribute (curveview.p1, ATTR_TITLE, "Polynomial Interpolation");
            InstallCtrlCallback (curveview.p2, INTERP_GO, PolyInterpCallback, 0);
            curveop.DoBatch = BatchPolyInterpCallback;
            break;
        case CURVEMENUS_INTERP_RAT:
            SetPanelAttribute (curveview.p1, ATTR_TITLE, "Rational Interpolation");
            InstallCtrlCallback (curveview.p2, INTERP_GO, RatInterpCallback, 0);
            curveop.DoBatch = BatchRatInterpCallback;
            break;
        case CURVEMENUS_INTERP_SPLINE:
            SetPanelAttribute (curveview.p1, ATTR_TITLE, "Spline Interpolation");
            InstallCtrlCallback (curveview.p2, INTERP_GO, SplineInterpCallback, 0);
            curveop.DoBatch = BatchSplineInterpCallback;
            break;
    }

    DisplayPanel (curveview.p2);
    SetPanelPos (curveview.p1, VAL_AUTO_CENTER, VAL_AUTO_CENTER);
    InstallPopup (curveview.p1);
}

void UpdateInterpPanel (curvePtr curve, int input)
{
    int c;
    GetCtrlVal (curveview.p2, INTERP_TOGGLE, &c);

    SetCtrlAttribute (curveview.p2, INTERP_X, ATTR_VISIBLE, !c);
    SetCtrlAttribute (curveview.p2, INTERP_Y, ATTR_VISIBLE, !c);
    SetCtrlAttribute (curveview.p2, INTERP_ERR, ATTR_VISIBLE, !c);

    SetCtrlAttribute (curveview.p2, INTERP_PTS, ATTR_VISIBLE, c);
    SetCtrlAttribute (curveview.p2, INTERP_GRAPHS, ATTR_VISIBLE, c);
    SetCtrlAttribute (curveview.p2, INTERP_KEEP, ATTR_VISIBLE, c);
    SetCtrlAttribute (curveview.p2, INTERP_BATCH, ATTR_VISIBLE, c);

    SetInputMode (curveview.p2, INTERP_Y, input);
    SetInputMode (curveview.p2, INTERP_ERR, input);
    SetInputMode (curveview.p2, INTERP_GRAPHS, input);
    SetInputMode (curveview.p2, INTERP_KEEP, input);
}

int  PolyFitCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int i, offset, n;
    double *poly, mse, *temp;
    curvePtr curve;
    char coeff[30];
    if (event == EVENT_COMMIT)
    {
        GetCtrlVal (panel, POLYFIT_ORDER, &n);
        curveop_ClearChannel (curveop.x);
        curveop_ClearChannel (curveop.y1);

        GetCtrlIndex (curveview.p1, CURVEVIEW_CURVES, &i);
        curve = curvelist_GetItem (curveG.curves->list, i);
        SetCtrlAttribute (panel, POLYFIT_KEEP, ATTR_CALLBACK_DATA, curve);

        offset = curve->offset + curveview.offset;
        Fmt (curveop.note, "");
        curveop.x = channel_Create();
        curveop.y1 = channel_Create();

        temp = (double *)calloc (curveview.pts, sizeof(double));
        Copy1D (curve->y->readings + offset, curveview.pts, temp);

        poly = (double *)calloc (n+1, sizeof(double));

        if (curveop.x && curveop.y1 &&
            channel_AllocMem (curveop.x, curveview.pts) &&
            channel_AllocMem (curveop.y1, curveview.pts) &&
            (PolyFit (curve->x->readings + offset, temp, curveview.pts, n,
                     curveop.y1->readings, poly, &mse) == NoErr))
        {
            free (temp);
            ClearListCtrl (panel, POLYFIT_COEFFS);
            for (i = 0; i < n+1; i++) {
                Fmt (coeff, "%i: %f[e2p4]", i, poly[i]);
                InsertListItem (panel, POLYFIT_COEFFS, -1, coeff, i);
                Fmt (curveop.note, "%s[a]<%i: %f[e2p4]\n", i, poly[i]);
            }
            free (poly);
            Fmt (curveop.note, "%s[a]<mse: %f[e2p4]\n", mse);

            SetCtrlVal (panel, POLYFIT_MSE, mse);

            Subset1D (curve->x->readings, curve->curvepts, offset,
                      curveview.pts, curveop.x->readings);

            Fmt (curveop.x->label, "Subset(%s)", curve->x->label);
            Fmt (curveop.y1->label, "Polynomial Fit(%s)", curve->y->label);

            Fmt (curveop.x->note, "%s\n%s\n", curveop.x->label, curve->x->note);
            Fmt (curveop.y1->note, "%s\n%s\n%s\n", curveop.y1->label, curve->y->note, curveop.note);

            Fmt (curveop.label, "Polynomial Fit(%s)", curve->attr.label);
            UpdatePolyFitPanel (NULL, TRUE);
            if (curveview.h2)
                DeleteGraphPlot (curveview.p1, CURVEVIEW_GRAPH, curveview.h2, VAL_DELAYED_DRAW);
            curveview.h2 = PlotXY (curveview.p1, CURVEVIEW_GRAPH,
                                   curveop.x->readings, curveop.y1->readings,
                                   curveview.pts, VAL_DOUBLE, VAL_DOUBLE,
                                   VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, 1,
                                   VAL_WHITE);
        }
        else MessagePopup ("Polynomial Fit Message", "Error during fitting calculation");
    }
    return 0;
}

int  PolyFitControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int g;
    switch (control)
    {
        default:
            if (event == EVENT_COMMIT)
                UpdatePolyFitPanel (NULL, FALSE);
            break;
        case POLYFIT_GRAPHS:
            if (event == EVENT_COMMIT)
            {
                GetCtrlIndex (panel, control, &g);
                curveop.graph = graphlist_GetItem (g);
            }
            break;
    }
    return 0;
}

void InitPolyFitCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    curveop.UpdatePanel = UpdatePolyFitPanel;

    curveop_InitOpPanel (POLYFIT, POLYFIT_GRAPHS, FALSE, FALSE);

    SetPanelAttribute (curveview.p1, ATTR_TITLE, "n-Polynomial Curve Fit");

    DisplayPanel (curveview.p2);
    SetPanelPos (curveview.p1, VAL_AUTO_CENTER, VAL_AUTO_CENTER);
    InstallPopup (curveview.p1);
}

void UpdatePolyFitPanel (curvePtr curve, int input)
{
    SetInputMode (curveview.p2, POLYFIT_KEEP, input);
    SetInputMode (curveview.p2, POLYFIT_GRAPHS, input);
    SetInputMode (curveview.p2, POLYFIT_COEFFS, input);
    SetInputMode (curveview.p2, POLYFIT_MSE, input);
}

int  ExpFitCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int i, offset;
    double A, B, mse, *temp;
    curvePtr curve;
    if (event == EVENT_COMMIT)
    {
        curveop_ClearChannel (curveop.x);
        curveop_ClearChannel (curveop.y1);

        GetCtrlIndex (curveview.p1, CURVEVIEW_CURVES, &i);
        curve = curvelist_GetItem (curveG.curves->list, i);
        SetCtrlAttribute (panel, EXPFIT_KEEP, ATTR_CALLBACK_DATA, curve);

        offset = curve->offset + curveview.offset;
        Fmt (curveop.note, "");
        curveop.x = channel_Create();
        curveop.y1 = channel_Create();
        temp = (double *)calloc (curveview.pts, sizeof(double));
        Copy1D (curve->y->readings + offset, curveview.pts, temp);
        if (curveop.x && curveop.y1 &&
            channel_AllocMem (curveop.x, curveview.pts) &&
            channel_AllocMem (curveop.y1, curveview.pts) &&
            (ExpFit (curve->x->readings + offset, temp, curveview.pts,
                     curveop.y1->readings, &A, &B, &mse) == NoErr))
        {
            free (temp);
            SetCtrlVal (panel, EXPFIT_A, A);
            SetCtrlVal (panel, EXPFIT_B, B);
            SetCtrlVal (panel, EXPFIT_MSE, mse);

            Fmt (curveop.note, "A: %f[e2p4]\nB: %f[e2p4]\nmse: %f[e2p4]\n", A, B, mse);

            Subset1D (curve->x->readings, curve->curvepts, offset,
                      curveview.pts, curveop.x->readings);

            Fmt (curveop.x->label, "Subset(%s)", curve->x->label);
            Fmt (curveop.y1->label, "Exponential Fit(%s)", curve->y->label);

            Fmt (curveop.x->note, "%s\n%s\n", curveop.x->label, curve->x->note);
            Fmt (curveop.y1->note, "%s\n%s\n%s\n", curveop.y1->label, curve->y->note, curveop.note);

            Fmt (curveop.label, "Exponential Fit(%s)", curve->attr.label);
            UpdateExpFitPanel (NULL, TRUE);
            if (curveview.h2)
                DeleteGraphPlot (curveview.p1, CURVEVIEW_GRAPH, curveview.h2, VAL_DELAYED_DRAW);
            curveview.h2 = PlotXY (curveview.p1, CURVEVIEW_GRAPH,
                                   curveop.x->readings, curveop.y1->readings,
                                   curveview.pts, VAL_DOUBLE, VAL_DOUBLE,
                                   VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, 1,
                                   VAL_WHITE);
        }
        else MessagePopup ("Exponential Fit Message", "Error during fitting calculation");
    }
    return 0;
}

int  ExpFitControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int g;
    switch (control)
    {
        default:
            if (event == EVENT_COMMIT)
                UpdateExpFitPanel (NULL, FALSE);
            break;
        case EXPFIT_GRAPHS:
            if (event == EVENT_COMMIT)
            {
                GetCtrlIndex (panel, control, &g);
                curveop.graph = graphlist_GetItem (g);
            }
            break;
    }
    return 0;
}

void InitExpFitCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    curveop.UpdatePanel = UpdateExpFitPanel;

    curveop_InitOpPanel (EXPFIT, EXPFIT_GRAPHS, FALSE, FALSE);

    SetPanelAttribute (curveview.p1, ATTR_TITLE, "Exponential Fit Curves");

    DisplayPanel (curveview.p2);
    SetPanelPos (curveview.p1, VAL_AUTO_CENTER, VAL_AUTO_CENTER);
    InstallPopup (curveview.p1);
}

void UpdateExpFitPanel (curvePtr curve, int input)
{
    SetInputMode (curveview.p2, EXPFIT_KEEP, input);
    SetInputMode (curveview.p2, EXPFIT_GRAPHS, input);
    SetInputMode (curveview.p2, EXPFIT_A, input);
    SetInputMode (curveview.p2, EXPFIT_B, input);
    SetInputMode (curveview.p2, EXPFIT_MSE, input);
}

int  LinFitCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int i, offset;
    double slope, intercept, mse;
    curvePtr curve;
    if (event == EVENT_COMMIT)
    {
        curveop_ClearChannel (curveop.x);
        curveop_ClearChannel (curveop.y1);

        GetCtrlIndex (curveview.p1, CURVEVIEW_CURVES, &i);
        curve = curvelist_GetItem (curveG.curves->list, i);
        SetCtrlAttribute (panel, LINFIT_KEEP, ATTR_CALLBACK_DATA, curve);

        offset = curve->offset + curveview.offset;
        Fmt (curveop.note, "");
        curveop.x = channel_Create();
        curveop.y1 = channel_Create();
        if (curveop.x && curveop.y1 &&
            channel_AllocMem (curveop.x, curveview.pts) &&
            channel_AllocMem (curveop.y1, curveview.pts) &&
            (LinFit (curve->x->readings + offset, curve->y->readings + offset,
                    curveview.pts, curveop.y1->readings, &slope, &intercept, &mse) == NoErr))
        {
            SetCtrlVal (panel, LINFIT_SLOPE, slope);
            SetCtrlVal (panel, LINFIT_INVSLOPE, 1/slope);
            SetCtrlVal (panel, LINFIT_INTERCEPT, intercept);
            SetCtrlVal (panel, LINFIT_MSE, mse);

            Fmt (curveop.note, "slope: %f[e2p4]\nintercept: %f[e2p4]\nmse: %f[e2p4]\n", slope, intercept, mse);

            Subset1D (curve->x->readings, curve->curvepts, offset,
                      curveview.pts, curveop.x->readings);

            Fmt (curveop.x->label, "Subset(%s)", curve->x->label);
            Fmt (curveop.y1->label, "Linear Fit(%s)", curve->y->label);

            Fmt (curveop.x->note, "%s\n%s\n", curveop.x->label, curve->x->note);
            Fmt (curveop.y1->note, "%s\n%s\n%s\n", curveop.y1->label, curve->y->note, curveop.note);

            Fmt (curveop.label, "Linear Fit(%s)", curve->attr.label);
            UpdateLinFitPanel (NULL, TRUE);
            if (curveview.h2)
                DeleteGraphPlot (curveview.p1, CURVEVIEW_GRAPH, curveview.h2, VAL_DELAYED_DRAW);
            curveview.h2 = PlotXY (curveview.p1, CURVEVIEW_GRAPH,
                                   curveop.x->readings, curveop.y1->readings,
                                   curveview.pts, VAL_DOUBLE, VAL_DOUBLE,
                                   VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, 1,
                                   VAL_WHITE);
        }
        else MessagePopup ("Linear Fit Message", "Error during fitting calculation");
    }
    return 0;
}

int  LinFitControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int g;
    switch (control)
    {
        default:
            if (event == EVENT_COMMIT)
                UpdateLinFitPanel (NULL, FALSE);
            break;
        case LINFIT_GRAPHS:
            if (event == EVENT_COMMIT)
            {
                GetCtrlIndex (panel, control, &g);
                curveop.graph = graphlist_GetItem (g);
            }
            break;
    }
    return 0;
}

void InitLinFitCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    curveop.UpdatePanel = UpdateLinFitPanel;

    curveop_InitOpPanel (LINFIT, LINFIT_GRAPHS, FALSE, FALSE);

    SetPanelAttribute (curveview.p1, ATTR_TITLE, "Linear Fit Curves");

    DisplayPanel (curveview.p2);
    SetPanelPos (curveview.p1, VAL_AUTO_CENTER, VAL_AUTO_CENTER);
    InstallPopup (curveview.p1);
}

void UpdateLinFitPanel (curvePtr curve, int input)
{
    SetInputMode (curveview.p2, LINFIT_KEEP, input);
    SetInputMode (curveview.p2, LINFIT_GRAPHS, input);
    SetInputMode (curveview.p2, LINFIT_INVSLOPE, input);
    SetInputMode (curveview.p2, LINFIT_SLOPE, input);
    SetInputMode (curveview.p2, LINFIT_INTERCEPT, input);
    SetInputMode (curveview.p2, LINFIT_MSE, input);
}

int  PowerSpecBatchCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int c, checked, n, freq = FALSE, i;
    channelPtr x = NULL, y1 = NULL;
    curvePtr curve, newcurve;
    double *temp;

    temp = NULL;
    if (event == EVENT_COMMIT)
    {
        curveop_ClearChannel (x);
        x = channel_Create();
        channel_AllocMem (x, curveview.pts/2);

        GetNumListItems (panel, BATCH_CURVES, &n);
        for (c = 0; c < n; c++)
        {
            IsListItemChecked (panel, BATCH_CURVES, 0, &checked);
            DeleteListItem (panel, BATCH_CURVES, 0, 1);
            if (checked)
            {
                curve = curvelist_GetItem (curveG.curves->list, c);
                curveop_CalcInterval (curve, FALSE);

                curveop_ClearChannel (y1);

                Fmt (curveop.note, "");
                y1 = channel_Create();

                if (!temp) temp = calloc (curveview.pts, sizeof (double));
                Copy1D (curve->y->readings+curve->offset+curveview.offset, curveview.pts, temp);
                if (curveop.win.w != -1) ScaledWindow (temp, curveview.pts, curveop.win.w, &curveop.win.constants);
                if (y1 && channel_AllocMem (y1, curveview.pts/2) &&
                    (AutoPowerSpectrum (temp, curveview.pts, curveop.interval, y1->readings,
                                &curveop.freq) == NoErr))
                {
                    if (!freq)
                    {
                        for (i = 0; i < curveview.pts/2; i++)
                            x->readings[i] = i*curveop.freq;
                        Fmt (x->label, "Freq(%s)", curve->x->label);
                        Fmt (x->note, "%s[a]<%s\n%s\n", x->label, curve->x->note);
                        freq = TRUE;
                    }

                    Fmt (y1->label, "Power Spectrum(%s)", curve->y->label);
                    Fmt (y1->note, "%s\n%s\n%s\n", y1->label, curve->y->note, curveop.note);

                    newcurve = curve_Create();
                    if (newcurve) {
                        curve_CopyAttrs (curve, newcurve);
                        newcurve->curvepts = curveview.pts/2;
                        newcurve->pts = curveview.pts/2;
                        newcurve->offset = 0;
                        newcurve->x0.reading = curve->x0.reading;
                        Fmt (newcurve->x0.label,curve->x0.label);

                        newcurve->x = x;
                        if (!x->curves.nItems) channellist_AddChannel (x);
                        channel_AddCurve (newcurve->x, newcurve->attr.label, curveop.graph->title);

                        Fmt (newcurve->attr.label, "Power Spectrum(%s)", curve->attr.label);
                        Fmt (newcurve->attr.note, "%s[a]<%s\n", curveop.note);

                        newcurve->y = y1;
                        if (!y1->curves.nItems) channellist_AddChannel (y1);
                        channel_AddCurve (newcurve->y, newcurve->attr.label, curveop.graph->title);

                        curvelist_AddCurve (&curveop.graph->curves, newcurve);
                        curve_Plot (curveop.graph->p, GRAPH_GRAPH, newcurve, NULL);
                    }
                    else {
                        util_OutofMemory ("Batch Message");
                        break;
                    }
                }
                else
                {
                    util_OutofMemory ("Batch Message");
                    break;
                }
            }
            else
            {
                GetNumCheckedItems (panel, BATCH_CURVES, &checked);
                if (!checked) break;
            }
        }
        curveop_ClearChannel (x);
        curveop_ClearChannel (y1);
        if (temp) free (temp);
        
        DiscardPanel(panel);
    }
    return 0;
}

int  PowerFreqEstimateCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    double srchfreq, peak_freq, peak_power;
    int span;
    if (event == EVENT_COMMIT)
    {
        GetCtrlVal (panel, POWERSPEC_SRCHFREQ, &srchfreq);
        GetCtrlVal (panel, POWERSPEC_SPAN, &span);
        if (PowerFrequencyEstimate (curveop.y1->readings, curveview.pts/2, srchfreq,
                                curveop.win.constants, curveop.freq, span,
                                &peak_freq, &peak_power) == NoErr) {
            SetCtrlVal (panel, POWERSPEC_PEAK_FREQ, peak_freq);
            SetCtrlVal (panel, POWERSPEC_PEAK_POWER, peak_power);
            SetInputMode (panel, POWERSPEC_PEAK_FREQ, TRUE);
            SetInputMode (panel, POWERSPEC_PEAK_POWER, TRUE);
        }
        else MessagePopup ("Power Frequency Estimate Message", "Error during calculation");
    }
    return 0;
}

int  PowerSpecCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int i;
    curvePtr curve;
    double *temp;

    if (event == EVENT_COMMIT)
    {
        GetCtrlVal (panel, POWERSPEC_WINDOW, &curveop.win.w);
        curveop_ClearChannel (curveop.x);
        curveop_ClearChannel (curveop.y1);

        GetCtrlIndex (curveview.p1, CURVEVIEW_CURVES, &i);
        curve = curvelist_GetItem (curveG.curves->list, i);
        curveop_CalcInterval (curve, FALSE);

        SetCtrlAttribute (panel, POWERSPEC_KEEP, ATTR_CALLBACK_DATA, curve);

        Fmt(curveop.note, "");
        curveop.x = channel_Create();
        curveop.y1 = channel_Create();
        temp = calloc (curveview.pts, sizeof(double));
        Copy1D (curve->y->readings+curve->offset+curveview.offset, curveview.pts, temp);
        if (curveop.win.w != -1) ScaledWindow (temp, curveview.pts, curveop.win.w, &curveop.win.constants);
        if (curveop.x && curveop.y1 && temp &&
            channel_AllocMem (curveop.x, curveview.pts/2) &&
            channel_AllocMem (curveop.y1, curveview.pts/2) &&
            (AutoPowerSpectrum (temp, curveview.pts, curveop.interval, curveop.y1->readings,
                                &curveop.freq) == NoErr))
        {
            free (temp);
            for (i = 0; i < curveview.pts/2; i++)
                curveop.x->readings[i] = i*curveop.freq;

            Fmt (curveop.x->label, "Freq(%s)", curve->x->label);
            Fmt (curveop.y1->label, "Power Spectrum(%s)", curve->y->label);

            Fmt (curveop.x->note, "%s[a]<%s\n%s\n", curveop.x->label, curve->x->note);
            Fmt (curveop.y1->note, "%s[a]<%s\n%s\n%s\n", curveop.y1->label, curve->y->note, curveop.note);

            Fmt (curveop.label, "Power Spectrum(%s)", curve->attr.label);
            UpdatePowerSpecPanel (curve, TRUE);
            DeleteGraphPlot (panel, POWERSPEC_GRAPH, -1, VAL_DELAYED_DRAW);
            PlotXY (panel, POWERSPEC_GRAPH,
                    curveop.x->readings,
                    curveop.y1->readings,
                    curveview.pts/2, VAL_DOUBLE, VAL_DOUBLE, VAL_THIN_LINE,
                    VAL_EMPTY_SQUARE, VAL_SOLID, 1, VAL_YELLOW);
        }
        else MessagePopup ("Power Spectrum Message", "Error during calculation");
    }
    return 0;
}

int  PowerSpecControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int g;
    switch (control)
    {
        default:
            if (event == EVENT_COMMIT)
                UpdatePowerSpecPanel (NULL, FALSE);
            break;
        case AMPPHASE_GRAPHS:
            if (event == EVENT_COMMIT)
            {
                GetCtrlIndex (panel, control, &g);
                curveop.graph = graphlist_GetItem (g);
            }
            break;
    }
    return 0;
}

void InitPowerSpecCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    curveop.UpdatePanel = UpdatePowerSpecPanel;

    curveop_InitOpPanel (POWERSPEC, POWERSPEC_GRAPHS, TRUE, TRUE);

    SetPanelAttribute (curveview.p1, ATTR_TITLE, "Power Spectrum Analysis");

    curveop.DoBatch = PowerSpecBatchCallback;

    DisplayPanel (curveview.p2);
    SetPanelPos (curveview.p1, VAL_AUTO_CENTER, VAL_AUTO_CENTER);
    InstallPopup (curveview.p1);
}

void UpdatePowerSpecPanel (curvePtr curve, int input)
{
    SetInputMode (curveview.p2, POWERSPEC_KEEP, input);
    SetInputMode (curveview.p2, POWERSPEC_GRAPH, input);
    SetInputMode (curveview.p2, POWERSPEC_GRAPHS, input);
    if (!input || (curveop.win.w != -1)) {
        SetInputMode (curveview.p2, POWERSPEC_SRCHFREQ, input);
        SetInputMode (curveview.p2, POWERSPEC_SPAN, input);
        SetInputMode (curveview.p2, POWERSPEC_PEAK_GO, input);
    }
    if (!input) {
        SetInputMode (curveview.p2, POWERSPEC_PEAK_FREQ, input);
        SetInputMode (curveview.p2, POWERSPEC_PEAK_POWER, input);
    }
}

int  InitAmpSpecBatchCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int batch_p, c, g;
    curvePtr curve;

    if (event == EVENT_COMMIT)
    {
        if (control == AMPPHASE_BATCH_1) curveop.amp = TRUE;
        else curveop.amp = FALSE;

        batch_p = LoadPanel (0, "curveopu.uir", BATCH);
        
        SetPanelPos (batch_p, VAL_AUTO_CENTER, VAL_AUTO_CENTER);
        util_InitClose (batch_p, BATCH_CANCEL, TRUE);

        graphlist_Copy (batch_p, BATCH_GRAPHS);
        GetCtrlIndex (batch_p, BATCH_GRAPHS, &g);
        curveop.graph = graphlist_GetItem (g);

        ClearListCtrl (batch_p, BATCH_CURVES);
        for (c = 0; c < curveG.curves->list.nItems; c++)
        {
            curve = curvelist_GetItem (curveG.curves->list, c);
            InsertListItem (batch_p, BATCH_CURVES, -1,
                            curve_CompleteListItem(curve, curveview.offset, curveview.pts), c);
        }
        InstallCtrlCallback (batch_p, BATCH_GO, AmpSpecBatchCallback, 0);
        InstallPopup (batch_p);
    }
    return 0;
}

int  AmpSpecBatchCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int c, checked, n, freq = FALSE, i;
    channelPtr x = NULL, y1 = NULL, y2 = NULL;
    curvePtr curve, newcurve;
    double *temp;

    temp = NULL;
    if (event == EVENT_COMMIT)
    {
        curveop_ClearChannel (x);
        x = channel_Create();
        channel_AllocMem (x, curveview.pts/2);

        GetNumListItems (panel, BATCH_CURVES, &n);
        for (c = 0; c < n; c++)
        {
            IsListItemChecked (panel, BATCH_CURVES, 0, &checked);
            DeleteListItem (panel, BATCH_CURVES, 0, 1);
            if (checked)
            {
                curve = curvelist_GetItem (curveG.curves->list, c);
                curveop_CalcInterval (curve, FALSE);

                curveop_ClearChannel (y1);
                curveop_ClearChannel (y2);

                Fmt (curveop.note, "");
                y1 = channel_Create();
                y2 = channel_Create();

                if (!temp) temp = calloc (curveview.pts, sizeof (double));

                if (y1 && y2 &&
                    channel_AllocMem (y1, curveview.pts/2) &&
                    channel_AllocMem (y2, curveview.pts/2) &&
                    (Window_Compute_IIR (curve->y->readings + curveview.offset + curve->offset,
                                curveview.pts, temp, curveop.note) == NoErr) &&
                    (AmpPhaseSpectrum (temp, curveview.pts, 0, curveop.interval,
                               y1->readings, y2->readings,
                               &curveop.freq) == NoErr))
                {
                    if (!freq)
                    {
                        for (i = 0; i < curveview.pts/2; i++)
                            x->readings[i] = i*curveop.freq;
                        Fmt (x->label, "Freq(%s)", curve->x->label);
                        Fmt (x->note, "%s[a]<%s\n%s\n", x->label, curve->x->note);
                        freq = TRUE;
                    }

                    Fmt (y1->label, "Amp Spectrum(%s)", curve->y->label);
                    Fmt (y2->label, "Phase Spectrum(%s)", curve->y->label);

                    Fmt (y1->note, "%s[a]<%s\n%s\n%s\n", y1->label, curve->y->note, curveop.note);
                    Fmt (y2->note, "%s[a]<%s\n%s\n%s\n", y2->label, curve->y->note, curveop.note);

                    newcurve = curve_Create();
                    if (newcurve) {
                        curve_CopyAttrs (curve, newcurve);
                        newcurve->curvepts = curveview.pts/2;
                        newcurve->pts = curveview.pts/2;
                        newcurve->offset = 0;
                        newcurve->x0.reading = curve->x0.reading;
                        Fmt (newcurve->x0.label,curve->x0.label);

                        newcurve->x = x;
                        if (!x->curves.nItems) channellist_AddChannel (x);
                        channel_AddCurve (newcurve->x, newcurve->attr.label, curveop.graph->title);

                        if (curveop.amp)
                        {
                            Fmt (newcurve->attr.label, "Amp Spectrum(%s)", curve->attr.label);
                            Fmt (newcurve->attr.note, "%s[a]<%s\n", curveop.note);

                            newcurve->y = y1;
                            if (!y1->curves.nItems) channellist_AddChannel (y1);
                        }
                        else
                        {
                            Fmt (newcurve->attr.label, "Phase Spectrum(%s)", curve->attr.label);
                            Fmt (newcurve->attr.note, "%s[a]<%s\n", curveop.note);

                            newcurve->y = y2;
                            if (!y2->curves.nItems) channellist_AddChannel (y2);
                        }

                        channel_AddCurve (newcurve->y, newcurve->attr.label, curveop.graph->title);

                        curvelist_AddCurve (&curveop.graph->curves, newcurve);
                        curve_Plot (curveop.graph->p, GRAPH_GRAPH, newcurve, NULL);
                    }
                    else
                    {
                        util_OutofMemory ("Batch Message");
                        break;
                    }
                }
                else
                {
                    util_OutofMemory ("Batch Message");
                    break;
                }
            }
            else
            {
                GetNumCheckedItems (panel, BATCH_CURVES, &checked);
                if (!checked) break;
            }
        }
        curveop_ClearChannel (y1);
        curveop_ClearChannel (y2);
        if (temp) free (temp);
        
        DiscardPanel(panel);
    }
    return 0;
}

int  KeepPhaseCurveCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    curvePtr curve, oldCurve;

    if (event == EVENT_COMMIT)
    {
        oldCurve = callbackData;
        curve = curve_Create();
        if (curve) {
            curve_CopyAttrs (oldCurve, curve);
            Fmt (curve->attr.label, curveop.label);
            curve->curvepts = curveview.pts/2;
            curve->pts = curveview.pts/2;
            curve->offset = 0;
            curve->x0.reading = oldCurve->x0.reading;
            Fmt (curve->x0.label, oldCurve->x0.label);
            Fmt (curve->attr.note, "%s[a]<%s\n", curveop.note);

            curve->x = curveop.x;
            if (!curveop.x->curves.nItems) channellist_AddChannel (curveop.x);
            channel_AddCurve (curve->x, curve->attr.label, curveop.graph->title);

            curve->y = curveop.y2;
            if (!curveop.y2->curves.nItems) channellist_AddChannel (curveop.y2);
            channel_AddCurve (curve->y, curve->attr.label, curveop.graph->title);

            curvelist_AddCurve (&curveop.graph->curves, curve);
            curve_Plot (curveop.graph->p, GRAPH_GRAPH, curve, NULL);
        }
        else util_OutofMemory ("Keep Curve Message");
    }
    return 0;
}

int  KeepAmpCurveCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    curvePtr curve, oldCurve;

    if (event == EVENT_COMMIT)
    {
        oldCurve = callbackData;
        curve = curve_Create();
        if (curve) {
            curve_CopyAttrs (oldCurve, curve);
            Fmt (curve->attr.label, curveop.label);
            curve->curvepts = curveview.pts/2;
            curve->pts = curveview.pts/2;
            curve->offset = 0;
            curve->x0.reading = oldCurve->x0.reading;
            Fmt (curve->x0.label, oldCurve->x0.label);
            Fmt (curve->attr.note, "%s[a]<%s\n", curveop.note);

            curve->x = curveop.x;
            if (!curveop.x->curves.nItems) channellist_AddChannel (curveop.x);
            channel_AddCurve (curve->x, curve->attr.label, curveop.graph->title);

            curve->y = curveop.y1;
            if (!curveop.y1->curves.nItems) channellist_AddChannel (curveop.y1);
            channel_AddCurve (curve->y, curve->attr.label, curveop.graph->title);

            curvelist_AddCurve (&curveop.graph->curves, curve);
            curve_Plot (curveop.graph->p, GRAPH_GRAPH, curve, NULL);
        }
        else util_OutofMemory ("Keep Curve Message");
    }
    return 0;
}

int  AmpPhaseCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int i;
    curvePtr curve;
    double *temp;

    if (event == EVENT_COMMIT)
    {
        curveop_ClearChannel (curveop.x);
        curveop_ClearChannel (curveop.y1);
        curveop_ClearChannel (curveop.y2);

        GetCtrlIndex (curveview.p1, CURVEVIEW_CURVES, &i);
        curve = curvelist_GetItem (curveG.curves->list, i);
        curveop_CalcInterval (curve, FALSE);

        SetCtrlAttribute (panel, AMPPHASE_KEEP_1, ATTR_CALLBACK_DATA, curve);
        SetCtrlAttribute (panel, AMPPHASE_KEEP_2, ATTR_CALLBACK_DATA, curve);

        Fmt(curveop.note, "");
        curveop.x = channel_Create();
        curveop.y1 = channel_Create();
        curveop.y2 = channel_Create();
        temp = calloc (curveview.pts, sizeof(double));
        if (curveop.x && curveop.y1 && temp &&
            channel_AllocMem (curveop.x, curveview.pts/2) &&
            channel_AllocMem (curveop.y1, curveview.pts/2) &&
            channel_AllocMem (curveop.y2, curveview.pts/2) &&
            (Window_Compute_IIR (curve->y->readings + curveview.offset + curve->offset,
                                curveview.pts, temp, curveop.note) == NoErr) &&
            (AmpPhaseSpectrum (temp, curveview.pts, 0, curveop.interval,
                               curveop.y1->readings, curveop.y2->readings,
                               &curveop.freq) == NoErr))
        {
            SetCtrlVal (panel, AMPPHASE_SAMPFREQ, curveop.freq*(curveview.pts-1));
            free (temp);
            for (i = 0; i < curveview.pts/2; i++)
                curveop.x->readings[i] = i*curveop.freq;

            Fmt (curveop.x->label, "Freq(%s)", curve->x->label);
            Fmt (curveop.y1->label, "Amplitude(%s)", curve->y->label);
            Fmt (curveop.y2->label, "Phase(%s)", curve->y->label);

            Fmt (curveop.x->note, "%s[a]<%s\n%s\n", curveop.x->label, curve->x->note);
            Fmt (curveop.y1->note, "%s[a]<%s\n%s\n%s\n", curveop.y1->label, curve->y->note, curveop.note);
            Fmt (curveop.y2->note, "%s[a]<%s\n%s\n%s\n", curveop.y2->label, curve->y->note, curveop.note);

            Fmt (curveop.label, "Amp Spectrum(%s)", curve->attr.label);
            Fmt (curveop.label2, "Phase Spectrum (%s)", curve->attr.label);
            UpdateAmpPhasePanel (curve, TRUE);
            DeleteGraphPlot (panel, AMPPHASE_GRAPH_1, -1, VAL_DELAYED_DRAW);
            DeleteGraphPlot (panel, AMPPHASE_GRAPH_2, -1, VAL_DELAYED_DRAW);
            PlotXY (panel, AMPPHASE_GRAPH_1,
                    curveop.x->readings,
                    curveop.y1->readings,
                    curveview.pts/2, VAL_DOUBLE, VAL_DOUBLE, VAL_THIN_LINE,
                    VAL_EMPTY_SQUARE, VAL_SOLID, 1, VAL_YELLOW);
            PlotXY (panel, AMPPHASE_GRAPH_2,
                    curveop.x->readings,
                    curveop.y2->readings,
                    curveview.pts/2, VAL_DOUBLE, VAL_DOUBLE, VAL_THIN_LINE,
                    VAL_EMPTY_SQUARE, VAL_SOLID, 1, VAL_YELLOW);
        }
        else MessagePopup ("Amplitude Phase Spectrum Message", "Error during calculation");
    }
    return 0;
}

int Window_Compute_IIR (double *y, int pts, double *win_y, char *note)
{
    int err, window;
    double beta, final, dty;
    char message[21];

    err = NoErr;
    GetCtrlVal (curveview.p2, AMPPHASE_WINDOW, &window);
    Copy1D (y, pts, win_y);
    switch (window)
    {
        case TRI_WIN:
            err = TriWin (win_y, pts);
            Fmt (note, "Triangular Window\n");
            break;
        case HAN_WIN:
            err = HanWin (win_y, pts);
            Fmt (note, "Hanning Window\n");
            break;
        case HAM_WIN:
            err = HamWin (win_y, pts);
            Fmt (note, "Hamming Window\n");
            break;
        case BKMAN_WIN:
            err = BkmanWin (win_y, pts);
            Fmt (note, "Blackman Window\n");
            break;
        case KSR_WIN:
            PromptPopup ("Kaiser Window Message", "Enter beta value for window:",
                         message, 20);
            if (Scan (message, "%s>%f", &beta) == 1)
            {
                err = KsrWin (win_y, pts, beta);
                Fmt (note, "Kaiser Window\n"
                       "beta: %f[e2p5]\n", beta);
            } else err = TRUE;
            break;
        case BLK_HARRIS_WIN:
            err = BlkHarrisWin (win_y, pts);
            Fmt (note, "Blackman-Harris Window\n");
            break;
        case COS_TAPERED_WIN:
            err = CosTaperedWin (win_y, pts);
            Fmt (note, "Cosine Tapered Window\n");
            break;
        case EX_BKMAN_WIN:
            err = ExBkmanWin (win_y, pts);
            Fmt (note, "Exact Blackman Window\n");
            break;
        case EXP_WIN:
            PromptPopup ("Exponential Window Message",
                         "Enter final value for window:", message, 20);
            if (Scan (message, "%s>%f", &final) == 1)
            {
                err = ExpWin (win_y, pts, final);
                Fmt (note, "Exponential Window\n"
                       "final value: %f[e2p5]\n", final);
            } else err = TRUE;
            break;
        case FLAT_TOP_WIN:
            err = FlatTopWin (win_y, pts);
            Fmt (note, "Flat Top Window\n");
            break;
        case FORCE_WIN:
            PromptPopup ("Force Window Message",
                         "Enter duty cycle in % for window:", message, 20);
            if (Scan (message, "%s>%f", &dty) == 1)
            {
                err = ForceWin (win_y, pts, dty);
                Fmt (note, "Force Window\n"
                       "duty cycle: %f[p3]\n", dty);
            } else err = TRUE;
            break;
    }
    return err;
}

int  AmpPhaseControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int g;
    switch (control)
    {
        default:
            if (event == EVENT_COMMIT)
                UpdateAmpPhasePanel (NULL, FALSE);
            break;
        case AMPPHASE_GRAPHS:
            if (event == EVENT_COMMIT)
            {
                GetCtrlIndex (panel, control, &g);
                curveop.graph = graphlist_GetItem (g);
            }
            break;
    }
    return 0;
}

void InitAmpPhaseCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    curveop.UpdatePanel = UpdateAmpPhasePanel;

    curveop_InitOpPanel (AMPPHASE, AMPPHASE_GRAPHS, TRUE, TRUE);

    SetPanelAttribute (curveview.p1, ATTR_TITLE, "Amplitude Phase Spectrum Analysis");

    DisplayPanel (curveview.p2);
    SetPanelPos (curveview.p1, VAL_AUTO_CENTER, VAL_AUTO_CENTER);
    InstallPopup (curveview.p1);
}

void UpdateAmpPhasePanel (curvePtr curve, int input)
{
    SetInputMode (curveview.p2, AMPPHASE_KEEP_1, input);
    SetInputMode (curveview.p2, AMPPHASE_GRAPH_1, input);
    SetInputMode (curveview.p2, AMPPHASE_KEEP_2, input);
    SetInputMode (curveview.p2, AMPPHASE_GRAPH_2, input);
    SetInputMode (curveview.p2, AMPPHASE_SAMPFREQ, input);
    SetInputMode (curveview.p2, AMPPHASE_GRAPHS, input);
}

int  ACDCCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int c;
    curvePtr curve;
    double ac, dc;
    if (event == EVENT_COMMIT)
    {
        GetCtrlIndex (curveview.p1, CURVEVIEW_CURVES, &c);
        curve = curvelist_GetItem (curveG.curves->list, c);

        ACDCEstimator (curve->y->readings+curve->offset+curveview.offset,
                       curveview.pts, &ac, &dc);
        SetCtrlVal (panel, ACDC_AC, ac);
        SetCtrlVal (panel, ACDC_DC, dc);
        UpdateACDCPanel (0, TRUE);
    }
    return 0;
}

void InitACDCCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    curveop.UpdatePanel = UpdateACDCPanel;

    curveop_InitOpPanel (ACDC, 0, FALSE, FALSE);

    SetPanelAttribute (curveview.p1, ATTR_TITLE, "Curve AC/DC Estimator");

    curveop.DoBatch = NULL;

    DisplayPanel (curveview.p2);
    SetPanelPos (curveview.p1, VAL_AUTO_CENTER, VAL_AUTO_CENTER);
    InstallPopup (curveview.p1);
}

void UpdateACDCPanel (curvePtr curve, int input)
{
    SetInputMode (curveview.p2, ACDC_AC, input);
    SetInputMode (curveview.p2, ACDC_DC, input);
}

int  BatchSmoothCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int c, checked, n, order;
    double *kernal, variance;
    channelPtr x = NULL, y = NULL;
    curvePtr curve, newcurve;
    if (event == EVENT_COMMIT)
    {
        GetCtrlVal (curveview.p2, SMOOTH_ORDER, &order);
        GetCtrlVal (curveview.p2, SMOOTH_VARIANCE, &variance);
        kernal = (double *)calloc (order, sizeof(double));
        if (curveop_SmoothingKernal (order, variance, kernal) != NoErr) {
            MessagePopup ("Smoothing Kernal Message", "Error calculating kernal");
            return 0;
        }
        GetNumListItems (panel, BATCH_CURVES, &n);
        for (c = 0; c < n; c++)
        {
            IsListItemChecked (panel, BATCH_CURVES, 0, &checked);
            DeleteListItem (panel, BATCH_CURVES, 0, 1);
            if (checked)
            {
                curve = curvelist_GetItem (curveG.curves->list, c);
                curveop_ClearChannel (x);
                curveop_ClearChannel (y);

                Fmt (curveop.note, "");
                x = channel_Create();
                y = channel_Create();

                if (x && y &&
                    channel_AllocMem (x, curveview.pts) &&
                    channel_AllocMem (y, curveview.pts) &&
                    (curveop_Smooth (kernal, order, curve->y->readings, curve->curvepts,
                                     curve->offset+curveview.offset, curveview.pts, y->readings) == NoErr))
                {
                    Subset1D (curve->x->readings, curve->curvepts,
                      curveview.offset + curve->offset, curveview.pts,
                      x->readings);
                    Fmt (x->label, "Subset(%s)", curve->x->label);
                    Fmt (y->label, "Smooth(%s)", curve->y->label);
                    Fmt (curveop.note, "order: %i\nvariance: %f[e2p5]\n", order, variance);

                    Fmt (x->note, "%s\n%s\n", x->label, curve->x->note);
                    Fmt (y->note, "%s\n%s\n%s\n", y->label, curve->y->note, curveop.note);

                    newcurve = curve_Create();
                    if (newcurve) {
                        curve_CopyAttrs (curve, newcurve);
                        Fmt (newcurve->attr.label, "Smooth(%s)", curve->attr.label);
                        newcurve->curvepts = curveview.pts;
                        newcurve->pts = curveview.pts;
                        newcurve->offset = 0;
                        newcurve->x0.reading = curve->x0.reading;
                        Fmt (newcurve->x0.label,curve->x0.label);
                        Fmt (newcurve->attr.note, "%s[a]<%s\n", curveop.note);

                        newcurve->x = x;
                        if (!x->curves.nItems) channellist_AddChannel (x);
                        channel_AddCurve (newcurve->x, newcurve->attr.label, curveop.graph->title);

                        newcurve->y = y;
                        if (!y->curves.nItems) channellist_AddChannel (y);
                        channel_AddCurve (newcurve->y, newcurve->attr.label, curveop.graph->title);

                        curvelist_AddCurve (&curveop.graph->curves, newcurve);
                        curve_Plot (curveop.graph->p, GRAPH_GRAPH, newcurve, NULL);
                    }
                    else
                    {
                        util_OutofMemory ("Batch Message");
                        break;
                    }
                }
                else
                {
                    util_OutofMemory ("Batch Message");
                    break;
                }
            }
            else
            {
                GetNumCheckedItems (panel, BATCH_CURVES, &checked);
                if (!checked) break;
            }
        }
        
        DiscardPanel(panel);
        free (kernal);
    }
    return 0;
}

int  SmoothCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int i, order;
    double variance, *kernal;
    curvePtr curve;
    if (event == EVENT_COMMIT)
    {
        GetCtrlVal (panel, SMOOTH_ORDER, &order);
        GetCtrlVal (panel, SMOOTH_VARIANCE, &variance);
        curveop_ClearChannel (curveop.x);
        curveop_ClearChannel (curveop.y1);

        GetCtrlIndex (curveview.p1, CURVEVIEW_CURVES, &i);
        curve = curvelist_GetItem (curveG.curves->list, i);
        SetCtrlAttribute (panel, SMOOTH_KEEP, ATTR_CALLBACK_DATA, curve);

        Fmt (curveop.note, "");
        curveop.x = channel_Create();
        curveop.y1 = channel_Create();
        kernal = (double *)calloc (order, sizeof(double));
        curveop_SmoothingKernal (order, variance, kernal);
        if (curveop.x && curveop.y1 &&
            channel_AllocMem (curveop.x, curveview.pts) &&
            channel_AllocMem (curveop.y1, curveview.pts) &&
            (curveop_Smooth (kernal, order, curve->y->readings, curve->curvepts,
                     curve->offset+curveview.offset, curveview.pts, curveop.y1->readings) == NoErr))
        {
            free (kernal);
            Fmt (curveop.note, "order: %i\nvariance: %f[p3]\n", order, variance);

            Subset1D (curve->x->readings, curve->curvepts,
                      curveview.offset + curve->offset, curveview.pts,
                      curveop.x->readings);
            Fmt (curveop.x->label, "Subset(%s)", curve->x->label);
            Fmt (curveop.y1->label, "Smooth(%s)", curve->y->label);

            Fmt (curveop.x->note, "%s[a]<%s\n%s\n", curveop.x->label, curve->x->note);
            Fmt (curveop.y1->note, "%s[a]<%s\n%s\n%s\n", curveop.y1->label, curve->y->note, curveop.note);

            Fmt (curveop.label, "Smooth(%s)", curve->attr.label);
            UpdateSmoothPanel (curve, TRUE);
            if (curveview.h2)
                DeleteGraphPlot (curveview.p1, CURVEVIEW_GRAPH, curveview.h2, VAL_DELAYED_DRAW);
            curveview.h2 = PlotXY (curveview.p1, CURVEVIEW_GRAPH, curveop.x->readings,
                    curveop.y1->readings, curveview.pts, VAL_DOUBLE, VAL_DOUBLE,
                    VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, 1, VAL_WHITE);
        }
        else MessagePopup ("Smooth Message", "Error during smooth calculation");
    }
    return 0;
}

int curveop_Smooth (double *kernal, int k_pts, double *y, int c_pts,
                     int offset, int pts, double *smooth_y)
{
    int i, win_offset;
    double k, *new_y;

    new_y = (double *) calloc (k_pts+c_pts, sizeof(double));
    if (!new_y) return !NoErr;

    win_offset = (k_pts-1)/2;

    Clear1D (new_y, c_pts);
    Convolve (kernal, k_pts, y, c_pts, new_y);
    Copy1D (new_y+offset+win_offset, pts, smooth_y);
    free (new_y);
    return NoErr;
}

int curveop_SmoothingKernal (int pts, double variance, double *kernal)
{
    int i;
    double z, k, expon, offset;
    z=0;
    offset = (double)(pts-1)/2;
    for (i = 0; i < pts; i++) {
        k = (double)i-offset;
        expon = -(k*k)/(2*variance);
        kernal[i]=exp(expon);
        z+=kernal[i];
    }
    return LinEv1D (kernal, pts, 1/z, 0.0, kernal);
}

int  SmoothControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int g, order;
    double *kernal, variance;
    switch (control)
    {
        default:
            if (event == EVENT_COMMIT)
            {
                UpdateSmoothPanel (NULL, FALSE);
                if (curveview.h2)
                {
                    DeleteGraphPlot (curveview.p1, CURVEVIEW_GRAPH,
                                 curveview.h2, VAL_IMMEDIATE_DRAW);
                    curveview.h2 = 0;
                }
            }
            break;
        case SMOOTH_ORDER:
            if (event == EVENT_COMMIT) {
                GetCtrlVal (panel, control, &order);
                if (!(order % 2)) {
                    MessagePopup ("Smooth Curve Message", "The order must be an odd number");
                    SetCtrlVal (panel, control, order - 1);
                }
            }
            break;
        case SMOOTH_KERNAL:
            if (event == EVENT_COMMIT) {
                GetCtrlVal (panel, SMOOTH_VARIANCE, &variance);
                GetCtrlVal (panel, SMOOTH_ORDER, &order);
                kernal = (double *) calloc (order, sizeof(double));
                if (curveop_SmoothingKernal (order, variance, kernal) == NoErr)
                    YGraphPopup ("Smoothing Kernal", kernal, order, VAL_DOUBLE);
                else MessagePopup ("Smoothing Kernal Message", "Error during kernal calculation");
                free (kernal);
            }
            break;
        case SMOOTH_GRAPHS:
            if (event == EVENT_COMMIT)
            {
                GetCtrlIndex (panel, control, &g);
                curveop.graph = graphlist_GetItem (g);
            }
            break;
    }
    return 0;
}

void InitSmoothCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    curveop.UpdatePanel = UpdateSmoothPanel;

    curveop_InitOpPanel (SMOOTH, SMOOTH_GRAPHS, FALSE, FALSE);

    SetPanelAttribute (curveview.p1, ATTR_TITLE, "Smooth Curves");

    curveop.DoBatch = BatchSmoothCallback;

    DisplayPanel (curveview.p2);
    SetPanelPos (curveview.p1, VAL_AUTO_CENTER, VAL_AUTO_CENTER);
    InstallPopup (curveview.p1);
}

void UpdateSmoothPanel (curvePtr curve, int input)
{
    SetInputMode (curveview.p2, SMOOTH_KEEP, input);
    SetInputMode (curveview.p2, SMOOTH_GRAPHS, input);
}

int  BatchFilterCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int c, checked, n;
    channelPtr x = NULL, y = NULL;
    curvePtr curve, newcurve;
    double init, final;
    if (event == EVENT_COMMIT)
    {
        GetNumListItems (panel, BATCH_CURVES, &n);
        for (c = 0; c < n; c++)
        {
            IsListItemChecked (panel, BATCH_CURVES, 0, &checked);
            DeleteListItem (panel, BATCH_CURVES, 0, 1);
            if (checked)
            {
                curve = curvelist_GetItem (curveG.curves->list, c);
                curveop_ClearChannel (x);
                curveop_ClearChannel (y);

                curveop_CalcFreq (curve, FALSE);

                Fmt (curveop.note, "");
                x = channel_Create();
                y = channel_Create();

                if (x && y &&
                    channel_AllocMem (x, curveview.pts) &&
                    channel_AllocMem (y, curveview.pts) &&
                    (Filter_Compute (curveop.freq, curveview.pts,
                             curve->y->readings + curve->offset + curveview.offset,
                             y->readings, curveop.note) == NoErr))
                {
                    Subset1D (curve->x->readings, curve->curvepts,
                      curveview.offset + curve->offset, curveview.pts,
                      x->readings);
                    Fmt (x->label, "Subset(%s)", curve->x->label);
                    Fmt (y->label, "Filter(%s)", curve->y->label);

                    Fmt (x->note, "%s[a]<%s\n%s\n", x->label, curve->x->note);
                    Fmt (y->note, "%s[a]<%s\n%s\n%s\n", y->label, curve->y->note, curveop.note);

                    newcurve = curve_Create();
                    if (newcurve) {
                        curve_CopyAttrs (curve, newcurve);
                        Fmt (newcurve->attr.label, "Filter(%s)", curve->attr.label);
                        newcurve->curvepts = curveview.pts;
                        newcurve->pts = curveview.pts;
                        newcurve->offset = 0;
                        newcurve->x0.reading = curve->x0.reading;
                        Fmt (newcurve->x0.label,curve->x0.label);
                        Fmt (newcurve->attr.note, "%s[a]<%s\n", curveop.note);

                        newcurve->x = x;
                        if (!x->curves.nItems) channellist_AddChannel (x);
                        channel_AddCurve (newcurve->x, newcurve->attr.label, curveop.graph->title);

                        newcurve->y = y;
                        if (!y->curves.nItems) channellist_AddChannel (y);
                        channel_AddCurve (newcurve->y, newcurve->attr.label, curveop.graph->title);

                        curvelist_AddCurve (&curveop.graph->curves, newcurve);
                        curve_Plot (curveop.graph->p, GRAPH_GRAPH, newcurve, NULL);
                    }
                    else
                    {
                        util_OutofMemory ("Batch Message");
                        break;
                    }
                }
                else
                {
                    util_OutofMemory ("Batch Message");
                    break;
                }
            }
            else
            {
                GetNumCheckedItems (panel, BATCH_CURVES, &checked);
                if (!checked) break;
            }
        }
        
        DiscardPanel(panel);
    }
    return 0;
}

int  FilterCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int i;
    curvePtr curve;
    if (event == EVENT_COMMIT)
    {
        curveop_ClearChannel (curveop.x);
        curveop_ClearChannel (curveop.y1);

        GetCtrlIndex (curveview.p1, CURVEVIEW_CURVES, &i);
        curve = curvelist_GetItem (curveG.curves->list, i);
        SetCtrlAttribute (panel, FILTER_KEEP, ATTR_CALLBACK_DATA, curve);

        Fmt (curveop.note, "");
        curveop.x = channel_Create();
        curveop.y1 = channel_Create();
        if (curveop.x && curveop.y1 &&
            channel_AllocMem (curveop.x, curveview.pts) &&
            channel_AllocMem (curveop.y1, curveview.pts) &&
            (Filter_Compute (curveop.freq, curveview.pts,
                             curve->y->readings + curve->offset + curveview.offset,
                             curveop.y1->readings, curveop.note) == NoErr))
        {
            Subset1D (curve->x->readings, curve->curvepts,
                      curveview.offset + curve->offset, curveview.pts,
                      curveop.x->readings);
            Fmt (curveop.x->label, "Subset(%s)", curve->x->label);
            Fmt (curveop.y1->label, "Filter(%s)", curve->y->label);

            Fmt (curveop.x->note, "%s[a]<%s\n%s\n", curveop.x->label, curve->x->note);
            Fmt (curveop.y1->note, "%s[a]<%s\n%s\n%s\n", curveop.y1->label, curve->y->note, curveop.note);

            Fmt (curveop.label, "Filter(%s)", curve->attr.label);
            UpdateFilterPanel (curve, TRUE);
            if (curveview.h2)
                DeleteGraphPlot (curveview.p1, CURVEVIEW_GRAPH, curveview.h2, VAL_DELAYED_DRAW);
            curveview.h2 = PlotXY (curveview.p1, CURVEVIEW_GRAPH, curveop.x->readings,
                    curveop.y1->readings, curveview.pts, VAL_DOUBLE, VAL_DOUBLE,
                    VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, 1, VAL_WHITE);
        }
        else MessagePopup ("Filter Message", "Error during filter calculation");
    }
    return 0;
}

int Filter_Compute (double fs, int pts, double *y, double *filter_y, char *note)
{
    double fl, fh, ripple, atten;
    int order, err, fType, fMode;

    GetCtrlVal (curveview.p2, FILTER_TYPE, &fType);
    GetCtrlVal (curveview.p2, FILTER_MODE, &fMode);
    GetCtrlVal (curveview.p2, FILTER_LOWFREQ, &fl);
    GetCtrlVal (curveview.p2, FILTER_HIGHFREQ, &fh);
    GetCtrlVal (curveview.p2, FILTER_ORDER, &order);
    GetCtrlVal (curveview.p2, FILTER_RIPPLE, &ripple);
    GetCtrlVal (curveview.p2, FILTER_ATTEN, &atten);

    err = NoErr;
    switch (fType)
    {
        case BUTTERWORTH:
            switch (fMode)
            {
                case BANDSTOP:
                    err = Bw_BSF (y, pts, fs, fl, fh, order, filter_y);
                    Fmt (note, "%s[a]<Butterworth: Bandstop\n"
                               "fs: %f[e2p5]\n"
                               "fl: %f[e2p5]\n"
                               "fh: %f[e2p5]\n"
                               "order: %i\n", fs, fl, fh, order);
                    break;
                case BANDPASS:
                    err = Bw_BPF (y, pts, fs, fl, fh, order, filter_y);
                    Fmt (note, "%s[a]<Butterworth: Bandstop\n"
                               "fs: %f[e2p5]\n"
                               "fl: %f[e2p5]\n"
                               "fh: %f[e2p5]\n"
                               "order: %i\n", fs, fl, fh, order);
                    break;
                case HIGHPASS:
                    err = Bw_HPF (y, pts, fs, fh, order, filter_y);
                    Fmt (note, "%s[a]<Butterworth: Bandstop\n"
                               "fs: %f[e2p5]\n"
                               "fh: %f[e2p5]\n"
                               "order: %i\n", fs, fh, order);
                    break;
                case LOWPASS:
                    err = Bw_LPF (y, pts, fs, fl, order, filter_y);
                    Fmt (note, "%s[a]<Butterworth: Bandstop\n"
                               "fs: %f[e2p5]\n"
                               "fl: %f[e2p5]\n"
                               "order: %i\n", fs, fl, order);
                    break;
            }
            break;
        case CHEBYSHEV:
            switch (fMode)
            {
                case BANDSTOP:
                    err = Ch_BSF (y, pts, fs, fl, fh, ripple, order, filter_y);
                    Fmt (note, "%s[a]<Chebyshev: Bandstop\n"
                               "fs: %f[e2p5]\n"
                               "fl: %f[e2p5]\n"
                               "fh: %f[e2p5]\n"
                               "ripple(dB): %f[e2p5]\n"
                               "order     : %i\n", fs, fl, fh, ripple, order);
                    break;
                case BANDPASS:
                    err = Ch_BPF (y, pts, fs, fl, fh, ripple, order, filter_y);
                    Fmt (note, "%s[a]<Chebyshev: Bandpass\n"
                               "fs: %f[e2p5]\n"
                               "fl: %f[e2p5]\n"
                               "fh: %f[e2p5]\n"
                               "ripple(dB): %f[e2p5]\n"
                               "order     : %i\n", fs, fl, fh, ripple, order);
                    break;
                case HIGHPASS:
                    err = Ch_HPF (y, pts, fs, fh, ripple, order, filter_y);
                    Fmt (note, "%s[a]<Chebyshev: Highpass\n"
                               "fs: %f[e2p5]\n"
                               "fh: %f[e2p5]\n"
                               "ripple(dB): %f[e2p5]\n"
                               "order     : %i\n", fs, fh, ripple, order);
                    break;
                case LOWPASS:
                    err = Ch_LPF (y, pts, fs, fl, ripple, order, filter_y);
                    Fmt (note, "%s[a]<Chebyshev: Lowpass\n"
                               "fs: %f[e2p5]\n"
                               "fl: %f[e2p5]\n"
                               "ripple(dB): %f[e2p5]\n"
                               "order     : %i\n", fs, fl, ripple, order);
                    break;
            }
            break;
        case INV_CHEBYSHEV:
            switch (fMode)
            {
                case BANDSTOP:
                    err = InvCh_BSF (y, pts, fs, fl, fh, atten, order, filter_y);
                    Fmt (note, "%s[a]<Inverse Chebyshev: Bandstop\n"
                               "fs: %f[e2p5]\n"
                               "fl: %f[e2p5]\n"
                               "fh: %f[e2p5]\n"
                               "atten(dB): %f[e2p5]\n"
                               "order    : %i\n", fs, fl, fh, atten, order);
                    break;
                case BANDPASS:
                    err = InvCh_BPF (y, pts, fs, fl, fh, atten, order, filter_y);
                    Fmt (note, "%s[a]<Inverse Chebyshev: Bandpass\n"
                               "fs: %f[e2p5]\n"
                               "fl: %f[e2p5]\n"
                               "fh: %f[e2p5]\n"
                               "atten(dB): %f[e2p5]\n"
                               "order    : %i\n", fs, fl, fh, atten, order);
                    break;
                case HIGHPASS:
                    err = InvCh_HPF (y, pts, fs, fh, atten, order, filter_y);
                    Fmt (note, "%s[a]<Inverse Chebyshev: Highpass\n"
                               "fs: %f[e2p5]\n"
                               "fh: %f[e2p5]\n"
                               "atten(dB): %f[e2p5]\n"
                               "order    : %i\n", fs, fh, atten, order);
                    break;
                case LOWPASS:
                    err = InvCh_LPF (y, pts, fs, fl, atten, order, filter_y);
                    Fmt (note, "%s[a]<Inverse Chebyshev: Lowpass\n"
                               "fs: %f[e2p5]\n"
                               "fl: %f[e2p5]\n"
                               "atten(dB): %f[e2p5]\n"
                               "order    : %i\n", fs, fl, atten, order);
                    break;
            }
            break;
        case ELLIPTIC:
            switch (fMode)
            {
                case BANDSTOP:
                    err = Elp_BSF (y, pts, fs, fl, fh, ripple, atten, order, filter_y);
                    Fmt (note, "%s[a]<Elliptic: Bandstop\n"
                               "fs: %f[e2p5]\n"
                               "fl: %f[e2p5]\n"
                               "fh: %f[e2p5]\n"
                               "ripple(dB): %f[e2p5]\n"
                               "atten(dB) : %f[e2p5]\n"
                               "order     : %i\n", fs, fl, fh, ripple, atten, order);
                    break;
                case BANDPASS:
                    err = Elp_BPF (y, pts, fs, fl, fh, ripple, atten, order, filter_y);
                    Fmt (note, "%s[a]<Elliptic: Bandpass\n"
                               "fs: %f[e2p5]\n"
                               "fl: %f[e2p5]\n"
                               "fh: %f[e2p5]\n"
                               "ripple(dB): %f[e2p5]\n"
                               "atten(dB): %f[e2p5]\n"
                               "order    : %i\n", fs, fl, fh, ripple, atten, order);
                    break;
                case HIGHPASS:
                    err = Elp_HPF (y, pts, fs, fh, ripple, atten, order, filter_y);
                    Fmt (note, "%s[a]<Elliptic: Highpass\n"
                               "fs: %f[e2p5]\n"
                               "fh: %f[e2p5]\n"
                               "ripple(dB): %f[e2p5]\n"
                               "atten(dB): %f[e2p5]\n"
                               "order    : %i\n", fs, fh, ripple, atten, order);
                    break;
                case LOWPASS:
                    err = Elp_LPF (y, pts, fs, fl, ripple, atten, order, filter_y);
                    Fmt (note, "%s[a]<Elliptic: Lowpass\n"
                               "fs: %f[e2p5]\n"
                               "fl: %f[e2p5]\n"
                               "ripple(dB): %f[e2p5]\n"
                               "atten(dB): %f[e2p5]\n"
                               "order    : %i\n", fs, fl, ripple, atten, order);
                    break;
            }
            break;
        default:
            Copy1D (y, pts, filter_y);
            break;
    }
    return err;
}

int  FilterControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int g;
    switch (control)
    {
        default:
            if (event == EVENT_COMMIT)
            {
                UpdateFilterPanel (NULL, FALSE);
                if (curveview.h2)
                {
                    DeleteGraphPlot (curveview.p1, CURVEVIEW_GRAPH,
                                 curveview.h2, VAL_IMMEDIATE_DRAW);
                    curveview.h2 = 0;
                }
            }
            break;
        case FILTER_GRAPHS:
            if (event == EVENT_COMMIT)
            {
                GetCtrlIndex (panel, control, &g);
                curveop.graph = graphlist_GetItem (g);
            }
            break;
    }
    return 0;
}

void InitFilterCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    curveop.UpdatePanel = UpdateFilterPanel;

    curveop_InitOpPanel (FILTER, FILTER_GRAPHS, FALSE, FALSE);

    SetPanelAttribute (curveview.p1, ATTR_TITLE, "Filter Curves");

    curveop.DoBatch = BatchFilterCallback;

    DisplayPanel (curveview.p2);
    SetPanelPos (curveview.p1, VAL_AUTO_CENTER, VAL_AUTO_CENTER);
    InstallPopup (curveview.p1);
}

void UpdateFilterPanel (curvePtr curve, int input)
{
    if (curve) curveop_CalcFreq (curve, FILTER_SAMPFREQ);

    Filter_Init_Type();
    Filter_Init_Mode();

    SetInputMode (curveview.p2, FILTER_KEEP, input);
    SetInputMode (curveview.p2, FILTER_GRAPHS, input);
}

void Filter_Init_Mode (void)
{
    int fType, fMode;

    GetCtrlVal (curveview.p2, FILTER_TYPE, &fType);
    GetCtrlVal (curveview.p2, FILTER_MODE, &fMode);
    if (fType != FILTER_NONE) /* NONE */
    switch (fMode)
    {
        case BANDSTOP:
            SetInputMode (curveview.p2, FILTER_LOWFREQ, TRUE);
            SetInputMode (curveview.p2, FILTER_HIGHFREQ, TRUE);
            break;
        case BANDPASS:
            SetInputMode (curveview.p2, FILTER_LOWFREQ, TRUE);
            SetInputMode (curveview.p2, FILTER_HIGHFREQ, TRUE);
            break;
        case HIGHPASS:
            SetInputMode (curveview.p2, FILTER_LOWFREQ, FALSE);
            SetInputMode (curveview.p2, FILTER_HIGHFREQ, TRUE);
            break;
        case LOWPASS:
            SetInputMode (curveview.p2, FILTER_LOWFREQ, TRUE);
            SetInputMode (curveview.p2, FILTER_HIGHFREQ, FALSE);
            break;
    }
}

void Filter_Init_Type (void)
{
    int fType;
    GetCtrlVal (curveview.p2, FILTER_TYPE, &fType);
    switch (fType)
    {
        case BUTTERWORTH:
            SetInputMode (curveview.p2, FILTER_MODE, TRUE);
            SetInputMode (curveview.p2, FILTER_ORDER, TRUE);
            SetInputMode (curveview.p2, FILTER_RIPPLE, FALSE);
            SetInputMode (curveview.p2, FILTER_ATTEN, FALSE);
            break;
        case CHEBYSHEV:
            SetInputMode (curveview.p2, FILTER_MODE, TRUE);
            SetInputMode (curveview.p2, FILTER_ORDER, TRUE);
            SetInputMode (curveview.p2, FILTER_RIPPLE, TRUE);
            SetInputMode (curveview.p2, FILTER_ATTEN, FALSE);
            break;
        case INV_CHEBYSHEV:
            SetInputMode (curveview.p2, FILTER_MODE, TRUE);
            SetInputMode (curveview.p2, FILTER_ORDER, TRUE);
            SetInputMode (curveview.p2, FILTER_RIPPLE, FALSE);
            SetInputMode (curveview.p2, FILTER_ATTEN, TRUE);
            break;
        case ELLIPTIC:
            SetInputMode (curveview.p2, FILTER_MODE, TRUE);
            SetInputMode (curveview.p2, FILTER_ORDER, TRUE);
            SetInputMode (curveview.p2, FILTER_RIPPLE, TRUE);
            SetInputMode (curveview.p2, FILTER_ATTEN, TRUE);
            break;
        case FILTER_NONE:
            SetInputMode (curveview.p2, FILTER_MODE, FALSE);
            SetInputMode (curveview.p2, FILTER_ORDER, FALSE);
            SetInputMode (curveview.p2, FILTER_RIPPLE, FALSE);
            SetInputMode (curveview.p2, FILTER_ATTEN, FALSE);
            SetInputMode (curveview.p2, FILTER_LOWFREQ, FALSE);
            SetInputMode (curveview.p2, FILTER_HIGHFREQ, FALSE);
            break;
    }
}

int  AscendingSortCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int c, checked, n, loop1, loop2, i, sorted;
    curvePtr curve;
    double temp;

    if (event == EVENT_COMMIT) {
        GetNumListItems (panel, BATCH_CURVES, &n);
        for (c = 0; c < n; c++) {
            IsListItemChecked (panel, BATCH_CURVES, 0, &checked);
            DeleteListItem (panel, BATCH_CURVES, 0, 1);
            if (checked) {
                curve = curvelist_GetItem (curveG.curves->list, c);
                if (curve->x->pts == curve->y->pts) {
                    for (loop1 = 0; loop1 < curve->curvepts; loop1++) {
                        sorted = TRUE;
                        for (loop2 = 1; loop2 < (curve->curvepts-loop1); loop2++) {
                            if (curve->x->readings[loop2-1] > curve->x->readings[loop2]) {
                                temp = curve->x->readings[loop2-1];
                                curve->x->readings[loop2-1] = curve->x->readings[loop2];
                                curve->x->readings[loop2]=temp;

                                temp = curve->y->readings[loop2-1];
                                curve->y->readings[loop2-1]=curve->y->readings[loop2];
                                curve->y->readings[loop2]=temp;

                                if (sorted) sorted = FALSE;
                            }
                        }
                        if (sorted) break;
                    }

                    Fmt (curve->x->note, "%s[a]<Ascending Sort(%s)\n", curve->x->label);
                    Fmt (curve->y->note, "%s[a]<Ascending Sort(%s)\n", curve->y->label);
                    Fmt (curve->attr.note, "%s[a]<Ascending Sort(%s)\n", curve->attr.label);
                }
            }
            else {
                GetNumCheckedItems (panel, BATCH_CURVES, &checked);
                if (!checked) break;
            }
        }
        
        DiscardPanel(panel);
    }
    return 0;
}

int  DescendingSortCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int c, checked, n, loop1, loop2, i, sorted;
    curvePtr curve;
    double temp;

    if (event == EVENT_COMMIT) {
        GetNumListItems (panel, BATCH_CURVES, &n);
        for (c = 0; c < n; c++) {
            IsListItemChecked (panel, BATCH_CURVES, 0, &checked);
            DeleteListItem (panel, BATCH_CURVES, 0, 1);
            if (checked) {
                curve = curvelist_GetItem (curveG.curves->list, c);
                if (curve->x->pts == curve->y->pts) {
                    for (loop1 = 0; loop1 < curve->curvepts; loop1++) {
                        sorted = TRUE;
                        for (loop2 = 1; loop2 < (curve->curvepts-loop1); loop2++) {
                            if (curve->x->readings[loop2-1] < curve->x->readings[loop2]) {
                                temp = curve->x->readings[loop2-1];
                                curve->x->readings[loop2-1] = curve->x->readings[loop2];
                                curve->x->readings[loop2]=temp;

                                temp = curve->y->readings[loop2-1];
                                curve->y->readings[loop2-1]=curve->y->readings[loop2];
                                curve->y->readings[loop2]=temp;

                                if (sorted) sorted = FALSE;
                            }
                        }
                        if (sorted) break;
                    }

                    Fmt (curve->x->note, "%s[a]<Descending Sort(%s)\n", curve->x->label);
                    Fmt (curve->y->note, "%s[a]<Descending Sort(%s)\n", curve->y->label);
                    Fmt (curve->attr.note, "%s[a]<Descending Sort(%s)\n", curve->attr.label);
                }
            }
            else {
                GetNumCheckedItems (panel, BATCH_CURVES, &checked);
                if (!checked) break;
            }
        }
        
        DiscardPanel(panel);
    }
    return 0;
}

void InitSortCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    int batch_p, c, g;
    curvePtr curve;
    char title[256];

    batch_p = LoadPanel (0, "curveopu.uir", BATCH);
    
    SetPanelPos (batch_p, VAL_AUTO_CENTER, VAL_AUTO_CENTER);
    util_InitClose (batch_p, BATCH_CANCEL, TRUE);

    graphlist_Copy (batch_p, BATCH_GRAPHS);
    GetCtrlIndex (batch_p, BATCH_GRAPHS, &g);
    curveop.graph = graphlist_GetItem (g);

    ClearListCtrl (batch_p, BATCH_CURVES);
    for (c = 0; c < curveG.curves->list.nItems; c++) {
        curve = curvelist_GetItem (curveG.curves->list, c);
        InsertListItem (batch_p, BATCH_CURVES, -1,
                        curve_CompleteListItem(curve, 0, curve->curvepts), c);
    }
    if (menuItem == CURVEMENUS_PROC_SORT_ASC) {
        Fmt (title, "Sort Curve Data (Ascending)");
        InstallCtrlCallback (batch_p, BATCH_GO, AscendingSortCallback, 0);
    } else {
        Fmt (title, "Sort Curve Data (Descending)");
        InstallCtrlCallback (batch_p, BATCH_GO, DescendingSortCallback, 0);
    }
    SetPanelAttribute (batch_p, ATTR_TITLE, title);
    InstallPopup (batch_p);
}

int  ReverseCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int c, checked, n;
    curvePtr curve;

    if (event == EVENT_COMMIT) {
        GetNumListItems (panel, BATCH_CURVES, &n);
        for (c = 0; c < n; c++) {
            IsListItemChecked (panel, BATCH_CURVES, 0, &checked);
            DeleteListItem (panel, BATCH_CURVES, 0, 1);
            if (checked) {
                curve = curvelist_GetItem (curveG.curves->list, c);
                if (curve->x->pts == curve->y->pts) {

                    Reverse (curve->x->readings, curve->curvepts, curve->x->readings);
                    Reverse (curve->y->readings, curve->curvepts, curve->y->readings);

                    Fmt (curve->x->note, "%s[a]<Reverse(%s)\n", curve->x->label);
                    Fmt (curve->y->note, "%s[a]<Reverse(%s)\n", curve->y->label);
                    Fmt (curve->attr.note, "%s[a]<Reverse(%s)\n", curve->attr.label);
                }
            }
            else
            {
                GetNumCheckedItems (panel, BATCH_CURVES, &checked);
                if (!checked) break;
            }
        }
        
        DiscardPanel(panel);
    }
    return 0;
}

void InitReverseCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    int batch_p, c, g;
    curvePtr curve;

    batch_p = LoadPanel (0, "curveopu.uir", BATCH);
    
    SetPanelPos (batch_p, VAL_AUTO_CENTER, VAL_AUTO_CENTER);
    util_InitClose (batch_p, BATCH_CANCEL, TRUE);

    graphlist_Copy (batch_p, BATCH_GRAPHS);
    GetCtrlIndex (batch_p, BATCH_GRAPHS, &g);
    curveop.graph = graphlist_GetItem (g);

    ClearListCtrl (batch_p, BATCH_CURVES);
    for (c = 0; c < curveG.curves->list.nItems; c++) {
        curve = curvelist_GetItem (curveG.curves->list, c);
        InsertListItem (batch_p, BATCH_CURVES, -1,
                        curve_CompleteListItem(curve, 0, curve->curvepts), c);
    }
    InstallCtrlCallback (batch_p, BATCH_GO, ReverseCallback, 0);
    SetPanelAttribute (batch_p, ATTR_TITLE, "Reverse Curve");
    InstallPopup (batch_p);
}

int  BatchDifferenceCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int c, checked, n;
    channelPtr x = NULL, y = NULL;
    curvePtr curve, newcurve;
    double init, final;

    if (event == EVENT_COMMIT)
    {
        GetNumListItems (panel, BATCH_CURVES, &n);
        for (c = 0; c < n; c++)
        {
            IsListItemChecked (panel, BATCH_CURVES, 0, &checked);
            DeleteListItem (panel, BATCH_CURVES, 0, 1);
            if (checked)
            {
                curve = curvelist_GetItem (curveG.curves->list, c);
                curveop_ClearChannel (x);
                curveop_ClearChannel (y);
                init = curve->y->readings[curve->offset+curveview.offset];
                final = curve->y->readings[curve->offset+curveview.offset+curveview.pts-1];
                curveop_CalcInterval(curve, FALSE);

                Fmt (curveop.note, "");
                x = channel_Create();
                y = channel_Create();

                if (x && y &&
                    channel_AllocMem (x, curveview.pts) &&
                    channel_AllocMem (y, curveview.pts) &&
                    (Difference (curve->y->readings + curveview.offset + curve->offset,
                                  curveview.pts, curveop.interval,
                                  init, final, y->readings) == NoErr))
                {
                    Subset1D (curve->x->readings, curve->curvepts,
                          curveview.offset + curve->offset, curveview.pts,
                          x->readings);
                    Fmt (x->label, "Subset(%s)", curve->x->label);
                    Fmt (y->label, "Difference(%s)", curve->y->label);

                    Fmt (x->note, "%s[a]<%s\n%s\n", x->label, curve->x->note);
                    Fmt (y->note, "%s[a]<%s\n%s\n%s\n", y->label, curve->y->note, curveop.note);

                    newcurve = curve_Create();
                    if (newcurve) {
                        curve_CopyAttrs (curve, newcurve);
                        Fmt (newcurve->attr.label, "Difference(%s)", curve->attr.label);
                        newcurve->curvepts = curveview.pts;
                        newcurve->pts = curveview.pts;
                        newcurve->offset = 0;
                        newcurve->x0.reading = curve->x0.reading;
                        Fmt (newcurve->x0.label,curve->x0.label);

                        newcurve->x = x;
                        if (!x->curves.nItems) channellist_AddChannel (x);
                        channel_AddCurve (newcurve->x, newcurve->attr.label, curveop.graph->title);

                        newcurve->y = y;
                        if (!y->curves.nItems) channellist_AddChannel (y);
                        channel_AddCurve (newcurve->y, newcurve->attr.label, curveop.graph->title);

                        curvelist_AddCurve (&curveop.graph->curves, newcurve);
                        curve_Plot (curveop.graph->p, GRAPH_GRAPH, newcurve, NULL);
                    }
                    else
                    {
                        util_OutofMemory ("Batch Message");
                        break;
                    }
                }
                else
                {
                    util_OutofMemory ("Batch Message");
                    break;
                }
            }
            else
            {
                GetNumCheckedItems (panel, BATCH_CURVES, &checked);
                if (!checked) break;
            }
        }
        
        DiscardPanel(panel);
    }
    return 0;
}

int  BatchIntegrateCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int c, checked, n;
    channelPtr x = NULL, y = NULL;
    curvePtr curve, newcurve;
    double init, final;

    if (event == EVENT_COMMIT)
    {
        GetNumListItems (panel, BATCH_CURVES, &n);
        for (c = 0; c < n; c++)
        {
            IsListItemChecked (panel, BATCH_CURVES, 0, &checked);
            DeleteListItem (panel, BATCH_CURVES, 0, 1);
            if (checked)
            {
                curve = curvelist_GetItem (curveG.curves->list, c);
                curveop_ClearChannel (x);
                curveop_ClearChannel (y);
                init = curve->y->readings[curve->offset+curveview.offset];
                final = curve->y->readings[curve->offset+curveview.offset+curveview.pts-1];
                curveop_CalcInterval(curve, FALSE);

                Fmt (curveop.note, "");
                x = channel_Create();
                y = channel_Create();

                if (x && y &&
                    channel_AllocMem (x, curveview.pts) &&
                    channel_AllocMem (y, curveview.pts) &&
                    (Integrate (curve->y->readings + curveview.offset + curve->offset,
                                  curveview.pts, curveop.interval,
                                  init, final, y->readings) == NoErr))
                {
                    Subset1D (curve->x->readings, curve->curvepts,
                          curveview.offset + curve->offset, curveview.pts,
                          x->readings);
                    Fmt (x->label, "Subset(%s)", curve->x->label);
                    Fmt (y->label, "Integrate(%s)", curve->y->label);

                    Fmt (x->note, "%s[a]<%s\n%s\n", x->label, curve->x->note);
                    Fmt (y->note, "%s[a]<%s\n%s\n%s\n", y->label, curve->y->note, curveop.note);

                    newcurve = curve_Create();
                    if (newcurve) {
                        curve_CopyAttrs (curve, newcurve);
                        Fmt (newcurve->attr.label, "Integrate(%s)", curve->attr.label);
                        newcurve->curvepts = curveview.pts;
                        newcurve->pts = curveview.pts;
                        newcurve->offset = 0;
                        newcurve->x0.reading = curve->x0.reading;
                        Fmt (newcurve->x0.label,curve->x0.label);

                        newcurve->x = x;
                        if (!x->curves.nItems) channellist_AddChannel (x);
                        channel_AddCurve (newcurve->x, newcurve->attr.label, curveop.graph->title);

                        newcurve->y = y;
                        if (!y->curves.nItems) channellist_AddChannel (y);
                        channel_AddCurve (newcurve->y, newcurve->attr.label, curveop.graph->title);

                        curvelist_AddCurve (&curveop.graph->curves, newcurve);
                        curve_Plot (curveop.graph->p, GRAPH_GRAPH, newcurve, NULL);
                    }
                    else
                    {
                        util_OutofMemory ("Batch Message");
                        break;
                    }
                }
                else
                {
                    util_OutofMemory ("Batch Message");
                    break;
                }
            }
            else
            {
                GetNumCheckedItems (panel, BATCH_CURVES, &checked);
                if (!checked) break;
            }
        }
        
        DiscardPanel(panel);
    }
    return 0;
}

int  DifferenceCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int i;
    double init, final;
    curvePtr curve;
    if (event == EVENT_COMMIT)
    {
        curveop_ClearChannel (curveop.x);
        curveop_ClearChannel (curveop.y1);

        GetCtrlIndex (curveview.p1, CURVEVIEW_CURVES, &i);
        curve = curvelist_GetItem (curveG.curves->list, i);
        SetCtrlAttribute (panel, INTEGDIFF_KEEP, ATTR_CALLBACK_DATA, curve);

        GetCtrlVal (panel, INTEGDIFF_INIT, &init);
        GetCtrlVal (panel, INTEGDIFF_FINAL, &final);

        Fmt(curveop.note, "");
        curveop.x = channel_Create();
        curveop.y1 = channel_Create();
        if (curveop.x && curveop.y1 &&
            channel_AllocMem (curveop.x, curveview.pts) &&
            channel_AllocMem (curveop.y1, curveview.pts) &&
            (Difference (curve->y->readings + curve->offset+curveview.offset,
                              curveview.pts, curveop.interval,
                              init, final, curveop.y1->readings) == NoErr))
        {
            Subset1D (curve->x->readings, curve->curvepts,
                      curveview.offset + curve->offset, curveview.pts,
                      curveop.x->readings);
            Fmt (curveop.x->label, "Subset(%s)", curve->x->label);
            Fmt (curveop.y1->label, "Difference(%s)", curve->y->label);
            Fmt (curveop.label, "Difference(%s)", curve->attr.label);

            Fmt (curveop.x->note, "%s[a]<%s\n%s\n", curveop.x->label, curve->x->note);
            Fmt (curveop.y1->note, "%s[a]<%s\n%s\n%s\n", curveop.y1->label, curve->y->note, curveop.note);

            UpdateIntegDiffPanel (curve, TRUE);
            DeleteGraphPlot (panel, INTEGDIFF_GRAPH_RESULT, -1, VAL_DELAYED_DRAW);
            PlotXY (panel, INTEGDIFF_GRAPH_RESULT,
                    curveop.x->readings,
                    curveop.y1->readings,
                    curveview.pts, VAL_DOUBLE, VAL_DOUBLE, VAL_THIN_LINE,
                    VAL_EMPTY_SQUARE, VAL_SOLID, 1, VAL_YELLOW);
        }
        else MessagePopup ("Difference Message", "Error during difference calculation");
    }
    return 0;
}

int  IntegrateCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int i;
    double init, final;
    curvePtr curve;
    if (event == EVENT_COMMIT)
    {
        curveop_ClearChannel (curveop.x);
        curveop_ClearChannel (curveop.y1);

        GetCtrlIndex (curveview.p1, CURVEVIEW_CURVES, &i);
        curve = curvelist_GetItem (curveG.curves->list, i);
        SetCtrlAttribute (panel, INTEGDIFF_KEEP, ATTR_CALLBACK_DATA, curve);

        GetCtrlVal (panel, INTEGDIFF_INIT, &init);
        GetCtrlVal (panel, INTEGDIFF_FINAL, &final);

        Fmt(curveop.note, "");
        curveop.x = channel_Create();
        curveop.y1 = channel_Create();
        if (curveop.x && curveop.y1 &&
            channel_AllocMem (curveop.x, curveview.pts) &&
            channel_AllocMem (curveop.y1, curveview.pts) &&
            (Integrate (curve->y->readings + curveview.offset + curve->offset,
                              curveview.pts, curveop.interval,
                              init, final, curveop.y1->readings) == NoErr))
        {
            Subset1D (curve->x->readings, curve->curvepts,
                      curveview.offset + curve->offset, curveview.pts,
                      curveop.x->readings);
            Fmt (curveop.x->label, "Subset(%s)", curve->x->label);
            Fmt (curveop.y1->label, "Integrate(%s)", curve->y->label);

            Fmt (curveop.x->note, "%s[a]<%s\n%s\n", curveop.x->label, curve->x->note);
            Fmt (curveop.y1->note, "%s[a]<%s\n%s\n%s\n", curveop.y1->label, curve->y->note, curveop.note);

            Fmt (curveop.label, "Integrate(%s)", curve->attr.label);
            UpdateIntegDiffPanel (curve, TRUE);
            DeleteGraphPlot (panel, INTEGDIFF_GRAPH_RESULT, -1, VAL_DELAYED_DRAW);
            PlotXY (panel, INTEGDIFF_GRAPH_RESULT,
                    curveop.x->readings,
                    curveop.y1->readings,
                    curveview.pts, VAL_DOUBLE, VAL_DOUBLE, VAL_THIN_LINE,
                    VAL_EMPTY_SQUARE, VAL_SOLID, 1, VAL_YELLOW);
        }
        else MessagePopup ("Integrate Message", "Error during integration calculation");
    }
    return 0;
}

int  IntegDiffControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int g;
    switch (control)
    {
        case INTEGDIFF_INIT:
        case INTEGDIFF_FINAL:
            if (event == EVENT_COMMIT) UpdateIntegDiffPanel (NULL, FALSE);
            break;
        case INTEGDIFF_GRAPHS:
            if (event == EVENT_COMMIT)
            {
                GetCtrlIndex (panel, control, &g);
                curveop.graph = graphlist_GetItem (g);
            }
            break;
    }
    return 0;
}

void InitIntegDiffCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    curveop.UpdatePanel = UpdateIntegDiffPanel;

    curveop_InitOpPanel (INTEGDIFF, INTEGDIFF_GRAPHS, FALSE, TRUE);

    if (menuItem == CURVEMENUS_PROC_INTEG)
    {
        SetPanelAttribute (curveview.p1, ATTR_TITLE, "Integrate Curves");
        InstallCtrlCallback (curveview.p2, INTEGDIFF_GO,
                             IntegrateCallback, 0);
        curveop.DoBatch = BatchIntegrateCallback;
    }
    else
    {
        SetPanelAttribute (curveview.p1, ATTR_TITLE, "Differentiate Curves");
        InstallCtrlCallback (curveview.p2, INTEGDIFF_GO,
                             DifferenceCallback, 0);
        curveop.DoBatch = BatchDifferenceCallback;
    }

    DisplayPanel (curveview.p2);
    SetPanelPos (curveview.p1, VAL_AUTO_CENTER, VAL_AUTO_CENTER);
    InstallPopup (curveview.p1);
}

void UpdateIntegDiffPanel (curvePtr curve, int input)
{
    if (curve)
    {
        curveop_CalcInterval (curve, INTEGDIFF_SAMPINTERVAL);

        SetCtrlVal (curveview.p2, INTEGDIFF_INIT,
            curve->y->readings[curveview.offset+curve->offset]);
        SetCtrlVal (curveview.p2, INTEGDIFF_FINAL,
            curve->y->readings[curveview.pts - 1 + curveview.offset+curve->offset]);
    }

    SetInputMode (curveview.p2, INTEGDIFF_GRAPH_RESULT, input);
    SetInputMode (curveview.p2, INTEGDIFF_KEEP, input);
    SetInputMode (curveview.p2, INTEGDIFF_GRAPHS, input);
}

void InitOffsetCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    char buffer[30];
    int err, i;
    double offset;
    curvePtr curve;

    err = PromptPopup ("Curve Offset", "Enter offset value for curves in view:",
                 buffer, 29);
    if (err >= 0) {
        err = Scan (buffer, "%s>%f", &offset);
        if (err > 0){ /* ok */
            for (i = 0; i < curveG.curves->list.nItems; i++) {
                curve = curvelist_GetItem (curveG.curves->list, i);
                if (!curve->attr.hidden) {
                    curve_Hide(curveG.panel, curveG.control, curve, TRUE);
                    LinEv1D (curve->y->readings, curve->y->pts, 1.0, i*offset,
                             curve->y->readings);
                    Fmt (curve->y->note, "%s[a]<Offset: %f[e2p5]\n", offset*i);
                    curve_Plot(curveG.panel, curveG.control, curve, NULL);
                }
            }
        }
    }
}

int  BatchControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int c, checked, g;

    switch (control)
    {
        case BATCH_ALL:
            if (event == EVENT_COMMIT)
            {
                GetCtrlVal (panel, control, &checked);
                for (c = 0; c < curveG.curves->list.nItems; c++)
                    CheckListItem (panel, BATCH_CURVES, c, checked);
            }
            break;
        case BATCH_GRAPHS:
            if (event == EVENT_COMMIT)
            {
                GetCtrlIndex (panel, control, &g);
                curveop.graph = graphlist_GetItem (g);
            }
            break;
    }
    return 0;
}

int  InitBatchCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int batch_p, c, g;
    curvePtr curve;

    if (event == EVENT_COMMIT)
    {
        batch_p = LoadPanel (0, "curveopu.uir", BATCH);
        
        SetPanelPos (batch_p, VAL_AUTO_CENTER, VAL_AUTO_CENTER);
        util_InitClose (batch_p, BATCH_CANCEL, TRUE);

        graphlist_Copy (batch_p, BATCH_GRAPHS);
        GetCtrlIndex (batch_p, BATCH_GRAPHS, &g);
        curveop.graph = graphlist_GetItem (g);

        ClearListCtrl (batch_p, BATCH_CURVES);
        for (c = 0; c < curveG.curves->list.nItems; c++)
        {
            curve = curvelist_GetItem (curveG.curves->list, c);
            InsertListItem (batch_p, BATCH_CURVES, -1,
                            curve_CompleteListItem(curve, curveview.offset, curveview.pts), c);
        }
        InstallCtrlCallback (batch_p, BATCH_GO, curveop.DoBatch, 0);
        InstallPopup (batch_p);
    }
    return 0;
}

int  CreateY2CurveCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    curvePtr curve, oldCurve;

    if (event == EVENT_COMMIT)
    {
        oldCurve = callbackData;
        curve = curve_Create();
        if (curve) {
            curve_CopyAttrs (oldCurve, curve);
            Fmt (curve->attr.label, curveop.label);
            curve->curvepts = curveview.pts;
            curve->pts = curveview.pts;
            curve->offset = 0;
            curve->x0.reading = oldCurve->x0.reading;
            Fmt (curve->x0.label, oldCurve->x0.label);
            Fmt (curve->attr.note, "%s[a]<%s\n", curveop.note);

            curve->x = curveop.x;
            if (!curveop.x->curves.nItems) channellist_AddChannel (curveop.x);
            channel_AddCurve (curve->x, curve->attr.label, curveop.graph->title);

            curve->y = curveop.y2;
            if (!curveop.y2->curves.nItems) channellist_AddChannel (curveop.y2);
            channel_AddCurve (curve->y, curve->attr.label, curveop.graph->title);

            curvelist_AddCurve (&curveop.graph->curves, curve);
            curve_Plot (curveop.graph->p, GRAPH_GRAPH, curve, NULL);
        }
        else util_OutofMemory ("Keep Curve Message");
    }
    return 0;
}

int  CreateY1CurveCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    curvePtr curve, oldCurve;

    if (event == EVENT_COMMIT)
    {
        oldCurve = callbackData;
        curve = curve_Create();
        if (curve) {
            curve_CopyAttrs (oldCurve, curve);
            Fmt (curve->attr.label, curveop.label);
            curve->curvepts = curveview.pts;
            curve->pts = curveview.pts;
            curve->offset = 0;
            curve->x0.reading = oldCurve->x0.reading;
            Fmt (curve->x0.label, oldCurve->x0.label);
            Fmt (curve->attr.note, "%s\n%s\n", curve->attr.label, curveop.note);

            curve->x = curveop.x;
            if (!curveop.x->curves.nItems) channellist_AddChannel (curveop.x);
            channel_AddCurve (curve->x, curve->attr.label, curveop.graph->title);

            curve->y = curveop.y1;
            if (!curveop.y1->curves.nItems) channellist_AddChannel (curveop.y1);
            channel_AddCurve (curve->y, curve->attr.label, curveop.graph->title);

            curvelist_AddCurve (&curveop.graph->curves, curve);
            curve_Plot (curveop.graph->p, GRAPH_GRAPH, curve, NULL);
        }
        else util_OutofMemory ("Keep Curve Message");
    }
    return 0;
}

int  CurveOpWindowCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int min_i, max_i, offset;
    double min, max;
    curvePtr curve;

    if ((event == EVENT_VAL_CHANGED) || (event == EVENT_COMMIT))
    {
        curve = callbackData;
        switch (control)
        {
            case CURVEVIEW_OFFSET:
                GetCtrlVal (panel, control, &curveview.offset);
                SetCtrlVal (panel, CURVEVIEW_OFFSET_VAL, curveview.offset);
                break;
            case CURVEVIEW_PTS_1:
            case CURVEVIEW_PTS_2:
                GetCtrlVal (panel, control, &curveview.pts);
                if ((curveview.offset + curveview.pts) > curve->pts)
                {
                    curveview.offset = curve->pts - curveview.pts;
                    SetCtrlVal (panel, CURVEVIEW_OFFSET, curveview.offset);
                    SetCtrlVal (panel, CURVEVIEW_OFFSET_VAL, curveview.offset);
                }
                SetInputMode (panel, CURVEVIEW_OFFSET, !(curve->pts == curveview.pts));
                SetCtrlAttribute (panel, CURVEVIEW_OFFSET,
                      ATTR_MAX_VALUE, curve->pts - curveview.pts);
                break;
            case CURVEVIEW_DIR:
                Reverse (curve->x->readings, curve->curvepts, curve->x->readings);
                Reverse (curve->y->readings, curve->curvepts, curve->y->readings);

                curve->offset = (curve->curvepts - 1) - curve->offset - (curve->pts - 1);
                curveview.offset = (curve->pts - 1) - curveview.offset - (curveview.pts - 1);

                SetCtrlVal (panel, CURVEVIEW_OFFSET, curveview.offset);
                SetCtrlVal (panel, CURVEVIEW_OFFSET_VAL, curveview.offset);
                Fmt (curve->x->note, "%s[a]<Reversed\n");
                Fmt (curve->y->note, "%s[a]<Reversed\n");
                if (curveview.h2)
                {
                    DeleteGraphPlot (curveview.p1, CURVEVIEW_GRAPH, curveview.h2,
                         VAL_DELAYED_DRAW);
                    curveview.h2 = 0;
                }
                break;
        }
        curve_UpdateViewGraph (curve);
        curveop.UpdatePanel (curve, FALSE);
    }
    return 0;
}

int  CurveOpSelectCurveCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int i;
    curvePtr curve;

    if (event == EVENT_COMMIT)
    {
        GetCtrlVal (panel, control, &i);
        curve = curvelist_GetItem (curveG.curves->list, i);
        GetCtrlAttribute (panel, CURVEVIEW_PTS_2, ATTR_VISIBLE, &i);
        if (curveview.h2)
        {
            DeleteGraphPlot (curveview.p1, CURVEVIEW_GRAPH, curveview.h2,
                         VAL_DELAYED_DRAW);
            curveview.h2 = 0;
        }
        curve_UpdateViewPanel (curve, i);
        curveop.UpdatePanel(curve, FALSE);
    }
    return 0;
}

static void curveop_InitOpPanel (int panel, int graphs, int pts2, int right)
{
    curvePtr curve;
    int width1, width2, height1, height2, g;

    curve = curvelist_GetSelection();
    curve_InitViewPanel (pts2);
    curve_UpdateViewPanel (curve, pts2);
    SetInputMode (curveview.p1, CURVEVIEW_MASK, FALSE);

    curveview.p2 = LoadPanel (curveview.p1, "curveopu.uir", panel);
    

    GetPanelAttribute (curveview.p1, ATTR_WIDTH, &width1);
    GetPanelAttribute (curveview.p1, ATTR_HEIGHT, &height1);

    GetPanelAttribute (curveview.p2, ATTR_WIDTH, &width2);
    GetPanelAttribute (curveview.p2, ATTR_HEIGHT, &height2);
    if (right)
    {
        SetPanelPos (curveview.p2, (height1 - height2)/2, width1);
        SetPanelAttribute (curveview.p1, ATTR_WIDTH, width1 + width2 + 6);
    }
    else
    {
        SetPanelPos (curveview.p2, height1 + 6, (width1 - width2)/2);
        SetPanelAttribute (curveview.p1, ATTR_HEIGHT, height1 + height2 + 12);
    }

    if (graphs)
    {
        graphlist_Copy (curveview.p2, graphs);
        GetCtrlIndex (curveview.p2, graphs, &g);
        curveop.graph = graphlist_GetItem (g);
    }

    InstallCtrlCallback (curveview.p1, CURVEVIEW_CURVES,
                         CurveOpSelectCurveCallback, 0);
    InstallCtrlCallback (curveview.p1, CURVEVIEW_OFFSET,
                         CurveOpWindowCallback, curve);
    InstallCtrlCallback (curveview.p1, CURVEVIEW_PTS_1,
                         CurveOpWindowCallback, curve);
    InstallCtrlCallback (curveview.p1, CURVEVIEW_PTS_2,
                         CurveOpWindowCallback, curve);
    InstallCtrlCallback (curveview.p1, CURVEVIEW_DIR,
                         CurveOpWindowCallback, curve);

    curveop.UpdatePanel (curve, FALSE);
}

static void curveop_CalcInterval (curvePtr curve, int control)
{
    curveop.interval =
        fabs (curve->x->readings[curveview.pts - 1 + curveview.offset + curve->offset] -
              curve->x->readings[curveview.offset + curve->offset])/(curveview.pts - 1);
    if (control)
        SetCtrlVal (curveview.p2, control, curveop.interval);
}

static void curveop_CalcFreq (curvePtr curve, int control)
{
    curveop.freq =
        (curveview.pts - 1)/
        fabs (curve->x->readings[curveview.pts - 1 + curveview.offset + curve->offset] -
              curve->x->readings[curveview.offset + curve->offset]);
    if (control)
        SetCtrlVal (curveview.p2, control, curveop.freq);
}

static void curveop_ClearChannel(channelPtr chan)
{
    if (chan)
    {
        if (chan->curves.nItems == 0) /* not in channel list */
        {
            if (chan->readings) free (chan->readings);
            list_RemoveAllItems (&chan->curves, FALSE);
            list_RemoveAllItems (&chan->graphs, FALSE);
            free (chan);
        }
    }
}
