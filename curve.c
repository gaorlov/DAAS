#include <analysis.h>
#include <formatio.h>
#include <ansi_c.h>
#include <userint.h>
#include "util.h"
#include "utilu.h"
#include "list.h"
#include "listu.h"
#include "channel.h"
#include "channelu.h"
#include "changen.h"
#include "chanfnc.h"
#include "chanops.h"
#include "acqchan.h"
#include "curve.h"
#include "curveu.h"
#include "acqcrv.h"
#include "graph.h"

#define TRUE 1
#define FALSE 0

struct curveGStruct curveG;
struct curveviewStruct curveview;

int curveMenus = 0;

void        curveG_Init (int graph, int control, char *title, curvelistPtr curves);
void        curve_Exit (void);
curvePtr    curve_Create (void);
void        curve_Plot (int panel, int control, curvePtr c, void *graphP);
void        curve_Hide (int panel, int control, curvePtr c, int delayed);
void        curve_Save (curvePtr c);
curvePtr    curve_Load (char *graphtitle);
void        curve_CopyAttrs (curvePtr curve, curvePtr copy);
char        *curve_CompleteListItem (curvePtr curve, int offset, int pts);

void        curvelist_Init (curvelistPtr curves);
curvePtr    curvelist_GetItem (listType list, int i);
curvePtr    curvelist_GetSelection (void);
void        curvelist_UpdatePanel (curvelistType curve);
void        curvelist_AddCurve (curvelistPtr curves, curvePtr c);
void        curvelist_ReplaceCurve (curvePtr c);
void        curvelist_RemoveCurve (curvelistPtr curves, curvePtr c);
void        curvelist_Copy (listType curves, int panel, int control, int pulldown);

void    curve_InitViewPanel (int pts2);
void    curve_UpdateViewPanel (curvePtr curve, int pts2);
void    curve_UpdateViewGraph (curvePtr curve);

void    curve_InitEditPanel (curvePtr curve);

/*
static curvelistType t_curves;
static void t_Init (void);

main()
{
    utilG_Init();
    listG_Init();
    fileG_Init();
    channelG_Init();

    changen_Init();
    chanfunc_Init();
    chanops_Init();


    t_Init();
    curveG_Init (0, 0, "", &t_curves);
    curvelist_UpdatePanel (t_curves);
    DisplayPanel (t_curves.panel);

    RunUserInterface();
}

static void t_Init (void)
{
    int i, n;
    channelPtr x, y;
    curvePtr curve;

    curvelist_Init (&t_curves);

    for (n = 0; n < 3; n++)
    {
    x = channel_Create(); Fmt (x->label, "X");
    y = channel_Create(); Fmt (y->label, "Y");

    channel_AllocMem (x, 20);
    channel_AllocMem (y, 20);

        for (i = 0; i < 20; i++)
        {
            x->readings[i] = i;
            y->readings[i] = (double) rand()/32768;
        }

    channellist_AddChannel (x);
    channellist_AddChannel (y);

    curve = curve_Create();
    curve->x = x;
    curve->y = y;
    curve->curvepts = 20;
    curve->pts = 20;
    curve->offset = 0;

    Fmt (curve->attr.label, "Curve %i", n);
    channel_AddCurve(curve->x, curve->attr.label, "Sample Graph");
    channel_AddCurve(curve->y, curve->attr.label, "Sample Graph");

    curvelist_AddCurve (&t_curves, curve);
    }
}
*/

/*****************************************************************************/

int  CurveInfoCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    char msg[10], info[1000];
    curvePtr curve;
    int p, c;

    if (event == EVENT_COMMIT) {
        p=LoadPanel (0, "curveu.uir", CURVEINFO);
        
        util_InitClose (p, CURVEINFO_CLOSE, FALSE);

        SetPanelPos (p, VAL_AUTO_CENTER, VAL_AUTO_CENTER);

        ClearListCtrl (p, CURVEINFO_LIST);

        for (c = 0; c < curveG.curves->list.nItems; c++) {

            curve = curvelist_GetItem (curveG.curves->list, c);
            Fmt (info, "%s<\033fg%x[w6]\033p%dr%i", curve->attr.color, 20, c);
            Fmt (info, "%s[a]<\033p%dl%s", 25, curve->attr.label);

            if (curve->offset) Fmt (msg, "Yes"); else Fmt (msg, "No");
            Fmt (info, "%s[a]<\033p%dc%s", 200, msg);

            if (curve->attr.hidden) Fmt (msg, "Yes"); else Fmt (msg, "No");
            Fmt (info, "%s[a]<\033p%dc%s", 250, msg);

            Fmt (info, "%s[a]<\033p%dc%i", 300, curve->pts);

            if (curve->x->readings[curve->offset] > curve->x->readings[curve->offset+curve->pts-1])
                Fmt (msg, "DOWN"); else Fmt (msg, "UP");
            Fmt (info, "%s[a]<\033p%dc%s", 350, msg);

            Fmt (info, "%s[a]<\033p%dr%f[e2p4]", 450, curve->x0.reading);

            Fmt (info, "%s[a]<\033p%dl%s", 460, curve->attr.note);

            InsertListItem (p, CURVEINFO_LIST, -1, info, c);
        }
        InstallPopup (p);
    }
    return 0;
}

int  RemoveSelectedCurvesCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int n, checked, nChecked, i;
    curvePtr curve;
    nodePtr node;

    if ((event == EVENT_COMMIT) &&
        ConfirmPopup ("Curve Remove Message",
                      "This function cannot be undone.\n"
                      "Do you want to proceed?"))
    {
        GetNumCheckedItems (panel, LIST_ITEMS, &nChecked);
        if (!nChecked)
        {
            MessagePopup ("Remove Curves Message", "No curves selected to remove");
            
            DiscardPanel (listG.p);
            return 0;
        }

        node = curveG.curves->list.first;
        GetNumListItems (listG.p, LIST_ITEMS, &n);
        for (i = 0; i < n; i++) {
            IsListItemChecked (listG.p, LIST_ITEMS, 0, &checked);
            DeleteListItem (listG.p, LIST_ITEMS, 0, 1);
            curve = node->item; node = node->next;
            GetNumCheckedItems (listG.p, LIST_ITEMS, &nChecked);
            if (checked) {
                if (nChecked) curve_Hide (curveG.panel, curveG.control, curve, TRUE);
                else curve_Hide (curveG.panel, curveG.control, curve, FALSE);
                curvelist_RemoveCurve (curveG.curves, curve);
            }
            if (!nChecked) break;
        }
        curvelist_UpdatePanel(*curveG.curves);
        
        DiscardPanel (listG.p);
    }
    return 0;
}

int  InitRemoveCurvesCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    if (event == EVENT_COMMIT)
    {
        list_InitPanel("Remove Curves", "Remove");
        curvelist_Copy (curveG.curves->list, listG.p, LIST_ITEMS, FALSE);
        InstallCtrlCallback (listG.p, LIST_CONT, RemoveSelectedCurvesCallback, 0);
        InstallPopup (listG.p);
    }
    return 0;
}
int  SaveOneXCurvesCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)

