#include <gpib.h>
#include <formatio.h>
#include <userint.h>
#include <ansi_c.h>
#include <utility.h>

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
#include "sr830.h"
#include "sr830u.h"

#define SR830_ID "Stanford_Research_Systems,SR830"

/* serial poll status byte */
#define SR830_SRE_SCN   1   /* no scan in progress */
#define SR830_SRE_IFC   2   /* no command execution in progress */
#define SR830_SRE_ERR   4   /* bit changed in error status byte */
#define SR830_SRE_LIA   8   /* bit changed in LIA status byte */
#define SR830_SRE_MAV   16  /* interface output buffer is nonempty */
#define SR830_SRE_ESB   32  /* bit changed in standard status byte */
#define SR830_SRE_SRQ   64  /* SRQ has occurred */

/* standard event */
#define SR830_ESE_INP   1   /* input queue overflow */
#define SR830_ESE_QRY   4   /* output queue overflow */
#define SR830_ESE_EXE   16  /* bad parameter */
#define SR830_ESE_CMD   32  /* bad command */
#define SR830_ESE_URQ   64  /* key pressed */
#define SR830_ESE_PON   128 /* power on */

/* LIA */
#define SR830_LIA_RESRV 1   /* input/reserve overload */
#define SR830_LIA_FILTR 2   /* time constant fileter overload */
#define SR830_LIA_OUTPT 4   /* output overload detected */
#define SR830_LIA_UNLK  8   /* reference unlock */
#define SR830_LIA_RANGE 16  /* detection frequency changes ranges */
#define SR830_LIA_TC    32  /* time constant changed */
#define SR830_LIA_TRIG  64  /* data storage triggered */

/* error */
#define SR830_ERR_BACKUP 2  /* battery backup failed */
#define SR830_ERR_RAM    4  /* RAM memory error */
#define SR830_ERR_ROM   16  /* ROM memory error */
#define SR830_ERR_GPIB  32  /* GPIB Error */
#define SR830_ERR_DSP   64  /* DSP error */
#define SR830_ERR_MATH  128 /* math error */

#define SR830MAXINSTRS 2

#define TRUE 1
#define FALSE 0

typedef enum {X, Y, M, P, ADC1, ADC2, ADC3, ADC4} sr830_channels;
typedef struct
{
    acqchanPtr  channels[8];
    sourcePtr   sources[5];
    int         sens, inputstatus, autosens, id;
    struct      {int reserve, filter, output;} overload;
	rangePtr *FREQrange, DACrange;
}   sr830Type;

typedef sr830Type *sr830Ptr;

void sr830_GetLIAStatusByte (gpibioPtr dev);
void sr830_GetStandardStatusByte (gpibioPtr dev);
void sr830_GetErrorStatusByte (gpibioPtr dev);
void sr830_CheckSRQ (gpibioPtr dev);

void    sr830_Out (gpibioPtr dev, char cmd[260]);
void    sr830_In (gpibioPtr dev, char msg[260]);

int     sr830_GetTimeConstant (gpibioPtr dev);
int     sr830_GetSensitivity (gpibioPtr dev);
int     sr830_GetFilterSlope (gpibioPtr dev);
int     sr830_GetSync (gpibioPtr dev);
int     sr830_GetDynReserve (gpibioPtr dev);
int     sr830_GetLineReject (gpibioPtr dev);
int     sr830_GetInputStatus (gpibioPtr dev);
double  sr830_GetRefPhase (gpibioPtr dev);
double  sr830_GetRefFreq (gpibioPtr dev);
double  sr830_GetRefAmpl (gpibioPtr dev);

double  sr830_Conv2Sensitivity (int sens, int inputstatus);
void    sr830_GetXYMP (gpibioPtr dev);
void    sr830_GetADCs (gpibioPtr dev);
void    sr830_SetDAC (gpibioPtr dev, int i, sourcePtr src);
void    sr830_GetDAC (gpibioPtr dev, int i, sourcePtr src);

void    sr830_UpdateReadings (int panel, void *dev);
void    sr830_XYMP_UpdateReadings (int panel, void *lia);
void    sr830_ADC_UpdateReadings (int panel, void *dev);
void    sr830_DAC_UpdateReadings (int panel, void *lia);

void    sr830_UpdateControls (int panel, gpibioPtr dev);

void    sr830_GetLIAReadings (acqchanPtr acqchan);
void    sr830_GetADCReading (acqchanPtr acqchan);
void    sr830_SetDAC1 (sourcePtr src);
void    sr830_SetDAC2 (sourcePtr src);
void    sr830_SetDAC3 (sourcePtr src);
void    sr830_SetDAC4 (sourcePtr src);
void    sr830_GetDAC1 (acqchanPtr aqcchan);
void    sr830_GetDAC2 (acqchanPtr aqcchan);
void    sr830_GetDAC3 (acqchanPtr aqcchan);
void    sr830_GetDAC4 (acqchanPtr aqcchan);

void    sr830_Save (gpibioPtr dev);
void    sr830_Load (gpibioPtr dev);

static  void *sr830_Create (gpibioPtr dev);
static void sr830_Remove (void *ptr);

static  int sr830_InitGPIB (gpibioPtr dev);
void    OperateSR830 (int menubar, int menuItem, void *callbackData, int panel);

void sr830_Init (void)
{
    devTypePtr devType;
    if (utilG.acq.status != ACQ_NONE) {
        util_ChangeInitMessage ("Stanford SR830 Control Utilities...");
        devType = malloc (sizeof (devTypeItem));
        if (devType) {
            Fmt (devType->label, "Stanford Research SR830");
            Fmt (devType->id, SR830_ID);
            devType->CreateDevice = sr830_Create;
            devType->InitDevice = sr830_InitGPIB;
            devType->OperateDevice = OperateSR830;
            devType->UpdateReadings = sr830_UpdateReadings;
            devType->SaveDevice = sr830_Save;
            devType->LoadDevice = sr830_Load;
            devType->RemoveDevice = sr830_Remove;
            devTypeList_AddItem (devType);
        }
    }
}


int sr830_GetIntVal(gpibioPtr dev, char *msg)
{
	char cmd[256];
	int freq;
    gpibio_Out (dev, msg);
    gpibio_In (dev, cmd);
    Scan (cmd, "%s>%i", &freq);
    return freq;
}
void sr830_CheckClear(gpibioPtr dev)
{
	int i = 0;
	while(!i)
	{
		if(i != 2 && i != 5 && i != 6)
			i = sr830_GetIntVal(dev, "*STB?");
	}
		
}
void sr830_GetCharVal(gpibioPtr dev, char *msg, char *cmd)
{
	sr830_CheckClear(dev);
	gpibio_Out (dev, msg);
    gpibio_In (dev, cmd);
}


double sr830_GetDoubleVal(gpibioPtr dev, char *msg)
{
    char cmd[256];
	double freq;
    sr830_CheckClear(dev);
	gpibio_Out (dev, msg);
    gpibio_In (dev, cmd);
    Scan (cmd, "%s>%f", &freq);
    return freq;
}


void sr830_Save (gpibioPtr dev)
{
    sr830_channels chan;
    sr830Ptr lia = dev->device;
	
	FmtFile (fileHandle.analysis, "%s<Ref Ampl   : %f\n", sr830_GetRefAmpl(dev));
	FmtFile (fileHandle.analysis, "%s<Ref Freq   : %f\n", sr830_GetRefFreq(dev));
    FmtFile (fileHandle.analysis, "%s<Ref Phase  : %f\n", sr830_GetRefPhase(dev));

    FmtFile (fileHandle.analysis, "%s<Time Const : %i\n", sr830_GetTimeConstant(dev));
    FmtFile (fileHandle.analysis, "%s<Filter Slp : %i\n", sr830_GetFilterSlope(dev));
    FmtFile (fileHandle.analysis, "%s<Sync Filter: %i\n", sr830_GetSync(dev));
    FmtFile (fileHandle.analysis, "%s<Dyn Reserve: %i\n", sr830_GetDynReserve(dev));
    FmtFile (fileHandle.analysis, "%s<Line Filter: %i\n", sr830_GetLineReject(dev));

    for (chan = X; chan <= ADC4; chan++) acqchan_Save (lia->channels[chan]);

    source_Save (lia->sources[0]);
    source_Save (lia->sources[1]);
    source_Save (lia->sources[2]);
    source_Save (lia->sources[3]);
	source_Save (lia->sources[4]); 
}

