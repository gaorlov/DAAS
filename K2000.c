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


#include "k2000.h"
#include "k2000u.h"


#define K2000_ID "KEITHLEY INSTRUMENTS INC.,MODEL 2000"

/* ESE - event status enable */

#define K2000_ESE_OPC 1     /* Operation complete */
#define K2000_ESE_QYE 4     /* Query error */
#define K2000_ESE_DDE 8     /* Device dependent error */
#define K2000_ESE_EXE 16    /* Execution error */
#define K2000_ESE_CME 32    /* Command error */
#define K2000_ESE_URQ 64    /* User request */
#define K2000_ESE_PON 128   /* Power cycled on / off */

#define K2000_ESE K2000_ESE_OPC+K2000_ESE_QYE+K2000_ESE_DDE+K2000_ESE_EXE+K2000_ESE_CME+K2000_ESE_URQ+K2000_ESE_PON

/* MSE - measurement status enable */

#define K2000_MSE_ROF 1     /* Reading overflow */
#define K2000_MSE_LL  2     /* Low limit */
#define K2000_MSE_HL  4     /* High limit */
#define K2000_MSE_RAV 32    /* Reading available */
#define K2000_MSE_BAV 128   /* Buffer available */
#define K2000_MSE_BHF 256   /* Buffer half full */
#define K2000_MSE_BFL 512   /* Buffer full */

#define K2000_MSE K2000_MSE_ROF+K2000_MSE_LL+K2000_MSE_HL+K2000_MSE_RAV+K2000_MSE_BAV+K2000_MSE_BHF+K2000_MSE_BFL

/* QSE - questionable status enable */

#define K2000_QSE_TEMP 16       /* bogus reference junction measurement */
#define K2000_QSE_CAL  256      /* invalid cal constant detected during power up */
#define K2000_QSE_WARN 16384    /* signal oriented measurement command parmeter ignored */

#define K2000_QSE K2000_QSE_TEMP+K2000_QSE_CAL+K2000_QSE_WARN

/* OSE - operation status enable */

#define K2000_OSE_MEAS 16       /* performing measurement */
#define K2000_OSE_TRIG 32       /* triggering */
#define K2000_OSE_IDLE 1024     /* idle */

#define K2000_OSE K2000_OSE_MEAS+K2000_OSE_TRIG+K2000_OSE_IDLE

/* SRE - service request enable */

#define K2000_SRE_CLEAR 0
#define K2000_SRE_MSB 1     /* Measurement summary */
#define K2000_SRE_EAV 4     /* Error available */
#define K2000_SRE_QSB 8     /* Questionable summary */
#define K2000_SRE_MAV 16    /* Message available */
#define K2000_SRE_ESB 32    /* Event summary */
#define K2000_SRE_OSB 128   /* Operation summary */

#define K2000_SRE K2000_SRE_EAV+K2000_SRE_QSB+K2000_SRE_ESB+K2000_SRE_MAV+K2000_SRE_MSB+K2000_SRE_OSB

#define TRUE 1
#define FALSE 0

typedef struct {
    acqchanPtr acqchan;
    struct {int enable, readings, moving;} filter;
    int overLimit;
    double rate, range;
}   k2000chanType;

typedef k2000chanType *k2000chanPtr;

typedef struct {
    k2000chanPtr chan[11];
    int id;
    int operation, func, scanCard;
}   k2000Type;

typedef k2000Type *k2000Ptr;

static void k2000_GetErrorMessage (gpibioPtr dev);
static void k2000_GetEventSummary (gpibioPtr dev);
static void k2000_GetQuestionableSummary (gpibioPtr dev);
static void k2000_GetOperationSummary (int panel, gpibioPtr dev);
static void k2000_CheckforProblems (gpibioPtr dev);

static void k2000_Out (gpibioPtr dev, char *cmd);
static void k2000_In (gpibioPtr dev, char *msg);

static char     *k2000_MeasFunction (gpibioPtr dev);
static int      k2000_GetChannel (gpibioPtr dev);
static int      k2000_GetFunction (gpibioPtr dev);
static double   k2000_GetRate (gpibioPtr dev);
static int      k2000_GetFilterStatus (gpibioPtr dev);
static int      k2000_GetMovingFilterType (gpibioPtr dev);
static int      k2000_GetFilterReadings (gpibioPtr dev);
static int      k2000_GetAutoRange (gpibioPtr dev);
static double   k2000_GetRange (gpibioPtr dev);
static int      k2000_GetMeasurementStatus (gpibioPtr dev);
static double   k2000_GetMeasurement (int chan, gpibioPtr dev);

static void     k2000_GetCh0Readings (acqchanPtr acqchan);
static void     k2000_GetCh1Readings (acqchanPtr acqchan);
static void     k2000_GetCh2Readings (acqchanPtr acqchan);
static void     k2000_GetCh3Readings (acqchanPtr acqchan);
static void     k2000_GetCh4Readings (acqchanPtr acqchan);
static void     k2000_GetCh5Readings (acqchanPtr acqchan);
static void     k2000_GetCh6Readings (acqchanPtr acqchan);
static void     k2000_GetCh7Readings (acqchanPtr acqchan);
static void     k2000_GetCh8Readings (acqchanPtr acqchan);
static void     k2000_GetCh9Readings (acqchanPtr acqchan);
static void     k2000_GetCh10Readings (acqchanPtr acqchan);

static void     k2000_UpdateControls (int panel, gpibioPtr dev);
static void     k2000_SaveChannel (k2000chanPtr chan);
static void     k2000_LoadChannel (int chan, gpibioPtr dev);

static int k2000_InitGPIB (gpibioPtr dev);
static void *k2000_Create (gpibioPtr dev);
static void k2000_UpdateReadings (int panel, void *dev);
static void k2000_Operate (int menubar, int menuItem, void *callbackData, int panel);
static void k2000_Save (gpibioPtr dev);
static void k2000_Load (gpibioPtr dev);
static void k2000_Remove (void *ptr);