{
    int filestatus, checked, col, row, nRows = 0, nCols = 0;
    char path[260];
    char header[16];
    curvePtr curve;

    if (event == EVENT_COMMIT) {
        filestatus = FileSelectPopup ("", "*.dat", "*.dat",
                                      "Save Data with common X column",
                                      VAL_SAVE_BUTTON, 0, 1, 1, 1, path);
        if ((filestatus == VAL_NEW_FILE_SELECTED) ||
            (filestatus == VAL_EXISTING_FILE_SELECTED)) {
            fileHandle.analysis = util_OpenFile(path, FILE_WRITE, TRUE);
/*
    - write first row w/ x0 reading and 0.0 as filler
    - find the maximum # of pts to be saved -> nRows
*/			strcpy (header, "Parameter:") ;
            WriteFile (fileHandle.analysis, header, 10);
            for (col = 0; col < curveG.curves->list.nItems; col++) {
                IsListItemChecked (panel, CURVESAVE_LIST, col, &checked);
                if (checked) {
                    curve = curvelist_GetItem (curveG.curves->list, col);
                    if (curve->pts > nRows) nRows = curve->pts;
                    FmtFile (fileHandle.analysis, "%s<%f[e2p5w15]", curve->x0.reading);
                    }
            }
            WriteFile (fileHandle.analysis, "\n", 1);

            for (row = 0; row < nRows; row++) {
                for (col = 0; col < curveG.curves->list.nItems; col++) {
                    IsListItemChecked (panel, CURVESAVE_LIST, col, &checked);
                    if (checked) {
                        curve = curvelist_GetItem (curveG.curves->list, col);
                        if (row > (curve->pts - 1)) {
                            if (col==0) {FmtFile (fileHandle.analysis, "%s<%f[e2p5w15]", 0.0);}
                            FmtFile (fileHandle.analysis, "%s<%f[e2p5w15]", 0.0);
                        } else {
                            if (col==0)
                               {FmtFile (fileHandle.analysis, "%s<%f[e2p5w15]", curve->x->readings[curve->offset+row]);}
                            FmtFile (fileHandle.analysis, "%s<%f[e2p5w15]", curve->y->readings[curve->offset+row]);
                        }
                    }
                }
                WriteFile (fileHandle.analysis, "\n", 1);
            }
            util_CloseFile();
            
            DiscardPanel (panel);
        }
    }
    return 0;
}


int  SaveIgorCurvesCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int filestatus, c, checked, i;
    char path[260];
    curvePtr curve;

    if (event == EVENT_COMMIT)
    {
        filestatus = FileSelectPopup ("", "*.dat", "*.dat",
                          "Save Curve Data for BR", VAL_SAVE_BUTTON, 0,
                          1, 1, 1, path);
        if ((filestatus == VAL_NEW_FILE_SELECTED) ||
            (filestatus == VAL_EXISTING_FILE_SELECTED)) {
            fileHandle.analysis = util_OpenFile(path, FILE_WRITE, TRUE);
             FmtFile (fileHandle.analysis, "X\t"); FmtFile (fileHandle.analysis, "Y\t"); FmtFile (fileHandle.analysis, "data\n");
				
			for (c = 0; c < curveG.curves->list.nItems; c++) {
                IsListItemChecked (panel, CURVESAVE_LIST, 0, &checked);
                DeleteListItem (panel, CURVESAVE_LIST, 0, 1);
                if (checked) {
                    curve = curvelist_GetItem (curveG.curves->list, c);
                    for (i = 0; i < (curve->pts); i++)
                            FmtFile (fileHandle.analysis,
                             "%s<%f[e2p5w12] %f[e2p5w12] %f[e2p5w12]\n", 
							//"%s<%f[e5] %f[e5] %f[e5]\n",  
                             curve->x->readings[i+curve->offset],
                             curve->x0.reading,
							 curve->y->readings[i+curve->offset]  );
                     //WriteFile (fileHandle.analysis, "\n", 1);
                }
                GetNumCheckedItems (panel, CURVESAVE_LIST, &checked);
                if (!checked) break;
            }
            util_CloseFile();
            
            DiscardPanel (panel);
        }
    }
    return 0;
}
int  SaveDTSCurvesCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int filestatus, c, checked, i;
    char path[260];
    curvePtr curve;

    if (event == EVENT_COMMIT)
    {
        filestatus = FileSelectPopup ("", "*.drm", "*.drm",
                          "Save Curve Data for DTS", VAL_SAVE_BUTTON, 0,
                          1, 1, 1, path);
        if ((filestatus == VAL_NEW_FILE_SELECTED) ||
            (filestatus == VAL_EXISTING_FILE_SELECTED)) {
            fileHandle.analysis = util_OpenFile(path, FILE_WRITE, TRUE);
            for (c = 0; c < curveG.curves->list.nItems; c++) {
                IsListItemChecked (panel, CURVESAVE_LIST, 0, &checked);
                DeleteListItem (panel, CURVESAVE_LIST, 0, 1);
                if (checked) {
                    curve = curvelist_GetItem (curveG.curves->list, c);
                    for (i = 0; i < (curve->pts); i++)
                          FmtFile (fileHandle.analysis,
                                 "%s<%f[e2p5w15]%f[e2p5w15]\n",
                                 curve->x->readings[i+curve->offset],
                                 curve->y->readings[i+curve->offset]);  
                    //if (StringLength (curve->attr.note) > 0)
                     //   FmtFile (fileHandle.analysis, curve->attr.note);
                   // else
                        WriteFile (fileHandle.analysis, "\n", 1);
                }
                GetNumCheckedItems (panel, CURVESAVE_LIST, &checked);
                if (!checked) break;
            }
            util_CloseFile();
            
            DiscardPanel (panel);
        }
    }
    return 0;
}

int  SaveMATLABCurvesCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int filestatus, checked, col, row, nRows = 0, nCols = 0;
    char path[260];
    curvePtr curve;

    if (event == EVENT_COMMIT) {
        filestatus = FileSelectPopup ("", "*.mat", "*.mat",
                                      "Save Curve Data for MATLAB",
                                      VAL_SAVE_BUTTON, 0, 1, 1, 1, path);
        if ((filestatus == VAL_NEW_FILE_SELECTED) ||
            (filestatus == VAL_EXISTING_FILE_SELECTED)) {
            fileHandle.analysis = util_OpenFile(path, FILE_WRITE, TRUE);
/*
    - write first row w/ x0 reading and 0.0 as filler
    - find the maximum # of pts to be saved -> nRows
*/
            for (col = 0; col < curveG.curves->list.nItems; col++) {
                IsListItemChecked (panel, CURVESAVE_LIST, col, &checked);
                if (checked) {
                    curve = curvelist_GetItem (curveG.curves->list, col);
                    if (curve->pts > nRows) nRows = curve->pts;
                    FmtFile (fileHandle.analysis, "%s<%f[e2p5w15]", curve->x0.reading);
                    FmtFile(fileHandle.analysis, "%s<%f[e2p5w15]", 0.0);
                }
            }
            WriteFile (fileHandle.analysis, "\n", 1);

            for (row = 0; row < nRows; row++) {
                for (col = 0; col < curveG.curves->list.nItems; col++) {
                    IsListItemChecked (panel, CURVESAVE_LIST, col, &checked);
                    if (checked) {
                        curve = curvelist_GetItem (curveG.curves->list, col);
                        if (row > (curve->pts - 1)) {
                            FmtFile (fileHandle.analysis, "%s<%f[e2p5w15]", 0.0);
                            FmtFile (fileHandle.analysis, "%s<%f[e2p5w15]", 0.0);
                        } else {
                            FmtFile (fileHandle.analysis, "%s<%f[e2p5w15]", curve->x->readings[curve->offset+row]);
                            FmtFile (fileHandle.analysis, "%s<%f[e2p5w15]", curve->y->readings[curve->offset+row]);
                        }
                    }
                }
                WriteFile (fileHandle.analysis, "\n", 1);
            }
            util_CloseFile();
            
            DiscardPanel (panel);
        }
    }
    return 0;
}


