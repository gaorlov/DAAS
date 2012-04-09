#include <utility.h>
#include <formatio.h>
#include <ansi_c.h>
#include <userint.h>

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
#include "graph.h"
#include "graphu.h"
#include "curveop.h"
#include "acquire.h"
#include "acquireu.h"
#include "gpibio.h"
#include "gpibiou.h"
#include "source.h"
#include "sourceu.h"

#include "k236.h"
#include "k236u.h"

#define K236_ID "236A06"

#define K236_SRE_SRQDISABLED 0
#define K236_SRE_WARNING 1
#define K236_SRE_SWEEPDONE 2
#define K236_SRE_TRIGGEROUT 4
#define K236_SRE_READINGDONE 8
#define K236_SRE_READYFORTRIGGER 16
#define K236_SRE_ERROR 32
#define K236_SRE_COMPLIANCE 128

#define TRUE 1
#define FALSE 0

typedef struct {
    acqchanPtr measure;
    sourcePtr source;
    int id, overLimit, sourceOn;
    int operate, sense;
    struct {int range, delay, current;} src;
    struct {int range, filter, inttime, supOn;} meas;
}   k236Type;

typedef k236Type *k236Ptr;

static void k236_GetErrorStatus (gpibioPtr dev);
static void k236_GetWarningStatus (gpibioPtr dev);

static void k236_In (gpibioPtr dev, char *msg);
static void k236_Out (gpibioPtr dev, char *cmd, double delay);
static void k236_CheckforProblems (gpibioPtr dev);
static void k236_GetLevels (gpibioPtr dev);
static void k236_GetReadings (acqchanPtr acqchan);
static void k236_SetLevel (sourcePtr src);

static void k236_GetMachineStatus (gpibioPtr dev);
static void k236_GetMeasurementParameters (gpibioPtr dev);
static double k236_GetCompLevel (gpibioPtr dev);
static double k236_GetSupLevel (gpibioPtr dev);
static void k236_Save (gpibioPtr dev);
static void k236_Load (gpibioPtr dev);
static void k236_Remove (void *ptr);

static void k236_UpdateControls (int panel, gpibioPtr dev);

static void k236_Meas_UpdateReadings (int panel, void *smu);

static int  k236_InitGPIB (gpibioPtr dev);
static void *k236_Create (gpibioPtr dev);
static void OperateK236 (int menubar, int menuItem, void *callbackData, int panel);
static void k236_UpdateReadings (int panel, void *dev);

void k236_Init (void)
{
    devTypePtr devType;
    if (utilG.acq.status != ACQ_NONE) {
        util_ChangeInitMessage ("Keithley 236 Control Utilities...");
        devType = malloc (sizeof (devTypeItem));
        if (devType) {
            Fmt (devType->label, "Keithley 236 SMU");
            Fmt (devType->id, K236_ID);
            devType->InitDevice = k236_InitGPIB;
            devType->CreateDevice = k236_Create;
            devType->OperateDevice = OperateK236;
            devType->UpdateReadings = k236_UpdateReadings;
            devType->SaveDevice = k236_Save;
            devType->LoadDevice = k236_Load;
            devType->RemoveDevice = k236_Remove;
            devTypeList_AddItem (devType);
        }
    }
}

static void k236_UpdateReadings (int panel, void *dev)
{
    gpibioPtr my_dev = dev;
    k236Ptr smu = my_dev->device;
    short statusbyte;
    char rsp[256];
    int control, dim, bg, mode, m;

	if (smu->operate) {
        if (utilG.acq.status == ACQ_BUSY)
			Delay(.05);
		if (!util_TakingData() || !(smu->source->acqchan->acquire || smu->measure->acquire)) {
            gpibio_GetStatusByte (dev, &statusbyte);
            if (statusbyte & K236_SRE_READINGDONE) 
			{
				k236_In (dev, rsp);
                Scan (rsp, "%s>%f,%f", &smu->source->acqchan->reading,
                      &smu->measure->reading);
            }
            if (statusbyte & K236_SRE_READYFORTRIGGER) 
				k236_Out (dev, "H0X", .02);
        }

        //if (expG.acqstatus != utilG.acq.status) 
		{
            m = GetPanelMenuBar (panel);
            dim = (util_TakingData() && smu->source->inlist &&
                   (utilG.exp == EXP_SOURCE));

            if (dim) { mode = VAL_INDICATOR; bg = VAL_PANEL_GRAY;}
                else { mode = VAL_HOT; bg = VAL_WHITE;}

            SetCtrlAttribute (panel, K236_SOURCE, ATTR_CTRL_MODE, mode);
            SetCtrlAttribute (panel, K236_SOURCE, ATTR_TEXT_BGCOLOR, bg);

            SetInputMode (panel, K236_OPERATE, !dim);
            SetInputMode (panel, K236_SELECT, !dim);
            SetInputMode (panel, K236_SENSE, !dim);
            SetMenuBarAttribute (m, K236MENUS_FILE_LOAD, ATTR_DIMMED, dim);
        }

        k236_CheckforProblems (my_dev);
        SetCtrlVal (panel, K236_OVERLIMIT, smu->overLimit);

        control = GetActiveCtrl (panel);
        if (util_TakingData() || (control != K236_SOURCE))
            SetCtrlVal (panel, K236_SOURCE, smu->source->acqchan->reading);
        SetCtrlVal (panel, K236_MEASURE, smu->measure->reading);
    }
}