/***************************************************************************/

void k2000_Init (void)
{
    devTypePtr devType;
    if (utilG.acq.status != ACQ_NONE) {
        util_ChangeInitMessage ("Keithley 2000 Control Utilities...");
        devType = malloc (sizeof (devTypeItem));
        if (devType) {
            Fmt (devType->label, "Keithley 2000 Multimeter");
            Fmt (devType->id, K2000_ID);
            devType->InitDevice = k2000_InitGPIB;
            devType->CreateDevice = k2000_Create;
            devType->UpdateReadings = k2000_UpdateReadings;
            devType->OperateDevice = k2000_Operate;
            devType->SaveDevice = k2000_Save;
            devType->LoadDevice = k2000_Load;
            devType->RemoveDevice = k2000_Remove;
            devTypeList_AddItem (devType);
        }
    }
}

static void k2000_Remove (void *ptr)
{
    k2000Ptr dmm = ptr;
    int i;
    acqchan_Remove(dmm->chan[0]->acqchan);
    if (dmm->scanCard)
        for (i = 1; i < 11; i++) acqchan_Remove(dmm->chan[i]->acqchan);
    free (dmm);
}

static void k2000_Load (gpibioPtr dev)
{
    char cmd[256];
    double r;
    int i, scanCard;
    k2000Ptr dmm;

    if (dev) dmm = dev->device;
    else dmm = k2000_Create (NULL);

    ScanFile (fileHandle.analysis, "%s>Scan Card: %i%s[dw1]", &scanCard);

    k2000_LoadChannel (0, dev);
    if (scanCard) {
        for (i = 1; i < 11; i++)
            k2000_LoadChannel (i, dev);
    }

    if (!dev) k2000_Remove (dmm);
}

static void k2000_Save (gpibioPtr dev)
{
    k2000Ptr dmm = dev->device;
    int i;

    FmtFile (fileHandle.analysis, "Scan Card: %i\n", dmm->scanCard);
    k2000_SaveChannel (dmm->chan[0]);
    if (dmm->scanCard) {
        for (i = 1; i < 11; i++)
            k2000_SaveChannel (dmm->chan[i]);
    }
}

static void k2000_Operate (int menubar, int menuItem, void *callbackData, int panel)
{
    int p, m;
    gpibioPtr dev = callbackData;
    k2000Ptr dmm = dev->device;
    char label[256];

    SetMenuBarAttribute (menubar, menuItem, ATTR_DIMMED, TRUE);

    p = dev->iPanel? dev->iPanel: LoadPanel (utilG.p, "k2000u.uir", K2000);
    dev->iPanel = p;
	
	SetPanelPos (p, VAL_AUTO_CENTER, VAL_AUTO_CENTER);

    Fmt (label, "Keithley 2000 Multimeter: %s", dev->label);
    SetPanelAttribute (p, ATTR_TITLE, label);

    m = LoadMenuBar (p, "k2000u.uir", K2000MENUS);
    
    SetPanelMenuBar (p, m);

    SetMenuBarAttribute (m, K2000MENUS_FILE_SAVE, ATTR_CALLBACK_DATA, dev);
    SetMenuBarAttribute (m, K2000MENUS_FILE_LOAD, ATTR_CALLBACK_DATA, dev);
    SetMenuBarAttribute (m, K2000MENUS_FILE_GPIB, ATTR_CALLBACK_DATA, dev);

    SetMenuBarAttribute (m, K2000MENUS_MEASURE_DMM, ATTR_CALLBACK_DATA, dmm->chan[0]->acqchan);

    SetMenuBarAttribute (m, K2000MENUS_MEASURE_CH1, ATTR_CALLBACK_DATA, dmm->chan[1]->acqchan);
    SetMenuBarAttribute (m, K2000MENUS_MEASURE_CH1, ATTR_DIMMED,!dmm->scanCard);

    SetMenuBarAttribute (m, K2000MENUS_MEASURE_CH2, ATTR_CALLBACK_DATA, dmm->chan[2]->acqchan);
    SetMenuBarAttribute (m, K2000MENUS_MEASURE_CH2, ATTR_DIMMED,!dmm->scanCard);

    SetMenuBarAttribute (m, K2000MENUS_MEASURE_CH3, ATTR_CALLBACK_DATA, dmm->chan[3]->acqchan);
    SetMenuBarAttribute (m, K2000MENUS_MEASURE_CH3, ATTR_DIMMED,!dmm->scanCard);

    SetMenuBarAttribute (m, K2000MENUS_MEASURE_CH4, ATTR_CALLBACK_DATA, dmm->chan[4]->acqchan);
    SetMenuBarAttribute (m, K2000MENUS_MEASURE_CH4, ATTR_DIMMED,!dmm->scanCard);

    SetMenuBarAttribute (m, K2000MENUS_MEASURE_CH5, ATTR_CALLBACK_DATA, dmm->chan[5]->acqchan);
    SetMenuBarAttribute (m, K2000MENUS_MEASURE_CH5, ATTR_DIMMED,!dmm->scanCard);

    SetMenuBarAttribute (m, K2000MENUS_MEASURE_CH6, ATTR_CALLBACK_DATA, dmm->chan[6]->acqchan);
    SetMenuBarAttribute (m, K2000MENUS_MEASURE_CH6, ATTR_DIMMED,!dmm->scanCard);

    SetMenuBarAttribute (m, K2000MENUS_MEASURE_CH7, ATTR_CALLBACK_DATA, dmm->chan[7]->acqchan);
    SetMenuBarAttribute (m, K2000MENUS_MEASURE_CH7, ATTR_DIMMED,!dmm->scanCard);

    SetMenuBarAttribute (m, K2000MENUS_MEASURE_CH8, ATTR_CALLBACK_DATA, dmm->chan[8]->acqchan);
    SetMenuBarAttribute (m, K2000MENUS_MEASURE_CH8, ATTR_DIMMED,!dmm->scanCard);

    SetMenuBarAttribute (m, K2000MENUS_MEASURE_CH9, ATTR_CALLBACK_DATA, dmm->chan[9]->acqchan);
    SetMenuBarAttribute (m, K2000MENUS_MEASURE_CH9, ATTR_DIMMED,!dmm->scanCard);

    SetMenuBarAttribute (m, K2000MENUS_MEASURE_CH10, ATTR_CALLBACK_DATA, dmm->chan[10]->acqchan);
    SetMenuBarAttribute (m, K2000MENUS_MEASURE_CH10, ATTR_DIMMED,!dmm->scanCard);

    SetPanelAttribute (p, ATTR_CALLBACK_DATA, dev);

    SetCtrlAttribute (p, K2000_MONITOR, ATTR_CALLBACK_DATA, dev);
    SetCtrlAttribute (p, K2000_DCV, ATTR_CALLBACK_DATA, dev);
    SetCtrlAttribute (p, K2000_ACV, ATTR_CALLBACK_DATA, dev);
    SetCtrlAttribute (p, K2000_DCI, ATTR_CALLBACK_DATA, dev);
    SetCtrlAttribute (p, K2000_ACI, ATTR_CALLBACK_DATA, dev);
    SetCtrlAttribute (p, K2000_OHMS2, ATTR_CALLBACK_DATA, dev);
    SetCtrlAttribute (p, K2000_OHMS4, ATTR_CALLBACK_DATA, dev);
    SetCtrlAttribute (p, K2000_RATE, ATTR_CALLBACK_DATA, dev);
    SetCtrlAttribute (p, K2000_FILTER_ENABLE, ATTR_CALLBACK_DATA, dev);
    SetCtrlAttribute (p, K2000_FILTER_READINGS, ATTR_CALLBACK_DATA, dev);
    SetCtrlAttribute (p, K2000_FILTER_TYPE, ATTR_CALLBACK_DATA, dev);
    SetCtrlAttribute (p, K2000_RANGE, ATTR_CALLBACK_DATA, dev);
    SetCtrlAttribute (p, K2000_RANGE_AUTO, ATTR_CALLBACK_DATA, dev);

    k2000_UpdateControls (p, dev);

    devPanel_Add (p, dev, k2000_UpdateReadings);

    DisplayPanel (p);
}