int  SaveDAASCurvesCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int filestatus, i, n, check;
    char path[260];
    curvePtr curve;

    if (event == EVENT_COMMIT)
    {
        filestatus = FileSelectPopup ("", "*.crv", "*.crv",
                                      "Save Curve Data for DAAS",
                                      VAL_SAVE_BUTTON, 0, 1, 1, 1, path);
        if ((filestatus == VAL_NEW_FILE_SELECTED) ||
            (filestatus == VAL_EXISTING_FILE_SELECTED)) {
            fileHandle.analysis = util_OpenFile(path, FILE_WRITE, FALSE);
            FmtFile (fileHandle.analysis, "#CURVEDATA\n");
            GetNumCheckedItems (panel, CURVESAVE_LIST, &n);
            FmtFile (fileHandle.analysis, "Curves %i\n", n);

            for (i = 0; i < curveG.curves->list.nItems; i++)
            {
                IsListItemChecked (panel, CURVESAVE_LIST, i, &check);
                if (check)
                {
                    curve = curvelist_GetItem (curveG.curves->list, i);
                    FmtFile (fileHandle.analysis, "%s;", curve->attr.label);
                }
            }

            WriteFile (fileHandle.analysis, "\n", 1);

            for (i = 0; i < curveG.curves->list.nItems; i++)
            {
                IsListItemChecked (panel, CURVESAVE_LIST, 0, &check);
                DeleteListItem (panel, CURVESAVE_LIST, 0, 1);
                if (check)
                {
                    curve_Save (curvelist_GetItem (curveG.curves->list, i));
                    GetNumCheckedItems (panel, CURVESAVE_LIST, &n);
                    if (n) FmtFile (fileHandle.analysis, FILE_ITEM_SEP);
                    else break;
                }
            }
            FmtFile (fileHandle.analysis, "#ENDCURVEDATA\n");
            util_CloseFile();
            
            DiscardPanel (panel);
        }
    }
    return 0;
}
int  SaveOriginCurvesCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int filestatus, c, checked, i;
    char path[260];
    curvePtr curve;

    if (event == EVENT_COMMIT)
    {
        filestatus = FileSelectPopup ("", "*.csv", "*.csv",
                          "Save Curve Data for DTS", VAL_SAVE_BUTTON, 0,
                          1, 1, 1, path);
        if ((filestatus == VAL_NEW_FILE_SELECTED) ||
            (filestatus == VAL_EXISTING_FILE_SELECTED)) {
            fileHandle.analysis = util_OpenFile(path, FILE_WRITE, TRUE);
            for (c = 0; c < curveG.curves->list.nItems; c++) {
                IsListItemChecked (panel, CURVESAVE_LIST, 0, &checked);
                DeleteListItem (panel, CURVESAVE_LIST, 0, 1);
                if (checked) {
                    curve = curvelist_GetItem (curveG.curves->list, c);
                    for (i = 0; i < (curve->pts); i++)
                        FmtFile (fileHandle.analysis,"%s<%f[e2p5w15]\n",curve->y->readings[i+curve->offset]);
                        /*FmtFile (fileHandle.analysis,
                             "%s<%f[e2p5w15]%f[e2p5w15]%f[e2p5w15]%f[e2p5w15]\n",
                             curve->x->readings[i+curve->offset],
                             curve->y->readings[i+curve->offset],
                             curve->x0.reading, 0.0);*/
                    //if (StringLength (curve->attr.note) > 0)
                     //   FmtFile (fileHandle.analysis, curve->attr.note);
                   // else
                   //     WriteFile (fileHandle.analysis, "\n", 1);
                }
                GetNumCheckedItems (panel, CURVESAVE_LIST, &checked);
                if (!checked) break;
            }
            util_CloseFile();
            
            DiscardPanel (panel);
        }
    }
    return 0;
}

int  SaveCurveSelectAllCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int i, check;

    if (event == EVENT_COMMIT)
    {
        GetCtrlVal (panel, control, &check);
        for (i = 0; i < curveG.curves->list.nItems; i++)
            CheckListItem (panel, CURVESAVE_LIST, i, check);
    }
    return 0;
}

int  InitSaveCurvesCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int save_panel, i;
    char color[40], ramp[10], item[255];
    curvePtr curve;

    if (event == EVENT_COMMIT)
    {
        save_panel = LoadPanel (0, "curveu.uir", CURVESAVE);
        
        SetPanelPos (save_panel, VAL_AUTO_CENTER, VAL_AUTO_CENTER);

        ClearListCtrl (save_panel, CURVESAVE_LIST);
        for (i = 0; i < curveG.curves->list.nItems; i++)
        {
            curve = curvelist_GetItem (curveG.curves->list, i);
            InsertListItem (save_panel, CURVESAVE_LIST, -1, curve_CompleteListItem(curve, 0, curve->pts), i);
        }

        util_InitClose (save_panel, CURVESAVE_CLOSE, FALSE);
        InstallPopup (save_panel);
    }
    return 0;
}

char *curve_CompleteListItem (curvePtr curve, int offset, int pts)
{
    char *item, ramp[20], color[30];

    item = "                                                                            "
           "                                                                            "
           "                                                                            ";
    if (curve->x->readings[curve->offset + offset] < curve->x->readings[curve->offset + pts - 1 + offset])
        Fmt (ramp, "UP");
    else
        Fmt (ramp, "DOWN");

    Fmt (color, "\033fg%x[w6]", curve->attr.color);

    Fmt (item, "%s<%s%s", color, curve->attr.label);
    Fmt (item, "%s[a]<\033p275r%s%i", color, curve->pts);
    Fmt (item, "%s[a]<\033p287l%s%s", color, ramp);
    return item;
}

int  LoadCurvesCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    char path[260], info[260], label[260];
    int filestatus, done, n, start, i, checked, button;
    curvePtr curve;

    if (event == EVENT_COMMIT)
    {
        filestatus = FileSelectPopup ("", "*.crv", "*.crv",
                                  "Load Curve Data", VAL_LOAD_BUTTON, 0,
                                  1, 1, 0, path);
        if (filestatus == VAL_EXISTING_FILE_SELECTED)
        {
            fileHandle.analysis = util_OpenFile (path, FILE_READ, FALSE);
            ReadLine (fileHandle.analysis, info, 255);
            if (CompareBytes (info, 0, "#CURVEDATA", 0, StringLength ("#CURVEDATA"), 1))
            {
                util_CloseFile();
                MessagePopup ("Load Curve File Error", "Wrong file type selected");
                return 1;
            }

            ScanFile (fileHandle.analysis, "%s>Curves %i%s[w1d]", &n);
            for (i = 0; i < n; i++)
                ScanFile(fileHandle.analysis, "%s>%s[t59]%s[w1d]", label);

            for (i = 0; i < n; i++)
            {
                curve = curve_Load (curveG.title);
                if (!curve) {
                    MessagePopup ("Load Curves Message", "Error loading curves...process aborted");
                    break;
                }
                ReadLine (fileHandle.analysis, info, 255);
                curvelist_AddCurve (curveG.curves, curve);
                curve_Plot (curveG.panel, curveG.control, curve, NULL);
            }
            util_CloseFile();
            curvelist_UpdatePanel(*curveG.curves);
        }
    }
    return 0;
}