void sr830_Load (gpibioPtr dev)
{
    char cmd[256];
    double r;
    int i;
    sr830_channels chan;
    sr830Ptr lia;

    if (dev) lia = dev->device;
    else lia = sr830_Create (NULL);

    ScanFile (fileHandle.analysis, "%s>Ref Ampl   : %f[x]", &r);
    if (dev) { Fmt (cmd, "SLVL%f", r); sr830_Out (dev, cmd);}

    ScanFile (fileHandle.analysis, "%s>Ref Freq   : %f[x]", &r);
    if (dev) { Fmt (cmd, "FREQ%f", r); sr830_Out (dev, cmd);}

    ScanFile (fileHandle.analysis, "%s>Ref Phase  : %f[x]", &r);
    if (dev) { Fmt (cmd, "PHAS%f", r); sr830_Out (dev, cmd);}

    ScanFile (fileHandle.analysis, "%s>Time Const : %i[x]", &i);
    if (dev) { Fmt (cmd, "OFLT%i", i); sr830_Out (dev, cmd);}

    ScanFile (fileHandle.analysis, "%s>Filter Slp : %i[x]", &i);
    if (dev) { Fmt (cmd, "OFSL%i", i); sr830_Out (dev, cmd);}

    ScanFile (fileHandle.analysis, "%s>Sync Filter: %i[x]", &i);
    if (dev) { Fmt (cmd, "SYNC%i", i); sr830_Out (dev, cmd);}

    ScanFile (fileHandle.analysis, "%s>Dyn Reserve: %i[x]", &i);
    if (dev) { Fmt (cmd, "RMOD%i", i); sr830_Out (dev, cmd);}

    ScanFile (fileHandle.analysis, "%s>Line Filter: %i[x]", &i);
    if (dev) { Fmt (cmd, "ILIN%i", i); sr830_Out (dev, cmd);}

    for (chan = X; chan <= ADC4; chan++) acqchan_Load (dev, lia->channels[chan]);

    for (i = 0; i < 4; i++) {
        lia->sources[i]->min = -10.5;
        lia->sources[i]->max = 10.5;
    }

    source_Load (dev, lia->sources[0]);
    source_Load (dev, lia->sources[1]);
    source_Load (dev, lia->sources[2]);
    source_Load (dev, lia->sources[3]);
	source_Load (dev, lia->sources[4]);   
	lia->sources[4]->min = 1.0E-3;
    lia->sources[4]->max = 102e3;
    if (!dev) sr830_Remove (lia);
}

static int sr830_InitGPIB (gpibioPtr dev)
{
    int byte_register;
    char cmd[80], rsp[256];

    gpibio_Remote (dev);

    if (gpibio_DeviceMatch (dev, "*IDN?", SR830_ID)) {
        gpibio_Out (dev, "OUTX1");
        gpibio_Out (dev, "*CLS");

        byte_register = SR830_SRE_ERR+
                        SR830_SRE_LIA+
  /*                    SR830_SRE_MAV+        */
                        SR830_SRE_ESB;

        Fmt (cmd, "*SRE%i", byte_register);
        gpibio_Out (dev, cmd);

        byte_register = SR830_ESE_EXE+
                        SR830_ESE_CMD+
                        SR830_ESE_PON;
        Fmt (cmd, "*ESE%i", byte_register);
        gpibio_Out (dev, cmd);

        byte_register = SR830_LIA_RESRV+
                        SR830_LIA_FILTR+
                        SR830_LIA_OUTPT;
        Fmt (cmd, "LIAE%i", byte_register);
        gpibio_Out (dev, cmd);

        byte_register = SR830_ERR_BACKUP+
                        SR830_ERR_RAM+
                        SR830_ERR_ROM+
                        SR830_ERR_GPIB+
                        SR830_ERR_DSP+
                        SR830_ERR_MATH;

        Fmt (cmd, "ERRE%i", byte_register);
        gpibio_Out (dev, cmd);
        return TRUE;
    }
    return FALSE;
}

static void sr830_Remove (void *ptr)
{
    sr830Ptr lia = ptr;
    sr830_channels chan;
    int i;
    for (chan = X; chan <= ADC4; chan++) acqchan_Remove(lia->channels[chan]);
    for (i = 0; i < 5; i ++) source_Remove (lia->sources[i]);

    free (lia);
}
void sr830_SetFREQ (sourcePtr src)
{
	gpibioPtr dev = src->acqchan->dev;
    char cmd[256];
    double dly;
    Fmt (cmd, "FREQ%f", src->biaslevel);
    gpibio_Out (dev, cmd);
    util_Delay (src->segments[src->seg]->delay);
}
void sr830_GetFREQ (acqchanPtr acqchan)
{
    gpibioPtr dev = acqchan->dev;
    sr830Ptr lia = dev->device;
    sourcePtr src = lia->sources[4];//last source, goes after DACs
    char cmd[256], rsp[256];
    src->acqchan->reading = sr830_GetDoubleVal(dev, "FREQ?");
    src->acqchan->newreading = TRUE;
}
static void *sr830_Create (gpibioPtr dev)
{
    sr830Ptr lia;

    lia = malloc (sizeof(sr830Type));
    lia->overload.reserve = FALSE;
    lia->overload.filter = FALSE;
    lia->overload.output = FALSE;
    lia->inputstatus = 0;
    lia->autosens = FALSE;
	lia->DACrange = range_Create (10.5, -10.5, 1e-3);
	lia->FREQrange = calloc(6, sizeof(rangeType));
	lia->FREQrange[0] = range_Create (19.999, 1e-3, 1e-3);
	lia->FREQrange[1] = range_Create (199.999, 20, 0.01);
	lia->FREQrange[2] = range_Create (1999.999, 200, 0.1);
	lia->FREQrange[3] = range_Create (19999.999, 2000, 1);
	lia->FREQrange[4] = range_Create (102e3, 20000, 10);
	lia->FREQrange[5] = NULL;
	if (dev) lia->id = dev->id;

    lia->channels[X] = acqchan_Create ("SR830 X", dev, sr830_GetLIAReadings);
    lia->channels[Y] = acqchan_Create ("SR830 Y", dev, sr830_GetLIAReadings);
    lia->channels[M] = acqchan_Create ("SR830 M", dev, sr830_GetLIAReadings);
    lia->channels[P] = acqchan_Create ("SR830 P", dev, sr830_GetLIAReadings);

    lia->channels[ADC1] = acqchan_Create ("SR830 ADC1", dev, sr830_GetADCReading);
    lia->channels[ADC2] = acqchan_Create ("SR830 ADC2", dev, sr830_GetADCReading);
    lia->channels[ADC3] = acqchan_Create ("SR830 ADC3", dev, sr830_GetADCReading);
    lia->channels[ADC4] = acqchan_Create ("SR830 ADC4", dev, sr830_GetADCReading);

    lia->sources[0] = source_Create ("SR830 DAC1", dev, sr830_SetDAC1, sr830_GetDAC1);
	lia->sources[0]->ranges.temprange[0] = lia->DACrange;
    
	lia->sources[1] = source_Create ("SR830 DAC2", dev, sr830_SetDAC2, sr830_GetDAC2);
    lia->sources[1]->ranges.temprange[0] = lia->DACrange;
	
	lia->sources[2] = source_Create ("SR830 DAC3", dev, sr830_SetDAC3, sr830_GetDAC3);
    lia->sources[2]->ranges.temprange[0] = lia->DACrange;
	
	lia->sources[3] = source_Create ("SR830 DAC4", dev, sr830_SetDAC4, sr830_GetDAC4);
	lia->sources[3]->ranges.temprange[0] = lia->DACrange;
	
	lia->sources[4] = source_Create ("SR830 Freq", dev, sr830_SetFREQ, sr830_GetFREQ);
	// similar to : lia->sources[2] = source_Create ("SR844 Freq", dev, sr844_SetFREQ, sr844_GetFREQ);
	lia->sources[4]->freq = 1;
	lia->sources[4]->ranges.autoscale = 1;
	lia->sources[4]->ranges.temprange = lia->FREQrange;
		
    if (dev) dev->device = lia;
    return lia;
}

void LoadSR830SetupCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    int fileselect, id;
    char path[260], info[260];
    gpibioPtr dev = callbackData;
    sr830Ptr lia = dev->device;

    fileselect = FileSelectPopup ("", "*.dev", "*.dev", "Load Stanford SR830 Lock-in Setup",
                                  VAL_LOAD_BUTTON, 0, 1, 1, 0, path);
    if (fileselect == VAL_EXISTING_FILE_SELECTED) {
        fileHandle.analysis = util_OpenFile (path, FILE_READ, FALSE);
        ScanFile (fileHandle.analysis, "%s>#INSTRSETUP %i", &id);
        if (lia->id == id) {
            sr830_Load (dev);
            ReadLine (fileHandle.analysis, info, 255);
            sr830_UpdateControls(panel, dev);
        }
        else MessagePopup ("Stanford Load Message", "Different instrument types--process aborted");
        util_CloseFile();
    }
}