static void k2000_UpdateReadings (int panel, void *dev)
{
    gpibioPtr my_dev = dev;
    k2000Ptr dmm = my_dev->device;
    char rsp[256];
    int chan, status = 0;
    short statusbyte;

    if (gpibio_SRQ (my_dev)) {
        gpibio_GetStatusByte (my_dev, &statusbyte);
        if (statusbyte & K2000_SRE_EAV) k2000_GetErrorMessage (my_dev);
        if (statusbyte & K2000_SRE_ESB) k2000_GetEventSummary (my_dev);
        if (statusbyte & K2000_SRE_QSB) k2000_GetQuestionableSummary (my_dev);
        if (statusbyte & K2000_SRE_MSB) status = k2000_GetMeasurementStatus (my_dev);
        if (statusbyte & K2000_SRE_OSB) k2000_GetOperationSummary(panel, my_dev);
    }

    chan = k2000_GetChannel (my_dev);
    if ((!util_TakingData() || !dmm->chan[chan]->acqchan->acquire)) {
        k2000_Out (dev, "fetch?");
        k2000_In (dev, rsp);
        Scan (rsp, "%s>%f", &dmm->chan[chan]->acqchan->reading);
        dmm->chan[chan]->overLimit =  status & K2000_MSE_ROF;
    }

    SetCtrlVal (panel, K2000_OVER, dmm->chan[chan]->overLimit);
    SetCtrlVal (panel, K2000_MEASURE, dmm->chan[chan]->acqchan->reading);
}

static void *k2000_Create (gpibioPtr dev)
{
    k2000Ptr dmm = NULL;
    char msg[256];
    int i;

    dmm = malloc (sizeof(k2000Type));
    for (i = 0; i < 11; i++) {
        dmm->chan[i] = malloc (sizeof(k2000chanType));
        if (dmm->chan[i]) {
            dmm->chan[i]->overLimit = FALSE;
            dmm->chan[i]->rate = 2;
            dmm->chan[i]->filter.enable = FALSE;
            dmm->chan[i]->filter.readings = 10;
            dmm->chan[i]->filter.moving = TRUE;
            dmm->chan[i]->range = 0;
        }
    }

    if (dmm->chan[0]) dmm->chan[0]->acqchan = acqchan_Create ("K2000 Multimeter", dev, k2000_GetCh0Readings);
    if (dmm->chan[1]) dmm->chan[1]->acqchan = acqchan_Create ("K2000 CH1", dev, k2000_GetCh1Readings);
    if (dmm->chan[2]) dmm->chan[2]->acqchan = acqchan_Create ("K2000 CH2", dev, k2000_GetCh2Readings);
    if (dmm->chan[3]) dmm->chan[3]->acqchan = acqchan_Create ("K2000 CH3", dev, k2000_GetCh3Readings);
    if (dmm->chan[4]) dmm->chan[4]->acqchan = acqchan_Create ("K2000 CH4", dev, k2000_GetCh4Readings);
    if (dmm->chan[5]) dmm->chan[5]->acqchan = acqchan_Create ("K2000 CH5", dev, k2000_GetCh5Readings);
    if (dmm->chan[6]) dmm->chan[6]->acqchan = acqchan_Create ("K2000 CH6", dev, k2000_GetCh6Readings);
    if (dmm->chan[7]) dmm->chan[7]->acqchan = acqchan_Create ("K2000 CH7", dev, k2000_GetCh7Readings);
    if (dmm->chan[8]) dmm->chan[8]->acqchan = acqchan_Create ("K2000 CH8", dev, k2000_GetCh8Readings);
    if (dmm->chan[9]) dmm->chan[9]->acqchan = acqchan_Create ("K2000 CH9", dev, k2000_GetCh9Readings);
    if (dmm->chan[10]) dmm->chan[10]->acqchan = acqchan_Create ("K2000 CH10", dev, k2000_GetCh10Readings);

    if (dmm && dev) {
        dmm->id = dev->id;
        dev->device = dmm;
        gpibio_Out (dev, "*OPT?");
        gpibio_In (dev, msg);
        if (CompareBytes ("0,0", 0, msg, 0, 3, 0) == 0) dmm->scanCard = FALSE;
            else dmm->scanCard = TRUE;
    }
    return dmm;
}