int  CreateCurveCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    char label[260];
    channelPtr chan;
    curvePtr curve;
    if (event == EVENT_COMMIT)
    {
        curve = curve_Create ();
        if (curve)
        {
            chan = channellist_GetItem(0);
            Fmt(curve->attr.label, "curve#%i", curveG.curves->list.nItems+1);
            curve->curvepts = chan->pts;
            curve->pts = chan->pts;

            curve->x = chan;
            channel_AddCurve (chan, curve->attr.label, curveG.title);

            curve->y = chan;
            channel_AddCurve (chan, curve->attr.label, curveG.title);

            curvelist_AddCurve (curveG.curves, curve);
            SetCtrlVal (curveG.curves->panel, CURVES_LIST, curveG.curves->list.nItems - 1);
            curve_InitViewPanel (FALSE);
            curve_UpdateViewPanel (curve, FALSE);
            curve_InitEditPanel (curve);
            curvelist_UpdatePanel (*curveG.curves);
        }
    }
    return 0;
}

int  CurveIndexCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int i;
    if (event == EVENT_COMMIT)
    {
        GetCtrlVal (panel, control, &i);
        SetCtrlIndex (panel, CURVES_LIST, i-1);
    }
    return 0;
}

int  CurveSelectCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int i;
    curvePtr curve;

    if (event == EVENT_VAL_CHANGED)
    {
        GetCtrlIndex (panel, CURVES_LIST, &i);
        SetCtrlVal (panel, CURVES_INDEX, i+1);
    }

    if (event == EVENT_RIGHT_CLICK)
    {
        GetCtrlIndex (panel, CURVES_LIST, &i);
        SetCtrlVal (panel, CURVES_INDEX, i+1);
        curve = curvelist_GetSelection ();
        if (curve->attr.hidden)
        {
            curve->attr.hidden = FALSE;
            curve_Plot (curveG.panel, curveG.control, curve, NULL);
        }
        else
        {
            curve->attr.hidden = TRUE;
            curve_Hide (curveG.panel, curveG.control, curve, FALSE);
        }
        curvelist_ReplaceCurve (curve);
        return 1;
    }

    if (event == EVENT_LEFT_DOUBLE_CLICK)
    {
        GetCtrlIndex (panel, CURVES_LIST, &i);
        SetCtrlVal (panel, CURVES_INDEX, i+1);

        curve = curvelist_GetSelection ();
        curve_InitViewPanel (FALSE);
        curve_UpdateViewPanel (curve, FALSE);
        curve_InitEditPanel (curve);
        return 1;
    }

    if (event == EVENT_COMMIT)
    {
        GetCtrlIndex (panel, CURVES_LIST, &i);
        SetCtrlVal (panel, CURVES_INDEX, i+1);
        curve = curvelist_GetSelection ();
        IsListItemChecked (panel, CURVES_LIST, i, &curve->attr.fat);
        curve_Plot(curveG.panel, curveG.control, curve, NULL);
        return 1;
    }
    return 0;
}

/*****************************************************************************/

int  CurveYChanCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int i;
    curvePtr curve;
    channelPtr newchan;

    if (event == EVENT_VAL_CHANGED)
    {
        curve = callbackData;
        GetCtrlIndex (panel, control, &i);
        newchan = channellist_GetItem (i);
        SetCtrlIndex (channelG.p.channels, CHANNELS_LIST, i);

        if (newchan->pts < curve->x->pts)
        {
            curve->curvepts = newchan->pts;
            curve->pts = newchan->pts;
            curve->offset = 0;
        }
        else
        {
            curve->curvepts = curve->x->pts;
            curve->pts = curve->x->pts;
            curve->offset = 0;
        }

        channel_RemoveCurve (curve->y, curve->attr.label);
        curve->y = newchan;
        channel_AddCurve (curve->y, curve->attr.label, curveG.title);
        curve_UpdateViewPanel(curve, FALSE);
        curve_Plot (curveG.panel, curveG.control, curve, NULL);
    }
    return 0;
}

int  CurveXChanCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int i;
    curvePtr curve;
    channelPtr newchan;

    if (event == EVENT_VAL_CHANGED)
    {
        curve = callbackData;
        GetCtrlIndex (panel, control, &i);
        newchan = channellist_GetItem (i);
        SetCtrlIndex (channelG.p.channels, CHANNELS_LIST, i);

        if (newchan->pts < curve->y->pts)
        {
            curve->curvepts = newchan->pts;
            curve->pts = newchan->pts;
            curve->offset = 0;
        }
        else
        {
            curve->curvepts = curve->y->pts;
            curve->pts = curve->y->pts;
            curve->offset = 0;
        }

        channel_RemoveCurve (curve->x, curve->attr.label);
        curve->x = newchan;
        channel_AddCurve (curve->x, curve->attr.label, curveG.title);
        curve_UpdateViewPanel(curve, FALSE);
        curve_Plot (curveG.panel, curveG.control, curve, NULL);
    }
    return 0;
}

int  CurveAttrCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    curvePtr curve;
    channelPtr chan;
    int c;
    char *label;
    double *reading;

    switch (control)
    {
        case CURVEATTR_LABEL:
            curve = callbackData;
            if (event == EVENT_VAL_CHANGED)
            {
                GetCtrlVal (panel, control, curve->attr.label);
                c = list_FindItem (curveG.curves->list, curve);
                ReplaceListItem (curveview.p1, CURVEVIEW_CURVES, c, curve->attr.label,
                         c);
                curvelist_ReplaceCurve (curve);
            }
            break;
        case CURVEATTR_X0_LABEL:
            label = callbackData;
            if (event == EVENT_VAL_CHANGED)
                GetCtrlVal (panel, control, label);
            break;
        case CURVEATTR_X0_READING:
            curve = callbackData;
            if (event == EVENT_COMMIT)
                GetCtrlVal (panel, control, &curve->x0.reading);
            break;
        case CURVEATTR_XCHANINFO:
        case CURVEATTR_YCHANINFO:
            chan = callbackData;
            if (event == EVENT_COMMIT)
            {
                channel_InitViewPanel ();
                channel_UpdateViewPanel (chan);
                channel_InitEditPanel (chan, FALSE);
            }
            break;
        case CURVEATTR_HIDDEN:
            curve = callbackData;
            if (event == EVENT_COMMIT)
            {
                GetCtrlVal (panel, control, &curve->attr.hidden);
                if (curve->attr.hidden)
                    curve_Hide(curveG.panel, curveG.control, curve, FALSE);
                else
                    curve_Plot (curveG.panel, curveG.control, curve, NULL);
                curvelist_ReplaceCurve (curve);
            }
            break;
        case CURVEATTR_POINTSTYLE:
        case CURVEATTR_PLOTSTYLE:
        case CURVEATTR_LINESTYLE:
        case CURVEATTR_PTFREQ:
        case CURVEATTR_COLOR:
            curve = callbackData;
            if (event == EVENT_COMMIT)
            {
                GetCtrlVal (panel, CURVEATTR_POINTSTYLE, &curve->attr.style.point);
                GetCtrlVal (panel, CURVEATTR_PLOTSTYLE, &curve->attr.style.plot);
                GetCtrlVal (panel, CURVEATTR_LINESTYLE, &curve->attr.style.line);
                GetCtrlVal (panel, CURVEATTR_PTFREQ, &curve->attr.ptfreq);
                GetCtrlVal (panel, CURVEATTR_COLOR, &curve->attr.color);
                curve_UpdateViewGraph (curve);
                curve_Plot (curveG.panel, curveG.control, curve, NULL);
                if (control == CURVEATTR_COLOR)
                    curvelist_ReplaceCurve (curve);
            }
            break;
    }
    return 0;
}