static void *k236_Create (gpibioPtr dev)
{
    k236Ptr smu;

    smu = malloc (sizeof(k236Type));
    smu->overLimit = FALSE;
    if (dev) smu->id = dev->id;
    smu->src.range = 0; /* AUTO */
    smu->source = source_Create ("K236 Source", dev, k236_SetLevel,
                                 k236_GetReadings);
    smu->measure = acqchan_Create ("K236 Measure", dev, k236_GetReadings);
	smu->sourceOn = 0;
    if (dev) dev->device = smu;
    return smu;
}

static int k236_InitGPIB (gpibioPtr dev)
{
    char cmd[256], rsp[256], buffer[256];
    int result;

    gpibio_Remote (dev); 
    
    if (gpibio_DeviceMatch (dev, "U0X", K236_ID)) {
        gpibio_Out (dev, "M128,0X"); /* SRQ Compliance */
        gpibio_Out (dev, "G5,2,X");
        gpibio_Out (dev, "R0X");
        gpibio_Out (dev, "T4,1,4,0X");
        gpibio_Out (dev, "R1X");
        return TRUE;
    }
    return FALSE;
}

void LoadK236Callback(int menubar, int menuItem, void *callbackData, int panel)
{
    int fileselect, id;
    char path[256], info[256];
    gpibioPtr dev = callbackData;
    k236Ptr smu = dev->device;

    fileselect = FileSelectPopup ("", "*.dev", "*.dev", "Load Keithley 236 SMU Setup",
                                  VAL_LOAD_BUTTON, 0, 1, 1, 0, path);
    if (fileselect == VAL_EXISTING_FILE_SELECTED) {
        fileHandle.analysis = util_OpenFile (path, FILE_READ, FALSE);
        ScanFile (fileHandle.analysis, "%s>#INSTRSETUP %i", &id);
        if (smu->id == id) {
            k236_Load (dev);
            ReadLine (fileHandle.analysis, info, 255);
            k236_UpdateControls(panel, dev);
        }
        else MessagePopup ("Keithley Load Message", "Different instrument types--process aborted");
        util_CloseFile();
    }
}

void SaveK236Callback(int menubar, int menuItem, void *callbackData, int panel)
{
    int fileselect;
    char path[256];
    gpibioPtr dev = callbackData;
    k236Ptr smu = dev->device;
    fileselect = FileSelectPopup ("", "*.dev", "*.dev", "Save K236 Setup",
                                  VAL_SAVE_BUTTON, 0, 1, 1, 1, path);
    if (fileselect == VAL_NEW_FILE_SELECTED) {
        fileHandle.analysis = util_OpenFile (path, FILE_WRITE, FALSE);
        FmtFile (fileHandle.analysis, "%s<#INSTRSETUP %i\n", smu->id);
        k236_Save(dev);
        FmtFile (fileHandle.analysis, "#ENDSETUP\n");
        util_CloseFile();
    }
}

