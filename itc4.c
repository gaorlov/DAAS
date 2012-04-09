#include <utility.h>
#include <ansi_c.h>
#include <formatio.h>
#include <userint.h>
#include <rs232.h>

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
#include "gpibio.h"
#include "curveop.h"
#include "acquire.h"
#include "acquireu.h"
#include "source.h"
#include "rs232util.h"
#include "ITC4u.h"
#include "itc4.h"

#define TRUE 1
#define FALSE 0
#define ITC4_ID "itc4"
//heater1 4 - 300
//heater2 0.25 - 9.999

typedef struct{
	int on, setp, number; 
	double min, max, multiplier;
	rs232Ptr dev;
}heaterType;
typedef heaterType *heaterPtr;

typedef struct{
	int id, meas, heater;
	double error;
	acqchanPtr channels[3];
	sourcePtr  source;
	struct{int panel; struct{int on, control; double val;}p,i,d;}pid;
	heaterType heater1, heater2;
}itc4Type;
typedef itc4Type *itc4Ptr;
/******************************************************************************/

void itc4_GetSensor1(acqchanPtr acqchan);
void itc4_GetSensor2(acqchanPtr acqchan);
void itc4_GetSensor3(acqchanPtr acqchan);
void itc4_GetHeaterLvl(acqchanPtr acqchan);
void itc4_SetHeaterLvl(sourcePtr src);