void SaveSR830SetupCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    int fileselect;
    char path[260];
    gpibioPtr dev = callbackData;
    sr830Ptr lia = dev->device;
    fileselect = FileSelectPopup ("", "*.dev", "*.dev", "Save Lock-in Setup",
                                  VAL_SAVE_BUTTON, 0, 1, 1, 1, path);
    if (fileselect == VAL_NEW_FILE_SELECTED) {
        fileHandle.analysis = util_OpenFile (path, FILE_WRITE, FALSE);
        FmtFile (fileHandle.analysis, "%s<#INSTRSETUP %i\n", lia->id);
        sr830_Save(dev);
        FmtFile (fileHandle.analysis, "#ENDSETUP\n");
        util_CloseFile();
    }
}

void DACCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    sourcePtr src;

    src = callbackData;
    src->min = -10.5;
    src->max = 10.5;

    switch (utilG.exp) {
        case EXP_SOURCE: source_InitPanel (src); break;
        case EXP_FLOAT: gensrc_InitPanel (src); break;
    }
}
void FreqCallback(int menubar, int menuItem, void *callbackData, int panel) 
{
	sourcePtr src;

    		src = callbackData;
			src->min = 1e-3;
    		src->max = 102e3;
			switch (utilG.exp) {
        		case EXP_SOURCE: source_InitPanel (src); break;
        		case EXP_FLOAT: gensrc_InitPanel (src); break;
    		}
			
}
int  DACControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    sr830Ptr lia;
    acqchanPtr acqchan;

    acqchan = callbackData;

    switch (control) {
        case SR830_DAC_NOTE_1:
        case SR830_DAC_NOTE_2:
        case SR830_DAC_NOTE_3:
        case SR830_DAC_NOTE_4:
            AcqDataNoteCallback (panel, control, event, callbackData, eventData1, eventData2);
            break;
        case SR830_DAC_ACQ_1:
        case SR830_DAC_ACQ_2:
        case SR830_DAC_ACQ_3:
        case SR830_DAC_ACQ_4:
            if (event == EVENT_VAL_CHANGED) {
                GetCtrlVal (panel, control, &acqchan->acquire);
                if (acqchan->acquire) acqchanlist_AddChannel (acqchan);
                    else acqchanlist_RemoveChannel (acqchan);
            }
            break;
        case SR830_DAC_COEFF_1:
        case SR830_DAC_COEFF_2:
        case SR830_DAC_COEFF_3:
        case SR830_DAC_COEFF_4:
            if (event == EVENT_COMMIT) {
                GetCtrlVal (panel, control, &acqchan->coeff);
                if (acqchan->p) SetCtrlVal (acqchan->p, ACQDATA_COEFF, acqchan->coeff);
            }
            break;
        case SR830_DAC_LABEL_1:
        case SR830_DAC_LABEL_2:
        case SR830_DAC_LABEL_3:
        case SR830_DAC_LABEL_4:
            if (event == EVENT_COMMIT) {
                GetCtrlVal (panel, control, acqchan->channel->label);
                acqchanlist_ReplaceChannel (acqchan);
                if (acqchan->p) SetPanelAttribute (acqchan->p, ATTR_TITLE, acqchan->channel->label);
            }
            break;
        case SR830_DAC_CLOSE:
            if (event == EVENT_COMMIT) {
                lia = callbackData;
                devPanel_Remove (panel);
                //util_Discard (panel);
                DiscardPanel (panel);
            }
            break;
    }
    return 0;
}

void sr830_DAC_UpdateReadings (int panel, void *lia)
{
    sr830Ptr my_lia = lia;

    sr830_GetDAC1 (my_lia->sources[0]->acqchan);
    sr830_GetDAC2 (my_lia->sources[1]->acqchan);
    sr830_GetDAC3 (my_lia->sources[2]->acqchan);
    sr830_GetDAC4 (my_lia->sources[3]->acqchan);
    SetCtrlVal (panel, SR830_DAC_MEAS_1, my_lia->sources[0]->acqchan->reading);
    SetCtrlVal (panel, SR830_DAC_MEAS_2, my_lia->sources[1]->acqchan->reading);
    SetCtrlVal (panel, SR830_DAC_MEAS_3, my_lia->sources[2]->acqchan->reading);
    SetCtrlVal (panel, SR830_DAC_MEAS_4, my_lia->sources[3]->acqchan->reading);
}

void MeasDACCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    int p;
    sr830Ptr lia;

    p = LoadPanel (0, "sr830u.uir", SR830_DAC);
    SetPanelPos (p, VAL_AUTO_CENTER, VAL_AUTO_CENTER);
    
    util_InitClose (p, SR830_DAC_CLOSE, FALSE);

    lia = callbackData;

    SetCtrlVal (p, SR830_DAC_LABEL_1, lia->sources[0]->acqchan->channel->label);
    SetCtrlVal (p, SR830_DAC_COEFF_1, lia->sources[0]->acqchan->coeff);
    SetCtrlVal (p, SR830_DAC_ACQ_1, lia->sources[0]->acqchan->acquire);
    SetCtrlVal (p, SR830_DAC_NOTE_1, lia->sources[0]->acqchan->note);

    SetCtrlVal (p, SR830_DAC_LABEL_2, lia->sources[1]->acqchan->channel->label);
    SetCtrlVal (p, SR830_DAC_COEFF_2, lia->sources[1]->acqchan->coeff);
    SetCtrlVal (p, SR830_DAC_ACQ_2, lia->sources[1]->acqchan->acquire);
    SetCtrlVal (p, SR830_DAC_NOTE_2, lia->sources[1]->acqchan->note);

    SetCtrlVal (p, SR830_DAC_LABEL_3, lia->sources[2]->acqchan->channel->label);
    SetCtrlVal (p, SR830_DAC_COEFF_3, lia->sources[2]->acqchan->coeff);
    SetCtrlVal (p, SR830_DAC_ACQ_3, lia->sources[2]->acqchan->acquire);
    SetCtrlVal (p, SR830_DAC_NOTE_3, lia->sources[2]->acqchan->note);

    SetCtrlVal (p, SR830_DAC_LABEL_4, lia->sources[3]->acqchan->channel->label);
    SetCtrlVal (p, SR830_DAC_COEFF_4, lia->sources[3]->acqchan->coeff);
    SetCtrlVal (p, SR830_DAC_ACQ_4, lia->sources[3]->acqchan->acquire);
    SetCtrlVal (p, SR830_DAC_NOTE_4, lia->sources[3]->acqchan->note);

    SetCtrlAttribute(p, SR830_DAC_CLOSE, ATTR_CALLBACK_DATA, lia);

    SetCtrlAttribute(p, SR830_DAC_LABEL_1, ATTR_CALLBACK_DATA, lia->sources[0]->acqchan);
    SetCtrlAttribute(p, SR830_DAC_COEFF_1, ATTR_CALLBACK_DATA, lia->sources[0]->acqchan);
    SetCtrlAttribute(p, SR830_DAC_ACQ_1, ATTR_CALLBACK_DATA, lia->sources[0]->acqchan);
    SetCtrlAttribute(p, SR830_DAC_NOTE_1, ATTR_CALLBACK_DATA, lia->sources[0]->acqchan);

    SetCtrlAttribute(p, SR830_DAC_LABEL_2, ATTR_CALLBACK_DATA, lia->sources[1]->acqchan);
    SetCtrlAttribute(p, SR830_DAC_COEFF_2, ATTR_CALLBACK_DATA, lia->sources[1]->acqchan);
    SetCtrlAttribute(p, SR830_DAC_ACQ_2, ATTR_CALLBACK_DATA, lia->sources[1]->acqchan);
    SetCtrlAttribute(p, SR830_DAC_NOTE_2, ATTR_CALLBACK_DATA, lia->sources[1]->acqchan);

    SetCtrlAttribute(p, SR830_DAC_LABEL_3, ATTR_CALLBACK_DATA, lia->sources[2]->acqchan);
    SetCtrlAttribute(p, SR830_DAC_COEFF_3, ATTR_CALLBACK_DATA, lia->sources[2]->acqchan);
    SetCtrlAttribute(p, SR830_DAC_ACQ_3, ATTR_CALLBACK_DATA, lia->sources[2]->acqchan);
    SetCtrlAttribute(p, SR830_DAC_NOTE_3, ATTR_CALLBACK_DATA, lia->sources[2]->acqchan);

    SetCtrlAttribute(p, SR830_DAC_LABEL_4, ATTR_CALLBACK_DATA, lia->sources[3]->acqchan);
    SetCtrlAttribute(p, SR830_DAC_COEFF_4, ATTR_CALLBACK_DATA, lia->sources[3]->acqchan);
    SetCtrlAttribute(p, SR830_DAC_ACQ_4, ATTR_CALLBACK_DATA, lia->sources[3]->acqchan);
    SetCtrlAttribute(p, SR830_DAC_NOTE_4, ATTR_CALLBACK_DATA, lia->sources[3]->acqchan);

    devPanel_Add (p, lia, sr830_DAC_UpdateReadings);
    InstallPopup (p);
}