int  K236MeasControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    acqchanPtr acqchan = callbackData;
    switch (control) {
        case K236_MEAS_ACQ:
            if (event == EVENT_VAL_CHANGED) {
                GetCtrlVal (panel, control, &acqchan->acquire);
                if (acqchan->acquire) acqchanlist_AddChannel (acqchan);
                    else acqchanlist_RemoveChannel (acqchan);
            }
            break;
        case K236_MEAS_COEFF:
            if (event == EVENT_COMMIT) {
                GetCtrlVal (panel, control, &acqchan->coeff);
                if (acqchan->p) SetCtrlVal (acqchan->p, ACQDATA_COEFF, acqchan->coeff);
            }
            break;
        case K236_MEAS_LABEL:
            if (event == EVENT_COMMIT) {
                GetCtrlVal (panel, control, acqchan->channel->label);
                acqchanlist_ReplaceChannel (acqchan);
                if (acqchan->p) SetPanelAttribute (acqchan->p, ATTR_TITLE, acqchan->channel->label);
            }
            break;
        case K236_MEAS_CLOSE:
            if (event == EVENT_COMMIT) {
                devPanel_Remove(panel);
                
                DiscardPanel (panel);
            }
            break;
    }
    return 0;
}

static void k236_Meas_UpdateReadings (int panel, void *measchan)
{
    acqchanPtr measure = measchan;

    SetCtrlVal (panel, K236_MEAS_MEAS, measure->reading);
}

void K236MeasureSetupCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    int p;
    acqchanPtr measure = callbackData;

    p = LoadPanel (0, "k236u.uir", K236_MEAS);
    SetPanelPos (p, VAL_AUTO_CENTER, VAL_AUTO_CENTER);
    util_InitClose (p, K236_MEAS_CLOSE, FALSE);

    SetCtrlVal (p, K236_MEAS_LABEL, measure->channel->label);
    SetCtrlVal (p, K236_MEAS_COEFF, measure->coeff);
    SetCtrlVal (p, K236_MEAS_ACQ, measure->acquire);
    SetCtrlVal (p, K236_MEAS_NOTE, measure->note);

    SetCtrlAttribute(p, K236_MEAS_LABEL, ATTR_CALLBACK_DATA, measure);
    SetCtrlAttribute(p, K236_MEAS_COEFF, ATTR_CALLBACK_DATA, measure);
    SetCtrlAttribute(p, K236_MEAS_ACQ, ATTR_CALLBACK_DATA, measure);
    SetCtrlAttribute(p, K236_MEAS_NOTE, ATTR_CALLBACK_DATA, measure);

    SetInputMode (p, K236_MEAS_ACQ, !util_TakingData());

    devPanel_Add (p, measure, k236_Meas_UpdateReadings);
    InstallPopup (p);
}

void K236SourceSetupCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    sourcePtr src = callbackData;
	switch (utilG.exp) {
        case EXP_SOURCE: source_InitPanel (src); break;
        case EXP_FLOAT: gensrc_InitPanel (src); break;
    }
}

int  K236SourceCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    sourcePtr src = callbackData;

    if (event == EVENT_COMMIT) {
        GetCtrlVal (panel, control, &src->biaslevel);
        k236_SetLevel (src);
    }
    return 0;
}