static int k2000_InitGPIB (gpibioPtr dev)
{
    char cmd[256], rsp[256], buffer[256];
    int result;

    gpibio_Remote (dev);

    if (gpibio_DeviceMatch (dev, "*IDN?", K2000_ID)) {
        gpibio_Out (dev, "*CLS");

        Fmt (cmd, "%s<*SRE %i", K2000_SRE);
        gpibio_Out (dev, cmd);

        Fmt (cmd, "%s<*ESE %i", K2000_ESE);
        gpibio_Out (dev, cmd);

        gpibio_Out (dev, "format:data ascii");
        gpibio_Out (dev, "initiate:continuous 1");
        gpibio_Out (dev, "trigger:source imm");

        Fmt (cmd, "status:measurement:enable %i", K2000_MSE);
        gpibio_Out (dev, cmd);

        Fmt (cmd, "status:operation:enable %i", K2000_OSE);
        gpibio_Out (dev, cmd);

        Fmt (cmd, "status:questionable:enable %i", K2000_QSE);
        gpibio_Out (dev, cmd);

        return TRUE;
    }
    return FALSE;
}

/***************************************************************************/

void k2000_LoadCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    int fileselect, id;
    char path[256], info[256];
    gpibioPtr dev = callbackData;
    k2000Ptr dmm = dev->device;

    fileselect = FileSelectPopup ("", "*.dev", "*.dev", "Load Keithley 2000 Setup",
                                  VAL_LOAD_BUTTON, 0, 1, 1, 0, path);
    if (fileselect == VAL_EXISTING_FILE_SELECTED) {
        fileHandle.analysis = util_OpenFile (path, FILE_READ, FALSE);
        ScanFile (fileHandle.analysis, "%s>#INSTRSETUP %i", &id);
        if (dmm->id == id) {
            k2000_Load (dev);
            ReadLine (fileHandle.analysis, info, 255);
            k2000_UpdateControls(panel, dev);
        }
        else MessagePopup ("Keithley Load Message", "Different instrument types--process aborted");
        util_CloseFile();
    }
}

static void k2000_LoadChannel (int chan, gpibioPtr dev)
{
    k2000Ptr dmm = NULL;
    char info[256];
    if (dev) {
        dmm = dev->device;
        ScanFile (fileHandle.analysis, "%s>Rate  : %f%s[dw1]", &dmm->chan[chan]->rate);
        ScanFile (fileHandle.analysis, "%s>Range : %f%s[dw1]", &dmm->chan[chan]->range);
        ScanFile (fileHandle.analysis, "%s>Filter: %i, %i, %i%s[dw1]",
              &dmm->chan[chan]->filter.enable, &dmm->chan[chan]->filter.moving,
              &dmm->chan[chan]->filter.readings);
        acqchan_Load (dev, dmm->chan[chan]->acqchan);
    } else {
        ReadLine (fileHandle.analysis, info, 255);
        ReadLine (fileHandle.analysis, info, 255);
        ReadLine (fileHandle.analysis, info, 255);
        acqchan_Load (dev, NULL);
    }
}

void k2000_SaveCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    int fileselect;
    char path[256];
    gpibioPtr dev = callbackData;
    k2000Ptr dmm = dev->device;
    fileselect = FileSelectPopup ("", "*.dev", "*.dev", "Save K2000 Setup",
                                  VAL_SAVE_BUTTON, 0, 1, 1, 1, path);
    if (fileselect == VAL_NEW_FILE_SELECTED) {
        fileHandle.analysis = util_OpenFile (path, FILE_WRITE, FALSE);
        FmtFile (fileHandle.analysis, "%s<#INSTRSETUP %i\n", dmm->id);
        k2000_Save(dev);
        FmtFile (fileHandle.analysis, "#ENDSETUP\n");
        util_CloseFile();
    }
}

static void k2000_SaveChannel (k2000chanPtr chan)
{
    FmtFile (fileHandle.analysis, "Rate  : %f[p2]\n", chan->rate);
    FmtFile (fileHandle.analysis, "Range : %f[e2p2]\n", chan->range);
    FmtFile (fileHandle.analysis, "Filter: %i, %i, %i\n",
             chan->filter.enable, chan->filter.moving, chan->filter.readings);
    acqchan_Save (chan->acqchan);
}

int  k2000_ChannelControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    acqchanPtr acqchan = callbackData;
    char label[256];
    switch (control) {
        case K2000_CHAN_ACQ:
            if (event == EVENT_VAL_CHANGED) {
                GetCtrlVal (panel, control, &acqchan->acquire);
                if (acqchan->acquire) acqchanlist_AddChannel (acqchan);
                    else acqchanlist_RemoveChannel (acqchan);
            }
            break;
        case K2000_CHAN_COEFF:
            if (event == EVENT_COMMIT) {
                GetCtrlVal (panel, control, &acqchan->coeff);
                if (acqchan->p) SetCtrlVal (acqchan->p, ACQDATA_COEFF, acqchan->coeff);
            }
            break;
        case K2000_CHAN_LABEL:
            if (event == EVENT_COMMIT) {
                GetCtrlVal (panel, control, acqchan->channel->label);
                acqchanlist_ReplaceChannel (acqchan);
                if (acqchan->p) SetPanelAttribute (acqchan->p, ATTR_TITLE, acqchan->channel->label);
                Fmt (label, "K2000 Channel Setup: %s", acqchan->channel->label);
                SetPanelAttribute (panel, ATTR_TITLE, label);
            }
            break;
        case K2000_CHAN_CLOSE:
            if (event == EVENT_COMMIT) {
                devPanel_Remove(panel);
                
                DiscardPanel (panel);
            }
            break;
    }
    return 0;
}

