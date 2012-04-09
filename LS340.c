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
#include "LS340.h"
#include "LS340u.h"

#define TRUE 1
#define FALSE 0
#define LS340_ID "LSCI"

typedef enum {SORB, ONEK, HE3P} inputs;
typedef enum {KELVIN, CELSIUS, SEN_UNITS} units;
typedef struct{
	acqchanPtr channels[3];
	sourcePtr source;
	int id;
	struct {int loop, units, power, maxpower, powerup, on;
		char *input;
		double setplimit, pchange, nchange, current, setpoint, rampspeed;} heater;
	struct {double p, i, d; int pon, ion, don;} pid;
	struct {int on; double level;} alarm;
	struct {char *serial, *format; int source, target, file;} curveload;
} LS340Type;
	
typedef LS340Type *LS340Ptr;

/*******************************index*********************************/
void GetHeaterLvl(acqchanPtr acqchan);
void SetHeaterLvl(sourcePtr src);
void GetSensor(acqchanPtr acqchan);

void LS340_UpdateSensorReadings (int panel, void *ptr);
void LS340_UpdateHeaterSettings(int panel, gpibioPtr dev);
void LS340_UpdateControls(int p, gpibioPtr dev);
int  LS340PanelCallback (int panel, int event, void *callbackData, int eventData1, int eventData2);
int  LS340ControlCallback (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  LS340HeatControlCallback (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  LS340SensorControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  LS340SendCurve (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void LS340menuCallack (int menuBar, int menuItem, void *callbackData, int panel);

void *LS340_Create(gpibioPtr dev);
int  LS340_InitGPIB(gpibioPtr dev);
void OperateLS340(int menubar, int menuItem, void *callbackData, int panel);
void LS340_UpdateReadings(int panel, void *ptr);
void LS340_Save(gpibioPtr dev);
void LS340_Load(gpibioPtr dev);
void LS340_Remove(void *ptr);
void LS340_Init(void);


/*******************************Communication functions*********************************/

void GetHeaterLvl(acqchanPtr acqchan)
{
	gpibioPtr dev = acqchan->dev;
	LS340Ptr ls = dev->device;
	char msg[10];
	
	Fmt(msg, "SETP? %i\n", ls->heater.loop);
	acqchan->reading = gpib_GetDoubleVal(dev, msg);
	acqchan->newreading = TRUE;
}

void SetHeaterLvl(sourcePtr src)
{
	gpibioPtr dev = src->acqchan->dev;
	LS340Ptr ls = dev->device;
	char msg[30];
	gpibPrint(dev, "SETP %i, %f", &ls->heater.loop, &src->biaslevel);
	util_Delay(src->segments[src->seg]->delay);
}

void GetSensor(acqchanPtr acqchan)
{	  
	gpibioPtr dev = acqchan->dev;
	LS340Ptr ls = dev->device;
	int alarm = 0;
	char *sens_name, msg[10];
	if(!strcmp(acqchan->channel->label, "sorb")) sens_name = "A";
	if(!strcmp(acqchan->channel->label, "1 k pot")) {sens_name = "B"; alarm = 1;}
	if(!strcmp(acqchan->channel->label, "He 3 pot")) sens_name = "C";
	Fmt(msg, "KRDG? %s", sens_name);
	acqchan->reading = gpib_GetDoubleVal(dev, msg);
	acqchan->newreading = TRUE;
	if(alarm && ls->alarm.on)
	{
		if(acqchan->reading > ls->alarm.level)
		{
			Beep();
			SetCtrlVal(dev->iPanel, LS340_CTRL_ALARMLED, 1);
		}
		else
			SetCtrlVal(dev->iPanel, LS340_CTRL_ALARMLED, 0);
	}
}

/******************************Callback Functions**********************************/
void LS340_UpdateSensorReadings (int panel, void *ptr)
{
	LS340Ptr ls = ptr;
	SetCtrlVal(panel, LS340_SENS_SORBMEAS, ls->channels[SORB]->reading);
	SetCtrlVal(panel, LS340_SENS_KPOTMEAS, ls->channels[ONEK]->reading);
	SetCtrlVal(panel, LS340_SENS_HE3PMEAS, ls->channels[HE3P]->reading);
	if(utilG.acq.status == ACQ_BUSY)
		HidePanel(panel);
}

void LS340_UpdateHeaterSettings(int panel, gpibioPtr dev)
{
	char msg[260];
	LS340Ptr ls = dev->device;
	gpibPrint(dev, "CSET? %i\n", &ls->heater.loop);
	gpibio_In(dev, msg);
	Scan(msg, "%s[w1],%i,%i,%i", ls->heater.input, &ls->heater.units, &ls->heater.on, &ls->heater.powerup);
	gpibPrint(dev, "CLIMIT? %i\n", &ls->heater.loop);
	gpibio_In(dev, msg);
	Scan(msg, "%f,%f,%f,%f,%i", &ls->heater.setplimit, &ls->heater.pchange, &ls->heater.nchange, &ls->heater.current, &ls->heater.maxpower);
	gpibPrint(dev, "PID? %i\n", &ls->heater.loop);
	gpibio_In(dev, msg);
	Scan(msg, "%f,%f,%f", &ls->pid.p, &ls->pid.i, &ls->pid.d);
}

void LS340_UpdateControls(int p, gpibioPtr dev)
{
	LS340Ptr ls = dev->device;
	char msg[260];
	
	ls->heater.power = gpib_GetIntVal(dev, "RANGE?\n");
	Fmt(msg, "SETP? %i\n", ls->heater.loop);
	ls->heater.setpoint = gpib_GetDoubleVal(dev, msg);
	Fmt(msg, "RAMP? %i\n", ls->heater.loop);
	gpib_GetCharVal(dev, msg, msg);
	Scan(msg, "%i,%f", &ls->heater.on, &ls->heater.rampspeed);
	
	SetCtrlVal(p, LS340_CTRL_POWER, ls->heater.power);
	SetCtrlVal(p, LS340_CTRL_HEATER, ls->heater.on);
	SetCtrlVal(p, LS340_CTRL_SORBTSET, ls->heater.setpoint);
	SetCtrlAttribute(p, LS340_CTRL_SORBTSET, ATTR_MAX_VALUE, ls->heater.setplimit);
	SetCtrlVal(p, LS340_CTRL_RAMPSPEED, ls->heater.rampspeed);
	SetCtrlVal(p, LS340_CTRL_ALARM, ls->alarm.on);
	SetCtrlVal(p, LS340_CTRL_ALARMLVL, ls->alarm.level);
}

int LS340PanelCallback (int panel, int event, void *callbackData, int eventData1, int eventData2)
{
	gpibioPtr dev = callbackData;
	if ((event == EVENT_KEYPRESS && eventData1 == VAL_ESC_VKEY) || (event == EVENT_RIGHT_DOUBLE_CLICK))
	{
		devPanel_Remove (panel);
        DiscardPanel (panel);
		dev->iPanel = 0;
		SetMenuBarAttribute (acquire_GetMenuBar(), dev->menuitem_id, ATTR_DIMMED, FALSE);

	}
	return 0;
}

int LS340ControlCallback (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	gpibioPtr dev = callbackData;
	LS340Ptr ls = dev->device;
	char msg[260];
	switch(control)
	{
		case LS340_CTRL_HEATER:
			if (event == EVENT_COMMIT)
			{
				GetCtrlVal(panel, control, &ls->heater.on);
				if(ls->heater.on)
				{
					GetCtrlVal(panel, LS340_CTRL_POWER, &ls->heater.power);
					GetCtrlVal(panel, LS340_CTRL_SORBTSET, &ls->heater.setpoint);
					GetCtrlVal(panel, LS340_CTRL_RAMPSPEED, &ls->heater.rampspeed);
					gpibPrint(dev, "SETP %i, %f\n", &ls->heater.loop, &ls->heater.setpoint);
					gpibPrint(dev, "RAMP %i, 1, %f\n", &ls->heater.loop, &ls->heater.rampspeed);
					gpibPrint(dev, "RANGE %i\n", &ls->heater.power);
				}
				else
					gpibPrint(dev, "RANGE 0\n");
			}
			break;
		case LS340_CTRL_HEATER_PROP:
			if (event == EVENT_COMMIT)
			{
				int heater = LoadPanel(utilG.p, "LS340u.uir", LS340_HEAT);
				SetCtrlAttribute(heater, LS340_HEAT_DON, ATTR_CALLBACK_DATA, dev);
				SetCtrlAttribute(heater, LS340_HEAT_ION, ATTR_CALLBACK_DATA, dev);
				SetCtrlAttribute(heater, LS340_HEAT_PON, ATTR_CALLBACK_DATA, dev);
				SetCtrlAttribute(heater, LS340_HEAT_ACCEPT, ATTR_CALLBACK_DATA, dev);
				SetCtrlAttribute(heater, LS340_HEAT_RESET, ATTR_CALLBACK_DATA, dev);
				SetCtrlAttribute(heater, LS340_HEAT_P, ATTR_DIMMED, !ls->pid.pon);
				SetCtrlAttribute(heater, LS340_HEAT_I, ATTR_DIMMED, !ls->pid.ion);
				SetCtrlAttribute(heater, LS340_HEAT_D, ATTR_DIMMED, !ls->pid.don);
				
				LS340_UpdateHeaterSettings(heater, dev);
				SetCtrlVal(heater, LS340_HEAT_LOOPNUM,	ls->heater.loop);
				SetCtrlVal(heater, LS340_HEAT_SETPLIM,	ls->heater.setplimit);
				SetCtrlVal(heater, LS340_HEAT_PCHANGE,	ls->heater.pchange);
				SetCtrlVal(heater, LS340_HEAT_NCHANGE,	ls->heater.nchange);
				SetCtrlVal(heater, LS340_HEAT_CURRENT,	ls->heater.current);
				SetCtrlVal(heater, LS340_HEAT_POWERUP,	ls->heater.powerup);
				SetCtrlVal(heater, LS340_HEAT_MXPOWER,	ls->heater.maxpower);
				SetCtrlVal(heater, LS340_HEAT_INPUTNM,	ls->heater.input);
				SetCtrlVal(heater, LS340_HEAT_UNITS,	ls->heater.units);
				SetCtrlVal(heater, LS340_HEAT_P,		ls->pid.p);
				SetCtrlVal(heater, LS340_HEAT_PON,		ls->pid.pon);
				SetCtrlVal(heater, LS340_HEAT_I,		ls->pid.i);
				SetCtrlVal(heater, LS340_HEAT_ION,		ls->pid.ion);
				SetCtrlVal(heater, LS340_HEAT_D,		ls->pid.d);
				SetCtrlVal(heater, LS340_HEAT_DON,		ls->pid.don);
				DisplayPanel(heater);
			}
			break;
		case LS340_CTRL_SORBTSET:
			if (event == EVENT_COMMIT)
			{
				GetCtrlVal(panel, control, &ls->heater.setpoint);
				gpibPrint(dev, "SETP %i, %f\n", &ls->heater.loop, &ls->heater.setpoint);
			}
			break;
		case LS340_CTRL_RAMPSPEED:
			if (event == EVENT_COMMIT)
			{
				GetCtrlVal(panel, control, &ls->heater.rampspeed);
				gpibPrint(dev, "RAMP %i, %i, %f\n", &ls->heater.loop, &ls->heater.on, &ls->heater.rampspeed);
			}
			break;
		case LS340_CTRL_POWER:
			if (event == EVENT_COMMIT)
			{
				GetCtrlVal(panel, control, &ls->heater.power);
				gpibPrint(dev, "RANGE %i\n", &ls->heater.power);
			}
			break;
		case LS340_CTRL_ALARM:
			if (event == EVENT_COMMIT)
			{
				GetCtrlVal(panel, control, &ls->alarm.on);
				if(!ls->alarm.on)
					SetCtrlVal(panel, LS340_CTRL_ALARMLED, 0);
			}
			break;
		case LS340_CTRL_ALARMLVL:
			if (event == EVENT_COMMIT)
				GetCtrlVal(panel, control, &ls->alarm.level);
			break;
	}
	return 0;
}

int LS340HeatControlCallback (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	gpibioPtr dev = callbackData;
	LS340Ptr ls = dev->device;
	switch (control)
	{
		case LS340_HEAT_PON:
			if(event == EVENT_COMMIT)
			{
				GetCtrlVal(panel, control, &ls->pid.pon);
				SetCtrlAttribute(panel, LS340_HEAT_P, ATTR_DIMMED, !ls->pid.pon);
			}
			break;
		case LS340_HEAT_ION:
			if(event == EVENT_COMMIT)
			{
				GetCtrlVal(panel, control, &ls->pid.ion);
				SetCtrlAttribute(panel, LS340_HEAT_I, ATTR_DIMMED, !ls->pid.ion);
			}
			break;
		case LS340_HEAT_DON:
			if(event == EVENT_COMMIT)
			{
				GetCtrlVal(panel, control, &ls->pid.don);
				SetCtrlAttribute(panel, LS340_HEAT_D, ATTR_DIMMED, !ls->pid.don);
			}
			break;
		case LS340_HEAT_ACCEPT:
			if(event == EVENT_COMMIT)
			{
				GetCtrlVal(panel, LS340_HEAT_LOOPNUM, &ls->heater.loop);
				GetCtrlVal(panel, LS340_HEAT_SETPLIM, &ls->heater.setplimit);
				GetCtrlVal(panel, LS340_HEAT_PCHANGE, &ls->heater.pchange);
				GetCtrlVal(panel, LS340_HEAT_NCHANGE, &ls->heater.nchange);
				GetCtrlVal(panel, LS340_HEAT_CURRENT, &ls->heater.current);
				GetCtrlVal(panel, LS340_HEAT_MXPOWER, &ls->heater.maxpower);
				GetCtrlVal(panel, LS340_HEAT_POWERUP, &ls->heater.powerup);
				GetCtrlVal(panel, LS340_HEAT_INPUTNM, ls->heater.input);   
				GetCtrlVal(panel, LS340_HEAT_UNITS,   &ls->heater.units);  //*
				SetCtrlAttribute(dev->iPanel, LS340_CTRL_SORBTSET, ATTR_MAX_VALUE, ls->heater.setplimit);
				gpibPrint(dev, "CSET %i, %s, %i, %i, %i\n", &ls->heater.loop,
															ls->heater.input,
															&ls->heater.units,
															&ls->heater.on,
															&ls->heater.powerup);
				gpibPrint(dev, "CLIMIT %i, %f, %f, %f, %f, %i\n",	&ls->heater.loop,
																	&ls->heater.setplimit,
																	&ls->heater.pchange,
																	&ls->heater.nchange,
																	&ls->heater.current,
																	&ls->heater.maxpower);//*/
				GetCtrlVal(panel, LS340_HEAT_P, &ls->pid.p);
				GetCtrlVal(panel, LS340_HEAT_I, &ls->pid.i);
				GetCtrlVal(panel, LS340_HEAT_D, &ls->pid.d);
				if(ls->pid.pon)
				{
					gpibPrint(dev, "PID %i, %f[, %f][, %f]\n", 	&ls->heater.loop,
															&ls->pid.p,
															ls->pid.ion,
															&ls->pid.i,
															(ls->pid.ion && ls->pid.don),
															&ls->pid.d);
				}			
			}
			break;
		case LS340_HEAT_RESET:
			if(event == EVENT_COMMIT)
			{
				SetCtrlVal(panel, LS340_HEAT_LOOPNUM, ls->heater.loop);
				SetCtrlVal(panel, LS340_HEAT_SETPLIM, ls->heater.setplimit);
				SetCtrlVal(panel, LS340_HEAT_PCHANGE, ls->heater.pchange);
				SetCtrlVal(panel, LS340_HEAT_NCHANGE, ls->heater.nchange);
				SetCtrlVal(panel, LS340_HEAT_CURRENT, ls->heater.current);
				SetCtrlVal(panel, LS340_HEAT_MXPOWER, ls->heater.maxpower);
				SetCtrlVal(panel, LS340_HEAT_POWERUP, ls->heater.powerup);
				SetCtrlVal(panel, LS340_HEAT_INPUTNM, ls->heater.input);   
				SetCtrlVal(panel, LS340_HEAT_UNITS,   ls->heater.units);  //*
				if(ls->pid.pon)
					SetCtrlVal(panel, LS340_HEAT_P, ls->pid.p);
				if(ls->pid.ion)
					SetCtrlVal(panel, LS340_HEAT_I, ls->pid.i);
				if(ls->pid.don)
					SetCtrlVal(panel, LS340_HEAT_D, ls->pid.d);
				
			}
			break;
		
	}
	return 0;
}

int  LS340SensorControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    acqchanPtr acqchan;
    acqchan = callbackData;

    switch (control) {
        case LS340_SENS_NOTE_1:
        case LS340_SENS_NOTE_2:
        case LS340_SENS_NOTE_3:
            AcqDataNoteCallback (panel, control, event, callbackData, eventData1, eventData2);
            break;
        case LS340_SENS_SORBACQ:
        case LS340_SENS_KPOTACQ:
        case LS340_SENS_HE3PACQ:
            if (event == EVENT_VAL_CHANGED) {
                GetCtrlVal (panel, control, &acqchan->acquire);
                if (acqchan->acquire) acqchanlist_AddChannel (acqchan);
                    else acqchanlist_RemoveChannel (acqchan);
            }
            break;
        case LS340_SENS_SORBCOEFF:
        case LS340_SENS_KPOTCOEFF:
        case LS340_SENS_HE3PCOEFF:
			if (event == EVENT_COMMIT) {
                GetCtrlVal (panel, control, &acqchan->coeff);
                if (acqchan->p) SetCtrlVal (acqchan->p, ACQDATA_COEFF, acqchan->coeff);
            }
            break;
        case LS340_SENS_SORBLABEL:
        case LS340_SENS_KPOTLABEL:
        case LS340_SENS_HE3PLABEL:
            if (event == EVENT_COMMIT) {
                GetCtrlVal (panel, control, acqchan->channel->label);
                acqchanlist_ReplaceChannel (acqchan);
                if (acqchan->p) SetPanelAttribute (acqchan->p, ATTR_TITLE, acqchan->channel->label);
            }
            break;
        case LS340_SENS_CLOSE:
            if (event == EVENT_COMMIT) 
				HidePanel (panel);
            break;
    }
	updateGraphSource();
    return 0;
}

int LS340SendCurve (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	if (event == EVENT_COMMIT)
	{
		char *curve="", *buffer, vars[30];
		gpibioPtr dev = callbackData;
		LS340Ptr ls = dev->device;
		
		GetCtrlVal(panel, LS340CURVE_CURVESRC, &ls->curveload.source);
		GetCtrlVal(panel, LS340CURVE_CURVENUM, &ls->curveload.target);
		GetCtrlVal(panel, LS340CURVE_SERIAL,   ls->curveload.serial);
		
		do{
			ScanFile(ls->curveload.file, "%s", vars);
			buffer = malloc(sizeof(char) * StringLength(curve) + sizeof(vars));
			Fmt(buffer, "%s%s", curve, vars);
			curve = buffer;
		}while(*vars);
		gpibPrint(dev, "SCAL %i, %i, %s, %s\n", &ls->curveload.source,
												&ls->curveload.target,
												ls->curveload.serial,
												curve);
	}
	return 0;
}

void LS340menuCallack (int menuBar, int menuItem, void *callbackData, int panel)
{
	switch(menuItem)
	{
		case LS340_CURVES_LOAD:
		{
			gpibioPtr dev = callbackData;
			LS340Ptr ls = dev->device;
			char pathname[260];
			int cPanel, i = FileSelectPopup ("", "*.crv", "*.crv", "Custom Curve", VAL_LOAD_BUTTON,
								 0, 0, 1, 0, pathname);
			if(i)
			{
				ls->curveload.file = OpenFile (pathname, VAL_READ_ONLY, VAL_OPEN_AS_IS, VAL_ASCII);
				cPanel = LoadPanel(utilG.p, "LS340u.uir", LS340CURVE);
				
				SetCtrlAttribute(cPanel, LS340CURVE_ACCEPT, ATTR_CALLBACK_DATA, dev);
				
				SetCtrlVal(cPanel, LS340CURVE_SERIAL,	 ls->curveload.serial);
				SetCtrlVal(cPanel, LS340CURVE_CURVESRC,  ls->curveload.source);
				SetCtrlVal(cPanel, LS340CURVE_CURVENUM,  ls->curveload.target);
				
				DisplayPanel(cPanel);
			}
		}
		break;
		case LS340_SOURCE_HEATER:
		{
			sourcePtr src = callbackData;
			switch(utilG.exp)
			{
				case EXP_SOURCE: source_InitPanel(src); break;
				case EXP_FLOAT : gensrc_InitPanel(src); break;
			}
		}
		break;
		case LS340_MEASURE_MEAS:
    	{
			LS340Ptr ls = callbackData;
    		int p = LoadPanel (utilG.p, "LS340u.uir", LS340_SENS);
    		
			SetCtrlVal (p, LS340_SENS_SORBLABEL, ls->channels[SORB]->channel->label);
			SetCtrlVal (p, LS340_SENS_SORBCOEFF, ls->channels[SORB]->coeff);
			SetCtrlVal (p, LS340_SENS_SORBACQ,   ls->channels[SORB]->acquire);
			SetCtrlVal (p, LS340_SENS_NOTE_1, 	 ls->channels[SORB]->note);
								  
			SetCtrlVal (p, LS340_SENS_KPOTLABEL, ls->channels[ONEK]->channel->label);
			SetCtrlVal (p, LS340_SENS_KPOTCOEFF, ls->channels[ONEK]->coeff);
			SetCtrlVal (p, LS340_SENS_KPOTACQ,   ls->channels[ONEK]->acquire);
			SetCtrlVal (p, LS340_SENS_NOTE_2,	 ls->channels[ONEK]->note);

			SetCtrlVal (p, LS340_SENS_HE3PLABEL, ls->channels[HE3P]->channel->label);
			SetCtrlVal (p, LS340_SENS_HE3PCOEFF, ls->channels[HE3P]->coeff);
			SetCtrlVal (p, LS340_SENS_HE3PACQ,   ls->channels[HE3P]->acquire);
			SetCtrlVal (p, LS340_SENS_NOTE_3, 	 ls->channels[HE3P]->note);

			SetCtrlAttribute(p, LS340_SENS_SORBLABEL, 	ATTR_CALLBACK_DATA, ls->channels[SORB]);
			SetCtrlAttribute(p, LS340_SENS_SORBCOEFF, 	ATTR_CALLBACK_DATA, ls->channels[SORB]);
			SetCtrlAttribute(p, LS340_SENS_SORBACQ, 	ATTR_CALLBACK_DATA, ls->channels[SORB]);
			SetCtrlAttribute(p, LS340_SENS_NOTE_1, 		ATTR_CALLBACK_DATA, ls->channels[SORB]);
	
			SetCtrlAttribute(p, LS340_SENS_KPOTLABEL, 	ATTR_CALLBACK_DATA, ls->channels[ONEK]);
			SetCtrlAttribute(p, LS340_SENS_KPOTCOEFF, 	ATTR_CALLBACK_DATA, ls->channels[ONEK]);
			SetCtrlAttribute(p, LS340_SENS_KPOTACQ,   	ATTR_CALLBACK_DATA, ls->channels[ONEK]);
			SetCtrlAttribute(p, LS340_SENS_NOTE_2, 		ATTR_CALLBACK_DATA, ls->channels[ONEK]);
		
			SetCtrlAttribute(p, LS340_SENS_HE3PLABEL, 	ATTR_CALLBACK_DATA, ls->channels[HE3P]);
			SetCtrlAttribute(p, LS340_SENS_HE3PCOEFF, 	ATTR_CALLBACK_DATA, ls->channels[HE3P]);
			SetCtrlAttribute(p, LS340_SENS_HE3PACQ,   	ATTR_CALLBACK_DATA, ls->channels[HE3P]);
			SetCtrlAttribute(p, LS340_SENS_NOTE_3, 		ATTR_CALLBACK_DATA, ls->channels[HE3P]);
	
			devPanel_Add (p, ls, LS340_UpdateSensorReadings);
			DisplayPanel (p);
		}
		break;
	}
}


/******************************Init and operation functions**********************************/
void *LS340_Create(gpibioPtr dev)
{
	LS340Ptr ls;
	ls = malloc(sizeof(LS340Type));
	if (dev){ dev->device = ls; ls->id = dev->id;}
	
	ls->source = source_Create("temperature", dev, SetHeaterLvl, GetHeaterLvl);
	ls->channels[SORB] = acqchan_Create("sorb", dev, GetSensor);
	ls->channels[ONEK] = acqchan_Create("1 k pot", dev, GetSensor);
	ls->channels[HE3P] = acqchan_Create("He 3 pot", dev, GetSensor);
	ls->alarm.on = 0;
	ls->alarm.level = 0;
	ls->curveload.format = "%s, %s,> %s";
	ls->curveload.serial = "no_number";
	ls->curveload.source = 1;
	ls->curveload.target = 21;
	ls->heater.current = 3;
	ls->heater.input = "A";
	ls->heater.loop = 1;
	ls->heater.maxpower = 5;
	ls->heater.nchange = 0;
	ls->heater.on = 0;
	ls->heater.pchange = 10;
	ls->heater.power = 5;
	ls->heater.powerup = 0;
	ls->heater.rampspeed = 150;
	ls->heater.setplimit = 350;
	ls->heater.setpoint = 0;
	ls->heater.units = KELVIN;
	ls->pid.d = 0;
	ls->pid.don = 0;
	ls->pid.i =0;
	ls->pid.ion = 0;
	ls->pid.p= 0;
	ls->pid.pon = 0;
	ls->source->min = 0;
	return ls;
}

int  LS340_InitGPIB(gpibioPtr dev)
{
	gpibio_Remote(dev);
	if(gpibio_DeviceMatch(dev, "*IDN?", LS340_ID))
		return TRUE;
	return FALSE;
}

void OperateLS340(int menubar, int menuItem, void *callbackData, int panel)
{
	gpibioPtr dev = callbackData;
	LS340Ptr ls = dev->device;
	int p, m;
	
	ls->source->max = ls->heater.setplimit;
	p = dev->iPanel? dev->iPanel: LoadPanel(utilG.p, "LS340u.uir", LS340_CTRL);
	dev->iPanel = p;
	
	SetMenuBarAttribute(menubar, menuItem, ATTR_DIMMED, 1);
	SetPanelAttribute(p, ATTR_TITLE, dev->label);
	
	m = LoadMenuBar(p, "LS340u.uir", LS340);
	SetPanelMenuBar(p, m);
	
	SetMenuBarAttribute(m, LS340_CURVES_LOAD, ATTR_CALLBACK_DATA, dev);
	SetMenuBarAttribute(m, LS340_SOURCE_HEATER, ATTR_CALLBACK_DATA, ls->source);
	SetMenuBarAttribute(m, LS340_MEASURE_MEAS, ATTR_CALLBACK_DATA, ls);
	
	SetCtrlAttribute(p, LS340_CTRL_HEATER, ATTR_CALLBACK_DATA, dev);
	SetCtrlAttribute(p, LS340_CTRL_HEATER_PROP, ATTR_CALLBACK_DATA, dev);
	SetCtrlAttribute(p, LS340_CTRL_SORBTSET, ATTR_CALLBACK_DATA, dev);
	SetCtrlAttribute(p, LS340_CTRL_RAMPSPEED, ATTR_CALLBACK_DATA, dev);
	SetCtrlAttribute(p, LS340_CTRL_POWER, ATTR_CALLBACK_DATA, dev);
	SetCtrlAttribute(p, LS340_CTRL_ALARM, ATTR_CALLBACK_DATA, dev);
	SetCtrlAttribute(p, LS340_CTRL_ALARMLVL, ATTR_CALLBACK_DATA, dev);
	
	SetCtrlAttribute(p, LS340_CTRL_SORBTSET, ATTR_MAX_VALUE, ls->heater.setplimit);
	SetCtrlAttribute(p, LS340_CTRL_SORBTSET, ATTR_MIN_VALUE, 0.);
	SetPanelAttribute(p, ATTR_CALLBACK_DATA, dev);
	
	LS340_UpdateControls(p, dev);
	devPanel_Add(p, dev, LS340_UpdateReadings);
	
	DisplayPanel(p);
}

void LS340_UpdateReadings(int panel, void *ptr)
{
	gpibioPtr dev = ptr;
	LS340Ptr ls = dev->device;
	GetSensor(ls->channels[SORB]);
	GetSensor(ls->channels[ONEK]);
	GetSensor(ls->channels[HE3P]);
	SetCtrlVal(panel, LS340_CTRL_SORBREAD, ls->channels[SORB]->reading);
	SetCtrlVal(panel, LS340_CTRL_KPOTREAD, ls->channels[ONEK]->reading);
	SetCtrlVal(panel, LS340_CTRL_HE3PREAD, ls->channels[HE3P]->reading);
}

void LS340_Save(gpibioPtr dev)
{
	int i;
	LS340Ptr ls = dev->device;
	FmtFile (fileHandle.analysis, "%s<Heater Properties  : %i, %i, %i, %i, %i, %s, %f, %f, %f, %f, %f, %f\n",
																			ls->heater.loop,
																			ls->heater.units,
																			ls->heater.power,
																			ls->heater.maxpower,
																			ls->heater.powerup,
																			ls->heater.input,
																			ls->heater.setplimit,
																			ls->heater.pchange,
																			ls->heater.nchange,
																			ls->heater.current,
																			ls->heater.setpoint,
																			ls->heater.rampspeed);
	FmtFile(fileHandle.analysis, "%s<PID properties      : %i, %i, %i, %f, %f, %f\n", 
																			ls->pid.pon,
																			ls->pid.ion,
																			ls->pid.don,
																			ls->pid.p,
																			ls->pid.i,
																			ls->pid.d);
	FmtFile(fileHandle.analysis, "%s<Alarm properties    : %i, %f\n", ls->alarm.on, ls->alarm.level);
	
    for (i = 0; i < 3; i++) acqchan_Save (ls->channels[i]);
    source_Save (ls->source);
}

void LS340_Load(gpibioPtr dev)
{
	int i;
	LS340Ptr ls = dev? dev->device:NULL;
	if(dev)
	{
		ScanFile (fileHandle.analysis, "%s>Heater Properties  : %i, %i, %i, %i, %i, %s[w1], %f, %f, %f, %f, %f, %f",
																			&ls->heater.loop,
																			&ls->heater.units,
																			&ls->heater.power,
																			&ls->heater.maxpower,
																			&ls->heater.powerup,
																			ls->heater.input,
																			&ls->heater.setplimit,
																			&ls->heater.pchange,
																			&ls->heater.nchange,
																			&ls->heater.current,
																			&ls->heater.setpoint,
																			&ls->heater.rampspeed);
		ScanFile(fileHandle.analysis, "%s>PID properties      : %i, %i, %i, %f, %f, %f", 
																			&ls->pid.pon,
																			&ls->pid.ion,
																			&ls->pid.don,
																			&ls->pid.p,
																			&ls->pid.i,
																			&ls->pid.d);
		ScanFile(fileHandle.analysis, "%s>Alarm properties    : %i, %f", 		&ls->alarm.on,
																			&ls->alarm.level);
	
    	for (i = 0; i < 3; i++) acqchan_Load (dev, ls->channels[i]);
    	source_Load (dev, ls->source);
		ls->source->max = ls->heater.setplimit;
		ls->source->min = 0;
	}
}

void LS340_Remove(void *dev)
{
	LS340Ptr ls = dev;
	acqchan_Remove(ls->channels[SORB]);
	acqchan_Remove(ls->channels[ONEK]);
	acqchan_Remove(ls->channels[HE3P]);
	source_Remove(ls->source);
	free(ls);
}

void LS340_Init(void)
{
	devTypePtr devType;
	if(utilG.acq.status != ACQ_NONE)
	{
		util_ChangeInitMessage("ls340 control utilities...");
		devType = malloc(sizeof(devTypeItem));
		if(devType)
		{
			Fmt(devType->label, "ls 340 tmperature controller");
			Fmt(devType->id, LS340_ID);
			devType->CreateDevice = LS340_Create;
			devType->InitDevice = LS340_InitGPIB;
			devType->OperateDevice = OperateLS340;
			devType->UpdateReadings = LS340_UpdateReadings;
			devType->SaveDevice = LS340_Save;
			devType->LoadDevice = LS340_Load;
			devType->RemoveDevice = LS340_Remove;
			devTypeList_AddItem(devType);
		}
	}
}
