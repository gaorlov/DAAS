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

#include "k213.h"
#include "k213u.h"

#define K213_ID "1.2"

#define K213_SRE_SRQDISABLED 0
#define K213_SRE_DAC1 1             /* DAC1 ready for trigger */
#define K213_SRE_DAC2 2             /* DAC2 ready for trigger */
#define K213_SRE_DAC3 4             /* DAC3 ready for trigger */
#define K213_SRE_DAC4 8             /* DAC4 ready for trigger */
#define K213_SRE_TRIGOVERRUN 16     /* trigger overrun */
#define K213_SRE_ERROR 32           /* command error */
#define K213_SRE_EXTTRANS 128       /* external input transition */

#define TRUE 1
#define FALSE 0

typedef struct {
    sourcePtr src;
    int panel, range, autorange;
}   portType;

typedef struct {
    portType port[4];
    int id;
	rangePtr range[5];
}   k213Type;

typedef k213Type *k213Ptr;

static void k213_GetErrorStatus (gpibioPtr dev);

static void k213_In (gpibioPtr dev, char *msg);
static void k213_Out (gpibioPtr dev, char *cmd);
static void k213_CheckforProblems (gpibioPtr dev);

static void k213_GetPortLevel (int port, gpibioPtr dev);
static void k213_GetPort1 (acqchanPtr acqchan);
static void k213_GetPort2 (acqchanPtr acqchan);
static void k213_GetPort3 (acqchanPtr acqchan);
static void k213_GetPort4 (acqchanPtr acqchan);

static void k213_SetPortLevel (int port, gpibioPtr dev);
static void k213_SetPort1 (sourcePtr src);
static void k213_SetPort2 (sourcePtr src);
static void k213_SetPort3 (sourcePtr src);
static void k213_SetPort4 (sourcePtr src);

static void k213_GetStatus (gpibioPtr dev);

static void k213_Save (gpibioPtr dev);
static void k213_Load (gpibioPtr dev);
static void k213_Remove (void *ptr);

static void k213_UpdateControls (int panel, gpibioPtr dev);

static int  k213_InitGPIB (gpibioPtr dev);
static void *k213_Create (gpibioPtr dev);
static void k213_Operate (int menubar, int menuItem, void *callbackData, int panel);
static void k213_UpdateReadings (int panel, void *dev);


void k213_Init (void)
{
    devTypePtr devType;
    if (utilG.acq.status != ACQ_NONE) {
        util_ChangeInitMessage ("Keithley 213 Control Utilities...");
        devType = malloc (sizeof (devTypeItem));
        if (devType) {
            Fmt (devType->label, "Keithley 213 Quad Voltage Source");
            Fmt (devType->id, K213_ID);
            devType->InitDevice = k213_InitGPIB;
            devType->CreateDevice = k213_Create;
            devType->OperateDevice = k213_Operate;
            devType->UpdateReadings = k213_UpdateReadings;
            devType->SaveDevice = k213_Save;
            devType->LoadDevice = k213_Load;
            devType->RemoveDevice = k213_Remove;
            devTypeList_AddItem (devType);
        }
    }
}

static void k213_UpdateReadings (int panel, void *dev)
{
    gpibioPtr my_dev = dev;
    k213Ptr quadsrc = my_dev->device;
    unsigned short statusbyte;
    char rsp[256];
    int control, dim, bg, mode, m, active_panel, port;

    for (port = 0; port < 4; port++) {
        if (!util_TakingData() || !quadsrc->port[port].src->acqchan->acquire)
            k213_GetPortLevel (port+1, dev);
    }

    if (expG.acqstatus != utilG.acq.status) {
        m = GetPanelMenuBar (panel);
        for (port = 0; port < 4; port++) {
            dim = (util_TakingData() && quadsrc->port[port].src->inlist &&
               (utilG.exp == EXP_SOURCE));

            if (dim) { mode = VAL_INDICATOR; bg = VAL_PANEL_GRAY;}
                else { mode = VAL_HOT; bg = VAL_WHITE;}

            SetCtrlAttribute (quadsrc->port[port].panel, K213PORT_DISPLAY, ATTR_CTRL_MODE, mode);
            SetCtrlAttribute (quadsrc->port[port].panel, K213PORT_DISPLAY, ATTR_TEXT_BGCOLOR, bg);

            SetInputMode (quadsrc->port[port].panel, K213PORT_AUTORANGE, !dim);
            SetInputMode (quadsrc->port[port].panel, K213PORT_RANGE, !dim);
        }
        SetMenuBarAttribute (m, K213MENUS_FILE_LOAD, ATTR_DIMMED, dim);
    }

    k213_CheckforProblems (my_dev);

    active_panel = GetActivePanel();
    control = GetActiveCtrl (active_panel);

    for (port = 0; port < 4; port++) {
        if (util_TakingData() || (control != K213PORT_DISPLAY))
            SetCtrlVal (quadsrc->port[port].panel, K213PORT_DISPLAY, quadsrc->port[port].src->acqchan->reading);
    }
}