void k2000_ChannelSetupCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    int p;
    acqchanPtr chan = callbackData;
    char label[256];

    p = LoadPanel (0, "k2000u.uir", K2000_CHAN);
    
    SetPanelPos (p, VAL_AUTO_CENTER, VAL_AUTO_CENTER);
    
    util_InitClose (p, K2000_CHAN_CLOSE, FALSE);

    SetCtrlVal (p, K2000_CHAN_LABEL, chan->channel->label);
    SetCtrlVal (p, K2000_CHAN_COEFF, chan->coeff);
    SetCtrlVal (p, K2000_CHAN_ACQ, chan->acquire);
    SetCtrlVal (p, K2000_CHAN_NOTE, chan->note);

    SetCtrlAttribute(p, K2000_CHAN_LABEL, ATTR_CALLBACK_DATA, chan);
    SetCtrlAttribute(p, K2000_CHAN_COEFF, ATTR_CALLBACK_DATA, chan);
    SetCtrlAttribute(p, K2000_CHAN_ACQ, ATTR_CALLBACK_DATA, chan);
    SetCtrlAttribute(p, K2000_CHAN_NOTE, ATTR_CALLBACK_DATA, chan);

    Fmt (label, "K2000 Channel Setup: %s", chan->channel->label);
    SetPanelAttribute (p, ATTR_TITLE, label);

    SetInputMode (p, K2000_CHAN_ACQ, !util_TakingData());
    InstallPopup (p);
}

int  k2000_ControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    gpibioPtr dev = callbackData;
    k2000Ptr dmm = dev->device;
    char cmd[256];
    int chan, autorange;

    switch (control) {
        case K2000_FILTER_TYPE:
            if (event == EVENT_COMMIT) {
                chan = k2000_GetChannel (dev);
                GetCtrlVal (panel, control, &dmm->chan[chan]->filter.moving);
                if (dmm->chan[chan]->filter.moving)
                    Fmt (cmd, "%s:average:tcontrol mov", k2000_MeasFunction(dev));
                else
                    Fmt (cmd, "%s:average:tcontrol rep", k2000_MeasFunction(dev));
                k2000_Out (dev, cmd);
            }
            break;
        case K2000_FILTER_READINGS:
            if (event == EVENT_COMMIT) {
                chan = k2000_GetChannel (dev);
                GetCtrlVal (panel, control, &dmm->chan[chan]->filter.readings);
                Fmt (cmd, "%s:average:count %i", k2000_MeasFunction(dev), dmm->chan[chan]->filter.readings);
                k2000_Out (dev, cmd);
            }
            break;
        case K2000_FILTER_ENABLE:
            if (event == EVENT_COMMIT) {
                chan = k2000_GetChannel (dev);
                GetCtrlVal (panel, control, &dmm->chan[chan]->filter.enable);
                Fmt (cmd, "%s:average:state %i", k2000_MeasFunction(dev), dmm->chan[chan]->filter.enable);
                k2000_Out (dev, cmd);
                SetInputMode (panel, K2000_FILTER_READINGS, dmm->chan[chan]->filter.enable);
                SetInputMode (panel, K2000_FILTER_TYPE, dmm->chan[chan]->filter.enable);
                if (dmm->chan[chan]->filter.enable) {
                    Fmt (cmd, "%s:average:count %i", k2000_MeasFunction(dev), dmm->chan[chan]->filter.readings);
                    k2000_Out (dev, cmd);
                    if (dmm->chan[chan]->filter.moving)
                        Fmt (cmd, "%s:average:tcontrol mov", k2000_MeasFunction(dev));
                    else
                        Fmt (cmd, "%s:average:tcontrol rep", k2000_MeasFunction(dev));
                    k2000_Out (dev, cmd);
                }
            }
            break;
        case K2000_RANGE_AUTO:
            if (event == EVENT_COMMIT) {
                chan = k2000_GetChannel (dev);
                GetCtrlVal (panel, control, &autorange);
                Fmt (cmd, "%s:range:auto %i", k2000_MeasFunction(dev), autorange);
                k2000_Out (dev, cmd);
                SetInputMode (panel, K2000_RANGE, !autorange);
                if (autorange) dmm->chan[chan]->range = 0.0;
                else {
                    dmm->chan[chan]->range = k2000_GetRange (dev);
                    SetCtrlVal (panel, K2000_RANGE, dmm->chan[chan]->range);
                }
            }
            break;
        case K2000_RANGE:
            if (event == EVENT_COMMIT) {
                chan = k2000_GetChannel (dev);
                GetCtrlVal (panel, control, &dmm->chan[chan]->range);
                Fmt (cmd, "%s:range:upper %f[p2]", k2000_MeasFunction (dev), dmm->chan[chan]->range);
                k2000_Out (dev, cmd);
            }
            break;
        case K2000_RATE:
            if (event == EVENT_COMMIT) {
                chan = k2000_GetChannel (dev);
                GetCtrlVal (panel, control, &dmm->chan[chan]->rate);
                Fmt (cmd, "%s:nplc %f[p2]", k2000_MeasFunction(dev), dmm->chan[chan]->rate*0.06);
                k2000_Out (dev, cmd);
            }
            break;
        case K2000_DCV:
        case K2000_ACV:
        case K2000_DCI:
        case K2000_ACI:
        case K2000_OHMS2:
        case K2000_OHMS4:
            if (event == EVENT_COMMIT) {
                if (dmm->func != control) {
                    SetCtrlAttribute (panel, dmm->func, ATTR_CMD_BUTTON_COLOR,
                                      VAL_PANEL_GRAY);
                    SetCtrlAttribute (panel, control, ATTR_CMD_BUTTON_COLOR, VAL_RED);
                    dmm->func = control;
                    Fmt (cmd, "function '%s'", k2000_MeasFunction(dev));
                    k2000_Out (dev, cmd);
                }
            }
            break;
        case K2000_MONITOR:
            if (event == EVENT_COMMIT) {
                GetCtrlVal (panel, control, &chan);
                k2000_GetMeasurement (chan, dev);
            }
            break;
    }

    if (event == EVENT_COMMIT) k2000_UpdateControls(panel, dev);
    return 0;
}