int  ADCControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    sr830Ptr lia;
    acqchanPtr acqchan;

    acqchan = callbackData;

    switch (control) {
        case SR830_ADC_NOTE_1:
        case SR830_ADC_NOTE_2:
        case SR830_ADC_NOTE_3:
        case SR830_ADC_NOTE_4:
            AcqDataNoteCallback (panel, control, event, callbackData, eventData1, eventData2);
            break;
        case SR830_ADC_ACQ_1:
        case SR830_ADC_ACQ_2:
        case SR830_ADC_ACQ_3:
        case SR830_ADC_ACQ_4:
            if (event == EVENT_VAL_CHANGED) {
                GetCtrlVal (panel, control, &acqchan->acquire);
                if (acqchan->acquire) acqchanlist_AddChannel (acqchan);
                    else acqchanlist_RemoveChannel (acqchan);
            }
            break;
        case SR830_ADC_CONVERSION_1:
        case SR830_ADC_CONVERSION_2:
        case SR830_ADC_CONVERSION_3:
        case SR830_ADC_CONVERSION_4:
            if (event == EVENT_VAL_CHANGED) {
                GetCtrlVal (panel, control, &acqchan->conversion);
                if (acqchan->p) SetCtrlVal (acqchan->p, ACQDATA_CONV, acqchan->conversion);
            }
            break;
        case SR830_ADC_COEFF_1:
        case SR830_ADC_COEFF_2:
        case SR830_ADC_COEFF_3:
        case SR830_ADC_COEFF_4:
            if (event == EVENT_COMMIT) {
                GetCtrlVal (panel, control, &acqchan->coeff);
                if (acqchan->p) SetCtrlVal (acqchan->p, ACQDATA_COEFF, acqchan->coeff);
            }
            break;
        case SR830_ADC_LABEL_1:
        case SR830_ADC_LABEL_2:
        case SR830_ADC_LABEL_3:
        case SR830_ADC_LABEL_4:
            if (event == EVENT_COMMIT) {
                GetCtrlVal (panel, control, acqchan->channel->label);
                acqchanlist_ReplaceChannel (acqchan);
                if (acqchan->p) SetPanelAttribute (acqchan->p, ATTR_TITLE, acqchan->channel->label);
            }
            break;
        case SR830_ADC_CLOSE:
            if (event == EVENT_COMMIT) {
                lia = callbackData;
                devPanel_Remove(panel);
                //util_Discard (panel);
                DiscardPanel (panel);
            }
            break;
    }
    return 0;
}

void sr830_ADC_UpdateReadings (int panel, void *dev)
{
    gpibioPtr my_dev = dev;
    sr830Ptr lia = my_dev->device;

    sr830_GetADCs (my_dev);

    SetCtrlVal (panel, SR830_ADC_MEAS_1, lia->channels[ADC1]->reading);
    SetCtrlVal (panel, SR830_ADC_MEAS_2, lia->channels[ADC2]->reading);
    SetCtrlVal (panel, SR830_ADC_MEAS_3, lia->channels[ADC3]->reading);
    SetCtrlVal (panel, SR830_ADC_MEAS_4, lia->channels[ADC4]->reading);
}

void MeasADCCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    int p;
    gpibioPtr dev = callbackData;
    sr830Ptr lia = dev->device;

    p = LoadPanel (0, "sr830u.uir", SR830_ADC);
    SetPanelPos (p, VAL_AUTO_CENTER, VAL_AUTO_CENTER);
    
    util_InitClose (p, SR830_ADC_CLOSE, FALSE);

    SetCtrlVal (p, SR830_ADC_LABEL_1, lia->channels[ADC1]->channel->label);
    SetCtrlVal (p, SR830_ADC_COEFF_1, lia->channels[ADC1]->coeff);
    SetCtrlVal (p, SR830_ADC_CONVERSION_1, lia->channels[ADC1]->conversion);
    SetCtrlVal (p, SR830_ADC_ACQ_1, lia->channels[ADC1]->acquire);
    SetCtrlVal (p, SR830_ADC_NOTE_1, lia->channels[ADC1]->note);

    SetCtrlVal (p, SR830_ADC_LABEL_2, lia->channels[ADC2]->channel->label);
    SetCtrlVal (p, SR830_ADC_COEFF_2, lia->channels[ADC2]->coeff);
    SetCtrlVal (p, SR830_ADC_CONVERSION_2, lia->channels[ADC2]->conversion);
    SetCtrlVal (p, SR830_ADC_ACQ_2, lia->channels[ADC2]->acquire);
    SetCtrlVal (p, SR830_ADC_NOTE_2, lia->channels[ADC2]->note);

    SetCtrlVal (p, SR830_ADC_LABEL_3, lia->channels[ADC3]->channel->label);
    SetCtrlVal (p, SR830_ADC_COEFF_3, lia->channels[ADC3]->coeff);
    SetCtrlVal (p, SR830_ADC_CONVERSION_3, lia->channels[ADC3]->conversion);
    SetCtrlVal (p, SR830_ADC_ACQ_3, lia->channels[ADC3]->acquire);
    SetCtrlVal (p, SR830_ADC_NOTE_3, lia->channels[ADC3]->note);

    SetCtrlVal (p, SR830_ADC_LABEL_4, lia->channels[ADC4]->channel->label);
    SetCtrlVal (p, SR830_ADC_COEFF_4, lia->channels[ADC4]->coeff);
    SetCtrlVal (p, SR830_ADC_CONVERSION_4, lia->channels[ADC4]->conversion);
    SetCtrlVal (p, SR830_ADC_ACQ_4, lia->channels[ADC4]->acquire);
    SetCtrlVal (p, SR830_ADC_NOTE_4, lia->channels[ADC4]->note);

    SetCtrlAttribute(p, SR830_ADC_CLOSE, ATTR_CALLBACK_DATA, lia);

    SetCtrlAttribute(p, SR830_ADC_LABEL_1, ATTR_CALLBACK_DATA, lia->channels[ADC1]);
    SetCtrlAttribute(p, SR830_ADC_COEFF_1, ATTR_CALLBACK_DATA, lia->channels[ADC1]);
    SetCtrlAttribute(p, SR830_ADC_CONVERSION_1, ATTR_CALLBACK_DATA, lia->channels[ADC1]);
    SetCtrlAttribute(p, SR830_ADC_ACQ_1, ATTR_CALLBACK_DATA, lia->channels[ADC1]);
    SetCtrlAttribute(p, SR830_ADC_NOTE_1, ATTR_CALLBACK_DATA, lia->channels[ADC1]);

    SetCtrlAttribute(p, SR830_ADC_LABEL_2, ATTR_CALLBACK_DATA, lia->channels[ADC2]);
    SetCtrlAttribute(p, SR830_ADC_COEFF_2, ATTR_CALLBACK_DATA, lia->channels[ADC2]);
    SetCtrlAttribute(p, SR830_ADC_CONVERSION_2, ATTR_CALLBACK_DATA, lia->channels[ADC2]);
    SetCtrlAttribute(p, SR830_ADC_ACQ_2, ATTR_CALLBACK_DATA, lia->channels[ADC2]);
    SetCtrlAttribute(p, SR830_ADC_NOTE_2, ATTR_CALLBACK_DATA, lia->channels[ADC2]);

    SetCtrlAttribute(p, SR830_ADC_LABEL_3, ATTR_CALLBACK_DATA, lia->channels[ADC3]);
    SetCtrlAttribute(p, SR830_ADC_COEFF_3, ATTR_CALLBACK_DATA, lia->channels[ADC3]);
    SetCtrlAttribute(p, SR830_ADC_CONVERSION_3, ATTR_CALLBACK_DATA, lia->channels[ADC3]);
    SetCtrlAttribute(p, SR830_ADC_ACQ_3, ATTR_CALLBACK_DATA, lia->channels[ADC3]);
    SetCtrlAttribute(p, SR830_ADC_NOTE_3, ATTR_CALLBACK_DATA, lia->channels[ADC3]);

    SetCtrlAttribute(p, SR830_ADC_LABEL_4, ATTR_CALLBACK_DATA, lia->channels[ADC4]);
    SetCtrlAttribute(p, SR830_ADC_COEFF_4, ATTR_CALLBACK_DATA, lia->channels[ADC4]);
    SetCtrlAttribute(p, SR830_ADC_CONVERSION_4, ATTR_CALLBACK_DATA, lia->channels[ADC4]);
    SetCtrlAttribute(p, SR830_ADC_ACQ_4, ATTR_CALLBACK_DATA, lia->channels[ADC4]);
    SetCtrlAttribute(p, SR830_ADC_NOTE_4, ATTR_CALLBACK_DATA, lia->channels[ADC4]);

	InsertListItem (p, SR830_ADC_CONVERSION_1, 3, "Load Temp Calibration", 3);
	InsertListItem (p, SR830_ADC_CONVERSION_2, 3, "Load Temp Calibration", 3);
	InsertListItem (p, SR830_ADC_CONVERSION_3, 3, "Load Temp Calibration", 3);
	InsertListItem (p, SR830_ADC_CONVERSION_4, 3, "Load Temp Calibration", 3);
	
	devPanel_Add (p, dev, sr830_ADC_UpdateReadings);
    InstallPopup (p);
}

