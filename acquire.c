#include <ansi_c.h>
#include <userint.h>
#include <utility.h>
#include <formatio.h>

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
#include "acquire.h"
#include "acquireu.h"
#include "source.h"

#define TRUE 1
#define FALSE 0

struct acquireStruct acqG;
struct expStruct expG = {NULL, NULL, NULL, 0, ACQ_DONE};

static struct datafileStruct {
    char dir[260], name[10], path[260];
    unsigned short ext;
}   dataFile;

static int acqinfoP = 0;

void acquire_Init (void);
void acquire_Exit(void);
int  acquire_GetMenuBar (void);
void acquire_UpdatePanel(void);
void acquire_UpdatePanelExp (void);
void acquire_UpdateDataFileInfo (void);

int  exp_Begin (void);

void exp_UpdateGeneralPanel (void);
void exp_InitGeneralExp (void);
void exp_DoGeneralExp (void);

void acquire_DataFileMakePath (void);


int  AcqSetupCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);



void exp_DoGeneralExp (void)
{
	acqchanlist_GetandSaveReadings();
    graphlist_PlotReadings ();
    util_Delay (expG.delay);
}

void exp_InitGeneralExp (void)
{
    int i;
    graphPtr graph;
    for (i = 0; i < graphG.graphs.nItems; i++)
    {
        graph = graphlist_GetItem (i);
        graph->acqcurve.marker.offset = 0;
        graph->acqcurve.marker.pts = utilG.acq.nPts;
    }
}

void exp_UpdateGeneralPanel (void)
{
    SetInputMode (acqG.p.setup, ACQSETUP_GEN_POINTS, !util_TakingData());
}

void GenExpCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    if (utilG.exp == EXP_SOURCE) {
        utilG.exp = EXP_FLOAT;
        acquire_UpdatePanelExp();
    }

    expG.InitExp = exp_InitGeneralExp;
    expG.UpdatePanel = exp_UpdateGeneralPanel;
    expG.DoExp = exp_DoGeneralExp;
    expG.delay = 0.0;

    GetCtrlVal (panel, ACQSETUP_GEN_POINTS, &utilG.acq.nPts);
            
    exp_UpdateGeneralPanel();
}

int exp_Begin (void)
{
    int size;

/*    if (GetFileInfo (dataFile.path, &size)) {
        MessagePopup ("Begin Experiment Message",
                      "Cannot save data to file w/ duplicate name");
        return FALSE;
    }//*/

    if (acqchanG.channels.nItems < 1)
    {
        MessagePopup ("Begin Experiment Message",
                      "Must select at least one channel for acquisition");
        return FALSE;
    }

    //updateGraphSource();
	if (!acqchanlist_AllocMemory()) return FALSE;

    utilG.acq.status = ACQ_BUSY;

    utilG.acq.pt = 0;
    expG.InitExp();
    acqchanlist_InitDataFile(dataFile.path);

    return TRUE;
}

void acquire_UpdateDataInfoPanel (void)
{
    double pasttime, timeperpt;
    int pts2go;

    if (acqinfoP) {
        pasttime = Timer()-acqchanG.time;
        pts2go = utilG.acq.nPts-1-utilG.acq.pt;
        timeperpt = pasttime/(utilG.acq.pt+1);
        SetCtrlVal (acqinfoP, ACQINFO_PTS2GO, pts2go);
        SetCtrlVal (acqinfoP, ACQINFO_TIMEPERPT, timeperpt);
        SetCtrlVal (acqinfoP, ACQINFO_ELAPSEDTIME, pasttime/60);
        SetCtrlVal (acqinfoP, ACQINFO_TIME2GO, (double)pts2go*timeperpt/60);
    }
}

