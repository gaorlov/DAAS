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
#include "sr844.h"
#include "sr844u.h"

#define SR844_ID "Stanford_Research_Systems,SR844"

/* serial poll status byte */
#define SR844_SRE_SCN   1   /* no scan in progress */
#define SR844_SRE_IFC   2   /* no command execution in progress */
#define SR844_SRE_ERR   4   /* bit changed in error status byte */
#define SR844_SRE_LIA   8   /* bit changed in LIA status byte */
#define SR844_SRE_MAV   16  /* interface output buffer is nonempty */
#define SR844_SRE_ESB   32  /* bit changed in standard status byte */
#define SR844_SRE_SRQ   64  /* SRQ has occurred */

/* standard event */
#define SR844_ESE_INP   1   /* input queue overflow */
#define SR844_ESE_QRY   4   /* output queue overflow */
#define SR844_ESE_EXE   16  /* bad parameter */
#define SR844_ESE_CMD   32  /* bad command */
#define SR844_ESE_URQ   64  /* key pressed */
#define SR844_ESE_PON   128 /* power on */

/* LIA */
#define SR844_LIA_RESRV 1   /* input/reserve overload */
#define SR844_LIA_FILTR 2   /* time constant fileter overload */
#define SR844_LIA_OUTPT 4   /* output overload detected */
#define SR844_LIA_UNLK  8   /* reference unlock */
#define SR844_LIA_RANGE 16  /* detection frequency changes ranges */
#define SR844_LIA_TC    32  /* time constant changed */
#define SR844_LIA_TRIG  64  /* data storage triggered */

/* error */
#define SR844_ERR_BACKUP 2  /* battery backup failed */
#define SR844_ERR_RAM    4  /* RAM memory error */
#define SR844_ERR_ROM   16  /* ROM memory error */
#define SR844_ERR_GPIB  32  /* GPIB Error */
#define SR844_ERR_DSP   64  /* DSP error */
#define SR844_ERR_MATH  128 /* math error */

#define SR844MAXINSTRS 2

#define TRUE 1
#define FALSE 0

typedef enum {X, Y, R, T, P, ADC1, ADC2} sr844_channels;
typedef struct
{
    acqchanPtr  channels[7];
    sourcePtr   sources[3];
    int         sens, autosens, id;
}   sr844Type;

typedef sr844Type *sr844Ptr;
 