int  XYMPControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    sr830Ptr lia;
    acqchanPtr acqchan;

    acqchan = callbackData;

    switch (control) {
        case SR830_XYMP_NOTE_1:
        case SR830_XYMP_NOTE_2:
        case SR830_XYMP_NOTE_3:
        case SR830_XYMP_NOTE_4:
            AcqDataNoteCallback (panel, control, event, callbackData, eventData1, eventData2);
            break;
        case SR830_XYMP_XACQ:
        case SR830_XYMP_YACQ:
        case SR830_XYMP_MACQ:
        case SR830_XYMP_PACQ:
            if (event == EVENT_VAL_CHANGED) {
                GetCtrlVal (panel, control, &acqchan->acquire);
                if (acqchan->acquire) acqchanlist_AddChannel (acqchan);
                    else acqchanlist_RemoveChannel (acqchan);
            }
            break;
        case SR830_XYMP_XCOEFF:
        case SR830_XYMP_YCOEFF:
        case SR830_XYMP_MCOEFF:
        case SR830_XYMP_PCOEFF:
            if (event == EVENT_COMMIT) {
                GetCtrlVal (panel, control, &acqchan->coeff);
                if (acqchan->p) SetCtrlVal (acqchan->p, ACQDATA_COEFF, acqchan->coeff);
            }
            break;
        case SR830_XYMP_XLABEL:
        case SR830_XYMP_YLABEL:
        case SR830_XYMP_MLABEL:
        case SR830_XYMP_PLABEL:
            if (event == EVENT_COMMIT) {
                GetCtrlVal (panel, control, acqchan->channel->label);
                acqchanlist_ReplaceChannel (acqchan);
                if (acqchan->p) SetPanelAttribute (acqchan->p, ATTR_TITLE, acqchan->channel->label);
            }
            break;
        case SR830_XYMP_CLOSE:
            if (event == EVENT_COMMIT) {
                lia = callbackData;
                devPanel_Remove(panel);
                //util_Discard (panel);
                DiscardPanel (panel);
            }
            break;
    }
    return 0;
}

void sr830_XYMP_UpdateReadings (int panel, void *lia)
{
    sr830Ptr my_lia = lia;

    SetCtrlVal (panel, SR830_XYMP_XMEAS, my_lia->channels[X]->reading*my_lia->channels[X]->coeff);
    SetCtrlVal (panel, SR830_XYMP_YMEAS, my_lia->channels[Y]->reading*my_lia->channels[Y]->coeff);
    SetCtrlVal (panel, SR830_XYMP_MMEAS, my_lia->channels[M]->reading*my_lia->channels[M]->coeff);
    SetCtrlVal (panel, SR830_XYMP_PMEAS, my_lia->channels[P]->reading*my_lia->channels[P]->coeff);
}

void MeasXYMPCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    int p;
    sr830Ptr lia;

    p = LoadPanel (0, "sr830u.uir", SR830_XYMP);
    SetPanelPos (p, VAL_AUTO_CENTER, VAL_AUTO_CENTER);
    
    util_InitClose (p, SR830_XYMP_CLOSE, FALSE);

    lia = callbackData;

    SetCtrlVal (p, SR830_XYMP_XLABEL, lia->channels[X]->channel->label);
    SetCtrlVal (p, SR830_XYMP_XCOEFF, lia->channels[X]->coeff);
    SetCtrlVal (p, SR830_XYMP_XACQ, lia->channels[X]->acquire);
    SetCtrlVal (p, SR830_XYMP_NOTE_1, lia->channels[X]->note);

    SetCtrlVal (p, SR830_XYMP_YLABEL, lia->channels[Y]->channel->label);
    SetCtrlVal (p, SR830_XYMP_YCOEFF, lia->channels[Y]->coeff);
    SetCtrlVal (p, SR830_XYMP_YACQ, lia->channels[Y]->acquire);
    SetCtrlVal (p, SR830_XYMP_NOTE_2, lia->channels[Y]->note);

    SetCtrlVal (p, SR830_XYMP_MLABEL, lia->channels[M]->channel->label);
    SetCtrlVal (p, SR830_XYMP_MCOEFF, lia->channels[M]->coeff);
    SetCtrlVal (p, SR830_XYMP_MACQ, lia->channels[M]->acquire);
    SetCtrlVal (p, SR830_XYMP_NOTE_3, lia->channels[M]->note);

    SetCtrlVal (p, SR830_XYMP_PLABEL, lia->channels[P]->channel->label);
    SetCtrlVal (p, SR830_XYMP_PCOEFF, lia->channels[P]->coeff);
    SetCtrlVal (p, SR830_XYMP_PACQ, lia->channels[P]->acquire);
    SetCtrlVal (p, SR830_XYMP_NOTE_4, lia->channels[P]->note);

    SetCtrlAttribute(p, SR830_XYMP_CLOSE, ATTR_CALLBACK_DATA, lia);

    SetCtrlAttribute(p, SR830_XYMP_XLABEL, ATTR_CALLBACK_DATA, lia->channels[X]);
    SetCtrlAttribute(p, SR830_XYMP_XCOEFF, ATTR_CALLBACK_DATA, lia->channels[X]);
    SetCtrlAttribute(p, SR830_XYMP_XACQ, ATTR_CALLBACK_DATA, lia->channels[X]);
    SetCtrlAttribute(p, SR830_XYMP_NOTE_1, ATTR_CALLBACK_DATA, lia->channels[X]);

    SetCtrlAttribute(p, SR830_XYMP_YLABEL, ATTR_CALLBACK_DATA, lia->channels[Y]);
    SetCtrlAttribute(p, SR830_XYMP_YCOEFF, ATTR_CALLBACK_DATA, lia->channels[Y]);
    SetCtrlAttribute(p, SR830_XYMP_YACQ, ATTR_CALLBACK_DATA, lia->channels[Y]);
    SetCtrlAttribute(p, SR830_XYMP_NOTE_2, ATTR_CALLBACK_DATA, lia->channels[Y]);

    SetCtrlAttribute(p, SR830_XYMP_MLABEL, ATTR_CALLBACK_DATA, lia->channels[M]);
    SetCtrlAttribute(p, SR830_XYMP_MCOEFF, ATTR_CALLBACK_DATA, lia->channels[M]);
    SetCtrlAttribute(p, SR830_XYMP_MACQ, ATTR_CALLBACK_DATA, lia->channels[M]);
    SetCtrlAttribute(p, SR830_XYMP_NOTE_3, ATTR_CALLBACK_DATA, lia->channels[M]);

    SetCtrlAttribute(p, SR830_XYMP_PLABEL, ATTR_CALLBACK_DATA, lia->channels[P]);
    SetCtrlAttribute(p, SR830_XYMP_PCOEFF, ATTR_CALLBACK_DATA, lia->channels[P]);
    SetCtrlAttribute(p, SR830_XYMP_PACQ, ATTR_CALLBACK_DATA, lia->channels[P]);
    SetCtrlAttribute(p, SR830_XYMP_NOTE_4, ATTR_CALLBACK_DATA, lia->channels[P]);

    devPanel_Add (p, lia, sr830_XYMP_UpdateReadings);
    InstallPopup (p);
}