void curve_UpdateEditPanel (curvePtr curve)
{
    int c;

    ResetTextBox (curveview.p2, CURVEATTR_NOTE, curve->attr.note);
    SetCtrlAttribute (curveview.p2, CURVEATTR_NOTE, ATTR_CALLBACK_DATA, &curve->attr.note);

    SetCtrlVal (curveview.p2, CURVEATTR_COLOR, curve->attr.color);
    SetCtrlAttribute (curveview.p2, CURVEATTR_COLOR, ATTR_CALLBACK_DATA, curve);

    SetCtrlVal (curveview.p2, CURVEATTR_HIDDEN, curve->attr.hidden);
    SetCtrlAttribute (curveview.p2, CURVEATTR_HIDDEN, ATTR_CALLBACK_DATA, curve);

    SetCtrlVal (curveview.p2, CURVEATTR_PTFREQ, curve->attr.ptfreq);
    SetCtrlAttribute (curveview.p2, CURVEATTR_PTFREQ, ATTR_CALLBACK_DATA, curve);

    SetCtrlVal (curveview.p2, CURVEATTR_LINESTYLE, curve->attr.style.line);
    SetCtrlAttribute (curveview.p2, CURVEATTR_LINESTYLE, ATTR_CALLBACK_DATA, curve);

    SetCtrlVal (curveview.p2, CURVEATTR_PLOTSTYLE, curve->attr.style.plot);
    SetCtrlAttribute (curveview.p2, CURVEATTR_PLOTSTYLE, ATTR_CALLBACK_DATA, curve);

    SetCtrlVal (curveview.p2, CURVEATTR_POINTSTYLE, curve->attr.style.point);
    SetCtrlAttribute (curveview.p2, CURVEATTR_POINTSTYLE, ATTR_CALLBACK_DATA, curve);

    SetCtrlAttribute (curveview.p2, CURVEATTR_YCHANINFO, ATTR_CALLBACK_DATA, curve->y);
    SetCtrlAttribute (curveview.p2, CURVEATTR_XCHANINFO, ATTR_CALLBACK_DATA, curve->x);

    c = list_FindItem (channelG.channels, curve->y);
    SetCtrlVal (curveview.p2, CURVEATTR_YCHAN, c);
    SetCtrlAttribute (curveview.p2, CURVEATTR_YCHAN, ATTR_CALLBACK_DATA, curve);

    c = list_FindItem (channelG.channels, curve->x);
    SetCtrlVal (curveview.p2, CURVEATTR_XCHAN, c);
    SetCtrlAttribute (curveview.p2, CURVEATTR_XCHAN, ATTR_CALLBACK_DATA, curve);

    SetCtrlVal(curveview.p2, CURVEATTR_X0_READING, curve->x0.reading);
    SetCtrlAttribute (curveview.p2, CURVEATTR_X0_READING, ATTR_CALLBACK_DATA, curve);

    SetCtrlVal(curveview.p2, CURVEATTR_X0_LABEL, curve->x0.label);
    SetCtrlAttribute (curveview.p2, CURVEATTR_X0_LABEL, ATTR_CALLBACK_DATA, &curve->x0.label);

    SetCtrlVal(curveview.p2, CURVEATTR_LABEL, curve->attr.label);
    SetCtrlAttribute (curveview.p2, CURVEATTR_LABEL, ATTR_CALLBACK_DATA, curve);
}

void curve_InitEditPanel (curvePtr curve)
{
    int width1, width2;

    curveview.p2 = LoadPanel (curveview.p1, "curveu.uir", CURVEATTR);
    

    GetPanelAttribute (curveview.p1, ATTR_WIDTH, &width1);
    SetPanelPos (curveview.p2, 6, width1);

    GetPanelAttribute (curveview.p2, ATTR_WIDTH, &width2);
    SetPanelAttribute (curveview.p1, ATTR_WIDTH, width1 + width2 + 6);

    channellist_Copy (curveview.p2, CURVEATTR_XCHAN);
    channellist_Copy (curveview.p2, CURVEATTR_YCHAN);

    curve_UpdateEditPanel (curve);

    DisplayPanel (curveview.p2);
    SetPanelPos (curveview.p1, VAL_AUTO_CENTER, VAL_AUTO_CENTER);
    InstallPopup (curveview.p1);
}

int  CurveViewMaskCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    curvePtr curve;
    int mask;

    if (event == EVENT_COMMIT)
    {
        curve = callbackData;
        GetCtrlVal (panel, control, &mask);
        if (mask)
        {
            curve->offset = curveview.offset;
            curve->pts = curveview.pts;
            curveview.offset = 0;
        }
        else
        {
            curveview.offset += curve->offset;
            curve->offset = 0;
            curve->pts = curve->curvepts;
        }
        SetInputMode (panel, CURVEVIEW_OFFSET, !(curve->pts == curveview.pts));
        SetCtrlAttribute (panel, CURVEVIEW_OFFSET,
                      ATTR_MAX_VALUE, curve->pts - curveview.pts);
        SetCtrlAttribute (panel, CURVEVIEW_PTS_1, ATTR_MAX_VALUE, curve->pts);
        SetCtrlVal (panel, CURVEVIEW_OFFSET, curveview.offset);
        SetCtrlVal (panel, CURVEVIEW_OFFSET_VAL, curveview.offset);
        curve_UpdateViewGraph (curve);
    }
    return 0;
}

int  CurveViewDirCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    curvePtr curve;
    if (event == EVENT_COMMIT)
    {
        curve = callbackData;
        Reverse (curve->x->readings, curve->curvepts, curve->x->readings);
        Reverse (curve->y->readings, curve->curvepts, curve->y->readings);

        curve->offset = (curve->curvepts - 1) - curve->offset - (curve->pts - 1);
        curveview.offset = (curve->pts - 1) - curveview.offset - (curveview.pts - 1);

        SetCtrlVal (panel, CURVEVIEW_OFFSET, curveview.offset);
        SetCtrlVal (panel, CURVEVIEW_OFFSET_VAL, curveview.offset);
        Fmt (curve->x->note, "%s[a]<Reversed\n");
        Fmt (curve->y->note, "%s[a]<Reversed\n");
        curve_UpdateViewGraph (curve);
    }
    return 0;
}

int  CurveViewAxisCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int y, grid, logscale;
    curvePtr curve;

    if (event == EVENT_COMMIT)
    {
        GetCtrlVal (panel, CURVEVIEW_AXIS, &y);
        switch (control)
        {
            case CURVEVIEW_GRID:
                GetCtrlVal (panel, control, &grid);
                if (y)
                    SetCtrlAttribute (panel, CURVEVIEW_GRAPH, ATTR_YGRID_VISIBLE, grid);
                else
                    SetCtrlAttribute (panel, CURVEVIEW_GRAPH, ATTR_XGRID_VISIBLE, grid);
                break;
            case CURVEVIEW_LOG:
                GetCtrlVal (panel, control, &logscale);
                if (y)
                    SetCtrlAttribute (panel, CURVEVIEW_GRAPH, ATTR_YMAP_MODE, logscale);
                else
                    SetCtrlAttribute (panel, CURVEVIEW_GRAPH, ATTR_XMAP_MODE, logscale);
                curve = callbackData;
                curve_UpdateViewGraph(curve);
                break;
        }
    }
    return 0;
}