static void *k213_Create (gpibioPtr dev)
{
    k213Ptr quadsrc;
    int port;
	quadsrc = malloc (sizeof(k213Type));
	if (dev) quadsrc->id = dev->id;
	quadsrc->range[0] = range_Create(0, 0, 0);
	quadsrc->range[1] = range_Create(1, -1, 0.00025);
	quadsrc->range[2] = range_Create(5, -5, 0.00125);
	quadsrc->range[3] = range_Create(10, -10, 0.0025);
    quadsrc->range[4] = NULL;
    for (port = 0; port < 4; port++) {
        quadsrc->port[port].autorange = TRUE;
        quadsrc->port[port].range = 0;
    }
	
    quadsrc->port[0].src = source_Create ("K213 Port 1", dev, k213_SetPort1,
                                 k213_GetPort1);
	
    quadsrc->port[1].src = source_Create ("K213 Port 2", dev, k213_SetPort2,
                                 k213_GetPort2);
    
	quadsrc->port[2].src = source_Create ("K213 Port 3", dev, k213_SetPort3,
                                 k213_GetPort3);
    
	quadsrc->port[3].src = source_Create ("K213 Port 4", dev, k213_SetPort4,
                                 k213_GetPort4);
	
    if (dev) dev->device = quadsrc;
    return quadsrc;
}

static int k213_InitGPIB (gpibioPtr dev)
{
    gpibio_Remote (dev);

    if (gpibio_DeviceMatch (dev, "U0X", K213_ID)) {
        gpibio_Out (dev, "M0G0O0Q0T0Y0X");
        return TRUE;
    }
    return FALSE;
}

void K213LoadCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    int fileselect, id;
    char path[256], info[256];
    gpibioPtr dev = callbackData;
    k213Ptr quadsrc = dev->device;

    fileselect = FileSelectPopup ("", "*.dev", "*.dev", "Load Keithley 213 Quad Voltage Source Setup",
                                  VAL_LOAD_BUTTON, 0, 1, 1, 0, path);
    if (fileselect == VAL_EXISTING_FILE_SELECTED) {
        fileHandle.analysis = util_OpenFile (path, FILE_READ, FALSE);
        ScanFile (fileHandle.analysis, "%s>#INSTRSETUP %i", &id);
        if (quadsrc->id == id) {
            k213_Load (dev);
            ReadLine (fileHandle.analysis, info, 255);
            k213_UpdateControls(panel, dev);
        }
        else MessagePopup ("Keithley Load Message", "Different instrument types--process aborted");
        util_CloseFile();
    }
}

void K213SaveCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    int fileselect;
    char path[256];
    gpibioPtr dev = callbackData;
    k213Ptr quadsrc = dev->device;
    fileselect = FileSelectPopup ("", "*.dev", "*.dev", "Save K213 Setup",
                                  VAL_SAVE_BUTTON, 0, 1, 1, 1, path);
    if (fileselect == VAL_NEW_FILE_SELECTED) {
        fileHandle.analysis = util_OpenFile (path, FILE_WRITE, FALSE);
        FmtFile (fileHandle.analysis, "%s<#INSTRSETUP %i\n", quadsrc->id);
        k213_Save(dev);
        FmtFile (fileHandle.analysis, "#ENDSETUP\n");
        util_CloseFile();
    }
}