int  K236ControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    gpibioPtr dev = callbackData;
    k236Ptr smu = dev->device;
    char cmd[256];
    double r, level;
    int id;

    switch (control) {
        case K236_SUPPRESS:
            if (event == EVENT_COMMIT) {
                GetCtrlVal (panel, control, &smu->meas.supOn);
                Fmt (cmd, "Z%iX", smu->meas.supOn);
                k236_Out (dev, cmd, .01);
            }
            break;
        case K236_INTTIME:
            if (event == EVENT_COMMIT) {
                GetCtrlVal (panel, control, &smu->meas.inttime);
                Fmt (cmd, "S%iX", smu->meas.inttime);
                k236_Out (dev, cmd, .07);
            }
            break;
        case K236_FILTER:
            if (event == EVENT_COMMIT) {
                GetCtrlVal (panel, control, &smu->meas.filter);
                Fmt (cmd, "P%iX", smu->meas.filter);
                k236_Out (dev, cmd, .02);
            }
            break;
        case K236_MEAS_V_RANGE:
        case K236_MEAS_I_RANGE:
            if (event == EVENT_COMMIT) {
                GetCtrlIndex (panel, control, &smu->meas.range);
                GetCtrlVal (panel, control, &r);
                level = k236_GetCompLevel(dev);
                if (r < level) level = r;
                Fmt (cmd, "L%f[e2p4],%iX", level, smu->meas.range);
                k236_Out (dev, cmd, .04);
            }
            break;
        case K236_LEVEL:
            if (event == EVENT_COMMIT) {
                GetCtrlVal (panel, control, &r);
                Fmt (cmd, "L%f[e2p4],%iH0X", r, smu->meas.range); /* trigger level change */
                k236_Out (dev, cmd, .04);
            }
            break;
        case K236_SENSE:
            if (event == EVENT_COMMIT) {
                GetCtrlVal (panel, control, &smu->sense);
				Fmt (cmd, "O%iX", smu->sense);
                k236_Out (dev, cmd, .02);
            }
            break;
        case K236_SELECT:
            if (event == EVENT_COMMIT) {
                GetCtrlVal (panel, control, &smu->src.current);
                Fmt (cmd, "F%i,0X", smu->src.current);
                k236_Out (dev, cmd, .07);
            }
            break;
        case K236_OPERATE:
            if (event == EVENT_COMMIT) {
                GetCtrlVal (panel, control, &smu->operate);
                Fmt (cmd, "N%iX", smu->operate);
                k236_Out (dev, cmd, .02);
            }
            break;
        case K236_IDENTIFY:
            if (event == EVENT_COMMIT) {
                GetCtrlVal (panel, control, &id);
				if(id)
				{
					Fmt(cmd, "D1,GPIB ADDR %i X", dev->paddr);
					k236_Out(dev, cmd, .5);
				}
				else
					k236_Out(dev, "D0X", .015);
            }
            break;
        case K236_DELAY:
            if (event == EVENT_COMMIT) {
                GetCtrlVal (panel, control, &smu->src.delay);
                Fmt (cmd, "W%iX", smu->src.delay);
                k236_Out (dev, cmd, .02);
            }
            break;
        case K236_SOURCE_V_RANGE:
        case K236_SOURCE_I_RANGE:
            if (event == EVENT_COMMIT) {
                GetCtrlIndex (panel, control, &smu->src.range);
                GetCtrlVal (panel, control, &r);
                GetCtrlVal (panel, K236_SOURCE, &level);
                if (r < level) level = r;
                Fmt (cmd, "%s<B%f[e2p4],%i,0X", level, smu->src.range);
                k236_Out (dev, cmd, .02);
                smu->source->min = -level;
                smu->source->max = level;
            }
            break;
    }

    if (event == EVENT_COMMIT) {
        k236_UpdateControls(panel, dev);
    } //*/
    return 0;
}


int  K236ControlPanelCallback(int panel, int event, void *callbackData, int eventData1, int eventData2)
{
    gpibioPtr dev = callbackData;
    k236Ptr smu = dev->device;
    int menubar;

    if ((event == EVENT_KEYPRESS && eventData1 == VAL_ESC_VKEY) || event == EVENT_RIGHT_DOUBLE_CLICK) {
        devPanel_Remove (panel);
		DiscardPanel (panel);
		dev->iPanel = 0;
		SetMenuBarAttribute (acquire_GetMenuBar(), dev->menuitem_id, ATTR_DIMMED, FALSE);
    }

    if (event == EVENT_GOT_FOCUS) {
        menubar = GetPanelMenuBar (panel);
        SetPanelAttribute (panel, ATTR_DIMMED, (dev->status != DEV_REMOTE));
        SetMenuBarAttribute (menubar, K236MENUS_FILE_SAVE, ATTR_DIMMED, (dev->status != DEV_REMOTE));
        SetMenuBarAttribute (menubar, K236MENUS_FILE_LOAD, ATTR_DIMMED, (dev->status != DEV_REMOTE));
        SetMenuBarAttribute (menubar, K236MENUS_SOURCE, ATTR_DIMMED, (dev->status != DEV_REMOTE));
        SetMenuBarAttribute (menubar, K236MENUS_MEASURE, ATTR_DIMMED, (dev->status != DEV_REMOTE));

        if (!util_TakingData()) k236_UpdateControls (panel, dev);
    }
    return 0;
}