int  CurveViewSelectAxisCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int y, logscale, grid;

    if (event == EVENT_COMMIT)
    {
        GetCtrlVal (panel, control, &y);
        if (y)
        {
            GetCtrlAttribute (panel, CURVEVIEW_GRAPH, ATTR_YMAP_MODE, &logscale);
            GetCtrlAttribute (panel, CURVEVIEW_GRAPH, ATTR_YGRID_VISIBLE, &grid);
        }
        else
        {
            GetCtrlAttribute (panel, CURVEVIEW_GRAPH, ATTR_XMAP_MODE, &logscale);
            GetCtrlAttribute (panel, CURVEVIEW_GRAPH, ATTR_XGRID_VISIBLE, &grid);
        }

        SetCtrlVal (panel, CURVEVIEW_LOG, logscale);
        SetCtrlVal (panel, CURVEVIEW_GRID, grid);
    }
    return 0;
}

int  CurveViewGraphCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    double m1x, m1y;
    int on;

    if (event == EVENT_VAL_CHANGED);
    {
        GetCtrlVal (panel, CURVEVIEW_MARKER, &on);
        if (on)
        {
            GetGraphCursor (panel, control, 1, &m1x, &m1y);
            SetCtrlVal (panel, CURVEVIEW_M1X, m1x);
            SetCtrlVal (panel, CURVEVIEW_M1Y, m1y);
        }
    }
    return 0;
}

int  CurveViewMarkerCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int marker_on;
    double m1x, m1y;

    if (event == EVENT_COMMIT)
    {
        GetCtrlVal (panel, control, &marker_on);
        SetInputMode (panel, CURVEVIEW_M1X, marker_on);
        SetInputMode (panel, CURVEVIEW_M1Y, marker_on);
        if (marker_on)
        {
            SetCtrlAttribute (panel, CURVEVIEW_GRAPH, ATTR_CTRL_MODE, VAL_HOT);
            SetCtrlAttribute (panel, CURVEVIEW_GRAPH, ATTR_NUM_CURSORS, 1);
            SetCursorAttribute (panel, CURVEVIEW_GRAPH, 1, ATTR_CURSOR_MODE,
                                VAL_SNAP_TO_POINT);
            SetCursorAttribute (panel, CURVEVIEW_GRAPH, 1, ATTR_CURSOR_COLOR,
                                VAL_CYAN);
            GetGraphCursor (panel, CURVEVIEW_GRAPH, 1, &m1x, &m1y);
            SetCtrlVal (panel, CURVEVIEW_M1X, m1x);
            SetCtrlVal (panel, CURVEVIEW_M1Y, m1y);
        }
        else
        {
            SetCtrlAttribute (panel, CURVEVIEW_GRAPH, ATTR_NUM_CURSORS, 0);
            SetCtrlAttribute (panel, CURVEVIEW_GRAPH, ATTR_CTRL_MODE, VAL_INDICATOR);
        }
    }
    return 0;
}

int  CurveViewWindowCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int min_i, max_i, offset;
    double min, max;
    curvePtr curve;

    if (event == EVENT_VAL_CHANGED)
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
        }

        curve_UpdateViewGraph (curve);
    }
    return 0;
}

int  CurveViewSelectCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int i;
    curvePtr curve;

    if (event == EVENT_COMMIT)
    {
        GetCtrlVal (panel, control, &i);
        curve = curvelist_GetItem (curveG.curves->list, i);
        GetCtrlAttribute (panel, CURVEVIEW_PTS_2, ATTR_VISIBLE, &i);
        curve_UpdateViewPanel (curve, i);
        curve_UpdateEditPanel (curve);
    }
    return 0;
}

int  CurveViewDoneCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    if (event == EVENT_COMMIT)
    {
        if (curveview.p2)
        {
            
            DiscardPanel (curveview.p2);
            curveview.p2 = 0;
        }
        
        DiscardPanel (curveview.p1);
    }
    return 0;
}

void curve_UpdateViewGraph (curvePtr curve)
{
    int min_i, max_i, offset, scatter, plotstyle, scale;
    double max, min;
    char label[25];

    GetCtrlVal (curveview.p1, CURVEVIEW_SCATTER, &scatter);
    if (scatter) plotstyle = VAL_SCATTER; else plotstyle = curve->attr.style.plot;

    if (StringLength (curve->x->label) > 24) Fmt (label, "%s[w21]...", curve->x->label);
        else Fmt (label, curve->x->label);
    SetCtrlAttribute (curveview.p1, CURVEVIEW_GRAPH, ATTR_XNAME, label);

    if (StringLength (curve->y->label) > 24) Fmt (label, "%s[w21]...", curve->y->label);
        else Fmt (label, curve->y->label);
    SetCtrlAttribute (curveview.p1, CURVEVIEW_GRAPH, ATTR_YNAME, label);

    if (curveview.h1)
        DeleteGraphPlot (curveview.p1, CURVEVIEW_GRAPH, curveview.h1, VAL_DELAYED_DRAW);

    curveview.h1 = PlotXY (curveview.p1, CURVEVIEW_GRAPH,
            curve->x->readings + curve->offset,
            curve->y->readings + curve->offset,
            curve->pts, VAL_DOUBLE, VAL_DOUBLE,
            plotstyle, curve->attr.style.point,
            curve->attr.style.line, curve->attr.ptfreq,
            curve->attr.color);
    offset = curve->offset + curveview.offset;
    MaxMin1D (curve->x->readings + offset, curveview.pts,
              &max, &max_i, &min, &min_i);
    if (min == max) scale = VAL_AUTO; else scale = VAL_MANUAL;
    SetAxisRange (curveview.p1, CURVEVIEW_GRAPH, scale, min,
                  max, VAL_NO_CHANGE, 0.0, 1.0);
    MaxMin1D (curve->y->readings + offset, curveview.pts,
              &max, &max_i, &min, &min_i);
    if (min == max) scale = VAL_AUTO; else scale = VAL_MANUAL;
    SetAxisRange (curveview.p1, CURVEVIEW_GRAPH, VAL_NO_CHANGE,
                  0.0, 1.0, scale, min, max);
    curve_Plot (curveG.panel, curveG.control, curve, NULL);
}