int itc4PanelCallback (int panel, int event, void *callbackData, int eventData1, int eventData2);
void itc4MenuCallback (int menuBar, int menuItem, void *callbackData, int panel);
int itc4ControlCallback (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int itc4SensorControlCallback (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int itc4pidControlCallback (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);

void itc4_Create (rs232Ptr dev);
void itc4_Remove (void *ptr);
int  itc4_InitIO (rs232Ptr dev);
void itc4Operate(int menubar, int menuItem, void *callbackData, int panel);
void itc4UpdateReadings(int panel, void* ptr);
void itc4_Save(rs232Ptr dev);
void itc4_Load(rs232Ptr dev);
void itc4_Init(void);



/******************************************************************************/
void itc4_GetSensor1(acqchanPtr acqchan)
{
	rs232Ptr dev = acqchan->dev;
	rs232Write(dev, "R1\r");
	if(rs232Read (dev, "R%f", &acqchan->reading))
	{
		acqchan->reading /= 10;
		acqchan->newreading = TRUE;
	}
}

void itc4_GetSensor2(acqchanPtr acqchan)
{
	rs232Ptr dev = acqchan->dev;
	rs232Write(dev, "R2\r");
	if(rs232Read (dev, "R%f", &acqchan->reading))
	{
		acqchan->reading /= 1000;
		acqchan->newreading = TRUE;
	}
}

void itc4_GetSensor3(acqchanPtr acqchan)
{
	rs232Ptr dev = acqchan->dev;
	rs232Write(dev, "R3\r");
	if(rs232Read (dev, "R%f", &acqchan->reading))
	{
		acqchan->reading /= 10;
		acqchan->newreading = TRUE;
	}
}

void itc4_GetHeaterLvl(acqchanPtr acqchan)
{
	rs232Ptr dev = acqchan->dev;
	itc4Ptr  itc = dev->device;
	
	rs232Write(dev, "R2\r");
	if(rs232Read (dev, "R%f", &acqchan->reading))
	{
		acqchan->reading /= 1000;
		acqchan->newreading = TRUE;
	}
}

void itc4_SetHeaterLvl(sourcePtr src)
{
	rs232Ptr dev = src->acqchan->dev;
	itc4Ptr  itc = dev->device;
	
	FlushOutQ(dev->COM->port);
	rs232Write(dev, "H2\r");
	Delay(0.01);
	rs232Write(dev, "T%f\r", &(src->biaslevel));
	util_Delay(src->segments[src->seg]->delay);
}

/******************************************************************************/


int itc4PanelCallback (int panel, int event, void *callbackData, int eventData1, int eventData2)
{
	rs232Ptr dev = callbackData;
	itc4Ptr  itc = dev->device;
	if ((event == EVENT_KEYPRESS && eventData1 == VAL_ESC_VKEY) || event == EVENT_RIGHT_DOUBLE_CLICK)
	{
		DiscardPanel (panel);
		dev->iPanel = 0;
		devPanel_Remove(panel);
		SetMenuBarAttribute (acquire_GetMenuBar(), dev->menuitem_id, ATTR_DIMMED, FALSE);
		if(itc->meas)
			HidePanel(itc->meas);
	}
	return 0;
}

void itc4MenuCallback (int menuBar, int menuItem, void *callbackData, int panel)
{
	switch (menuItem)
	{
		case ITC4MENU_SOURCE_TEMP:
		{
			sourcePtr src = callbackData;
			switch(utilG.exp)
			{
				case EXP_SOURCE: source_InitPanel(src); break;
				case EXP_FLOAT : gensrc_InitPanel(src); break;
			}
		}
		break;
		case ITC4MENU_MEASURE_SENS:
		{
			rs232Ptr dev = callbackData;
			itc4Ptr  itc = dev->device;
			if(!itc->meas)
			{
				itc->meas = LoadPanel(utilG.p, "ITC4u.uir", MEASURE);
				SetCtrlVal (itc->meas, MEASURE_NOTE_1, itc->channels[0]->note);
				SetCtrlVal (itc->meas, MEASURE_NOTE_2, itc->channels[1]->note);
				SetCtrlVal (itc->meas, MEASURE_NOTE_3, itc->channels[2]->note);
				SetCtrlVal (itc->meas, MEASURE_SENS1ACQ, itc->channels[0]->acquire);
				SetCtrlVal (itc->meas, MEASURE_SENS2ACQ, itc->channels[1]->acquire);
				SetCtrlVal (itc->meas, MEASURE_SENS3ACQ, itc->channels[2]->acquire);
				SetCtrlVal (itc->meas, MEASURE_SENS1COEFF, itc->channels[0]->coeff);
				SetCtrlVal (itc->meas, MEASURE_SENS2COEFF, itc->channels[1]->coeff);
				SetCtrlVal (itc->meas, MEASURE_SENS3COEFF, itc->channels[2]->coeff);
				SetCtrlVal (itc->meas, MEASURE_SENS1LABEL, itc->channels[0]->channel->label);
				SetCtrlVal (itc->meas, MEASURE_SENS2LABEL, itc->channels[1]->channel->label);
				SetCtrlVal (itc->meas, MEASURE_SENS3LABEL, itc->channels[2]->channel->label);
				
				SetCtrlAttribute (itc->meas, MEASURE_NOTE_1, ATTR_CALLBACK_DATA, itc->channels[0]);
				SetCtrlAttribute (itc->meas, MEASURE_NOTE_2, ATTR_CALLBACK_DATA, itc->channels[1]);
				SetCtrlAttribute (itc->meas, MEASURE_NOTE_3, ATTR_CALLBACK_DATA, itc->channels[2]);
				SetCtrlAttribute (itc->meas, MEASURE_SENS1ACQ, ATTR_CALLBACK_DATA, itc->channels[0]);
				SetCtrlAttribute (itc->meas, MEASURE_SENS2ACQ, ATTR_CALLBACK_DATA, itc->channels[1]);
				SetCtrlAttribute (itc->meas, MEASURE_SENS3ACQ, ATTR_CALLBACK_DATA, itc->channels[2]);
				SetCtrlAttribute (itc->meas, MEASURE_SENS1COEFF, ATTR_CALLBACK_DATA, itc->channels[0]);
				SetCtrlAttribute (itc->meas, MEASURE_SENS2COEFF, ATTR_CALLBACK_DATA, itc->channels[1]);
				SetCtrlAttribute (itc->meas, MEASURE_SENS3COEFF, ATTR_CALLBACK_DATA, itc->channels[2]);
				SetCtrlAttribute (itc->meas, MEASURE_SENS1LABEL, ATTR_CALLBACK_DATA, itc->channels[0]);
				SetCtrlAttribute (itc->meas, MEASURE_SENS2LABEL, ATTR_CALLBACK_DATA, itc->channels[1]);
				SetCtrlAttribute (itc->meas, MEASURE_SENS3LABEL, ATTR_CALLBACK_DATA, itc->channels[2]);
			}
			DisplayPanel(itc->meas);
		}
		break;
		case ITC4MENU_SETTINGS_PID:
		{
			rs232Ptr dev = callbackData;
			itc4Ptr  itc = dev->device;
			if(!itc->pid.panel)
			{
				itc->pid.panel = LoadPanel(utilG.p, "ITC4u.uir", ITC4_PID);
				SetCtrlAttribute (itc->pid.panel, ITC4_PID_PON, 	ATTR_CALLBACK_DATA, &itc->pid.p);
				SetCtrlAttribute (itc->pid.panel, ITC4_PID_ION, 	ATTR_CALLBACK_DATA, &itc->pid.i);
				SetCtrlAttribute (itc->pid.panel, ITC4_PID_DON, 	ATTR_CALLBACK_DATA, &itc->pid.d);
				SetCtrlAttribute (itc->pid.panel, ITC4_PID_ACCEPT, 	ATTR_CALLBACK_DATA, dev);
				SetCtrlAttribute (itc->pid.panel, ITC4_PID_P,		ATTR_CALLBACK_DATA, &itc->pid.p.val);
				SetCtrlAttribute (itc->pid.panel, ITC4_PID_I, 		ATTR_CALLBACK_DATA, &itc->pid.i.val);
				SetCtrlAttribute (itc->pid.panel, ITC4_PID_D,		ATTR_CALLBACK_DATA, &itc->pid.d.val);
				SetCtrlVal (itc->pid.panel, ITC4_PID_P, itc->pid.p.val);
				SetCtrlVal (itc->pid.panel, ITC4_PID_I, itc->pid.i.val);
				SetCtrlVal (itc->pid.panel, ITC4_PID_D, itc->pid.d.val);
				itc->pid.p.control = ITC4_PID_P;
				itc->pid.i.control = ITC4_PID_I;
				itc->pid.d.control = ITC4_PID_D;
			}
			DisplayPanel(itc->pid.panel);
		}
		break;
	}
	
}

int itc4ControlCallback (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	switch (control)
	{
		case ITC4_CTRL_HEATER:
			if(event == EVENT_COMMIT)
			{
				rs232Ptr dev = callbackData;
				itc4Ptr itc = dev->device;
				GetCtrlVal(panel,control, &itc->heater);
				if(itc->heater == 1)
				{
					SetCtrlAttribute(panel, ITC4_CTRL_CONTROL, ATTR_CALLBACK_DATA, &itc->heater1);
					SetCtrlAttribute(panel, ITC4_CTRL_SETP, ATTR_PRECISION, 1);
					SetCtrlAttribute(panel, ITC4_CTRL_SETP, ATTR_CALLBACK_DATA, &itc->heater1);
					SetCtrlAttribute(panel, ITC4_CTRL_SETP, ATTR_MIN_VALUE, (double)((double)(itc->heater1.min)/itc->heater1.multiplier));
					SetCtrlAttribute(panel, ITC4_CTRL_SETP, ATTR_MAX_VALUE, (double)((double)(itc->heater1.max)/itc->heater1.multiplier));
					SetCtrlVal(panel, ITC4_CTRL_SETP, (double)((double)itc->heater1.setp/(double)itc->heater1.multiplier));
					FlushOutQ(dev->COM->port);
					Delay(0.1);
					rs232Write (dev, "[H2\r]", itc->heater2.on);
					Delay(0.1);
					rs232Write (dev, "[A0\r]", itc->heater2.on);
					Delay(0.1);
					rs232Write (dev, "[H1\r]", itc->heater1.on);
					Delay(0.1);
					rs232Write (dev, "[A1\r]", itc->heater1.on);
					Delay(0.1);
					rs232Write (dev, "[T%i\r]", itc->heater1.on, &itc->heater1.setp);
					Delay(0.1);
				
					
				}
				else
				{
					SetCtrlAttribute(dev->iPanel, ITC4_CTRL_CONTROL, ATTR_CALLBACK_DATA, &itc->heater2);
					SetCtrlAttribute(panel, ITC4_CTRL_SETP, ATTR_PRECISION, 2);
					SetCtrlAttribute(panel, ITC4_CTRL_SETP, ATTR_CALLBACK_DATA, &itc->heater2);
					SetCtrlAttribute(panel, ITC4_CTRL_SETP, ATTR_MIN_VALUE, (double)((double)(itc->heater2.min)/itc->heater2.multiplier));
					SetCtrlAttribute(panel, ITC4_CTRL_SETP, ATTR_MAX_VALUE, (double)((double)(itc->heater2.max)/itc->heater2.multiplier));
					SetCtrlVal(panel, ITC4_CTRL_SETP, (double)((double)itc->heater2.setp/(double)itc->heater2.multiplier));
					FlushOutQ(dev->COM->port);
					Delay(0.1);
					rs232Write (dev, "[H1\r]", itc->heater1.on);
					Delay(0.1);
					rs232Write (dev, "[A0\r]", itc->heater1.on);
					Delay(0.1);
					rs232Write (dev, "[H2\r]", itc->heater2.on);
					Delay(0.1);
					rs232Write (dev, "[A1\r]", itc->heater2.on);
					Delay(0.1);
					rs232Write (dev, "[T%i\r]", itc->heater2.on, &itc->heater2.setp);
					Delay(0.1);
				}
			}
			break;
		case ITC4_CTRL_CONTROL:
			if(event == EVENT_COMMIT)
			{
				heaterPtr htr = callbackData;
				double temp;
				GetCtrlVal(panel, control, &htr->on);
				GetCtrlVal(panel, ITC4_CTRL_SETP, &temp);
				Delay(0.1);
				htr->setp = htr->multiplier * temp;
				FlushOutQ(htr->dev->COM->port);
				Delay(0.1);
				rs232Write (htr->dev, "H[1][2]\r",(htr->number == 1), (htr->number == 2));
				Delay(0.1);
				rs232Write (htr->dev, "[A1\r]", htr->on);
				Delay(0.1);
				rs232Write (htr->dev, "[T%i\r]", htr->on, &htr->setp);
				Delay(0.1);
				rs232Write (htr->dev, "[A0\r]", !htr->on);
				Delay(0.1);
				rs232Write (htr->dev, "[O0\r]", !htr->on);
				Delay(0.1);
			}
			break;
		case ITC4_CTRL_SETP:
			if(event == EVENT_COMMIT)
			{
				heaterPtr htr = callbackData;
				double temp;
				GetCtrlVal(panel, control, &temp);
				Delay(0.1);
				htr->setp = htr->multiplier * temp;
				FlushOutQ(htr->dev->COM->port);
				Delay(0.1);
				rs232Write (htr->dev, "[H[1][2]\r]", htr->on, (htr->number == 1), (htr->number == 2));
				Delay(0.1);
				rs232Write (htr->dev, "[[T%i\r]]", htr->on, (int)htr->setp, &htr->setp);
				Delay(0.1);
			}
			break;
	}
	return 0;
}

int itc4SensorControlCallback (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	acqchanPtr acqchan = callbackData;

    switch (control) {
        case MEASURE_NOTE_1:
        case MEASURE_NOTE_2:
        case MEASURE_NOTE_3:
            AcqDataNoteCallback (panel, control, event, callbackData, eventData1, eventData2);
            break;
        case MEASURE_SENS1ACQ:
        case MEASURE_SENS2ACQ:
        case MEASURE_SENS3ACQ:
            if (event == EVENT_VAL_CHANGED) {
                GetCtrlVal (panel, control, &acqchan->acquire);
                if (acqchan->acquire) acqchanlist_AddChannel (acqchan);
                    else acqchanlist_RemoveChannel (acqchan);
            }
            break;
        case MEASURE_SENS1COEFF:
        case MEASURE_SENS2COEFF:
        case MEASURE_SENS3COEFF:
			if (event == EVENT_COMMIT) {
                GetCtrlVal (panel, control, &acqchan->coeff);
                if (acqchan->p) SetCtrlVal (acqchan->p, ACQDATA_COEFF, acqchan->coeff);
            }
            break;
        case MEASURE_SENS1LABEL:
        case MEASURE_SENS2LABEL:
        case MEASURE_SENS3LABEL:
            if (event == EVENT_COMMIT) {
                GetCtrlVal (panel, control, acqchan->channel->label);
                acqchanlist_ReplaceChannel (acqchan);
                if (acqchan->p) SetPanelAttribute (acqchan->p, ATTR_TITLE, acqchan->channel->label);
            }
            break;
        case MEASURE_CLOSE:
            if (event == EVENT_COMMIT)
			{
				rs232Ptr dev = acqchan->dev;
				itc4Ptr  itc = dev->device;
				itc->meas = 0;
				DiscardPanel (panel);
			}
            break;
    }
	updateGraphSource();
    return 0;
}

int itc4pidControlCallback (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	switch (control)
	{
		case ITC4_PID_PON:
		case ITC4_PID_ION:
		case ITC4_PID_DON:
			if(event == EVENT_COMMIT)
			{
				struct {int on, control; double val;} *ctrl = callbackData;
				GetCtrlVal (panel, control, &ctrl->on);
				SetCtrlAttribute (panel, ctrl->control, ATTR_DIMMED, !ctrl->on);
			}
			break;
		case ITC4_PID_ACCEPT:
			if(event == EVENT_COMMIT)
			{
				rs232Ptr dev = callbackData;
				itc4Ptr  itc = dev->device;
				rs232Write (dev, "[P%f\r][I%f\r][D%f\r]", 	itc->pid.p.on, &itc->pid.p.val,
															itc->pid.i.on, &itc->pid.i.val,
															itc->pid.d.on, &itc->pid.d.val);
			}
			break;
		case ITC4_PID_P:
		case ITC4_PID_I:
		case ITC4_PID_D:
			if(event == EVENT_COMMIT)
			{
				double *val = callbackData;
				GetCtrlVal(panel, control, val);
			}
			break;
	}
	return 0;
}




/******************************************************************************/

void itc4_Create (rs232Ptr dev)
{
	int i;
	itc4Ptr itc = malloc(sizeof(itc4Type));
	if(dev){dev->device = itc;itc->id = dev->id;}
	itc->meas = 0;
	itc->pid.d.val = 0;
	itc->pid.d.on = 0;
	itc->pid.i.val = 0;
	itc->pid.i.on = 0;
	itc->pid.p.val = 0;
	itc->pid.p.on = 0;
	itc->pid.panel = 0;
	itc->heater = 1;
	itc->heater1.on = 0;
	itc->heater1.setp = 40;
	itc->heater1.min = 40;
	itc->heater1.max = 3000;
	itc->heater1.number = 1;
	itc->heater1.dev = dev;
	itc->heater1.multiplier = 10;
	itc->heater2.on = 0;
	itc->heater2.setp = 250;
	itc->heater2.min = 25;
	itc->heater2.max = 9999;
	itc->heater2.number = 2;
	itc->heater2.dev = dev;
	itc->heater2.multiplier = 1000;
	itc->channels[0] = acqchan_Create("sensor 1", dev, itc4_GetSensor1);
	itc->channels[1] = acqchan_Create("sensor 2", dev, itc4_GetSensor2);
	itc->channels[2] = acqchan_Create("sensor 3", dev, itc4_GetSensor3);
	itc->source = source_Create("temperature he3", dev, itc4_SetHeaterLvl, itc4_GetHeaterLvl);
	itc->source->min = 0;
	itc->source->max = 254.9;
}

void itc4_Remove (void *ptr)
{
	rs232Ptr dev = ptr;
	itc4Ptr itc = dev->device;
	acqchan_Remove(itc->channels[0]);
	acqchan_Remove(itc->channels[1]);
	acqchan_Remove(itc->channels[2]);
	source_Remove(itc->source);
	free(itc);
}


int  itc4_InitIO (rs232Ptr dev)
{
	FlushOutQ(dev->COM->port);
	rs232Write(dev, "C3\r"); //sets the device to be REMOTE & UNLOCKED
	rs232Write(dev, "A0\r");
	return TRUE;
}
				
void itc4Operate(int menubar, int menuItem, void *callbackData, int panel)
{
	rs232Ptr dev = callbackData;
	itc4Ptr itc = dev->device;
	int m;
	
	if(!dev->iPanel)
	{
		dev->iPanel = LoadPanel(utilG.p, "ITC4u.uir", ITC4_CTRL);
	
		SetMenuBarAttribute(menubar, menuItem, ATTR_DIMMED, 1);
		SetPanelAttribute(dev->iPanel, ATTR_TITLE, dev->label);
	
		m = LoadMenuBar (dev->iPanel, "ITC4u.uir", ITC4MENU);
		SetPanelMenuBar (dev->iPanel, m);
	
		SetMenuBarAttribute(m, ITC4MENU_SOURCE_TEMP,  ATTR_CALLBACK_DATA, itc->source);
		SetMenuBarAttribute(m, ITC4MENU_MEASURE_SENS, ATTR_CALLBACK_DATA, dev);
		SetMenuBarAttribute(m, ITC4MENU_SETTINGS_PID, ATTR_CALLBACK_DATA, dev);
	
		SetCtrlAttribute(dev->iPanel, ITC4_CTRL_HEATER,  ATTR_CALLBACK_DATA, dev);
		if(itc->heater == 1)
		{
			SetCtrlAttribute(dev->iPanel, ITC4_CTRL_CONTROL, ATTR_CALLBACK_DATA, &itc->heater1);
			SetCtrlAttribute(dev->iPanel, ITC4_CTRL_SETP,	 ATTR_CALLBACK_DATA, &itc->heater1);
		}
		else
		{
			SetCtrlAttribute(dev->iPanel, ITC4_CTRL_CONTROL, ATTR_CALLBACK_DATA, &itc->heater2);
			SetCtrlAttribute(dev->iPanel, ITC4_CTRL_SETP,	 ATTR_CALLBACK_DATA, &itc->heater2);
		}
	
		SetPanelAttribute(dev->iPanel, ATTR_CALLBACK_DATA, dev);
	}
	SetCtrlVal(dev->iPanel, ITC4_CTRL_SETP,  (double)(itc->heater1.setp/10));
	devPanel_Add(dev->iPanel, dev, itc4UpdateReadings);
	DisplayPanel(dev->iPanel);
}

void itc4UpdateReadings(int panel, void* ptr)
{
	rs232Ptr dev = ptr;
	itc4Ptr itc = dev->device;
	itc4_GetSensor1(itc->channels[0]);
	itc4_GetSensor2(itc->channels[1]);
	itc4_GetSensor3(itc->channels[2]);
	SetCtrlVal(panel, ITC4_CTRL_SENS1, itc->channels[0]->reading);
	SetCtrlVal(panel, ITC4_CTRL_SENS2, itc->channels[1]->reading);
	SetCtrlVal(panel, ITC4_CTRL_SENS3, itc->channels[2]->reading);
	if(itc->heater == 1)
	{
		SetCtrlVal(panel, ITC4_CTRL_CONTROL, itc->heater1.on);
		SetCtrlVal(panel, ITC4_CTRL_HEATER_LED, itc->heater1.on); 
	}
	else
	{
		SetCtrlVal(panel, ITC4_CTRL_CONTROL, itc->heater2.on);
		SetCtrlVal(panel, ITC4_CTRL_HEATER_LED, itc->heater2.on); 
	}
	
	SetCtrlVal(panel, ITC4_CTRL_HEATER, itc->heater);
	if(itc->meas)
	{
		SetCtrlVal(itc->meas, MEASURE_SENS1MEAS, itc->channels[0]->reading);
		SetCtrlVal(itc->meas, MEASURE_SENS2MEAS, itc->channels[1]->reading);
		SetCtrlVal(itc->meas, MEASURE_SENS3MEAS, itc->channels[2]->reading);
	}
}

void itc4_Save(rs232Ptr dev)
{
	itc4Ptr itc = dev->device;
	
	FmtFile(fileHandle.analysis, "Heater1   :%i, %i\n", itc->heater1.on, itc->heater1.setp);
	FmtFile(fileHandle.analysis, "Heater pid:%f, %f, %f\n", itc->pid.p.val, itc->pid.i.val, itc->pid.d.val);
	source_Save(itc->source);
	acqchan_Save(itc->channels[0]);
	acqchan_Save(itc->channels[1]);
	acqchan_Save(itc->channels[2]);
}

void itc4_Load(rs232Ptr dev)
{
	itc4Ptr itc = dev? dev->device : NULL;
	if(itc)
	{
		ScanFile(fileHandle.analysis, "Heater1   :%i, %i\n", &itc->heater1.on, &itc->heater1.setp);
		ScanFile(fileHandle.analysis, "Heater pid:%f, %f, %f\n", &itc->pid.p.val, &itc->pid.i.val, &itc->pid.d.val);
		source_Load(dev, itc->source);
		acqchan_Load(dev, itc->channels[0]);
		acqchan_Load(dev, itc->channels[1]);
		acqchan_Load(dev, itc->channels[2]);
	}
}

void itc4_Init(void)
{
	rsDevTypePtr devType;
	if(utilG.acq.status != ACQ_NONE)
	{
		util_ChangeInitMessage("oxford itc...");
		devType = malloc(sizeof(rsDevTypeItem));
		if(devType)
		{
			Fmt(devType->label, "oxford itc");
			Fmt(devType->id, ITC4_ID);
			devType->CreateDevice = 	itc4_Create;
			devType->RemoveDevice = 	itc4_Remove;
			devType->InitDevice = 		itc4_InitIO;
			devType->SaveDevice = 		itc4_Save;
			devType->LoadDevice = 		itc4_Load;
			devType->OperateDevice = 	itc4Operate;
			devType->UpdateReadings = 	itc4UpdateReadings;
			rsDevTypeItem_AddItem(devType);
		}
	}
}