void acquire_UpdatePanelExp (void)
{
    int bTop, cTop, height, bottom;

/* Floating experiment */

    SetCtrlAttribute (acqG.p.setup, ACQSETUP_GEN_POINTS, ATTR_VISIBLE, utilG.exp == EXP_FLOAT);
    SetCtrlAttribute (acqG.p.setup, ACQSETUP_GEN_DELAY, ATTR_VISIBLE, utilG.exp == EXP_FLOAT);
    SetCtrlAttribute (acqG.p.setup, ACQSETUP_GEN_TIME, ATTR_VISIBLE, utilG.exp == EXP_FLOAT);
    SetMenuBarAttribute (acquire_GetMenuBar(), ACQMENUS_EXP_GENERAL, ATTR_CHECKED, utilG.exp == EXP_FLOAT);

/* Source experiment */

    SetCtrlAttribute (acqG.p.setup, ACQSETUP_SRC_LIST, ATTR_VISIBLE, utilG.exp == EXP_SOURCE);
    SetCtrlAttribute (acqG.p.setup, ACQSETUP_SRC_MOVEUP, ATTR_VISIBLE, utilG.exp == EXP_SOURCE);
    SetCtrlAttribute (acqG.p.setup, ACQSETUP_SRC_MOVEDOWN, ATTR_VISIBLE, utilG.exp == EXP_SOURCE);
    SetCtrlAttribute (acqG.p.setup, ACQSETUP_SRC_REMOVE, ATTR_VISIBLE, utilG.exp == EXP_SOURCE);
    SetCtrlAttribute (acqG.p.setup, ACQSETUP_SRC_POINTS, ATTR_VISIBLE, utilG.exp == EXP_SOURCE);
    SetCtrlAttribute (acqG.p.setup, ACQSETUP_SRC_TIME, ATTR_VISIBLE, utilG.exp == EXP_SOURCE);
    SetMenuBarAttribute (acquire_GetMenuBar(), ACQMENUS_EXP_SOURCE, ATTR_CHECKED, utilG.exp == EXP_SOURCE);


    switch (utilG.exp) {
        case EXP_FLOAT:
            GetCtrlAttribute (acqG.p.setup, ACQSETUP_GEN_DELAY, ATTR_TOP, &cTop);
            GetCtrlAttribute (acqG.p.setup, ACQSETUP_GEN_DELAY, ATTR_HEIGHT, &height);
            SetCtrlVal (acqG.p.setup, ACQSETUP_EXPTITLE, "Floating Experiment");
            break;
        case EXP_SOURCE:
            GetCtrlAttribute (acqG.p.setup, ACQSETUP_SRC_POINTS, ATTR_TOP, &cTop);
            GetCtrlAttribute (acqG.p.setup, ACQSETUP_SRC_POINTS, ATTR_HEIGHT, &height);
            SetCtrlVal (acqG.p.setup, ACQSETUP_EXPTITLE, "source control experiment");
            break;
    }

    GetCtrlAttribute (acqG.p.setup, ACQSETUP_BOX_1, ATTR_TOP, &bTop);
    SetCtrlAttribute (acqG.p.setup, ACQSETUP_BOX_1, ATTR_HEIGHT, cTop+height+10-bTop);
    SetPanelAttribute (acqG.p.setup, ATTR_HEIGHT, cTop+height+36);
}