static void k2000_GetCh0Readings (acqchanPtr acqchan)
{
    gpibioPtr dev = acqchan->dev;
    k2000_GetMeasurement (0, dev);
}

static void k2000_GetCh1Readings (acqchanPtr acqchan)
{
    gpibioPtr dev = acqchan->dev;
    k2000_GetMeasurement (1, dev);
}

static void k2000_GetCh2Readings (acqchanPtr acqchan)
{
    gpibioPtr dev = acqchan->dev;
    k2000_GetMeasurement (2, dev);
}

static void k2000_GetCh3Readings (acqchanPtr acqchan)
{
    gpibioPtr dev = acqchan->dev;
    k2000_GetMeasurement (3, dev);
}

static void k2000_GetCh4Readings (acqchanPtr acqchan)
{
    gpibioPtr dev = acqchan->dev;
    k2000_GetMeasurement (4, dev);
}

static void k2000_GetCh5Readings (acqchanPtr acqchan)
{
    gpibioPtr dev = acqchan->dev;
    k2000_GetMeasurement (5, dev);
}

static void k2000_GetCh6Readings (acqchanPtr acqchan)
{
    gpibioPtr dev = acqchan->dev;
    k2000_GetMeasurement (6, dev);
}

static void k2000_GetCh7Readings (acqchanPtr acqchan)
{
    gpibioPtr dev = acqchan->dev;
    k2000_GetMeasurement (7, dev);
}

static void k2000_GetCh8Readings (acqchanPtr acqchan)
{
    gpibioPtr dev = acqchan->dev;
    k2000_GetMeasurement (8, dev);
}

static void k2000_GetCh9Readings (acqchanPtr acqchan)
{
    gpibioPtr dev = acqchan->dev;
    k2000_GetMeasurement (9, dev);
}

static void k2000_GetCh10Readings (acqchanPtr acqchan)
{
    gpibioPtr dev = acqchan->dev;
    k2000_GetMeasurement (10, dev);
}

static double k2000_GetMeasurement (int chan, gpibioPtr dev)
{
    char cmd[256], rsp[256];
    k2000Ptr dmm = dev->device;
    double reading;
    int status = 0;
    short statusbyte;

    if (chan != k2000_GetChannel (dev)) {
        if (chan == 0) {
            k2000_Out (dev, "route:open:all");
            k2000_CheckforProblems (dev);
        } else {
            Fmt (cmd, "route:close (@%i)", chan);
            k2000_Out (dev, cmd);
        }

        Fmt (cmd, "%s:nplc %f[p2]", k2000_MeasFunction (dev), dmm->chan[chan]->rate*0.06);
        k2000_Out (dev, cmd);
        k2000_CheckforProblems(dev);

        if (dmm->chan[chan]->range == 0.0) {
            Fmt (cmd, "%s:range:auto 1", k2000_MeasFunction (dev));
            k2000_Out (dev, cmd);
        } else {
            Fmt (cmd, "%s:range:auto 0", k2000_MeasFunction (dev));
            k2000_Out (dev, cmd);
            Fmt (cmd, "%s:range:upper %f", k2000_MeasFunction (dev), dmm->chan[chan]->range);
            k2000_Out (dev, cmd);
        }
        k2000_CheckforProblems (dev);

        Fmt (cmd, "%s:average:state %i", k2000_MeasFunction (dev), dmm->chan[chan]->filter.enable);
        k2000_Out (dev, cmd);
        k2000_CheckforProblems (dev);
        if (dmm->chan[chan]->filter.enable) {
            if (dmm->chan[chan]->filter.moving) Fmt (rsp, "MOV"); else Fmt (rsp, "REP");
            Fmt (cmd, "%s:average:tcontrol %s", k2000_MeasFunction (dev), rsp);
            k2000_Out (dev, cmd);
            k2000_CheckforProblems (dev);

            Fmt (cmd, "%s:average:count %i", k2000_MeasFunction (dev), dmm->chan[chan]->filter.readings);
            k2000_Out (dev, cmd);
            k2000_CheckforProblems (dev);
        }
    }

    gpibio_Out (dev, "abort");

    k2000_GetMeasurementStatus (dev); /* clear */
    while (!(status & K2000_MSE_RAV)) {
        if (gpibio_SRQ (dev)) {
            gpibio_GetStatusByte (dev, &statusbyte);
            if (statusbyte & K2000_SRE_MSB)
                status = k2000_GetMeasurementStatus (dev);
        }
        ProcessSystemEvents();
    }
    dmm->chan[chan]->overLimit = status & K2000_MSE_ROF;
    k2000_Out (dev, "fetch?");
    k2000_In (dev, rsp);
    Scan (rsp, "%s>%f", &reading);
    dmm->chan[chan]->acqchan->reading = reading;
    dmm->chan[chan]->acqchan->newreading = TRUE;
    return reading;
}