int  K213ControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    gpibioPtr dev = callbackData;
    k213Ptr quadsrc = dev->device;
    int port;

    for (port = 0; port < 4; port++)
        if (quadsrc->port[port].panel == panel) {
            switch (control) {
                case K213PORT_DISPLAY:
                    if (event == EVENT_COMMIT) {
                        GetCtrlVal (panel, control, &quadsrc->port[port].src->biaslevel);
                        k213_SetPortLevel (port+1, dev);
						util_Delay(1);
                    }
                    break;
                case K213PORT_AUTORANGE:
                    if (event == EVENT_COMMIT) {
                        GetCtrlVal (panel, control, &quadsrc->port[port].autorange);
                        k213_SetPortLevel (port+1, dev);
                    }
                    break;
                case K213PORT_RANGE:
                    if (event == EVENT_COMMIT) {
                        GetCtrlVal (panel, control, &quadsrc->port[port].range);
                        k213_SetPortLevel (port+1, dev);
                    }
                    break;
                case K213PORT_SETUP:
                    if (event == EVENT_COMMIT) {
                        switch (utilG.exp) {
                            case EXP_SOURCE: source_InitPanel (quadsrc->port[port].src); break;
                            case EXP_FLOAT: gensrc_InitPanel (quadsrc->port[port].src); break;
                        }
                    }
            }
            break;
        }

    if (event == EVENT_COMMIT) {
        k213_GetStatus(dev);
        k213_UpdateControls(panel, dev);
    }
    return 0;
}

int  K213ControlPanelCallback(int panel, int event, void *callbackData, int eventData1, int eventData2)
{
    gpibioPtr dev = callbackData;
    k213Ptr quadsrc = dev->device;
    int menubar, port;

    if ((event == EVENT_KEYPRESS && eventData1 == VAL_ESC_VKEY) || (event == EVENT_RIGHT_DOUBLE_CLICK)) {
        devPanel_Remove (panel);

        for (port = 0; port < 4; port++) {
            HidePanel (quadsrc->port[port].panel);
        }
        DiscardPanel (panel);
		dev->iPanel = 0;
		SetMenuBarAttribute (acquire_GetMenuBar(), dev->menuitem_id, ATTR_DIMMED, FALSE);
    }

    if (event == EVENT_GOT_FOCUS && panel == dev->iPanel) {
        menubar = GetPanelMenuBar (panel);
        SetPanelAttribute (panel, ATTR_DIMMED, (dev->status != DEV_REMOTE));
        SetMenuBarAttribute (menubar, K213MENUS_FILE_SAVE, ATTR_DIMMED, (dev->status != DEV_REMOTE));
        SetMenuBarAttribute (menubar, K213MENUS_FILE_LOAD, ATTR_DIMMED, (dev->status != DEV_REMOTE));

        if (!util_TakingData()) k213_UpdateControls (panel, dev);
    }
    return 0;
}

static void k213_Operate (int menubar, int menuItem, void *callbackData, int panel)
{
    int p, m, port, width, height;
    gpibioPtr dev = callbackData;
    k213Ptr quadsrc = dev->device;
    char label[256];

    SetMenuBarAttribute (menubar, menuItem, ATTR_DIMMED, TRUE);

    p = dev->iPanel? dev->iPanel: LoadPanel (utilG.p, "k213u.uir", K213);
    dev->iPanel = p;
	
    Fmt (label, "Keithley 213 : %s", dev->label);
    SetPanelAttribute (p, ATTR_TITLE, label);

    m = LoadMenuBar (p, "k213u.uir", K213MENUS);
    
    SetPanelMenuBar (p, m);

    SetPanelAttribute (p, ATTR_CALLBACK_DATA, dev);

    SetMenuBarAttribute (m, K213MENUS_FILE_SAVE, ATTR_CALLBACK_DATA, dev);
    SetMenuBarAttribute (m, K213MENUS_FILE_LOAD, ATTR_CALLBACK_DATA, dev);
    SetMenuBarAttribute (m, K213MENUS_FILE_GPIB, ATTR_CALLBACK_DATA, dev);

    for (port = 0; port < 4; port++) {
        quadsrc->port[port].panel = LoadPanel (p, "k213u.uir", K213PORT);
        GetPanelAttribute (quadsrc->port[port].panel, ATTR_WIDTH, &width);
        GetPanelAttribute (quadsrc->port[port].panel, ATTR_HEIGHT, &height);
        
        SetPanelPos (quadsrc->port[port].panel, 17+2, (port*(width+4))+2);
        Fmt (label, "port %i", port+1);
        SetCtrlVal (quadsrc->port[port].panel, K213PORT_TITLE, label);

        SetCtrlAttribute (quadsrc->port[port].panel, K213PORT_DISPLAY, ATTR_CALLBACK_DATA, dev);
        SetCtrlAttribute (quadsrc->port[port].panel, K213PORT_AUTORANGE, ATTR_CALLBACK_DATA, dev);
        SetCtrlAttribute (quadsrc->port[port].panel, K213PORT_RANGE, ATTR_CALLBACK_DATA, dev);
        SetCtrlAttribute (quadsrc->port[port].panel, K213PORT_SETUP, ATTR_CALLBACK_DATA, dev);
		SetPanelAttribute(quadsrc->port[port].panel, ATTR_CALLBACK_DATA, dev);
        DisplayPanel (quadsrc->port[port].panel);
    }

    SetPanelAttribute (p, ATTR_WIDTH, (width+4)*port);
    SetPanelAttribute (p, ATTR_HEIGHT, height+4+17);
    SetPanelPos (p, VAL_AUTO_CENTER, VAL_AUTO_CENTER);
    k213_UpdateControls (p, dev);

    devPanel_Add (p, dev, k213_UpdateReadings);
    DisplayPanel (p);
}

