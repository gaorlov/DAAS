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
#include "HP33120A.h"
#include "HP33120Au.h"

#define TRUE 1
#define FALSE 0
#define HP33120A_ID "hewlett-packard"
/*
Specifications:
Amp: 50mV - 10V

Sin: 100uHz - 15MHz
Squ: 100uHz - 15MHz
Tri: 100uHz - 100KHz

Duty:
100uHz - 5MHz: 20 - 80%
5MHz - 15MHz : 40 - 60%

*/
typedef enum{SIN, SQU, TRI}waveforms;

typedef enum{FREQ, AMPL} hp3312aCh;
typedef struct{
	sourcePtr sources[2];
	int id;
	double duty;
	char wave[256];
}hp33120aType;

typedef hp33120aType *hp33120aPtr;

/****************************************************************/

void SetHP3312aFreqLvl (sourcePtr src);
void GetHP3312aFreqLvl (acqchanPtr acqchan);
void SetHP3312aAmplLvl (sourcePtr src);
void GetHP3312aAmplLvl (acqchanPtr acqchan);

void hp33120a_UpdateControls(int p, gpibioPtr dev);
int hp33120aControlCallback (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int hp33120aPanelCallback (int panel, int event, void *callbackData, int eventData1, int eventData2);
void hp33120aMenuCallback (int menuBar, int menuItem, void *callbackData, int panel);

void *hp33120a_Create(gpibioPtr dev);
int  hp33120a_InitGPIB(gpibioPtr dev);
void Operatehp33120a (int menubar, int menuItem, void *callbackData, int panel);
void hp33120a_UpdateReadings(int panel, void *ptr);
void hp33120a_Save(gpibioPtr dev);
void hp33120a_Load(gpibioPtr dev);
void hp33120a_Remove(void *ptr);
void hp33120a_Init(void);





/****************************************************************/
void SetHP33120aFreqLvl (sourcePtr src)
{
	gpibioPtr dev = src->acqchan->dev;
	hp33120aPtr hp = dev->device;
	char cmd[256];
	
	Fmt(cmd, "APPLY:%s %f, %f\n", hp->wave, src->biaslevel, hp->sources[AMPL]->acqchan->reading);
	gpibio_Out (dev, cmd);
    util_Delay (src->segments[src->seg]->delay);
}								 

void GetHP33120aFreqLvl (acqchanPtr acqchan)
{
	acqchan->reading = gpib_GetDoubleVal(acqchan->dev, "SOURCE:FREQ?");
}

void SetHP33120aAmplLvl (sourcePtr src)
{
	gpibioPtr dev = src->acqchan->dev;
	hp33120aPtr hp = dev->device;
	char cmd[256];
	
	Fmt(cmd, "APPLY:%s %f,%f\n", hp->wave, hp->sources[FREQ]->acqchan->reading, src->biaslevel);
	gpibio_Out (dev, cmd);
    util_Delay (src->segments[src->seg]->delay);
}

void GetHP33120aAmplLvl (acqchanPtr acqchan)
{
	acqchan->reading = gpib_GetDoubleVal(acqchan->dev, "SOURCE:VOLTAGE?");
}

/****************************************************************/
void hp33120a_UpdateControls(int p, gpibioPtr dev)
{
	hp33120aPtr hp = dev->device;
	
	gpib_GetCharVal(dev, "SOURCE:FUNC:SHAPE?", hp->wave);
	Fmt(hp->wave, "%s[w3]", hp->wave);
	SetCtrlVal(p, HP33120A_WAVE, hp->wave);
	hp->sources[FREQ]->min = .001;
	if(!strcmp(hp->wave, "SIN") || !strcmp(hp->wave, "SQU"))
		hp->sources[FREQ]->max = 15000000;
	else
		hp->sources[FREQ]->max = 1000000;
	hp->sources[FREQ]->freq = 1;
	hp->sources[AMPL]->freq = 1;
	
	hp->sources[FREQ]->acqchan->reading = gpib_GetDoubleVal(dev, "SOURCE:FREQ?");
	SetCtrlVal(p, HP33120A_FREQ, hp->sources[FREQ]->acqchan->reading);
	
	hp->sources[AMPL]->acqchan->reading = gpib_GetDoubleVal(dev, "SOURCE:VOLTAGE?");
	SetCtrlVal(p, HP33120A_AMPL, hp->sources[AMPL]->acqchan->reading);
	
	hp->duty = gpib_GetDoubleVal(dev, "SOURCE:PULSE:DCYCLE?");
	SetCtrlVal(p, HP33120A_DUTY, hp->duty);
}

int hp33120aControlCallback (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	gpibioPtr dev = callbackData;
	hp33120aPtr hp = dev->device;
	char msg[60];
	switch (control)
	{
		case HP33120A_DUTY:
		if(event == EVENT_COMMIT)
		{
			GetCtrlVal(panel, control, &hp->duty);
			Fmt(msg, "SOUR:PULS:DCYC %f\n", hp->duty);
			gpibio_Out(dev, msg);
		}
		break;
		case HP33120A_WAVE:
		if(event == EVENT_COMMIT)
		{
			GetCtrlVal(panel, control, hp->wave);
			Fmt(msg, "APPLY:%s\n", hp->wave);
			gpibio_Out(dev, msg);
		}
		break;
		
	}
	return 0;
}
		  
int hp33120aPanelCallback (int panel, int event, void *callbackData, int eventData1, int eventData2)
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

void hp33120aMenuCallback (int menuBar, int menuItem, void *callbackData, int panel)
{
	switch(menuItem)
	{
		case HP3312MENU_SOURCES_FREQ:
		case HP3312MENU_SOURCES_AMPL:
			{
				sourcePtr src = callbackData;
				switch(utilG.exp)
				{
					case EXP_SOURCE: source_InitPanel(src); break;
					case EXP_FLOAT : gensrc_InitPanel(src); break;
				}
			}
			break;
	}
}

/****************************************************************/
void *hp33120a_Create(gpibioPtr dev)
{
	hp33120aPtr hp;
	hp = malloc(sizeof(hp33120aType));
	if(dev) hp->id = dev->id;
	
	hp->sources[FREQ] = source_Create("hp3312a frequency", dev, SetHP33120aFreqLvl, GetHP33120aFreqLvl);
	hp->sources[AMPL] = source_Create("hp3312a amplitude", dev, SetHP33120aAmplLvl, GetHP33120aAmplLvl);
	if(dev) dev->device = hp;
	return hp;
}

int  hp33120a_InitGPIB(gpibioPtr dev)
{
	gpibio_Remote(dev);
	if(gpibio_DeviceMatch(dev, "*IDN?", HP33120A_ID))
		return TRUE;
	return FALSE;
}

void Operatehp33120a (int menubar, int menuItem, void *callbackData, int panel)
{
	gpibioPtr dev = callbackData;
	hp33120aPtr hp = dev->device;
	char label[256];
	int p, m;
	
	hp->sources[FREQ]->min = .001;
	hp->sources[AMPL]->min = .05;
	hp->sources[AMPL]->max = 10;
	
	SetMenuBarAttribute(menubar, menuItem, ATTR_DIMMED, 1);
	
	p = dev->iPanel? dev->iPanel: LoadPanel(utilG.p, "HP33120Au.uir", HP33120A);
    dev->iPanel = p;
	
	SetPanelAttribute(p, ATTR_TITLE, dev->label);
	
	m = LoadMenuBar(p, "HP33120Au.uir", HP3312MENU);
	SetPanelMenuBar(p, m);
	
	SetMenuBarAttribute(m, HP3312MENU_SOURCES_FREQ, ATTR_CALLBACK_DATA, hp->sources[FREQ]);
	SetMenuBarAttribute(m, HP3312MENU_SOURCES_AMPL, ATTR_CALLBACK_DATA, hp->sources[AMPL]);
	
	SetPanelAttribute(p, ATTR_CALLBACK_DATA, dev);
	
	SetCtrlAttribute(p, HP33120A_FREQ, ATTR_CALLBACK_DATA, dev);
	SetCtrlAttribute(p, HP33120A_AMPL, ATTR_CALLBACK_DATA, dev);
	SetCtrlAttribute(p, HP33120A_DUTY, ATTR_CALLBACK_DATA, dev);
	SetCtrlAttribute(p, HP33120A_WAVE, ATTR_CALLBACK_DATA, dev);
	SetCtrlVal(p, HP33120A_GPIB, dev->paddr);
	
	hp33120a_UpdateControls(p, dev);
	devPanel_Add(p, dev, hp33120a_UpdateReadings);
	
	DisplayPanel(p);
}

void hp33120a_UpdateReadings(int panel, void *ptr)
{
	gpibioPtr dev = ptr;
	hp33120aPtr hp = dev->device;
	
	SetCtrlVal(panel, HP33120A_FREQ, hp->sources[FREQ]->acqchan->reading);
	SetCtrlVal(panel, HP33120A_AMPL, hp->sources[AMPL]->acqchan->reading);
}

void hp33120a_Save(gpibioPtr dev)
{
	hp33120aPtr hp;
	hp = dev->device;
	
	FmtFile (fileHandle.analysis, "Wave : %s\n", hp->wave);
	FmtFile (fileHandle.analysis, "Duty : %f\n", hp->duty);
	FmtFile (fileHandle.analysis, "Freq : %f\n", hp->sources[FREQ]->acqchan->reading);
	FmtFile (fileHandle.analysis, "Ampl : %f\n", hp->sources[AMPL]->acqchan->reading);
	source_Save(hp->sources[FREQ]);
	source_Save(hp->sources[AMPL]);
}

void hp33120a_Load(gpibioPtr dev)
{
	hp33120aPtr hp;
	char msg[256];
	double r;
	
	if(dev)
	{
		hp = dev->device;
		ScanFile(fileHandle.analysis, "Wave : %s", hp->wave);
		ScanFile(fileHandle.analysis, "Duty : %f", &r); hp->duty = r;
		ScanFile(fileHandle.analysis, "Freq : %f", &r); hp->sources[FREQ]->acqchan->reading = r;
		ScanFile(fileHandle.analysis, "Ampl : %f", &r); hp->sources[AMPL]->acqchan->reading = r;
		Fmt(msg, "APPLY:%s %f, %f\n",hp->wave, 
			hp->sources[FREQ]->acqchan->reading,
			hp->sources[AMPL]->acqchan->reading);
		gpibio_Out(dev, msg);
		Fmt(msg, "SOUR:PULS:DCYC %f\n",hp->duty); gpibio_Out(dev, msg);
		source_Load(dev, hp->sources[FREQ]);
		source_Load(dev, hp->sources[AMPL]);
		
		if(!strcmp(hp->wave, "SIN") || !strcmp(hp->wave, "SQU"))
			hp->sources[FREQ]->max = 15000000;
		else
			hp->sources[FREQ]->max = 100000;
		hp->sources[FREQ]->min = .001;
		hp->sources[AMPL]->min = .05;
		hp->sources[AMPL]->max = 10;
		hp->sources[FREQ]->freq = 1;
		hp->sources[AMPL]->freq = 1;
	}
}
	  
void hp33120a_Remove(void *ptr)
{
	hp33120aPtr hp = ptr;
	int i;
	for(i = 0; i < 2; i++)
		source_Remove(hp->sources[i]);
	free(hp);
}


void hp33120a_Init(void)
{
	devTypePtr devType;
	if(utilG.acq.status != ACQ_NONE)
	{
		util_ChangeInitMessage("hp33120a control utilities...");
		devType = malloc(sizeof(devTypeItem));
		if (devType)
		{
			Fmt(devType->label, "hewlett packard 33120a");
			Fmt(devType->id, HP33120A_ID);
			devType->CreateDevice = hp33120a_Create;
			devType->InitDevice = hp33120a_InitGPIB;
			devType->OperateDevice = Operatehp33120a;
			devType->UpdateReadings = hp33120a_UpdateReadings;
			devType->SaveDevice = hp33120a_Save;
			devType->LoadDevice = hp33120a_Load;
			devType->RemoveDevice = hp33120a_Remove;
			devTypeList_AddItem(devType);
		}
	}
}