static void OperateK236 (int menubar, int menuItem, void *callbackData, int panel)
{
    int p, m;
    gpibioPtr dev = callbackData;
    k236Ptr smu = dev->device;
    char label[256];

    SetMenuBarAttribute (menubar, menuItem, ATTR_DIMMED, TRUE);

    p = dev->iPanel? dev->iPanel:LoadPanel (utilG.p, "k236u.uir", K236);
	dev->iPanel = p;
    
    SetPanelPos (p, VAL_AUTO_CENTER, VAL_AUTO_CENTER);

    Fmt (label, "Keithley 236 Source Measure Unit: %s", dev->label);
    SetPanelAttribute (p, ATTR_TITLE, label);

    m = LoadMenuBar (p, "k236u.uir", K236MENUS);
    
    SetPanelMenuBar (p, m);

    SetMenuBarAttribute (m, K236MENUS_FILE_SAVE, ATTR_CALLBACK_DATA, dev);
    SetMenuBarAttribute (m, K236MENUS_FILE_LOAD, ATTR_CALLBACK_DATA, dev);
    SetMenuBarAttribute (m, K236MENUS_FILE_GPIB, ATTR_CALLBACK_DATA, dev);
    SetMenuBarAttribute (m, K236MENUS_SOURCE, ATTR_CALLBACK_DATA, smu->source);
    SetMenuBarAttribute (m, K236MENUS_MEASURE, ATTR_CALLBACK_DATA, smu->measure);

    SetPanelAttribute (p, ATTR_CALLBACK_DATA, dev);

    SetCtrlAttribute (p, K236_SOURCE, ATTR_CALLBACK_DATA, smu->source);

    SetCtrlAttribute (p, K236_SOURCE_I_RANGE, ATTR_CALLBACK_DATA, dev);
    SetCtrlAttribute (p, K236_SOURCE_V_RANGE, ATTR_CALLBACK_DATA, dev);
    SetCtrlAttribute (p, K236_DELAY, ATTR_CALLBACK_DATA, dev);
    SetCtrlAttribute (p, K236_IDENTIFY, ATTR_CALLBACK_DATA, dev);
    SetCtrlAttribute (p, K236_OPERATE, ATTR_CALLBACK_DATA, dev);
    SetCtrlAttribute (p, K236_SELECT, ATTR_CALLBACK_DATA, dev);
    SetCtrlAttribute (p, K236_SENSE, ATTR_CALLBACK_DATA, dev);
    SetCtrlAttribute (p, K236_MEAS_I_RANGE, ATTR_CALLBACK_DATA, dev);
    SetCtrlAttribute (p, K236_MEAS_V_RANGE, ATTR_CALLBACK_DATA, dev);
    SetCtrlAttribute (p, K236_LEVEL, ATTR_CALLBACK_DATA, dev);
    SetCtrlAttribute (p, K236_FILTER, ATTR_CALLBACK_DATA, dev);
    SetCtrlAttribute (p, K236_INTTIME, ATTR_CALLBACK_DATA, dev);
    SetCtrlAttribute (p, K236_SUPPRESS, ATTR_CALLBACK_DATA, dev);

    k236_UpdateControls (p, dev);

    devPanel_Add (p, dev, k236_UpdateReadings);
    DisplayPanel (p);
}

static void k236_UpdateControls (int panel, gpibioPtr dev)
{
    char s_text[20], m_text[20];
    k236Ptr smu = dev->device;
    double absmax;
	
    k236_GetMachineStatus (dev);
    k236_GetMeasurementParameters (dev);

    SetCtrlVal (panel, K236_SOURCE, smu->source->acqchan->reading);
    SetCtrlVal (panel, K236_MEASURE, smu->measure->reading);

    SetCtrlVal (panel, K236_OPERATE, smu->operate);
    SetInputMode (panel, K236_SOURCE, smu->operate);
    SetInputMode (panel, K236_MEASURE, smu->operate);

    SetCtrlVal (panel, K236_SELECT, smu->src.current);
    SetCtrlVal (panel, K236_SENSE, smu->sense);

    if (smu->src.current) {
        Fmt (s_text, "A"); Fmt (m_text, "V");
    } else {
        Fmt (s_text, "V"); Fmt (m_text, "A");
    }

    SetCtrlVal (panel, K236_SOURCETEXT, s_text);
    SetCtrlVal (panel, K236_MEASURETEXT, m_text);

    SetCtrlAttribute (panel, K236_SOURCE_I_RANGE, ATTR_VISIBLE, smu->src.current);
    SetCtrlAttribute (panel, K236_SOURCE_V_RANGE, ATTR_VISIBLE, !smu->src.current);

    if (smu->src.current) {
        SetCtrlIndex (panel, K236_SOURCE_I_RANGE, smu->src.range);
        GetCtrlVal (panel, K236_SOURCE_I_RANGE, &smu->source->max);
    } else {
        SetCtrlIndex (panel, K236_SOURCE_V_RANGE, smu->src.range);
        GetCtrlVal (panel, K236_SOURCE_V_RANGE, &smu->source->max);
    }

    smu->source->min = -smu->source->max;
    SetCtrlAttribute (panel, K236_SOURCE, ATTR_MIN_VALUE, smu->source->min);
    SetCtrlAttribute (panel, K236_SOURCE, ATTR_MAX_VALUE, smu->source->max);
    SetCtrlVal (panel, K236_DELAY, smu->src.delay);

    SetCtrlAttribute (panel, K236_MEAS_I_RANGE, ATTR_VISIBLE, !smu->src.current);
    SetCtrlAttribute (panel, K236_MEAS_V_RANGE, ATTR_VISIBLE, smu->src.current);

    SetCtrlVal (panel, K236_LEVEL, k236_GetCompLevel (dev));

    if (smu->src.current) {
        SetCtrlIndex (panel, K236_MEAS_V_RANGE, smu->meas.range);
        GetCtrlVal (panel, K236_MEAS_V_RANGE, &absmax);
    } else {
        SetCtrlIndex (panel, K236_MEAS_I_RANGE, smu->meas.range);
        GetCtrlVal (panel, K236_MEAS_I_RANGE, &absmax);
    }

    SetCtrlAttribute (panel, K236_LEVEL, ATTR_MIN_VALUE, -absmax);
    SetCtrlAttribute (panel, K236_LEVEL, ATTR_MAX_VALUE, absmax);

    SetCtrlVal (panel, K236_FILTER, smu->meas.filter);
    SetCtrlVal (panel, K236_INTTIME, smu->meas.inttime);

    SetCtrlVal (panel, K236_SUPPRESS, smu->meas.supOn);
    SetInputMode (panel, K236_SUPDISPLAY, smu->meas.supOn);
    if (smu->meas.supOn)
        SetCtrlVal (panel, K236_SUPDISPLAY, k236_GetSupLevel(dev));
}