void acquire_UpdatePanel(void)
{
    int menuBar;

    if (expG.UpdatePanel) expG.UpdatePanel();

    
    menuBar  = GetPanelMenuBar (acqG.p.setup);
    SetMenuBarAttribute (menuBar, ACQMENUS_EXP_GENERAL, ATTR_DIMMED, util_TakingData());
    SetMenuBarAttribute (menuBar, ACQMENUS_EXP_SOURCE, ATTR_DIMMED, util_TakingData());

    SetMenuBarAttribute (menuBar, ACQMENUS_EXP_BEGIN, ATTR_DIMMED, util_TakingData());
    SetMenuBarAttribute (menuBar, ACQMENUS_EXP_PAUSE, ATTR_DIMMED, !(utilG.acq.status == ACQ_BUSY));
    SetMenuBarAttribute (menuBar, ACQMENUS_EXP_CONTINUE, ATTR_DIMMED, !(utilG.acq.status == ACQ_PAUSED));
    SetMenuBarAttribute (menuBar, ACQMENUS_EXP_END, ATTR_DIMMED, !util_TakingData());

    SetMenuBarAttribute (menuBar, ACQMENUS_EXP_INFO, ATTR_DIMMED, !util_TakingData());

    acqchanlist_Dimmed (util_TakingData());

    switch (utilG.acq.status) {
        case ACQ_BUSY:
            SetCtrlVal (acqG.p.setup, ACQSETUP_STATUS, "BUSY");
            break;
        case ACQ_STOPPED:
            SetCtrlVal (acqG.p.setup, ACQSETUP_STATUS, "STOPPED");
            break;
        case ACQ_DONE:
            SetCtrlVal (acqG.p.setup, ACQSETUP_STATUS, "DONE");
            break;
        case ACQ_PAUSED:
            SetCtrlVal (acqG.p.setup, ACQSETUP_STATUS, "PAUSED");
            break;
    }
}

int acquire_GetMenuBar (void)
{
    return GetPanelMenuBar (acqG.p.setup);
}

void acquire_Init (void)
{
    char *date, mon[10], day[10], yr[10];
    if (utilG.acq.status != ACQ_NONE) {
        util_ChangeInitMessage ("Acquisition Utilities...");

        acqG.p.setup = LoadPanel (utilG.p, "acquireu.uir", ACQSETUP);
        
        //SetPanelPos (acqG.p.setup, VAL_AUTO_CENTER, VAL_AUTO_CENTER);

        GetProjectDir (dataFile.dir);
        date = DateStr();
        Scan (date, "%s>%s[xt45]%s[xt45]%s", mon, day, yr);
        Fmt (dataFile.name ,"%s<%s[i2]%s%s", yr, mon, day);
        dataFile.ext = 1;
        acquire_UpdateDataFileInfo();

        InstallCtrlCallback (utilG.p, BG_ACQSETUP, AcqSetupCallback, 0);

    } else SetCtrlAttribute (utilG.p, BG_ACQSETUP, ATTR_VISIBLE, FALSE);
}

void acquire_Exit(void)
{
    if (utilG.acq.status != ACQ_NONE) {
        
		DiscardPanel (acqG.p.setup);

        if (acqinfoP) {
            
            DiscardPanel (acqinfoP);
        }
    }

    graphG_Exit();
}

void ExpStatusCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    switch (menuItem)
    {
        case ACQMENUS_EXP_BEGIN:
            utilG.acq.status = ACQ_BEGIN;
            break;
        case ACQMENUS_EXP_PAUSE:
            utilG.acq.status = ACQ_PAUSED;
            break;
        case ACQMENUS_EXP_CONTINUE:
            utilG.acq.status = ACQ_BUSY;
            break;
        case ACQMENUS_EXP_END:
            utilG.acq.status = ACQ_TERMINATE;
            break;
    }
    acquire_UpdatePanel();
}

int  GenExpControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    if (event == EVENT_VAL_CHANGED){
        GetCtrlVal (panel, ACQSETUP_GEN_POINTS, &utilG.acq.nPts);
        GetCtrlVal (panel, ACQSETUP_GEN_DELAY, &expG.delay);
        SetCtrlVal (panel, ACQSETUP_GEN_TIME, (double)utilG.acq.nPts*expG.delay/60);
    }
    return 0;
}

void AcqInfoCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    if (!acqinfoP) {
        acqinfoP = LoadPanel (utilG.p, "acquireu.uir", ACQINFO);
        if (acqinfoP < 0) acqinfoP = 0;
        else {
            SetPanelPos (acqinfoP, VAL_AUTO_CENTER, VAL_AUTO_CENTER);
            
            DisplayPanel (acqinfoP);
        }
    }
}