static void k2000_UpdateControls (int panel, gpibioPtr dev)
{
    char msg[256], rsp[256];
    k2000Ptr dmm = dev->device;
    int chan, autorange, m;
    double r;

    if (expG.acqstatus != utilG.acq.status) {
        m = GetPanelMenuBar (panel);
        SetMenuBarAttribute (m, K2000MENUS_FILE_LOAD, ATTR_DIMMED, util_TakingData());
        SetInputMode (panel, K2000_DCV, !util_TakingData());
        SetInputMode (panel, K2000_ACV, !util_TakingData());
        SetInputMode (panel, K2000_DCI, !util_TakingData());
        SetInputMode (panel, K2000_ACI, !util_TakingData());
        SetInputMode (panel, K2000_OHMS2, !util_TakingData());
        SetInputMode (panel, K2000_OHMS4, !util_TakingData());
    }

    k2000_Out (dev, "initiate:continuous on");
    k2000_CheckforProblems (dev);

    chan = k2000_GetChannel (dev);
    SetCtrlVal (panel, K2000_MONITOR, chan);
    SetInputMode (panel, K2000_MONITOR, dmm->scanCard);

    SetCtrlAttribute (panel, k2000_GetFunction (dev), ATTR_CMD_BUTTON_COLOR, VAL_RED);

    dmm->chan[chan]->rate = k2000_GetRate (dev);
    SetCtrlVal (panel, K2000_RATE, dmm->chan[chan]->rate);

    dmm->chan[chan]->filter.enable = k2000_GetFilterStatus (dev);
    SetCtrlVal (panel, K2000_FILTER_ENABLE, dmm->chan[chan]->filter.enable);
    SetInputMode (panel, K2000_FILTER_READINGS, dmm->chan[chan]->filter.enable);
    SetInputMode (panel, K2000_FILTER_TYPE, dmm->chan[chan]->filter.enable);

    if (dmm->chan[chan]->filter.enable) {
        dmm->chan[chan]->filter.moving = k2000_GetMovingFilterType (dev);
        SetCtrlVal (panel, K2000_FILTER_TYPE, dmm->chan[chan]->filter.moving);

        dmm->chan[chan]->filter.readings = k2000_GetFilterReadings (dev);
        SetCtrlVal (panel, K2000_FILTER_READINGS, dmm->chan[chan]->filter.readings);
    }

    autorange = k2000_GetAutoRange (dev);
    SetCtrlVal (panel, K2000_RANGE_AUTO, autorange);
    SetInputMode (panel, K2000_RANGE, !autorange);
    if (!autorange) {
        dmm->chan[chan]->range = k2000_GetRange(dev);
        SetCtrlVal (panel, K2000_RANGE, dmm->chan[chan]->range);
    } else dmm->chan[chan]->range = 0.0;
}

int  k2000_ControlPanelCallback(int panel, int event, void *callbackData, int eventData1, int eventData2)
{
    gpibioPtr dev = callbackData;
    k2000Ptr dmm = dev->device;
    int menubar;

    if ((event == EVENT_KEYPRESS && eventData1 == VAL_ESC_VKEY) || event == EVENT_RIGHT_DOUBLE_CLICK) 
	{
    	devPanel_Remove(panel);   
		DiscardPanel (panel);
		dev->iPanel = 0;
		SetMenuBarAttribute (acquire_GetMenuBar(), dev->menuitem_id, ATTR_DIMMED, FALSE);
    }

    if (event == EVENT_GOT_FOCUS) {
        menubar = GetPanelMenuBar (panel);
        SetPanelAttribute (panel, ATTR_DIMMED, (dev->status != DEV_REMOTE));
        SetMenuBarAttribute (menubar, K2000MENUS_FILE_SAVE, ATTR_DIMMED, (dev->status != DEV_REMOTE));
        SetMenuBarAttribute (menubar, K2000MENUS_FILE_LOAD, ATTR_DIMMED, (dev->status != DEV_REMOTE));
        SetMenuBarAttribute (menubar, K2000MENUS_MEASURE, ATTR_DIMMED, (dev->status != DEV_REMOTE));

        if (!util_TakingData()) k2000_UpdateControls (panel, dev);
    }
    return 0;
}

static int k2000_GetMeasurementStatus (gpibioPtr dev)
{
    char rsp[256];
    int status;
    k2000_Out (dev, "status:measurement:event?");
    k2000_In (dev, rsp);
    Scan (rsp, "%s>%i", &status);
    return status;
}

static double k2000_GetRange (gpibioPtr dev)
{
    char msg[256], rsp[256];
    double range;
    Fmt (msg, "%s:range:upper?", k2000_MeasFunction (dev));
    k2000_Out (dev, msg);
    k2000_In (dev, rsp);
    k2000_CheckforProblems (dev);
    Scan (rsp, "%s>%f", &range);
    return range;
}

static int k2000_GetAutoRange (gpibioPtr dev)
{
    char msg[256], rsp[256];
    int autorange;
    Fmt (msg, "%s:range:auto?", k2000_MeasFunction (dev));
    k2000_Out (dev, msg);
    k2000_In (dev, rsp);
    k2000_CheckforProblems (dev);
    Scan (rsp, "%s>%i", &autorange);
    return autorange;
}

static int k2000_GetFilterReadings (gpibioPtr dev)
{
    char msg[256], rsp[256];
    int readings = 0;

    Fmt (msg, "%s:average:count?", k2000_MeasFunction (dev));
    k2000_Out (dev, msg);
    k2000_In (dev, rsp);
    k2000_CheckforProblems (dev);
    Scan (rsp, "%s>%i", &readings);
    return readings;
}

static int k2000_GetMovingFilterType (gpibioPtr dev)
{
    char msg[256], rsp[256];
    int moving = FALSE;
    Fmt (msg, "%s:average:tcontrol?", k2000_MeasFunction (dev));
    k2000_Out (dev, msg);
    k2000_In (dev, rsp);
    k2000_CheckforProblems (dev);
    if (CompareBytes (rsp, 0, "MOV", 0, 3, 0) == 0) moving = TRUE;
    return moving;
}

static int k2000_GetFilterStatus (gpibioPtr dev)
{
    char msg[256], rsp[256];
    int status;
    Fmt (msg, "%s:average:state?", k2000_MeasFunction (dev));
    k2000_Out (dev, msg);
    k2000_In (dev, rsp);
    k2000_CheckforProblems (dev);
    Scan (rsp, "%s>%i", &status);
    return status;
}

static double k2000_GetRate (gpibioPtr dev)
{
    char msg[256], rsp[256];
    double rate;
    Fmt (msg, "%s:nplc?", k2000_MeasFunction (dev));
    k2000_Out (dev, msg);
    k2000_In (dev, rsp);
    k2000_CheckforProblems (dev);
    Scan (rsp, "%s>%f", &rate);
    rate/=0.06;     /* convert to msec */
    return rate;
}