void curve_UpdateViewPanel (curvePtr curve, int pts2)
{
    double expon;
    char info[260];
    int i, pts;

    SetCtrlVal (curveview.p1, CURVEVIEW_OFFSET, 0);
    SetCtrlVal (curveview.p1, CURVEVIEW_OFFSET_VAL, 0);
    SetCtrlAttribute (curveview.p1, CURVEVIEW_OFFSET,
                      ATTR_CALLBACK_DATA, curve);
    curveview.offset = 0;

    SetCtrlVal (curveview.p1, CURVEVIEW_TOTALPTS, curve->curvepts);
    SetCtrlAttribute (curveview.p1, CURVEVIEW_PTS_1, ATTR_MAX_VALUE, curve->curvepts);

    if (pts2)
    {
        modf(log((double)curve->curvepts)/log(2), &expon);
        ClearListCtrl (curveview.p1, CURVEVIEW_PTS_2);
        for (i = 1; i <= (int)expon; i++)
        {
            pts = (unsigned int)pow(2, i);
            Fmt (info, "%s<%i", pts);
            InsertListItem (curveview.p1, CURVEVIEW_PTS_2, -1, info, pts);
        }
        GetNumListItems (curveview.p1, CURVEVIEW_PTS_2, &i);
        SetCtrlIndex (curveview.p1, CURVEVIEW_PTS_2, i - 1);
        GetCtrlVal (curveview.p1, CURVEVIEW_PTS_2, &curveview.pts);
    }
    else
    {
        SetCtrlVal (curveview.p1, CURVEVIEW_PTS_1, curve->pts);
        SetCtrlAttribute (curveview.p1, CURVEVIEW_PTS_1,
                          ATTR_MAX_VALUE, curve->pts);
        curveview.pts = curve->pts;
    }

    SetInputMode (curveview.p1, CURVEVIEW_OFFSET, !(curve->pts == curveview.pts));
    SetCtrlAttribute (curveview.p1, CURVEVIEW_OFFSET,
                      ATTR_MAX_VALUE, curveview.pts - curveview.offset);

    SetCtrlAttribute (curveview.p1, CURVEVIEW_PTS_1,
                      ATTR_CALLBACK_DATA, curve);
    SetCtrlAttribute (curveview.p1, CURVEVIEW_PTS_2,
                      ATTR_CALLBACK_DATA, curve);

    SetCtrlAttribute (curveview.p1, CURVEVIEW_LOG,
                      ATTR_CALLBACK_DATA, curve);
    SetCtrlAttribute (curveview.p1, CURVEVIEW_GRID,
                      ATTR_CALLBACK_DATA, curve);

    SetCtrlVal (curveview.p1, CURVEVIEW_DIR,
                (curve->x->readings[curve->offset] < curve->x->readings[curve->offset + curve->pts - 1]));
    SetCtrlAttribute (curveview.p1, CURVEVIEW_DIR,
                      ATTR_CALLBACK_DATA, curve);

    SetCtrlVal (curveview.p1, CURVEVIEW_SCATTER, FALSE);
    SetCtrlAttribute (curveview.p1, CURVEVIEW_SCATTER,
                      ATTR_CALLBACK_DATA, curve);

    SetCtrlVal (curveview.p1, CURVEVIEW_MASK, curve->offset);
    SetCtrlAttribute (curveview.p1, CURVEVIEW_MASK,
                      ATTR_CALLBACK_DATA, curve);

    curve_UpdateViewGraph (curve);
}

void curve_InitViewPanel (int pts2)
{
    int i;

    curveview.p1 = LoadPanel (0, "curveu.uir", CURVEVIEW);
    

    util_InitClose (curveview.p1, CURVEVIEW_DONE, TRUE);

    SetPanelPos (curveview.p1, VAL_AUTO_CENTER, VAL_AUTO_CENTER);

    curveview.p2 = 0;
    curveview.h1 = 0;
    curveview.h2 = 0;

    curvelist_Copy (curveG.curves->list, curveview.p1, CURVEVIEW_CURVES, TRUE);

    GetCtrlVal (curveG.curves->panel, CURVES_LIST, &i);
    SetCtrlVal (curveview.p1, CURVEVIEW_CURVES, i);

    SetCtrlAttribute (curveview.p1, CURVEVIEW_GRAPH, ATTR_NUM_CURSORS, 0);
    SetCtrlAttribute (curveview.p1, CURVEVIEW_PTS_1, ATTR_VISIBLE, !pts2);
    SetCtrlAttribute (curveview.p1, CURVEVIEW_PTS_2, ATTR_VISIBLE, pts2);
    SetInputMode (curveview.p1, CURVEVIEW_MASK, !pts2);
}

curvePtr curvelist_GetItem (listType list, int i)
{
    nodePtr node;
    node = list_GetNode (list, i); return node->item;
}

curvePtr curvelist_GetSelection (void)
{
    int i;
    GetCtrlIndex (curveG.curves->panel, CURVES_LIST, &i);
    return curvelist_GetItem (curveG.curves->list, i);
}

void curvelist_Copy (listType curves, int panel, int control, int pulldown)
{
    int i;
    curvePtr curve;
    char info[260];

    ClearListCtrl (panel, control);
    if (!pulldown)
    {
        SetCtrlAttribute (panel, control, ATTR_TEXT_BGCOLOR, 0x000000);
        SetCtrlAttribute (panel, control, ATTR_TEXT_COLOR, 0xFFFFFF);
    }
    for (i = 0; i < curves.nItems; i++)
    {
        curve = curvelist_GetItem (curves, i);
        if (pulldown)
            InsertListItem (panel, control, -1, curve->attr.label, i);
        else
        {
            Fmt (info, "\033fg%x[w6]%s", curve->attr.color, curve->attr.label);
            InsertListItem (panel, control, -1, info, i);
        }
    }
}

void curvelist_UpdatePanel (curvelistType curves)
{
    int i, menubar;
    menubar = GetPanelMenuBar (curves.panel);

    SetMenuBarAttribute (menubar, CURVEMENUS_PROC, ATTR_DIMMED, !curves.list.nItems);
    SetMenuBarAttribute (menubar, CURVEMENUS_MEAS, ATTR_DIMMED, !curves.list.nItems);
    SetMenuBarAttribute (menubar, CURVEMENUS_INTERP, ATTR_DIMMED, !curves.list.nItems);
    SetMenuBarAttribute (menubar, CURVEMENUS_FIT, ATTR_DIMMED, !curves.list.nItems);

    SetInputMode (curves.panel, CURVES_LIST, curves.list.nItems);
    SetInputMode (curves.panel, CURVES_SAVE, curves.list.nItems);
    SetInputMode (curves.panel, CURVES_REMOVE, curves.list.nItems && (utilG.acq.status != ACQ_BUSY));
    SetInputMode (curves.panel, CURVES_INDEX, curves.list.nItems);
    SetInputMode (curves.panel, CURVES_CREATE, channelG.channels.nItems > 1);

    SetCtrlVal (curves.panel, CURVES_NCURVES, curves.list.nItems);
    GetCtrlIndex (curves.panel, CURVES_LIST, &i);
    SetCtrlVal (curves.panel, CURVES_INDEX, i+1);
}

void curvelist_AddCurve (curvelistPtr curves, curvePtr c)
{
    char label[260];
    char bg[260];
    if (c && list_AddItem (&curves->list, c))
    {
        if (c->attr.hidden) Fmt (bg, "\033bgFFFFFF");
        else Fmt (bg, "");
        Fmt (label, "%s\033fg%x[w6]%s", bg, c->attr.color, c->attr.label);
        InsertListItem (curves->panel, CURVES_LIST, -1, label,
                        curves->list.nItems-1);
        SetCtrlVal (curves->panel, CURVES_NCURVES, curves->list.nItems);
        SetCtrlAttribute (curves->panel, CURVES_INDEX, ATTR_MAX_VALUE, curves->list.nItems);
    }
}

void curvelist_ReplaceCurve (curvePtr c)
{
    int i;
    char label[260], bg[40];
    if (c && curveG.curves)
    {
        i = list_FindItem (curveG.curves->list, c);
        if (i != NOT_IN_LIST)
        {
            if (c->attr.hidden) Fmt (bg, "\033bgFFFFFF");
            else Fmt (bg, "");
            Fmt (label, "%s\033fg%x[w6]%s", bg, c->attr.color, c->attr.label);
            ReplaceListItem (curveG.curves->panel, CURVES_LIST, i, label, i);
        }
    }
}