int  RemoveAcqInfoCallback(int panel, int event, void *callbackData, int eventData1, int eventData2)
{
    if (((event == EVENT_KEYPRESS) && (eventData1 == VAL_ESC_VKEY)) || (event == EVENT_RIGHT_DOUBLE_CLICK))
	{
		DiscardPanel (panel);
        acqinfoP = 0;
    }
    return 0;
}

int  AcqSetupCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    if (event == EVENT_COMMIT) {
        acquire_UpdatePanel();
        DisplayPanel (acqG.p.setup);
    }
    return 0;
}

int  DataFileControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    char name[256];
    int ext, err, size;

    if (event == EVENT_VAL_CHANGED) {
        Fmt (name, dataFile.name);
        ext = dataFile.ext;

        GetCtrlVal (panel, ACQSETUP_FILENAME, dataFile.name);
        GetCtrlVal (panel, ACQSETUP_FILEEXT, &dataFile.ext);

        acquire_DataFileMakePath();
        SetCtrlVal (panel, ACQSETUP_FILEPATH, dataFile.path);
    }
    return 0;
}

int  DataFileSaveAsCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    char path[300], path1[300], path2[300], name[300];
    int i, filestatus;

    if (event == EVENT_COMMIT) {
        Fmt (name, "%s<%s.%i", dataFile.name, dataFile.ext);
        filestatus = FileSelectPopup (dataFile.dir, name, "",
                                      "Save As [file extension must be a #!]:",
                                      VAL_OK_BUTTON, 0, 0, 1, 1, path);
        switch (filestatus) {
            case VAL_EXISTING_FILE_SELECTED:
                MessagePopup ("Save As Message", "Cannot select duplicate file paths");
                break;
            case VAL_NEW_FILE_SELECTED:
                Fmt (path1, path);
                while (Scan(path1, "%s>%s[xdt92]%s", path2) == 2)
                    Fmt (path1, path2);
                Scan (path2, "%s>%s[t46]", dataFile.name);
                i = Scan (path2, "%s>%s[xdt46]%i[b2]", &dataFile.ext);
                if (i != 2)
                    MessagePopup ("File name error",
                              "File extension must be a number..."
                              "extension ignored");
                Fmt (path1, path);
                i = FindPattern (path1, 0, StringLength (path1), path2, 0, 0);
                CopyString (dataFile.dir, 0, path1, 0, i-1);
                acquire_UpdateDataFileInfo();
                break;
        }
    }
    return 0;
}

void acquire_UpdateDataFileInfo (void)
{
    SetInputMode (acqG.p.setup, ACQSETUP_FILENAME, !util_TakingData());
    SetInputMode (acqG.p.setup, ACQSETUP_FILEEXT, !util_TakingData());
    SetInputMode (acqG.p.setup, ACQSETUP_SAVEAS, !util_TakingData());

    SetCtrlVal (acqG.p.setup, ACQSETUP_FILENAME, dataFile.name);
    SetCtrlVal (acqG.p.setup, ACQSETUP_FILEEXT, dataFile.ext);
    acquire_DataFileMakePath();
    SetCtrlVal (acqG.p.setup, ACQSETUP_FILEPATH, dataFile.path);
}


void acquire_IncDataFileExt(void)
{
	dataFile.ext++;
    acquire_UpdateDataFileInfo ();
}

void acquire_DataFileMakePath (void)
{
   	char name[80], ext[10];

	Fmt (ext, "%s<%f[p3]", (double)dataFile.ext/1000);
    CopyBytes (ext, 0, ext, 1, 5);
	
    Fmt(name, "%s<%s%s", dataFile.name, ext);
    MakePathname(dataFile.dir, name, dataFile.path);
}
int CVICALLBACK BeepCallback (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	int b;
	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal(panel,control, &b);
			utilG.beep = b;
		break;
	}
	return 0;
}