void sr830_GetDAC4 (acqchanPtr acqchan)
{
    gpibioPtr dev = acqchan->dev;
    sr830Ptr lia = dev->device;
    sourcePtr src = lia->sources[3];
    sr830_GetDAC (dev, 4, src);
}

void sr830_GetDAC3 (acqchanPtr acqchan)
{
    gpibioPtr dev = acqchan->dev;
    sr830Ptr lia = dev->device;
    sourcePtr src = lia->sources[2];
    sr830_GetDAC (dev, 3, src);
}

void sr830_GetDAC2 (acqchanPtr acqchan)
{
    gpibioPtr dev = acqchan->dev;
    sr830Ptr lia = dev->device;
    sourcePtr src = lia->sources[1];
    sr830_GetDAC (dev, 2, src);
}

void sr830_GetDAC1 (acqchanPtr acqchan)
{
    gpibioPtr dev = acqchan->dev;
    sr830Ptr lia = dev->device;
    sourcePtr src = lia->sources[0];
    sr830_GetDAC (dev, 1, src);
}

void sr830_SetDAC4 (sourcePtr src)
{
    gpibioPtr dev = src->acqchan->dev;
    sr830_SetDAC (dev, 4, src);
}

void sr830_SetDAC3 (sourcePtr src)
{
    gpibioPtr dev = src->acqchan->dev;
    sr830_SetDAC (dev, 3, src);
}

void sr830_SetDAC2 (sourcePtr src)
{
    gpibioPtr dev = src->acqchan->dev;
    sr830_SetDAC (dev, 2, src);
}

void sr830_SetDAC1 (sourcePtr src)
{
    gpibioPtr dev = src->acqchan->dev;
    sr830_SetDAC (dev, 1, src);
}

void sr830_GetADCReading (acqchanPtr acqchan)
{
    gpibioPtr dev = acqchan->dev;
    sr830Ptr lia = dev->device;

    sr830_GetADCs (dev);
}

void sr830_GetLIAReadings (acqchanPtr acqchan)
{
    gpibioPtr dev = acqchan->dev;
    sr830Ptr lia = dev->device;

    sr830_GetXYMP (dev);
}

int  SR830RefCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    gpibioPtr dev = callbackData;
    double r;
    char cmd[256];

    switch (control) {
        case SR830_REF_AMPLITUDE:
            if (event == EVENT_VAL_CHANGED) {
                GetCtrlVal (panel, control, &r);
                Fmt (cmd, "SLVL%f", r);
                sr830_Out (dev, cmd);
                r = sr830_GetRefAmpl(dev);
                SetCtrlVal (panel, control, r);
                SetCtrlAttribute (panel, control, ATTR_INCR_VALUE, r/10);
            }
            break;
        case SR830_REF_FREQ:
            if (event == EVENT_VAL_CHANGED) {
                GetCtrlVal (panel, control, &r);
                Fmt (cmd, "FREQ%f", r);
                sr830_Out (dev, cmd);
                r = sr830_GetRefFreq (dev);
                SetCtrlVal (panel, control, r);
                SetCtrlAttribute (panel, control, ATTR_INCR_VALUE, r/10);
            }
            break;
        case SR830_REF_PHASE:
            if (event == EVENT_VAL_CHANGED) {
                GetCtrlVal (panel, control, &r);
                Fmt (cmd, "PHAS%f", r);
                sr830_Out (dev, cmd);
                SetCtrlVal (panel, control, sr830_GetRefPhase(dev));
            }
            break;
    }
    return 0;
}

int  OperateReferenceCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int p;
    gpibioPtr dev = callbackData;
    if (event == EVENT_COMMIT) {
        p = LoadPanel (0, "sr830u.uir", SR830_REF);
        
        SetPanelPos (p, VAL_AUTO_CENTER, VAL_AUTO_CENTER);

        util_InitClose (p, SR830_REF_CLOSE, FALSE);

        SetCtrlVal (p, SR830_REF_AMPLITUDE, sr830_GetRefAmpl(dev));
        SetCtrlVal (p, SR830_REF_FREQ, sr830_GetRefFreq(dev));
        SetCtrlVal (p, SR830_REF_PHASE, sr830_GetRefPhase(dev));

        SetCtrlAttribute (p, SR830_REF_AMPLITUDE, ATTR_CALLBACK_DATA, dev);
        SetCtrlAttribute (p, SR830_REF_FREQ, ATTR_CALLBACK_DATA, dev);
        SetCtrlAttribute (p, SR830_REF_PHASE, ATTR_CALLBACK_DATA, dev);
        InstallPopup (p);
    }
    return 0;
}

int  SR830ControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int i;
    char cmd[256], rsp[256];
    gpibioPtr dev;
    sr830Ptr lia;

    dev = callbackData;
    switch (control) {
        case SR830_CTRL_TC:
            if (event == EVENT_COMMIT) {
                GetCtrlVal (panel, control, &i);
                Fmt (cmd, "OFLT%i", i);
                sr830_Out (dev, cmd);
                SetCtrlIndex (panel, control, sr830_GetTimeConstant (dev));
            }
            break;
        case SR830_CTRL_SENS:
            if (event == EVENT_COMMIT) {
                GetCtrlVal (panel, control, &i);
                Fmt (cmd, "SENS%i", i);
                sr830_Out (dev, cmd);
                SetCtrlIndex (panel, control, sr830_GetSensitivity(dev));
            }
            break;
        case SR830_CTRL_AUTOSENSE:
            if (event == EVENT_COMMIT) {
                lia = dev->device;
                GetCtrlVal (panel, control, &lia->autosens);
            }
            break;
        case SR830_CTRL_FILTSLOPE:
            if (event == EVENT_COMMIT) {
                GetCtrlVal (panel, control, &i);
                Fmt (cmd, "OFSL%i", i);
                sr830_Out (dev, cmd);
                SetCtrlIndex (panel, control, sr830_GetFilterSlope(dev));
            }
            break;
        case SR830_CTRL_SYNC:
            if (event == EVENT_COMMIT) {
                if (sr830_GetRefFreq (dev) <= 200.0) {
                    GetCtrlVal (panel, control, &i);
                    Fmt (cmd, "SYNC%i", i);
                    sr830_Out (dev, cmd);
                    SetCtrlVal (panel, control, sr830_GetSync(dev));
                } else {
                    MessagePopup ("SR830 Sync Message", "Frequency must be < 200 Hz");
                    SetCtrlVal (panel, control, 0);
                }
            }
            break;
        case SR830_CTRL_DYNRES:
            if (event == EVENT_COMMIT) {
                GetCtrlVal (panel, control, &i);
                Fmt (cmd, "RMOD%i", i);
                sr830_Out (dev, cmd);
                SetCtrlIndex (panel, control, sr830_GetDynReserve(dev));
            }
            break;
        case SR830_CTRL_REJECT:
            if (event == EVENT_COMMIT) {
                GetCtrlVal (panel, control, &i);
                Fmt (cmd, "ILIN%i", i);
                sr830_Out (dev, cmd);
                SetCtrlIndex (panel, control, sr830_GetLineReject(dev));
            }
            break;
		case SR830_CTRL_HARMON:
			if(event == EVENT_COMMIT) {
				int Harm, HarmF;
				GetCtrlVal(panel, SR830_CTRL_HARMON, &Harm);
				GetCtrlVal(panel, SR830_CTRL_HARMVAL, &HarmF);
				if(Harm)
					Fmt (cmd, "HARM%i", HarmF);
				else
					Fmt (cmd, "HARM1");
				sr830_Out(dev, cmd);
			}
			break;
		case SR830_CTRL_HARMVAL:
			if(event == EVENT_VAL_CHANGED) {
				int Harm, HarmF;
				GetCtrlVal(panel, SR830_CTRL_HARMON, &Harm);
				GetCtrlVal(panel, SR830_CTRL_HARMVAL, &HarmF);
				if(Harm)
				{
					Fmt (cmd, "HARM%i", HarmF);
					sr830_Out(dev, cmd);
				}
			}
			break;
    }
    return 0;
}