static int k2000_GetFunction (gpibioPtr dev)
{
    char rsp[256];
    k2000Ptr dmm = dev->device;

    k2000_Out (dev, "function?");
    k2000_In (dev, rsp);
    k2000_CheckforProblems (dev);

    if (CompareBytes (rsp, 1, "VOLT:DC", 0, 7, 0) == 0) dmm->func = K2000_DCV;
    else if (CompareBytes (rsp, 1, "VOLT:AC", 0, 7, 0) == 0) dmm->func = K2000_ACV;
    else if (CompareBytes (rsp, 1, "CURR:DC", 0, 7, 0) == 0) dmm->func = K2000_DCI;
    else if (CompareBytes (rsp, 1, "CURR:AC", 0, 7, 0) == 0) dmm->func = K2000_ACI;
    else if (CompareBytes (rsp, 1, "RES", 0, 3, 0) == 0) dmm->func = K2000_OHMS2;
    else if (CompareBytes (rsp, 1, "FRES", 0, 4, 0) == 0) dmm->func = K2000_OHMS4;
    return dmm->func;
}

static int k2000_GetChannel (gpibioPtr dev)
{
    char rsp[256];
    int chan;
    k2000Ptr dmm = dev->device;
    if (dmm->scanCard) {
        k2000_Out (dev, "route:close:state?");
        k2000_In (dev, rsp);
        if (Scan (rsp, "%s>(@%i)", &chan) != 1) chan = 0;
    } else chan = 0;
    return chan;
}

static char *k2000_MeasFunction (gpibioPtr dev)
{
    char *msg;
    k2000Ptr dmm = dev->device;

    msg = "                          ";
    switch (dmm->func) {
        case K2000_DCV: Fmt (msg, "voltage:dc"); break;
        case K2000_ACV: Fmt (msg, "voltage:ac"); break;
        case K2000_DCI: Fmt (msg, "current:dc"); break;
        case K2000_ACI: Fmt (msg, "current:ac"); break;
        case K2000_OHMS2: Fmt (msg, "resistance"); break;
        case K2000_OHMS4: Fmt (msg, "fresistance"); break;
    }
    return msg;
}

static void k2000_Out (gpibioPtr dev, char *cmd)
{
    gpibio_Out (dev, cmd);
}

static void k2000_In (gpibioPtr dev, char *msg)
{
    gpibio_In (dev, msg);
}

static void k2000_CheckforProblems (gpibioPtr dev)
{
    char msg[256];
    short statusbyte;

    if (gpibio_SRQ(dev)) {
        gpibio_GetStatusByte (dev, &statusbyte);
        if (statusbyte & K2000_SRE_EAV) k2000_GetErrorMessage (dev);
        if (statusbyte & K2000_SRE_ESB) k2000_GetEventSummary (dev);
        if (statusbyte & K2000_SRE_QSB) k2000_GetQuestionableSummary (dev);
    }
}

static void k2000_GetOperationSummary (int panel, gpibioPtr dev)
{
    char msg[256];
    int statusbyte;
    k2000Ptr dmm = dev->device;

    statusbyte = 0;
    gpibio_Out (dev, "status:operation:event?");
    gpibio_In (dev, msg);
    Scan (msg, "%s>%i", &statusbyte);
    if (statusbyte != dmm->operation) {
        SetCtrlVal (panel, K2000_MEAS, statusbyte & K2000_OSE_MEAS);
        SetCtrlVal (panel, K2000_TRIG, statusbyte & K2000_OSE_TRIG);
        SetCtrlVal (panel, K2000_IDLE, statusbyte & K2000_OSE_IDLE);
    }
    dmm->operation = statusbyte;
}

static void k2000_GetQuestionableSummary (gpibioPtr dev)
{
    char msg[256];
    int statusbyte;

    statusbyte = 0;
    gpibio_Out (dev, "status:questionable:event?");
    gpibio_In (dev, msg);
    Scan (msg, "%s>%i", &statusbyte);
    if (statusbyte & K2000_QSE_TEMP)
        MessagePopup ("Keithley 2000 Questionable Event Message", "Invalid reference junction measurement");
    if (statusbyte & K2000_QSE_CAL)
        MessagePopup ("Keithley 2000 Questionable Event Message", "Invalid calibration constant detected during power up");
    if (statusbyte & K2000_QSE_WARN)
        MessagePopup ("Keithley 2000 Questionable Event Message", "Signal oriented measurement command ignored");
}

static void k2000_GetEventSummary (gpibioPtr dev)
{
    char msg[256];
    int statusbyte;

    statusbyte = 0;
    gpibio_Out (dev, "*ESR?");
    gpibio_In (dev, msg);
    Scan (msg, "%s>%i", &statusbyte);
    if (statusbyte & K2000_ESE_QYE)
        MessagePopup ("Keithley 2000 Event Message", "No data in output queue");
    if (statusbyte & K2000_ESE_DDE)
        MessagePopup ("Keithley 2000 Event Message", "Instrument operation error due to some internal condition");
    if (statusbyte & K2000_ESE_EXE)
        MessagePopup ("Keithley 2000 Event Message", "Command execution error");
    if (statusbyte & K2000_ESE_CME)
        MessagePopup ("Keithley 2000 Event Message", "Illegal command received");
    if (statusbyte & K2000_ESE_URQ)
        MessagePopup ("Keithley 2000 Event Message", "LOCAL key on front panel pressed");
    if (statusbyte & K2000_ESE_PON)
        MessagePopup ("Keithley 2000 Event Message", "Power cycled off and on");
}

static void k2000_GetErrorMessage (gpibioPtr dev)
{
    char msg[256];
    int statusbyte;

    gpibio_Out (dev, "system:error?");
    gpibio_In (dev, msg);
	MessagePopup ("Keithley 2000 Error Message", msg);
}