static void k213_UpdateControls (int panel, gpibioPtr dev)
{
    k213Ptr quadsrc = dev->device;
    int port;

    k213_GetStatus (dev);
    for (port = 0; port < 4; port++) {
        SetCtrlVal (quadsrc->port[port].panel, K213PORT_DISPLAY, quadsrc->port[port].src->acqchan->reading);
        SetCtrlVal (quadsrc->port[port].panel, K213PORT_AUTORANGE, quadsrc->port[port].autorange);
        SetCtrlVal (quadsrc->port[port].panel, K213PORT_RANGE, quadsrc->port[port].range);
        SetInputMode (quadsrc->port[port].panel, K213PORT_RANGE, !quadsrc->port[port].autorange);
        if (quadsrc->port[port].autorange || (quadsrc->port[port].range == 3)) {
            quadsrc->port[port].src->min = -10.0;
            quadsrc->port[port].src->max = 10.0;
			
        } else {
            switch (quadsrc->port[port].range) {
                case 0:
                    quadsrc->port[port].src->min = -0.0;
                    quadsrc->port[port].src->max = 0.0;
                    break;
                case 1:
                    quadsrc->port[port].src->min = -1.0;
                    quadsrc->port[port].src->max = 1.0;
                    break;
                case 2:
                    quadsrc->port[port].src->min = -5.0;
                    quadsrc->port[port].src->max = 5.0;
                    break;
            }
        }
		if (quadsrc->port[port].autorange)
		{
			quadsrc->port[port].src->ranges.autoscale = 1;
			quadsrc->port[port].src->ranges.temprange = quadsrc->range;
		}
		else
		{
			quadsrc->port[port].src->ranges.autoscale = 0;
			quadsrc->port[port].src->ranges.temprange[0] = quadsrc->range[quadsrc->port[port].range];
		}
        SetCtrlAttribute (quadsrc->port[port].panel, K213PORT_DISPLAY, ATTR_MIN_VALUE, quadsrc->port[port].src->min);
        SetCtrlAttribute (quadsrc->port[port].panel, K213PORT_DISPLAY, ATTR_MAX_VALUE, quadsrc->port[port].src->max);
    }
}

static void k213_Remove (void *ptr)
{
    int port;
    k213Ptr quadsrc = ptr;

    for (port = 0; port < 4; port++)
        source_Remove (quadsrc->port[port].src);
    free (quadsrc);
}

static void k213_Load (gpibioPtr dev)
{
    int port;
    k213Ptr quadsrc;

    if (dev) quadsrc = dev->device;
    else quadsrc = k213_Create (NULL);

    for (port = 0; port < 4; port++)
        source_Load (dev, quadsrc->port[port].src);

    if (!dev) k213_Remove (quadsrc);
}

static void k213_Save (gpibioPtr dev)
{
    int port;
    k213Ptr quadsrc = dev->device;

    k213_GetStatus(dev);
    for (port = 0; port < 4; port++)
        source_Save (quadsrc->port[port].src);
}

static void k213_GetStatus (gpibioPtr dev)
{
    int port;
    char msg[256], rsp[256];
    k213Ptr quadsrc = dev->device;

    for (port = 0; port < 4; port++) {
        Fmt (msg, "P%iX", port+1);
        gpibio_Out (dev, msg);

        gpibio_Out (dev, "A?");
        gpibio_In (dev, rsp);
        Scan (rsp, "%s>A%i", &quadsrc->port[port].autorange);

        gpibio_Out (dev, "U7X");
        gpibio_In (dev, rsp);

        Scan (rsp, "%s>%s[w4d]R%iV%f",
            &quadsrc->port[port].range,
            &quadsrc->port[port].src->biaslevel);
    }
}

static void k213_SetPort4 (sourcePtr src)
{
    gpibioPtr dev = src->acqchan->dev;

    k213_SetPortLevel (4, dev);
    util_Delay (src->segments[src->seg]->delay);
}