int  SR830ControlPanelCallback(int panel, int event, void *callbackData, int eventData1, int eventData2)
{
    gpibioPtr dev;
    sr830Ptr lia;
    int menubar;
    if ((event == EVENT_KEYPRESS) && (eventData1 == VAL_ESC_VKEY) || event == EVENT_RIGHT_DOUBLE_CLICK) {
        dev = callbackData;
        lia = dev->device;
        devPanel_Remove (panel);
        HidePanel (panel);
		dev->iPanel = 0;
		SetMenuBarAttribute (acquire_GetMenuBar(), dev->menuitem_id, ATTR_DIMMED, FALSE);
    }

    if (event == EVENT_GOT_FOCUS) {
        dev = callbackData;
        menubar = GetPanelMenuBar (panel);
        SetPanelAttribute (panel, ATTR_DIMMED, (dev->status != DEV_REMOTE));
        SetMenuBarAttribute (menubar, SR830MENUS_SOURCES, ATTR_DIMMED, (dev->status != DEV_REMOTE));
        SetMenuBarAttribute (menubar, SR830MENUS_MEASURE, ATTR_DIMMED, (dev->status != DEV_REMOTE));
        SetMenuBarAttribute (menubar, SR830MENUS_FILE_SAVE, ATTR_DIMMED, (dev->status != DEV_REMOTE));
        SetMenuBarAttribute (menubar, SR830MENUS_FILE_LOAD, ATTR_DIMMED, (dev->status != DEV_REMOTE));
        if (!util_TakingData()) sr830_UpdateControls (panel, dev);
    }
    return 0;
}

void OperateSR830 (int menubar, int menuItem, void *callbackData, int panel)
{
    int p, m;
    gpibioPtr dev = callbackData;
    sr830Ptr lia = dev->device;
    char label[256];

    SetMenuBarAttribute (menubar, menuItem, ATTR_DIMMED, TRUE);

    p = dev->iPanel? dev->iPanel: LoadPanel (utilG.p, "sr830u.uir", SR830_CTRL);
    dev->iPanel = p;
	
    SetPanelPos (p, VAL_AUTO_CENTER, VAL_AUTO_CENTER);

    Fmt (label, "Stanford Research Systems SR830 DSP Lock-in Amplifier: %s", dev->label);
    SetPanelAttribute (p, ATTR_TITLE, label);

    m = LoadMenuBar (p, "sr830u.uir", SR830MENUS);
    
    SetPanelMenuBar (p, m);

    SetMenuBarAttribute (m, SR830MENUS_FILE_SAVE, ATTR_CALLBACK_DATA, dev);
    SetMenuBarAttribute (m, SR830MENUS_FILE_LOAD, ATTR_CALLBACK_DATA, dev);
    SetMenuBarAttribute (m, SR830MENUS_FILE_GPIB, ATTR_CALLBACK_DATA, dev);

    SetMenuBarAttribute (m, SR830MENUS_SOURCES_DAC1, ATTR_CALLBACK_DATA, lia->sources[0]);
    SetMenuBarAttribute (m, SR830MENUS_SOURCES_DAC2, ATTR_CALLBACK_DATA, lia->sources[1]);
    SetMenuBarAttribute (m, SR830MENUS_SOURCES_DAC3, ATTR_CALLBACK_DATA, lia->sources[2]);
    SetMenuBarAttribute (m, SR830MENUS_SOURCES_DAC4, ATTR_CALLBACK_DATA, lia->sources[3]);
	SetMenuBarAttribute (m, SR830MENUS_SOURCES_FREQ, ATTR_CALLBACK_DATA, lia->sources[4]); 

	
    SetMenuBarAttribute (m, SR830MENUS_MEASURE_LIA, ATTR_CALLBACK_DATA, lia);
    SetMenuBarAttribute (m, SR830MENUS_MEASURE_DACS, ATTR_CALLBACK_DATA, lia);
    SetMenuBarAttribute (m, SR830MENUS_MEASURE_ADCS, ATTR_CALLBACK_DATA, dev);

    SetPanelAttribute (p, ATTR_CALLBACK_DATA, dev);

    SetCtrlAttribute (p, SR830_CTRL_REF, ATTR_CALLBACK_DATA, dev);
    SetCtrlAttribute (p, SR830_CTRL_TC, ATTR_CALLBACK_DATA, dev);
    SetCtrlAttribute (p, SR830_CTRL_SENS, ATTR_CALLBACK_DATA, dev);
    SetCtrlAttribute (p, SR830_CTRL_AUTOSENSE, ATTR_CALLBACK_DATA, dev);
    SetCtrlAttribute (p, SR830_CTRL_FILTSLOPE, ATTR_CALLBACK_DATA, dev);
    SetCtrlAttribute (p, SR830_CTRL_HARMON, ATTR_CALLBACK_DATA, dev);
    SetCtrlAttribute (p, SR830_CTRL_HARMVAL, ATTR_CALLBACK_DATA, dev);  
    SetCtrlAttribute (p, SR830_CTRL_SYNC, ATTR_CALLBACK_DATA, dev);
    SetCtrlAttribute (p, SR830_CTRL_DYNRES, ATTR_CALLBACK_DATA, dev);
    SetCtrlAttribute (p, SR830_CTRL_REJECT, ATTR_CALLBACK_DATA, dev);

    lia->inputstatus = sr830_GetInputStatus(dev);
    sr830_UpdateControls (p, dev);

    devPanel_Add (p, dev, sr830_UpdateReadings);
    DisplayPanel (p);
}

void sr830_UpdateControls (int panel, gpibioPtr dev)
{
    char msg[260];
    int i = 0, ktest;
    double r = 0;
    sr830Ptr lia;

    lia = dev->device;
   /* printf(" Time constant is %d\n", sr830_GetTimeConstant(dev));*/
    SetCtrlIndex (panel, SR830_CTRL_TC, sr830_GetTimeConstant(dev));

    SetCtrlIndex (panel, SR830_CTRL_SENS, sr830_GetSensitivity(dev));
    
	/*printf(" Slope is %d\n", sr830_GetFilterSlope (dev)); */
    SetCtrlIndex (panel, SR830_CTRL_FILTSLOPE, sr830_GetFilterSlope (dev));

    SetCtrlVal (panel, SR830_CTRL_SYNC, sr830_GetSync(dev));

    SetCtrlIndex (panel, SR830_CTRL_DYNRES, sr830_GetDynReserve (dev));

    SetCtrlIndex (panel, SR830_CTRL_REJECT, sr830_GetLineReject(dev));
    SetCtrlVal (panel, SR830_CTRL_AUTOSENSE, lia->autosens); 
}

void sr830_UpdateReadings (int panel, void *dev)
{
    gpibioPtr my_dev = dev;
    sr830Ptr lia = my_dev->device;
    int m;

    if (!util_TakingData() ||
        !(lia->channels[X]->acquire ||
          lia->channels[Y]->acquire ||
          lia->channels[M]->acquire ||
          lia->channels[P]->acquire)) sr830_GetXYMP (my_dev);

    if (expG.acqstatus != utilG.acq.status) {
        m = GetPanelMenuBar (panel);
        SetMenuBarAttribute (m, SR830MENUS_MEASURE, ATTR_DIMMED, util_TakingData());
        SetMenuBarAttribute (m, SR830MENUS_FILE_LOAD, ATTR_DIMMED, util_TakingData());
    }

    if (lia->autosens) SetCtrlIndex (panel, SR830_CTRL_SENS, sr830_GetSensitivity (dev));

    SetCtrlVal (panel, SR830_CTRL_XDISP, lia->channels[X]->reading);
    SetCtrlVal (panel, SR830_CTRL_YDISP, lia->channels[Y]->reading);
    SetCtrlVal (panel, SR830_CTRL_MAG, lia->channels[M]->reading);
    SetCtrlVal (panel, SR830_CTRL_PHASE, lia->channels[P]->reading);

    SetCtrlVal (panel, SR830_CTRL_INPUTOVERLOAD, lia->overload.reserve);
    SetCtrlVal (panel, SR830_CTRL_FILTEROVERLOAD, lia->overload.filter);
    SetCtrlVal (panel, SR830_CTRL_OUTPUTOVERLOAD, lia->overload.output);
}

void sr830_GetDAC (gpibioPtr dev, int i, sourcePtr src)
{
    char cmd[256], rsp[256];

    Fmt (cmd, "AUXV?%i", i);
    sr830_Out (dev, cmd);
    sr830_In (dev, rsp);
    Scan (rsp, "%s>%f", &src->acqchan->reading);
    src->acqchan->newreading = TRUE;
}

void sr830_SetDAC (gpibioPtr dev, int i, sourcePtr src)
{
    char cmd[256];
    double dly;

    Fmt (cmd, "AUXV%i,%f", i, src->biaslevel);
    sr830_Out (dev, cmd);
    util_Delay (src->segments[src->seg]->delay);
}