static void k236_Remove (void *ptr)
{
    k236Ptr smu = ptr;
    acqchan_Remove(smu->measure);
    source_Remove (smu->source);
    free (smu);
}

static void k236_Load (gpibioPtr dev)
{
    char cmd[256];
    double r;
    int i;
    k236Ptr smu;

    if (dev) smu = dev->device;
    else smu = k236_Create (NULL);

/*  if (dev) k236_Out (dev, "N0X");*/

    ScanFile (fileHandle.analysis, "%s>Sense   : %i%s[dw1]", &smu->sense);
    if (dev) {Fmt (cmd, "O%iX", smu->sense); k236_Out (dev, cmd, 0);}

    ScanFile (fileHandle.analysis, "%s>Source  : %i, %i%s[dw1]",
              &smu->src.current, &smu->src.delay);
    if (dev) {
        Fmt (cmd, "F%i,0B,0,0W%iX", smu->src.current, smu->src.delay);
        k236_Out (dev, cmd, 0);
    }
    smu->src.range = 0; /*AUTO*/
    smu->source->min = -110.0;
    smu->source->max = 110.0;
    source_Load (dev, smu->source);

    ScanFile (fileHandle.analysis, "%s>Measure : %f, %i, %i, %i\n",
             &r, &smu->meas.range, &smu->meas.filter, &smu->meas.inttime);
    if (dev) {
        Fmt (cmd, "L%f[e2p4],%iP%iS%iZ0H0X",
             r, smu->meas.range, smu->meas.filter, smu->meas.inttime);
        k236_Out (dev, cmd, 0);
    }
    smu->meas.supOn = FALSE;

    acqchan_Load (dev, smu->measure);

    if (!dev) k236_Remove (smu);
}

static void k236_Save (gpibioPtr dev)
{
    k236Ptr smu = dev->device;

    k236_GetMachineStatus(dev);
    k236_GetMeasurementParameters(dev);

    FmtFile (fileHandle.analysis, "Sense   : %i\n", smu->sense);
    FmtFile (fileHandle.analysis, "Source  : %i, %i\n", smu->src.current, smu->src.delay);
    source_Save (smu->source);
    FmtFile (fileHandle.analysis, "Measure : %f[e2p5], %i, %i, %i\n",
             k236_GetCompLevel (dev), smu->meas.range,
             smu->meas.filter, smu->meas.inttime);
    acqchan_Save (smu->measure);
}

static double k236_GetSupLevel (gpibioPtr dev)
{
    char msg[256];
    double level;

    k236_Out (dev, "U6X", .008);
    k236_In (dev, msg);

    Scan (msg, "%s[i3]>%f", &level);
    return level;
}