double sr844_GetDoubleVal(gpibioPtr dev, char *msg);
void sr844_GetCharVal(gpibioPtr dev, char *msg, char *cmd);
int sr844_GetIntVal(gpibioPtr dev, char *msg);
void sr844_CheckClear(gpibioPtr dev);
int  OperateReferenceCallback844(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  SR844RefCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);			
int  SR844ControlPanelCallback(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  SR844ControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  ADCControlCallback844(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  XYMPControlCallback844(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
static void sr844_Remove (void *ptr);
static int sr844_InitGPIB (gpibioPtr dev);
double sr844_Conv2Sensitivity (int i);
void sr844_GetXYRTP(gpibioPtr dev);
void sr844_UpdateReadings(int panel, void *dev);
void sr844_UpdateControls(int panel, gpibioPtr dev);
void OperateSR844 (int menubar, int menuItem, void *callbackData, int panel);
void sr844_GetLIAReadings (acqchanPtr acqchan);
void sr844_XYRP_UpdateReadings (int panel, void *lia);
void sr844_GetADCs (gpibioPtr dev);
void sr844_ADC_UpdateReadings (int panel, void *dev);
void sr844_GetDAC (gpibioPtr dev, int i, sourcePtr src);
void sr844_SetDAC (gpibioPtr dev, int i, sourcePtr src);
void sr844_SetFREQ (sourcePtr src);
void sr844_GetFREQ (acqchanPtr acqchan);
void sr844_GetDAC1 (acqchanPtr acqchan);
void sr844_GetDAC2 (acqchanPtr acqchan);
void sr844_SetDAC1 (sourcePtr src);
void sr844_SetDAC2 (sourcePtr src);
void sr844_GetADCReading (acqchanPtr acqchan);
static void *sr844_Create (gpibioPtr dev);
void sr844_Save (gpibioPtr dev);
void sr844_Load (gpibioPtr dev);
void sr844_Init (void);
void CVICALLBACK sr844_MenuCallback (int menuBar, int menuItem, void *callbackData, int panel);
/*************************************************************/


double sr844_GetDoubleVal(gpibioPtr dev, char *msg)
{
    char cmd[256];
	double freq;
    sr844_CheckClear(dev);
	gpibio_Out (dev, msg);
    gpibio_In (dev, cmd);
    Scan (cmd, "%s>%f", &freq);
    return freq;
}
void sr844_GetCharVal(gpibioPtr dev, char *msg, char *cmd)
{
	sr844_CheckClear(dev);
	gpibio_Out (dev, msg);
    gpibio_In (dev, cmd);
}
int sr844_GetIntVal(gpibioPtr dev, char *msg)
{
	char cmd[256];
	int freq;
    gpibio_Out (dev, msg);
    gpibio_In (dev, cmd);
    Scan (cmd, "%s>%i", &freq);
    return freq;
}
void sr844_CheckClear(gpibioPtr dev)
{
	int i = 0;
	while(!i)
	{
		if(i != 2 && i != 5 && i != 6)
			i = sr844_GetIntVal(dev, "*STB?");
	}
		
}
int  OperateReferenceCallback844(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int p;
    gpibioPtr dev = callbackData;
    if (event == EVENT_COMMIT) {
        p = LoadPanel (0, "sr844u.uir", SR844_REF);
        
        SetPanelPos (p, VAL_AUTO_CENTER, VAL_AUTO_CENTER);

        util_InitClose (p, SR844_REF_CLOSE, FALSE);
		
        SetCtrlVal (p, SR844_REF_FREQ, sr844_GetDoubleVal(dev, "FREQ?"));
		sr844_CheckClear(dev);
        SetCtrlVal (p, SR844_REF_PHASE, sr844_GetDoubleVal(dev, "PHAS?"));

        SetCtrlAttribute (p, SR844_REF_FREQ, ATTR_CALLBACK_DATA, dev);
        SetCtrlAttribute (p, SR844_REF_PHASE, ATTR_CALLBACK_DATA, dev);
        InstallPopup (p);
    }
    return 0;
}

int  SR844RefCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    gpibioPtr dev = callbackData;
    double r;
    char cmd[256];

    switch (control) {
        case SR844_REF_FREQ:
            if (event == EVENT_VAL_CHANGED) {
                GetCtrlVal (panel, control, &r);
                Fmt (cmd, "FREQ%f", r);
                gpibio_Out (dev, cmd);
                r = sr844_GetDoubleVal (dev, "FREQ?");
                SetCtrlVal (panel, control, r);
                SetCtrlAttribute (panel, control, ATTR_INCR_VALUE, r/10);
            }
            break;
        case SR844_REF_PHASE:
            if (event == EVENT_VAL_CHANGED) {
                GetCtrlVal (panel, control, &r);
                Fmt (cmd, "PHAS%f", r);
                gpibio_Out (dev, cmd);
                SetCtrlVal (panel, control, sr844_GetDoubleVal(dev, "PHAS?"));
            }
            break;
    }
    return 0;
}
int  SR844ControlPanelCallback(int panel, int event, void *callbackData, int eventData1, int eventData2)
{
    gpibioPtr dev;
    sr844Ptr lia;
    if (((event == EVENT_KEYPRESS) && (eventData1 == VAL_ESC_VKEY)) || (event == EVENT_RIGHT_DOUBLE_CLICK))
	{
		dev = callbackData;
        lia = dev->device;
        devPanel_Remove (panel);
        DiscardPanel (panel);
		dev->iPanel = 0;
		SetMenuBarAttribute (acquire_GetMenuBar(), dev->menuitem_id, ATTR_DIMMED, FALSE);
    }
    return 0;
}
int  SR844ControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int i;
    char cmd[256], rsp[256];
    gpibioPtr dev;
    sr844Ptr lia;

    dev = callbackData;
    switch (control) {
        case SR844_CTRL_TC:
            if (event == EVENT_COMMIT) {
				GetCtrlVal (panel, control, &i);
                Fmt (cmd, "OFLT%i", i);
                sr844_CheckClear(dev);
				gpibio_Out (dev, cmd);
            }
            break;
        case SR844_CTRL_SENS:
            if (event == EVENT_COMMIT) {
                GetCtrlVal (panel, control, &i);
				Fmt (cmd, "SENS%i", i);
                sr844_CheckClear(dev);
				gpibio_Out (dev, cmd);
            }
            break;
        case SR844_CTRL_AUTOSENSE:
            if (event == EVENT_COMMIT) {
				lia = dev->device;
                GetCtrlVal (panel, control, &lia->autosens);
                sr844_CheckClear(dev);
				if(lia->autosens)
				{
					GetCtrlVal (panel, SR844_CTRL_SENS, &i);
                	Fmt (cmd, "SENS%i", i);
					gpibio_Out (dev, cmd);
				}
            }
            break;
        case SR844_CTRL_FILTSLOPE:
            if (event == EVENT_COMMIT) {
                GetCtrlVal (panel, control, &i);
                Fmt (cmd, "OFSL%i", i);
                sr844_CheckClear(dev);
				gpibio_Out (dev, cmd);
            }
            break;
       case SR844_CTRL_1MO50O:
			if(event == EVENT_COMMIT)
			{
				GetCtrlVal (panel, control, &i);
				Fmt (cmd, "INPZ%i", i);
				sr844_CheckClear(dev);
				gpibio_Out (dev, cmd);
			}
			break;
    }
    return 0;
}
int  ADCControlCallback844(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    sr844Ptr lia;
    acqchanPtr acqchan;

    acqchan = callbackData;

    switch (control) {
        case SR844_ADC_NOTE_1:
        case SR844_ADC_NOTE_2:
            AcqDataNoteCallback (panel, control, event, callbackData, eventData1, eventData2);
            break;
        case SR844_ADC_ACQ_1:
        case SR844_ADC_ACQ_2:
            if (event == EVENT_VAL_CHANGED) {
                GetCtrlVal (panel, control, &acqchan->acquire);
                if (acqchan->acquire) acqchanlist_AddChannel (acqchan);
                    else acqchanlist_RemoveChannel (acqchan);
            }
            break;
        case SR844_ADC_CONVERSION_1:
        case SR844_ADC_CONVERSION_2:
            if (event == EVENT_VAL_CHANGED) {
                GetCtrlVal (panel, control, &acqchan->conversion);
                if (acqchan->p) SetCtrlVal (acqchan->p, ACQDATA_CONV, acqchan->conversion);
            }
            break;
        case SR844_ADC_COEFF_1:
        case SR844_ADC_COEFF_2:
            if (event == EVENT_COMMIT) {
                GetCtrlVal (panel, control, &acqchan->coeff);
                if (acqchan->p) SetCtrlVal (acqchan->p, ACQDATA_COEFF, acqchan->coeff);
            }
            break;
        case SR844_ADC_LABEL_1:
        case SR844_ADC_LABEL_2:
            if (event == EVENT_COMMIT) {
                GetCtrlVal (panel, control, acqchan->channel->label);
                acqchanlist_ReplaceChannel (acqchan);
                if (acqchan->p) SetPanelAttribute (acqchan->p, ATTR_TITLE, acqchan->channel->label);
            }
            break;
        case SR844_ADC_CLOSE:
            if (event == EVENT_COMMIT) {
                lia = callbackData;
                devPanel_Remove(panel);
                DiscardPanel (panel);
            }
            break;
    }
    return 0;
}
int  XYMPControlCallback844(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    sr844Ptr lia;
    acqchanPtr acqchan;

    acqchan = callbackData;

    switch (control) {
        case SR844_XYMP_NOTE_1:
        case SR844_XYMP_NOTE_2:
        case SR844_XYMP_NOTE_3:
        case SR844_XYMP_NOTE_4:
            AcqDataNoteCallback (panel, control, event, callbackData, eventData1, eventData2);
            break;
        case SR844_XYMP_XACQ:
        case SR844_XYMP_YACQ:
        case SR844_XYMP_RACQ:
        case SR844_XYMP_PACQ:
            if (event == EVENT_VAL_CHANGED) {
                GetCtrlVal (panel, control, &acqchan->acquire);
                if (acqchan->acquire) acqchanlist_AddChannel (acqchan);
                    else acqchanlist_RemoveChannel (acqchan);
            }
            break;
        case SR844_XYMP_XCOEFF:
        case SR844_XYMP_YCOEFF:
        case SR844_XYMP_RCOEFF:
        case SR844_XYMP_PCOEFF:
            if (event == EVENT_COMMIT) {
                GetCtrlVal (panel, control, &acqchan->coeff);
                if (acqchan->p) SetCtrlVal (acqchan->p, ACQDATA_COEFF, acqchan->coeff);
            }
            break;
        case SR844_XYMP_XLABEL:
        case SR844_XYMP_YLABEL:
        case SR844_XYMP_RLABEL:
        case SR844_XYMP_PLABEL:
            if (event == EVENT_COMMIT) {
                GetCtrlVal (panel, control, acqchan->channel->label);
                acqchanlist_ReplaceChannel (acqchan);
                if (acqchan->p) SetPanelAttribute (acqchan->p, ATTR_TITLE, acqchan->channel->label);
            }
            break;
        case SR844_XYMP_CLOSE:
            if (event == EVENT_COMMIT) {
                lia = callbackData;
                devPanel_Remove(panel);
                DiscardPanel (panel);
            }
            break;
    }
	updateGraphSource();
    return 0;
}

static void sr844_Remove (void *ptr)
{						
    sr844Ptr lia = ptr; 
    sr844_channels chan;
    int i;
	
    for (chan = X; chan <= ADC2; chan++) acqchan_Remove(lia->channels[chan]);
    for (i = 0; i < 3; i ++){
		source_Remove (lia->sources[i]);
	}
    free (lia);//*/
}

static int sr844_InitGPIB (gpibioPtr dev)
{
    int byte_register;
    char cmd[80], rsp[256];

    gpibio_Remote (dev);

    if (gpibio_DeviceMatch (dev, "*IDN?", SR844_ID)) {
        gpibio_Out (dev, "OUTX1");
        gpibio_Out (dev, "*CLS");

        byte_register = SR844_SRE_ERR+
                        SR844_SRE_LIA+
  /*                    SR844_SRE_MAV+        */
                        SR844_SRE_ESB;

        Fmt (cmd, "*SRE%i", byte_register);
        gpibio_Out (dev, cmd);

        byte_register = SR844_ESE_EXE+
                        SR844_ESE_CMD+
                        SR844_ESE_PON;
        Fmt (cmd, "*ESE%i", byte_register);
        gpibio_Out (dev, cmd);

        byte_register = SR844_LIA_RESRV+
                        SR844_LIA_FILTR+
                        SR844_LIA_OUTPT;
        Fmt (cmd, "LIAE%i", byte_register);
        gpibio_Out (dev, cmd);

        byte_register = SR844_ERR_BACKUP+
                        SR844_ERR_RAM+
                        SR844_ERR_ROM+
                        SR844_ERR_GPIB+
                        SR844_ERR_DSP+
                        SR844_ERR_MATH;

        Fmt (cmd, "ERRE%i", byte_register);
        gpibio_Out (dev, cmd);
        return TRUE;
    }
    return FALSE;
}
double sr844_Conv2Sensitivity (int i)
{
    double sensi, d1;
    char s[80];

	d1 = TruncateRealNumber((double)i / 2);
    sensi = pow (10, d1 - 7);
    if(i%2)
		sensi *= 3;
	return sensi;
}
void sr844_GetXYRTP(gpibioPtr dev)
{
	double hi, lo;
	int i = 0, sens, senso;
	char str[256];
	sr844Ptr lia;
	sr844_channels chan;
	lia = dev->device;
	sr844_GetCharVal (dev, "SNAP?1,2,3,5", str);
	Scan (str, "%s>%f,%f,%f,%f",
		&lia->channels[X]->reading,
		&lia->channels[Y]->reading,
		&lia->channels[R]->reading,
		&lia->channels[T]->reading);
	sr844_CheckClear(dev);
	lia->channels[P]->reading = sr844_GetDoubleVal (dev, "PHAS?");
	for (chan = X; chan <= P; chan++) lia->channels[chan]->newreading = TRUE;
	if(lia->autosens)
	{
		senso = lia->sens;
        hi = sr844_Conv2Sensitivity (lia->sens);
        lo = sr844_Conv2Sensitivity (lia->sens-1);
		if (lia->channels[R]->reading > hi) lia->sens++;
        else if (lia->channels[R]->reading < lo) lia->sens--;
		if(lia->sens < 0) lia->sens = 0;
		if(lia->sens >14) lia->sens = 14;
		Fmt (str, "SENS%i", lia->sens);
        if(senso != lia->sens)
			gpibio_Out (dev, str);
	}
}
void sr844_UpdateReadings(int panel, void *dev)
{
	gpibioPtr devT = dev;
	sr844Ptr lia = devT->device;
	GetCtrlVal(panel, SR844_CTRL_SENS, &lia->sens);
	sr844_GetXYRTP(devT);
	SetCtrlVal(panel, SR844_CTRL_SENS, lia->sens);
	SetCtrlVal(panel, SR844_CTRL_AUTOSENSE, lia->autosens);
	SetCtrlVal(panel, SR844_CTRL_XDISP, lia->channels[X]->reading);
	SetCtrlVal(panel, SR844_CTRL_YDISP, lia->channels[Y]->reading);
	SetCtrlVal(panel, SR844_CTRL_RV,    lia->channels[R]->reading);
	SetCtrlVal(panel, SR844_CTRL_THETA, lia->channels[T]->reading);
	SetCtrlVal(panel, SR844_CTRL_PHASE, lia->channels[P]->reading);
}
void sr844_UpdateControls(int panel, gpibioPtr dev)
{
	sr844_CheckClear(dev);
	SetCtrlIndex(panel, SR844_CTRL_TC, sr844_GetIntVal(dev, "OFLT?"));
	sr844_CheckClear(dev);
	SetCtrlIndex(panel, SR844_CTRL_SENS, sr844_GetIntVal(dev, "SENS?"));
	sr844_CheckClear(dev);
	SetCtrlVal(panel, SR844_CTRL_1MO50O, sr844_GetIntVal(dev, "INPZ?"));
	sr844_CheckClear(dev);
	SetCtrlVal(panel, SR844_CTRL_FILTSLOPE, sr844_GetIntVal(dev, "OFSL?"));
}
void OperateSR844 (int menubar, int menuItem, void *callbackData, int panel)
{
    int p, m;
    gpibioPtr dev = callbackData;
    sr844Ptr lia = dev->device;
    char label[256];

    SetMenuBarAttribute (menubar, menuItem, ATTR_DIMMED, TRUE);

	p = dev->iPanel? dev->iPanel: LoadPanel (utilG.p, "sr844u.uir", SR844_CTRL);
    dev->iPanel = p;

	SetPanelPos (p, VAL_AUTO_CENTER, VAL_AUTO_CENTER);

    Fmt (label, "stanford research systems sr844 dsp lock-in amplifier: %s", dev->label);
    SetPanelAttribute (p, ATTR_TITLE, label);
	
	m = LoadMenuBar (p, "sr844u.uir", SR844MENU);
    SetPanelMenuBar (p, m);

    SetMenuBarAttribute (m, SR844MENU_SOURCES_DAC1, ATTR_CALLBACK_DATA, lia->sources[0]);
    SetMenuBarAttribute (m, SR844MENU_SOURCES_DAC2, ATTR_CALLBACK_DATA, lia->sources[1]);
    SetMenuBarAttribute (m, SR844MENU_SOURCES_FREQ, ATTR_CALLBACK_DATA, lia->sources[2]);
    SetMenuBarAttribute (m, SR844MENU_MEASURE_LIA, ATTR_CALLBACK_DATA, lia);
    SetMenuBarAttribute (m, SR844MENU_MEASURE_ADCS, ATTR_CALLBACK_DATA, dev);

	SetPanelAttribute (p, ATTR_CALLBACK_DATA, dev);

    SetCtrlAttribute (p, SR844_CTRL_REF, ATTR_CALLBACK_DATA, dev);
    SetCtrlAttribute (p, SR844_CTRL_TC, ATTR_CALLBACK_DATA, dev);
    SetCtrlAttribute (p, SR844_CTRL_SENS, ATTR_CALLBACK_DATA, dev);
    SetCtrlAttribute (p, SR844_CTRL_AUTOSENSE, ATTR_CALLBACK_DATA, dev);
    SetCtrlAttribute (p, SR844_CTRL_FILTSLOPE, ATTR_CALLBACK_DATA, dev);
	SetCtrlAttribute (p, SR844_CTRL_1MO50O, ATTR_CALLBACK_DATA, dev);
	SetCtrlVal(p, SR844_CTRL_GPIBADDR, dev->paddr);
    sr844_UpdateControls (p, dev);		   ///*/

    devPanel_Add (p, dev, sr844_UpdateReadings);//*/
    DisplayPanel (p);
}
void sr844_GetLIAReadings (acqchanPtr acqchan)
{
    gpibioPtr dev = acqchan->dev;
    sr844Ptr lia = dev->device;

    sr844_GetXYRTP (dev);
} 
void sr844_XYRP_UpdateReadings (int panel, void *lia)
{
    sr844Ptr my_lia = lia;

    SetCtrlVal (panel, SR844_XYMP_XMEAS, my_lia->channels[X]->reading);
    SetCtrlVal (panel, SR844_XYMP_YMEAS, my_lia->channels[Y]->reading);
    SetCtrlVal (panel, SR844_XYMP_RMEAS, my_lia->channels[R]->reading);
    SetCtrlVal (panel, SR844_XYMP_PMEAS, my_lia->channels[T]->reading);
	if(utilG.acq.status == ACQ_BUSY)
		HidePanel(panel);
}
void sr844_GetADCs (gpibioPtr dev)
{
    char msg[256];
    sr844Ptr lia;
    sr844_channels chan;

    sr844_GetCharVal(dev, "SNAP?6,7", msg);
    lia = dev->device;
    Scan (msg, "%s>%f,%f",
          &lia->channels[ADC1]->reading,
          &lia->channels[ADC2]->reading);

    for (chan = ADC1; chan <= ADC2; chan++) lia->channels[chan]->newreading = TRUE;
}
void sr844_ADC_UpdateReadings (int panel, void *dev)
{
    gpibioPtr my_dev = dev;
    sr844Ptr lia = my_dev->device;

    sr844_GetADCs (my_dev);

    SetCtrlVal (panel, SR844_ADC_MEAS_1, lia->channels[ADC1]->reading);
    SetCtrlVal (panel, SR844_ADC_MEAS_2, lia->channels[ADC2]->reading);
	if(utilG.acq.status == ACQ_BUSY)
		HidePanel(panel);
}
void sr844_GetDAC (gpibioPtr dev, int i, sourcePtr src)
{
    char cmd[256], rsp[256];

    Fmt (cmd, "AUXO?%i", i);
    sr844_GetCharVal(dev, cmd, rsp);
    Scan (rsp, "%s>%f", &src->acqchan->reading);
    src->acqchan->newreading = TRUE;
}
void sr844_SetDAC (gpibioPtr dev, int i, sourcePtr src)
{
    char cmd[256];
    double dly;

    Fmt (cmd, "AUXO%i,%f", i, src->biaslevel);
    gpibio_Out (dev, cmd);
    util_Delay (src->segments[src->seg]->delay);
}
void sr844_SetFREQ (sourcePtr src)
{
	gpibioPtr dev = src->acqchan->dev;
    char cmd[256];
    double dly;
    Fmt (cmd, "FREQ%f", src->biaslevel);
    gpibio_Out (dev, cmd);
    util_Delay (src->segments[src->seg]->delay);
}
void sr844_GetFREQ (acqchanPtr acqchan)
{
    gpibioPtr dev = acqchan->dev;
    sr844Ptr lia = dev->device;
    sourcePtr src = lia->sources[2];
    char cmd[256], rsp[256];
    src->acqchan->reading = sr844_GetDoubleVal(dev, "FREQ?");
    src->acqchan->newreading = TRUE;
}

void sr844_GetDAC1 (acqchanPtr acqchan)
{
    gpibioPtr dev = acqchan->dev;
    sr844Ptr lia = dev->device;
    sourcePtr src = lia->sources[0];
    sr844_GetDAC (dev, 1, src);
}
void sr844_GetDAC2 (acqchanPtr acqchan)
{
    gpibioPtr dev = acqchan->dev;
    sr844Ptr lia = dev->device;
    sourcePtr src = lia->sources[1];
    sr844_GetDAC (dev, 2, src);
}

void sr844_SetDAC1 (sourcePtr src)
{
    gpibioPtr dev = src->acqchan->dev;
    sr844_SetDAC (dev, 1, src);
}
void sr844_SetDAC2 (sourcePtr src)
{
    gpibioPtr dev = src->acqchan->dev;
    sr844_SetDAC (dev, 2, src);
}
void sr844_GetADCReading (acqchanPtr acqchan)
{
    gpibioPtr dev = acqchan->dev;
    sr844Ptr lia = dev->device;

    sr844_GetADCs (dev);
}
static void *sr844_Create (gpibioPtr dev)
{
    sr844Ptr lia;

    lia = malloc (sizeof(sr844Type));
	lia->autosens = FALSE;
    if (dev) lia->id = dev->id;
    
	lia->channels[X] = acqchan_Create ("SR844 X", dev, sr844_GetLIAReadings);
    lia->channels[Y] = acqchan_Create ("SR844 Y", dev, sr844_GetLIAReadings);
    lia->channels[R] = acqchan_Create ("SR844 R", dev, sr844_GetLIAReadings);
    lia->channels[T] = acqchan_Create ("SR844 T", dev, sr844_GetLIAReadings);
	lia->channels[P] = acqchan_Create ("SR844 P", dev, sr844_GetLIAReadings);

    lia->channels[ADC1] = acqchan_Create ("SR844 ADC1", dev, sr844_GetADCReading);
    lia->channels[ADC2] = acqchan_Create ("SR844 ADC2", dev, sr844_GetADCReading);
    
    lia->sources[0] =
		source_Create ("SR844 DAC1", dev, sr844_SetDAC1, sr844_GetDAC1);
	
    lia->sources[1] =
		source_Create ("SR844 DAC2", dev, sr844_SetDAC2, sr844_GetDAC2);
	lia->sources[2] =
		source_Create ("SR844 Freq", dev, sr844_SetFREQ, sr844_GetFREQ);
	lia->sources[2]->freq = 1;
//*/    
	if (dev) dev->device = lia;
    return lia;
}
void sr844_Save (gpibioPtr dev)
{
    sr844_channels chan;
    sr844Ptr lia = dev->device;
	FmtFile (fileHandle.analysis, "%s<Ref Freq     : %f\n", sr844_GetDoubleVal(dev, "FREQ?"));
    FmtFile (fileHandle.analysis, "%s<Time Const   : %i\n", sr844_GetIntVal(dev, "OFLT?"));
    FmtFile (fileHandle.analysis, "%s<Filter Slp   : %i\n", sr844_GetIntVal(dev, "OFSL?"));
    FmtFile (fileHandle.analysis, "%s<Sensitivity  : %i\n", sr844_GetIntVal(dev, "SENS?"));
    FmtFile (fileHandle.analysis, "%s<Signal In Imp: %i\n", sr844_GetIntVal(dev, "INPZ?"));

    for (chan = X; chan <= ADC2; chan++) acqchan_Save (lia->channels[chan]);

    source_Save (lia->sources[0]);
    source_Save (lia->sources[1]);
	source_Save (lia->sources[2]);
}
void sr844_Load (gpibioPtr dev)
{
    char cmd[256];
    double r;
    int i;
    sr844_channels chan;
    sr844Ptr lia;

    if (dev)
	{
		lia = dev->device;
    	ScanFile (fileHandle.analysis, "%s>Ref Freq     : %f[x]", &r);
    	Fmt (cmd, "FREQ%f", r); gpibio_Out (dev, cmd);
		ScanFile (fileHandle.analysis, "%s>Time Const   : %i[x]", &i);
    	Fmt (cmd, "OFLT%i", i); gpibio_Out (dev, cmd);
    	
		ScanFile (fileHandle.analysis, "%s>Filter Slp   : %i[x]", &i);
		Fmt (cmd, "OFSL%i", i); gpibio_Out (dev, cmd);
		
		ScanFile (fileHandle.analysis, "%s>Sensitivity  : %i[x]", &i);
    	Fmt (cmd, "SENS%i", i); gpibio_Out (dev, cmd);
		
		ScanFile (fileHandle.analysis, "%s>Signal In Imp: %i[x]", &i);
    	Fmt (cmd, "INPZ%i", i); gpibio_Out (dev, cmd);
		
		for (chan = X; chan <= ADC2; chan++) acqchan_Load (dev, lia->channels[chan]);
		for (i = 0; i < 2; i++) {
        	lia->sources[i]->min = -10.5;
        	lia->sources[i]->max = 10.5;
    	}
		source_Load (dev, lia->sources[0]);
    	source_Load (dev, lia->sources[1]);
		source_Load (dev, lia->sources[2]);
		lia->sources[2]->min = 2.5E4;
        lia->sources[2]->max = 2.0E8;
	}
}
void sr844_Init (void)
{
    devTypePtr devType;
    if (utilG.acq.status != ACQ_NONE) {
        util_ChangeInitMessage ("stanford sr844 control utilities...");
        devType = malloc (sizeof (devTypeItem));
        if (devType) {
            Fmt (devType->label, "stanford research sr844");
            Fmt (devType->id, SR844_ID);
            devType->CreateDevice = sr844_Create;
            devType->InitDevice = sr844_InitGPIB;
            devType->OperateDevice = OperateSR844;
            devType->UpdateReadings = sr844_UpdateReadings;
            devType->SaveDevice = sr844_Save;
            devType->LoadDevice = sr844_Load;
            devType->RemoveDevice = sr844_Remove;
            devTypeList_AddItem (devType);
        }
    }
}

void CVICALLBACK sr844_MenuCallback (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	switch(menuItem)
	{
		case SR844MENU_MEASURE_LIA:
			{
				int p, busy;
    			sr844Ptr lia;

    			p = LoadPanel (utilG.p, "sr844u.uir", SR844_XYMP);
    			SetPanelPos (p, VAL_AUTO_CENTER, VAL_AUTO_CENTER);
    			lia = callbackData;
				
				SetCtrlVal (p, SR844_XYMP_XLABEL, lia->channels[X]->channel->label);
			    SetCtrlVal (p, SR844_XYMP_XCOEFF, lia->channels[X]->coeff);
			    SetCtrlVal (p, SR844_XYMP_XACQ,   lia->channels[X]->acquire);
			    SetCtrlVal (p, SR844_XYMP_NOTE_1, lia->channels[X]->note);
								  
				SetCtrlVal (p, SR844_XYMP_YLABEL, lia->channels[Y]->channel->label);
			    SetCtrlVal (p, SR844_XYMP_YCOEFF, lia->channels[Y]->coeff);
			    SetCtrlVal (p, SR844_XYMP_YACQ,   lia->channels[Y]->acquire);
			    SetCtrlVal (p, SR844_XYMP_NOTE_2, lia->channels[Y]->note);

			    SetCtrlVal (p, SR844_XYMP_RLABEL, lia->channels[R]->channel->label);
			    SetCtrlVal (p, SR844_XYMP_RCOEFF, lia->channels[R]->coeff);
			    SetCtrlVal (p, SR844_XYMP_RACQ,   lia->channels[R]->acquire);
			    SetCtrlVal (p, SR844_XYMP_NOTE_3, lia->channels[R]->note);

			    SetCtrlVal (p, SR844_XYMP_PLABEL, lia->channels[T]->channel->label);
			    SetCtrlVal (p, SR844_XYMP_PCOEFF, lia->channels[T]->coeff);
			    SetCtrlVal (p, SR844_XYMP_PACQ,   lia->channels[T]->acquire);
			    SetCtrlVal (p, SR844_XYMP_NOTE_4, lia->channels[T]->note);
	
			    SetCtrlAttribute(p, SR844_XYMP_CLOSE, ATTR_CALLBACK_DATA, lia);
		
			    SetCtrlAttribute(p, SR844_XYMP_XLABEL, ATTR_CALLBACK_DATA, lia->channels[X]);
			    SetCtrlAttribute(p, SR844_XYMP_XCOEFF, ATTR_CALLBACK_DATA, lia->channels[X]);
			    SetCtrlAttribute(p, SR844_XYMP_XACQ,   ATTR_CALLBACK_DATA, lia->channels[X]);
			    SetCtrlAttribute(p, SR844_XYMP_NOTE_1, ATTR_CALLBACK_DATA, lia->channels[X]);
	
			    SetCtrlAttribute(p, SR844_XYMP_YLABEL, ATTR_CALLBACK_DATA, lia->channels[Y]);
			    SetCtrlAttribute(p, SR844_XYMP_YCOEFF, ATTR_CALLBACK_DATA, lia->channels[Y]);
			    SetCtrlAttribute(p, SR844_XYMP_YACQ,   ATTR_CALLBACK_DATA, lia->channels[Y]);
			    SetCtrlAttribute(p, SR844_XYMP_NOTE_2, ATTR_CALLBACK_DATA, lia->channels[Y]);
		
			    SetCtrlAttribute(p, SR844_XYMP_RLABEL, ATTR_CALLBACK_DATA, lia->channels[R]);
			    SetCtrlAttribute(p, SR844_XYMP_RCOEFF, ATTR_CALLBACK_DATA, lia->channels[R]);
			    SetCtrlAttribute(p, SR844_XYMP_RACQ,   ATTR_CALLBACK_DATA, lia->channels[R]);
			    SetCtrlAttribute(p, SR844_XYMP_NOTE_3, ATTR_CALLBACK_DATA, lia->channels[R]);
	
			    SetCtrlAttribute(p, SR844_XYMP_PLABEL, ATTR_CALLBACK_DATA, lia->channels[T]);
			    SetCtrlAttribute(p, SR844_XYMP_PCOEFF, ATTR_CALLBACK_DATA, lia->channels[T]);
			    SetCtrlAttribute(p, SR844_XYMP_PACQ,   ATTR_CALLBACK_DATA, lia->channels[T]);
			    SetCtrlAttribute(p, SR844_XYMP_NOTE_4, ATTR_CALLBACK_DATA, lia->channels[T]);

		    	devPanel_Add (p, lia, sr844_XYRP_UpdateReadings);
    			DisplayPanel (p);
			}
			break;
		case SR844MENU_MEASURE_ADCS:
			{
				int p;
			    gpibioPtr dev = callbackData;
			    sr844Ptr lia = dev->device;
		
			    p = LoadPanel (utilG.p, "sr844u.uir", SR844_ADC);
			    SetPanelPos (p, VAL_AUTO_CENTER, VAL_AUTO_CENTER);
			    	
			    SetCtrlVal (p, SR844_ADC_LABEL_1, lia->channels[ADC1]->channel->label);
			    SetCtrlVal (p, SR844_ADC_COEFF_1, lia->channels[ADC1]->coeff);
			    SetCtrlVal (p, SR844_ADC_CONVERSION_1, lia->channels[ADC1]->conversion);
			    SetCtrlVal (p, SR844_ADC_ACQ_1, lia->channels[ADC1]->acquire);
			    SetCtrlVal (p, SR844_ADC_NOTE_1, lia->channels[ADC1]->note);
		
			    SetCtrlVal (p, SR844_ADC_LABEL_2, lia->channels[ADC2]->channel->label);
			    SetCtrlVal (p, SR844_ADC_COEFF_2, lia->channels[ADC2]->coeff);
			    SetCtrlVal (p, SR844_ADC_CONVERSION_2, lia->channels[ADC2]->conversion);
			    SetCtrlVal (p, SR844_ADC_ACQ_2, lia->channels[ADC2]->acquire);
			    SetCtrlVal (p, SR844_ADC_NOTE_2, lia->channels[ADC2]->note);
	
			    SetCtrlAttribute(p, SR844_ADC_CLOSE, ATTR_CALLBACK_DATA, lia);
	
			    SetCtrlAttribute(p, SR844_ADC_LABEL_1, ATTR_CALLBACK_DATA, lia->channels[ADC1]);
			    SetCtrlAttribute(p, SR844_ADC_COEFF_1, ATTR_CALLBACK_DATA, lia->channels[ADC1]);
			    SetCtrlAttribute(p, SR844_ADC_CONVERSION_1, ATTR_CALLBACK_DATA, lia->channels[ADC1]);
			    SetCtrlAttribute(p, SR844_ADC_ACQ_1, ATTR_CALLBACK_DATA, lia->channels[ADC1]);
				SetCtrlAttribute(p, SR844_ADC_NOTE_1, ATTR_CALLBACK_DATA, lia->channels[ADC1]);

		    	SetCtrlAttribute(p, SR844_ADC_LABEL_2, ATTR_CALLBACK_DATA, lia->channels[ADC2]);
			    SetCtrlAttribute(p, SR844_ADC_COEFF_2, ATTR_CALLBACK_DATA, lia->channels[ADC2]);
				SetCtrlAttribute(p, SR844_ADC_CONVERSION_2, ATTR_CALLBACK_DATA, lia->channels[ADC2]);
				SetCtrlAttribute(p, SR844_ADC_ACQ_2, ATTR_CALLBACK_DATA, lia->channels[ADC2]);
				SetCtrlAttribute(p, SR844_ADC_NOTE_2, ATTR_CALLBACK_DATA, lia->channels[ADC2]);
	
		    	devPanel_Add (p, dev, sr844_ADC_UpdateReadings);
    			DisplayPanel (p);
			}
			break;
		case SR844MENU_SOURCES_DAC1:
		case SR844MENU_SOURCES_DAC2:
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
		break;
		case SR844MENU_SOURCES_FREQ:
		{
			sourcePtr src;

    		src = callbackData;
			src->min = 2.5E4;
    		src->max = 2.0E8;
			switch (utilG.exp) {
        		case EXP_SOURCE: source_InitPanel (src); break;
        		case EXP_FLOAT: gensrc_InitPanel (src); break;
    		}
		}
		break; 
	}
}