void sr830_GetADCs (gpibioPtr dev)
{
    char msg[256];
    sr830Ptr lia;
    sr830_channels chan;

    sr830_Out (dev, "SNAP?5,6,7,8");
    sr830_In (dev, msg);
    lia = dev->device;
    Scan (msg, "%s>%f,%f,%f,%f",
          &lia->channels[ADC1]->reading,
          &lia->channels[ADC2]->reading,
          &lia->channels[ADC3]->reading,
          &lia->channels[ADC4]->reading);

    for (chan = ADC1; chan <= ADC4; chan++) lia->channels[chan]->newreading = TRUE;
}

void sr830_GetXYMP (gpibioPtr dev)
{
    double hi, lo;
    char msg[256];
    sr830Ptr lia = dev->device;
    int sens;
    sr830_channels chan;

    sr830_Out (dev, "SNAP?1,2,3,4");
    sr830_In (dev, msg);
    Scan (msg, "%s>%f,%f,%f,%f",
          &lia->channels[X]->reading,
          &lia->channels[Y]->reading,
          &lia->channels[M]->reading,
          &lia->channels[P]->reading);

    for (chan = X; chan <= P; chan++) lia->channels[chan]->newreading = TRUE;

    if (lia->autosens) {
        hi = sr830_Conv2Sensitivity (lia->sens, lia->inputstatus);
        lo = sr830_Conv2Sensitivity (lia->sens-1, lia->inputstatus);

        sens = lia->sens;
        if (lia->channels[M]->reading > hi) lia->sens++;
        else { if (lia->channels[M]->reading < lo) lia->sens--;}
        if (sens != lia->sens) {
            Fmt (msg, "SENS%i", lia->sens);
            sr830_Out (dev, msg);
        }
    }
}

double sr830_Conv2Sensitivity (int i, int inputstatus)
{
    double sensi, d1, d2;
    char s[80];

    d1=TruncateRealNumber((double)i/3);
    sensi = pow (10, d1-9);
    d2=(double)i-(d1*3);
    if (d2 == 0.0) sensi*=2;
    else {if (d2 == 1.0) sensi*=5;
    else {if (d2 == 2.0) sensi*=10;}}
    if ((inputstatus == 2) || (inputstatus == 3))
        sensi*=1e-6;
    return sensi;
}

double sr830_GetRefAmpl (gpibioPtr dev)
{
    char msg[256];
    double amp;
    sr830_Out (dev, "SLVL?");
    sr830_In (dev, msg);
    Scan (msg, "%s>%f", &amp);
    return amp;
}

double sr830_GetRefFreq (gpibioPtr dev)
{
    char msg[256];
    double freq;
    sr830_Out (dev, "FREQ?");
    sr830_In (dev, msg);
    Scan (msg, "%s>%f", &freq);
    return freq;
}

double sr830_GetRefPhase (gpibioPtr dev)
{
    char msg[256];
    double phase;
    sr830_Out (dev, "PHAS?");
    sr830_In (dev, msg);
    Scan (msg, "%s>%f", &phase);
    return phase;
}

int sr830_GetInputStatus (gpibioPtr dev)
{
    char msg[256];
    int i;
    sr830_Out (dev, "ISRC?");
    sr830_In (dev, msg);
    Scan (msg, "%s>%i", &i);
    return i;
}

int sr830_GetLineReject (gpibioPtr dev)
{
    char msg[256];
    int i;
    sr830_Out (dev, "ILIN?");
    sr830_In (dev, msg);
    Scan (msg, "%s>%i", &i);
    return i;
}

int sr830_GetDynReserve (gpibioPtr dev)
{
    char msg[256];
    int i;
    sr830_Out (dev, "RMOD?");
    sr830_In (dev, msg);
    Scan (msg, "%s>%i", &i);
    return i;
}

int sr830_GetSync (gpibioPtr dev)
{
    char msg[256];
    int i;
    sr830_Out (dev, "SYNC?");
    sr830_In (dev, msg);
    Scan (msg, "%s>%i", &i);
    return i;
}

int sr830_GetFilterSlope (gpibioPtr dev)
{
    int i;
    char msg[260];
    sr830_Out (dev, "OFSL?");
    sr830_In (dev, msg);
    Scan (msg, "%s>%i", &i);
    /*printf( "slope %d\n",i);*/
    return i;
}

int sr830_GetTimeConstant (gpibioPtr dev)
{
    int i;
    char msg[260];
    sr830_Out (dev, "OFLT?");
    sr830_In (dev, msg);
    Scan (msg, "%s>%i", &i);
    /*printf( "Time constant %d\n",i);*/
    return i;
}

int sr830_GetSensitivity (gpibioPtr dev)
{
    char msg[260];
    int i;
    sr830Ptr lia;

    sr830_Out (dev, "SENS?");
    sr830_In (dev, msg);
    Scan (msg, "%s>%i", &i);
    lia = dev->device;
    lia->sens = i;
   /* printf( "sensitivity %d\n",i);*/ 
    return i;
    
}

void sr830_Out (gpibioPtr dev, char *cmd )
{
    gpibio_Out (dev, cmd);
}

void sr830_In (gpibioPtr dev, char *msg)
{
    gpibio_In (dev, msg);
    sr830_CheckSRQ(dev);
}

void sr830_CheckSRQ (gpibioPtr dev)
{
	short statusbyte;
    char msg[256];
    sr830Ptr lia;

    if (gpibio_SRQ(dev)) {
        gpibio_GetStatusByte (dev, &statusbyte);

        if (statusbyte & SR830_SRE_LIA)
            sr830_GetLIAStatusByte(dev);
        if (statusbyte & SR830_SRE_ERR)
            sr830_GetErrorStatusByte(dev);
        if (statusbyte & SR830_SRE_ESB)
            sr830_GetStandardStatusByte(dev);
        if (statusbyte & SR830_SRE_MAV)
            dev_StoreCommand (dev, "status byte MAV: info waiting @ output");
    }
    else {
        lia = dev->device;
        lia->overload.reserve = FALSE;
        lia->overload.filter = FALSE;
        lia->overload.output = FALSE;
    }
}

void sr830_GetLIAStatusByte (gpibioPtr dev)
{
    char msg[256];
    int statusbyte;
    sr830Ptr lia;

    gpibio_Out (dev, "LIAS?");
    gpibio_In (dev, msg);
    Scan (msg, "%s>%i", &statusbyte);

    lia = dev->device;
    lia->overload.reserve = ((statusbyte & SR830_LIA_RESRV) == SR830_LIA_RESRV);
    lia->overload.filter = ((statusbyte & SR830_LIA_FILTR) == SR830_LIA_FILTR);
    lia->overload.output = ((statusbyte & SR830_LIA_OUTPT) == SR830_LIA_OUTPT);
}

void sr830_GetStandardStatusByte (gpibioPtr dev)
{
    char msg[256];
    int statusbyte;

    statusbyte = 0;
    gpibio_Out (dev, "*ESR?");
    gpibio_In (dev, msg);
    Scan (msg, "%s>%i", &statusbyte);
    if (statusbyte & SR830_ESE_EXE)
        dev_StoreCommand (dev, "parameter out of range");
    if (statusbyte & SR830_ESE_CMD)
        dev_StoreCommand (dev, "illegal command received");
    if (statusbyte & SR830_ESE_PON)
        dev_StoreCommand (dev, "power cycled off and on");
/*  OperateDevCallback (0, 0, dev, 0); */
}

void sr830_GetErrorStatusByte (gpibioPtr dev)
{
    char msg[256];
    int statusbyte;

    statusbyte = 0;
    gpibio_Out (dev, "ERRS?");
    gpibio_In (dev, msg);
    Scan (msg, "%s>%i", &statusbyte);
    if (statusbyte & SR830_ERR_BACKUP)
        Fmt (msg, "ERROR: Battery backup has failed");
    if (statusbyte & SR830_ERR_RAM)
        Fmt (msg, "ERROR: RAM error has occured");
    if (statusbyte & SR830_ERR_ROM)
        Fmt (msg, "ERROR: ROM error has occured");
    if (statusbyte & SR830_ERR_GPIB)
        Fmt (msg, "ERROR: GPIB fast data transfer mode aborted");
    if (statusbyte & SR830_ERR_DSP)
        Fmt (msg, "ERROR: DSP error");
    if (statusbyte & SR830_ERR_MATH)
        Fmt (msg, "ERROR: internal math error");
    MessagePopup ("Stanford SR830 Error Status Message", msg);
}