static double k236_GetCompLevel (gpibioPtr dev)
{
    char msg[256];
    double level;

    k236_Out (dev, "U5X", .008);
    k236_In (dev, msg);

    Scan (msg, "%s[i3]>%f", &level);
    return level;
}

static void k236_GetMeasurementParameters (gpibioPtr dev)
{
    char msg[256];
    int srctype;
    k236Ptr smu = dev->device;

    k236_Out (dev, "U4X", .008);
    k236_In (dev, msg);

    Scan (msg, "%s[i3]>L,%i", &smu->meas.range);
    Scan (msg, "%s[i7]>F%i",  &smu->src.current);
    Scan (msg, "%s[i11]>O%i", &smu->sense);
    Scan (msg, "%s[i13]>P%i", &smu->meas.filter);
    Scan (msg, "%s[i15]>S%i", &smu->meas.inttime);
    Scan (msg, "%s[i17]>W%i", &smu->src.delay);
    Scan (msg, "%s[i19]>Z%i", &smu->meas.supOn);
}

static void k236_GetMachineStatus (gpibioPtr dev)
{
    char msg[256];
    k236Ptr smu = dev->device;

    k236_Out (dev, "U3X", .008);
    k236_In (dev, msg);

    Scan (msg, "%s[i19]>%i", &smu->operate);
}

static void k236_SetLevel (sourcePtr src)
{
    char cmd[256];
    gpibioPtr dev = src->acqchan->dev;

    Fmt (cmd, "%s<B%f[e2p4],,0H0X", src->biaslevel); /* trigger new source value */
    k236_Out (dev, cmd, .02);
    util_Delay (src->segments[src->seg]->delay);
}

static void k236_GetReadings (acqchanPtr acqchan)
{
    gpibioPtr dev = acqchan->dev;
	k236Ptr smu = dev->device;
	if (!smu->source->sourceOn)
		k236_GetLevels (dev);
}

static void k236_GetLevels (gpibioPtr dev)
{
    char msg[256];
    short statusbyte;
    k236Ptr smu = dev->device;

    gpibio_GetStatusByte (dev, &statusbyte);
	util_Delay(.05);
    while (!(statusbyte & K236_SRE_READINGDONE)) 
	{
        if (statusbyte & K236_SRE_READYFORTRIGGER)
			k236_Out (dev, "H0X", .02);
		ProcessSystemEvents();
		util_Delay(.05);
        gpibio_GetStatusByte (dev, &statusbyte);
    }
	k236_Out (dev, "H0X", .02);
	k236_In (dev, msg);
    Scan (msg, "%s>%f,%f", &smu->source->acqchan->reading, &smu->measure->reading);
    smu->source->acqchan->newreading = TRUE;
    smu->measure->newreading = TRUE;
}

static void k236_CheckforProblems (gpibioPtr dev)
{
	char msg[256];
    short statusbyte;
    k236Ptr smu = dev->device;

    gpibio_GetStatusByte (dev, &statusbyte);

    if (statusbyte & K236_SRE_WARNING) k236_GetWarningStatus (dev);
    if (statusbyte & K236_SRE_ERROR) k236_GetErrorStatus (dev);

    if (gpibio_SRQ(dev)) gpibio_GetStatusByte (dev, &statusbyte);
    smu->overLimit = statusbyte & K236_SRE_COMPLIANCE;
	//*/
}

static void k236_Out (gpibioPtr dev, char *cmd, double delay)
{
	gpibio_Out (dev, cmd);
	Delay(delay);
}

static void k236_In (gpibioPtr dev, char *msg)
{
	gpibio_In (dev, msg);
	Delay(.02);
}

static void k236_GetWarningStatus (gpibioPtr dev)
{
	int i, byte;
    char msg[256], rsp[256];

    k236_Out (dev, "U9X", .008);
    k236_In (dev, rsp);

    Scan (rsp, "%s>WRS%s", msg);
    for (i = 0; i<9; i++) {
        byte = msg[i];
        if (byte == 49) {
            switch (i) {
                case 0:
                    util_MessagePopup ("Keithley 236 Message", "WARNING: Uncalibrated");
                    break;
                case 1:
                    util_MessagePopup ("Keithley 236 Message", "WARNING: Temporary cal");
                    break;
                case 2:
                    util_MessagePopup ("Keithley 236 Message", "WARNING: Value out of range");
                    break;
                case 3:
                    util_MessagePopup ("Keithley 236 Message", "WARNING: Sweep buffer filled");
                    break;
                case 4:
                    util_MessagePopup ("Keithley 236 Message", "WARNING: No sweep points, must create");
                    break;
                case 5:
                    util_MessagePopup ("Keithley 236 Message", "WARNING: Pulse times not met");
                    break;
                case 6:
                    util_MessagePopup ("Keithley 236 Message", "WARNING: Not in remote");
                    break;
                case 7:
                    util_MessagePopup ("Keithley 236 Message", "WARNING: Measure range changed");
                    break;
                case 8:
                    util_MessagePopup ("Keithley 236 Message", "WARNING: Measurement overflow (OFLO)/Sweep aborted");
                    break;
                case 9:
                    util_MessagePopup ("Keithley 236 Message", "WARNING: Pending trigger");
                    break;
            }
        }
    } //*/
}