void curvelist_RemoveCurve (curvelistPtr curves, curvePtr c)
{
    int i;

    i = list_FindItem (curves->list, c);
    if (i != -1)
    {
        channel_RemoveCurve (c->x, c->attr.label);
        channel_RemoveCurve (c->y, c->attr.label);

        list_RemoveItem (&curves->list, i, TRUE);
        DeleteListItem (curves->panel, CURVES_LIST, i, 1);
        SetCtrlVal (curves->panel, CURVES_NCURVES, curves->list.nItems);
        SetCtrlAttribute (curves->panel, CURVES_INDEX, ATTR_MAX_VALUE, curves->list.nItems);
    }
}

void curve_CopyAttrs (curvePtr curve, curvePtr copy)
{
    copy->attr.style.plot = curve->attr.style.plot;
    copy->attr.style.point = curve->attr.style.point;
    copy->attr.style.line = curve->attr.style.line;
    copy->attr.ptfreq = curve->attr.ptfreq;
    copy->attr.color = curve->attr.color;
    copy->attr.hidden = curve->attr.hidden;
    copy->attr.fat = curve->attr.fat;
    Fmt (copy->attr.label, curve->attr.label);
    Fmt (copy->attr.note, curve->attr.note);
}

curvePtr curve_Load (char *graphtitle)
{
    curvePtr c;
    channelPtr x, y;
    char info[260];

    c = curve_Create();
    if (!c) return NULL;

    ScanFile (fileHandle.analysis, "%s>Curve     : %s[xt59]%s[dw1]", c->attr.label);
    ScanFile (fileHandle.analysis, "%s>Indexing  : %i [CurvePts] %i [Offset] %i [Points]%s[w1d]",
              &c->curvepts, &c->offset, &c->pts);
    ScanFile (fileHandle.analysis, "%s>Attributes: %i [PlotStyle], %i [PtStyle],"
             "%i [LineStyle] %i [PtFreq] %i [Color] %i [Hidden] %i [Fat]%s[w1d]",
             &c->attr.style.plot, &c->attr.style.point, &c->attr.style.line,
             &c->attr.ptfreq, &c->attr.color, &c->attr.hidden, &c->attr.fat);

    ReadLine (fileHandle.analysis, info, 255);
    if (CompareBytes ("X0", 0, info, 0, 2, 0) == 0) /*match*/
        Scan (info, "%s[i12]>%f,%s[xt59]%s[dw1]", &c->x0.reading, c->x0.label);
    else
        SetFilePtr (fileHandle.analysis, -(StringLength(info)+2), 1);

    util_LoadNote (c->attr.note);

    x = channel_Create();
    if (!x) {free (c); return NULL;}

    y = channel_Create();
    if (!y) {free (x); free (c); return NULL;}

    util_LoadNote (x->note);
    util_LoadNote (y->note);

    if (!channel_AllocMem (x, c->curvepts))
    { free (c); free (x); free (y); return NULL;}

    if (!channel_AllocMem (y, c->curvepts))
    { free (c); free (x); free (y); return NULL;}

    ScanFile (fileHandle.analysis, "%s>%s[xt58]%*f%s[dw1]", x->label, c->curvepts, x->readings);
    ScanFile (fileHandle.analysis, "%s>%s[xt58]%*f%s[dw1]", y->label, c->curvepts, y->readings);

    c->x = x; channel_AddCurve (x, c->attr.label, graphtitle);
    c->y = y; channel_AddCurve (y, c->attr.label, graphtitle);

    channellist_AddChannel (x);
    channellist_AddChannel (y);

    return c;
}

void curve_Save (curvePtr c)
{
    FmtFile (fileHandle.analysis, "%s<Curve     : %s;\n", c->attr.label);
    FmtFile (fileHandle.analysis, "%s<Indexing  : %i [CurvePts] %i [Offset] %i [Points]\n",
             c->curvepts, c->offset, c->pts);
    FmtFile (fileHandle.analysis, "%s<Attributes: %i [PlotStyle], %i [PtStyle],"
             "%i [LineStyle] %i [PtFreq] %i [Color] %i [Hidden] %i [Fat]\n",
             c->attr.style.plot, c->attr.style.point, c->attr.style.line,
             c->attr.ptfreq, c->attr.color, c->attr.hidden, c->attr.fat);
    FmtFile (fileHandle.analysis, "%s<X0 Channel: %f[e2p5], %s;\n", c->x0.reading, c->x0.label);
    util_SaveNote (c->attr.note);
    util_SaveNote (c->x->note);
    util_SaveNote (c->y->note);
    FmtFile (fileHandle.analysis, "%s<%s:%*f[e2p5w13]\n", c->x->label, c->curvepts, c->x->readings);
    FmtFile (fileHandle.analysis, "%s<%s:%*f[e2p5w13]\n", c->y->label, c->curvepts, c->y->readings);
}

void curve_Hide (int panel, int control, curvePtr c, int delayed)
{
    int redraw;
    if (c->plothandle)
    {
        if (delayed) redraw = VAL_DELAYED_DRAW;
        else redraw = VAL_IMMEDIATE_DRAW;
        DeleteGraphPlot (panel, control, c->plothandle, redraw);
        c->plothandle = 0;
    }
}

void curve_Plot (int panel, int control, curvePtr c, void *graphP)
{
    graphPtr graph = graphP;
	int style;
    if (panel && c->x && c->y && !c->attr.hidden)
    {
        curve_Hide (panel, control, c, TRUE);
        if (c->attr.fat) style = VAL_FAT_LINE;
        else style = c->attr.style.plot;
		if(graph)
		{
			c->plothandle = PlotXY (panel, control, c->x->readings, c->y->readings,
								c->pts, VAL_DOUBLE, VAL_DOUBLE, style,
                                c->attr.style.point, c->attr.style.line,
                                c->attr.ptfreq, c->attr.color);
		}
		else
			c->plothandle = PlotXY (panel, control, 
								c->x->readings + c->offset,
                                c->y->readings + c->offset, 
								c->pts, VAL_DOUBLE, VAL_DOUBLE, style,
                                c->attr.style.point, c->attr.style.line,
                                c->attr.ptfreq, c->attr.color);
		
    }
}

curvePtr curve_Create (void)
{
    curvePtr c;

    c = malloc (sizeof(curveType));
    if (!c) {util_OutofMemory("Create Curve Error"); return NULL;}

    c->attr.style.plot = 0;
    c->attr.style.point = 0;
    c->attr.style.line = 0;
    c->attr.ptfreq = 1;
    c->attr.color = 0xFFFF00;
    c->attr.hidden = FALSE;
    c->attr.fat = FALSE;
    Fmt (c->attr.label, "New Curve");
    Fmt (c->attr.note, "");

    c->curvepts = 0;
    c->offset = 0;
    c->pts = 0;
    c->plothandle = FALSE;
    c->x = NULL;
    c->y = NULL;
    Fmt (c->x0.label, "NONE");
    c->x0.reading = 0.0;
    return c;
}

void curvelist_Init (curvelistPtr curves)
{
    list_Init (&curves->list);
    curves->panel = LoadPanel (utilG.p, "curveu.uir", CURVES);
    SetPanelPos (curves->panel, VAL_AUTO_CENTER, VAL_AUTO_CENTER);
    SetPanelMenuBar (curves->panel, curveG.menuBar);
}

void curveG_Init (int graph, int control, char *title, curvelistPtr curves)
{
    curveG.panel = graph;
    curveG.control = control;
    curveG.title = title;
    curveG.curves = curves;
}

void curve_Exit (void)
{
    if (curveG.menuBar) {
        
        DiscardMenuBar (curveG.menuBar);
    }

    acqchan_Exit();
}