static void k213_SetPort3 (sourcePtr src)
{
    gpibioPtr dev = src->acqchan->dev;

    k213_SetPortLevel (3, dev);
    util_Delay (src->segments[src->seg]->delay);
}

static void k213_SetPort2 (sourcePtr src)
{
    gpibioPtr dev = src->acqchan->dev;

    k213_SetPortLevel (2, dev);
    util_Delay (src->segments[src->seg]->delay);
}

static void k213_SetPort1 (sourcePtr src)
{
    gpibioPtr dev = src->acqchan->dev;

    k213_SetPortLevel (1, dev);
    util_Delay (src->segments[src->seg]->delay);
}

static void k213_SetPortLevel (int port, gpibioPtr dev)
{
    char cmd[256];
    k213Ptr quadsrc = dev->device;
    sourcePtr src = quadsrc->port[port-1].src;

    Fmt (cmd, "%s<C0P%iA%i", port, quadsrc->port[port-1].autorange);

    if (!quadsrc->port[port-1].autorange)
        Fmt (cmd, "%s[a]<R%i", quadsrc->port[port-1].range);

    if (quadsrc->port[port-1].range == 0) Fmt (cmd, "%s[a]<V0X");
        else Fmt (cmd, "%s[a]<V%f[p5]X", src->biaslevel);
    k213_Out (dev, cmd);
}

static void k213_GetPort4 (acqchanPtr acqchan)
{
    gpibioPtr dev = acqchan->dev;
    k213_GetPortLevel (4, dev);
}

static void k213_GetPort3 (acqchanPtr acqchan)
{
    gpibioPtr dev = acqchan->dev;
    k213_GetPortLevel (3, dev);
}

static void k213_GetPort2 (acqchanPtr acqchan)
{
    gpibioPtr dev = acqchan->dev;
    k213_GetPortLevel (2, dev);
}

static void k213_GetPort1 (acqchanPtr acqchan)
{
    gpibioPtr dev = acqchan->dev;
    k213_GetPortLevel (1, dev);
}

static void k213_GetPortLevel (int port, gpibioPtr dev)
{
    char msg[256];
    unsigned short statusbyte;
    k213Ptr quadsrc = dev->device;

    Fmt (msg, "P%iX", port);
    k213_Out (dev, msg);
    k213_Out (dev, "V?");
    k213_In (dev, msg);
    Scan (msg, "%s>V%f", &quadsrc->port[port-1].src->acqchan->reading);
    quadsrc->port[port-1].src->acqchan->newreading = TRUE;
}

static void k213_CheckforProblems (gpibioPtr dev)
{
    char msg[256];
    short statusbyte;
    k213Ptr quadsrc = dev->device;

    gpibio_GetStatusByte (dev, &statusbyte);

    if (statusbyte & K213_SRE_TRIGOVERRUN)
        MessagePopup ("K213 Message", "Trigger overrun");
    if (statusbyte & K213_SRE_ERROR) k213_GetErrorStatus (dev);
    if (statusbyte & K213_SRE_EXTTRANS)
        MessagePopup ("K213 Message", "External input transition detected");
}

static void k213_Out (gpibioPtr dev, char *cmd)
{
    gpibio_Out (dev, cmd);
}

static void k213_In (gpibioPtr dev, char *msg)
{
    gpibio_In (dev, msg);
}

static void k213_GetErrorStatus (gpibioPtr dev)
{
    int error, s;
    char e_msg[256], *s_msg, rsp[256];

    gpibio_Out (dev, "E?");
    gpibio_In (dev, rsp);

    Scan (rsp, "%s>E%i", &error);
    switch (error) {
        case 1: MessagePopup ("Keithley 213 Message", "ERROR: Unrecognized command"); break;
        case 2: MessagePopup ("Keithley 213 Message", "ERROR: Invalid command parameter"); break;
        case 3: MessagePopup ("Keithley 213 Message", "ERROR: Command conflict"); break;
        case 4: MessagePopup ("Keithley 213 Message", "ERROR: Calibration switch not closed"); break;
        case 5:
            gpibio_Out (dev, "S?");
            gpibio_In (dev, rsp);
            Scan (rsp, "%s>S%i", &s);
            Fmt (e_msg, "ERROR: Non-volatile RAM error, system defaults or calibration may be lost.");
            if (s) s_msg = "User programmed defaults are still valid";
                else s_msg = "Factory defaults are in use, user programmed defaults were lost";
            Fmt (e_msg, "%s[a]<\n%s", s_msg);
            MessagePopup ("Keithley 213 Message", e_msg);
            break;
    }
}