static void k236_GetErrorStatus (gpibioPtr dev)
{
    int i, byte;
    char msg[256], rsp[256];

    k236_Out (dev, "U1X", .008);
    k236_In (dev, rsp);

    Scan (rsp, "%s>ERS%s", msg);
    for (i = 0; i<26; i++) {
        byte = msg[i];
        if (byte == 49) {
            switch (i) {
                case 0:
                    util_MessagePopup ("Keithley 236 Message", "ERROR: Trigger Overrun");
					break;
                case 1:
                    util_MessagePopup ("Keithley 236 Message", "ERROR: Illegal device dependent command");
                    break;
                case 2:
                    util_MessagePopup ("Keithley 236 Message", "ERROR: Illegal device dependent command option");
                    break;
                case 3:
                    util_MessagePopup ("Keithley 236 Message", "ERROR: Interlock present");
                    break;
                case 4:
                    util_MessagePopup ("Keithley 236 Message", "ERROR: Illegal measure range");
                    break;
                case 5:
                    util_MessagePopup ("Keithley 236 Message", "ERROR: Illegal source range");
                    break;
                case 6:
                    util_MessagePopup ("Keithley 236 Message", "ERROR: Invalid sweep mix");
                    break;
                case 7:
                    util_MessagePopup ("Keithley 236 Message", "ERROR: Log cannot cross zero");
                    break;
                case 8:
                    util_MessagePopup ("Keithley 236 Message", "ERROR: Autoranging source w/ pulse sweep");
                    break;
                case 9:
                    util_MessagePopup ("Keithley 236 Message", "ERROR: In calibration");
                    break;
                case 10:
                    util_MessagePopup ("Keithley 236 Message", "ERROR: In standby");
                    break;
                case 11:
                    util_MessagePopup ("Keithley 236 Message", "ERROR: Unit is a 236");
                    break;
                case 12:
                    util_MessagePopup ("Keithley 236 Message", "ERROR: IOU DPRAM failed");
                    break;
                case 13:
                    util_MessagePopup ("Keithley 236 Message", "ERROR: IOU EEROM failed");
                    break;
                case 14:
                    util_MessagePopup ("Keithley 236 Message", "ERROR: IOU Cal checksum error");
                    break;
                case 15:
                    util_MessagePopup ("Keithley 236 Message", "ERROR: DPRAM lockup");
                    break;
                case 16:
                    util_MessagePopup ("Keithley 236 Message", "ERROR: DPRAM link error");
                    break;
                case 17:
                    util_MessagePopup ("Keithley 236 Message", "ERROR: Cal ADC zero error");
                    break;
                case 18:
                    util_MessagePopup ("Keithley 236 Message", "ERROR: Cal ADC gain error");
                    break;
                case 19:
                    util_MessagePopup ("Keithley 236 Message", "ERROR: Cal SRC zero error");
                    break;
                case 20:
                    util_MessagePopup ("Keithley 236 Message", "ERROR: Cal SRC gain error");
                    break;
                case 21:
                    util_MessagePopup ("Keithley 236 Message", "ERROR: Cal common mode error");
                    break;
                case 22:
                    util_MessagePopup ("Keithley 236 Message", "ERROR: Cal compsmunce error");
                    break;
                case 23:
                    util_MessagePopup ("Keithley 236 Message", "ERROR: Cal value error");
                    break;
                case 24:
                    util_MessagePopup ("Keithley 236 Message", "ERROR: Cal constants error");
                    break;
                case 25:
                    util_MessagePopup ("Keithley 236 Message", "ERROR: Cal invalid error");
                    break;
            }
        }
    } //*/
}

