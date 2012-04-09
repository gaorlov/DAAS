#include <userint.h>
#include <ansi_c.h>
#include <cbw.h>
#include <formatio.h>
#include <utility.h>

#include "util.h"
#include "list.h"
#include "channel.h"
#include "changen.h"
#include "chanfnc.h"
#include "chanops.h"
#include "acqchan.h"
#include "curve.h"
#include "acqcrv.h"
#include "graph.h"
#include "gpibio.h"
#include "curveop.h"
#include "acquire.h"
#include "acquireu.h"
#include "source.h"
#include "MCCdevices.h"
#include "dda08.h"
#include "DDA08u.h"

#define TRUE 1
#define FALSE 0
#define DDA08_ID "PCI-DDA08/16"

typedef struct{
	int range, analog_out, source;
}onoffType;
typedef onoffType *onoffPtr;
typedef struct{
	portPtr Achannels[8];
	int panel;
	rangePtr range[3];
	onoffPtr buttons[8];
}dda08Type;
typedef dda08Type *dda08Ptr;
/***********************************************************************************/
onoffPtr onoff_Create(int range, int analog_out, int source);
int  dda08_PanelCallback		(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  dda08_ControlCallback 		(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  dda08_IndexFromRange 		(int val);

void ReadAnalogue				(acqchanPtr acqchan);
void ReadAnalogueOut 			(acqchanPtr acqchan);
void SetAnalogue 				(sourcePtr src);
unsigned short DAC_conversion	(double Value);

void dda08_Create		(MCCdevPtr dev);
void dda08_Remove			(void* ptr);
int  dda08_InitIO			(MCCdevPtr dev);
void dda08Operate			(int menubar, int menuItem, void *callbackData, int panel);
void dda08_UpdateReadings (int panel, void *ptr);
void dda08_Save			(MCCdevPtr dev);
void dda08_Load			(MCCdevPtr dev);
void dda08_Init			(void);





/***********************************************************************************/
onoffPtr onoff_Create(int range, int analog_out, int source)
{
	onoffPtr on = malloc(sizeof(onoffType));
	on->range = range;
	on->analog_out = analog_out;
	on->source = source;
	return on;
}

int dda08_ControlCallback (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	int i;
	switch (control)
	{
		case DDA_CTRL_OUT_ON_0:
		case DDA_CTRL_OUT_ON_1:
		case DDA_CTRL_OUT_ON_2:
		case DDA_CTRL_OUT_ON_3:
		case DDA_CTRL_OUT_ON_4:
		case DDA_CTRL_OUT_ON_5:
		case DDA_CTRL_OUT_ON_6:
		case DDA_CTRL_OUT_ON_7:
			if(event == EVENT_COMMIT)
			{
				onoffPtr local = callbackData;
				GetCtrlVal(panel, control, &i);
				SetCtrlAttribute (panel, local->range, ATTR_DIMMED, !i);
				SetCtrlAttribute (panel, local->analog_out, ATTR_DIMMED, !i);
				SetCtrlAttribute (panel, local->source, ATTR_DIMMED, !i);
			}
			break;
		case DDA_CTRL_RANGE_0:
		case DDA_CTRL_RANGE_1:
		case DDA_CTRL_RANGE_2:
		case DDA_CTRL_RANGE_3:
		case DDA_CTRL_RANGE_4:
		case DDA_CTRL_RANGE_5:
		case DDA_CTRL_RANGE_6:
		case DDA_CTRL_RANGE_7:
			if (event == EVENT_COMMIT)
			{
				int *range = callbackData;
				GetCtrlVal (panel, control, range);
			}
			break;
		case DDA_CTRL_SOURCE_0:
		case DDA_CTRL_SOURCE_1:
		case DDA_CTRL_SOURCE_2:
		case DDA_CTRL_SOURCE_3:
		case DDA_CTRL_SOURCE_4:
		case DDA_CTRL_SOURCE_5:
		case DDA_CTRL_SOURCE_6:
		case DDA_CTRL_SOURCE_7:
			if (event == EVENT_COMMIT)
			{
				sourcePtr src = callbackData;
				portPtr port = src->acqchan->upLvl;
				MCCdevPtr dev = src->acqchan->dev;
				dda08Ptr dda = dev->device;
				src->ranges.range[0] = dda->range[port->port.analogueIOport.range];
				switch(utilG.exp)
				{
					case EXP_SOURCE: source_InitPanel(src); break;
					case EXP_FLOAT : gensrc_InitPanel(src); break;
				}
			}
			break;
		case DDA_CTRL_ANALOG_OUT_0:
		case DDA_CTRL_ANALOG_OUT_1:
		case DDA_CTRL_ANALOG_OUT_2:
		case DDA_CTRL_ANALOG_OUT_3:
		case DDA_CTRL_ANALOG_OUT_4:
		case DDA_CTRL_ANALOG_OUT_5:
		case DDA_CTRL_ANALOG_OUT_6:
		case DDA_CTRL_ANALOG_OUT_7:
			if(event == EVENT_COMMIT)
			{
				sourcePtr src = callbackData;
				portPtr port = src->acqchan->upLvl;
				MCCdevPtr dev = src->acqchan->dev;
				dda08Ptr dda = dev->device;
				SetCtrlAttribute (panel, control, ATTR_MAX_VALUE, dda->range[port->port.analogueIOport.range]->maxVal);
				SetCtrlAttribute (panel, control, ATTR_MIN_VALUE, dda->range[port->port.analogueIOport.range]->maxVal);
				GetCtrlVal (panel, control, &src->biaslevel);
				if(src->SetLevel) src->SetLevel(src);
				if(src->acqchan->GetReading) src->acqchan->GetReading(src->acqchan);
				SetCtrlVal (panel, control, src->acqchan->reading);
			}
			break;
	}
	return 0;
}

int  dda08_IndexFromRange (int val)
{
	int range;
	switch(val)
	{
		case BIP10VOLTS:	range = 0;break;
		case BIP5VOLTS:		range = 1;break;
		case BIP2PT5VOLTS:	range = 2;break; 
	}
	return range;
}


/***********************************************************************************/
void dda08_Create (MCCdevPtr dev)
{
	int i;
	char name[50];
	dda08Ptr dda = malloc(sizeof(dda08Type));
	
	dev->device = dda;
	dda->panel = 0;
	for(i = 0; i < 8; i++)
	{
		//portPtr create_Port(*dev, *name, type, direction, GetReading, SetLevel, channel, range)
		Fmt (name, "DDA08 analog out %i", i);
		dda->Achannels[i] = create_Port(dev, name, ANALOGUE, OUT_PORT, ReadAnalogueOut, SetAnalogue, i, BIP10VOLTS);
		dda->Achannels[i]->port.analogueIOport.IO.source->max = 10;
		dda->Achannels[i]->port.analogueIOport.IO.source->min = -10;
		dda->Achannels[i]->port.analogueIOport.IO.source->ranges.temprange[0] = range_Create(10, -10, 0.000305);
	}
	dda->range[0] = range_Create(10, -10, 0.000305);
	dda->range[1] = range_Create(5, -5, 0.0001525);
	dda->range[2] = range_Create(2.5, -2.5, 0.00007625);
	dda->buttons[0] = onoff_Create(DDA_CTRL_RANGE_0, DDA_CTRL_ANALOG_OUT_0, DDA_CTRL_SOURCE_0);
	dda->buttons[1] = onoff_Create(DDA_CTRL_RANGE_1, DDA_CTRL_ANALOG_OUT_1, DDA_CTRL_SOURCE_1);
	dda->buttons[2] = onoff_Create(DDA_CTRL_RANGE_2, DDA_CTRL_ANALOG_OUT_2, DDA_CTRL_SOURCE_2);
	dda->buttons[3] = onoff_Create(DDA_CTRL_RANGE_3, DDA_CTRL_ANALOG_OUT_3, DDA_CTRL_SOURCE_3);
	dda->buttons[4] = onoff_Create(DDA_CTRL_RANGE_4, DDA_CTRL_ANALOG_OUT_4, DDA_CTRL_SOURCE_4);
	dda->buttons[5] = onoff_Create(DDA_CTRL_RANGE_5, DDA_CTRL_ANALOG_OUT_5, DDA_CTRL_SOURCE_5);
	dda->buttons[6] = onoff_Create(DDA_CTRL_RANGE_6, DDA_CTRL_ANALOG_OUT_6, DDA_CTRL_SOURCE_6);
	dda->buttons[7] = onoff_Create(DDA_CTRL_RANGE_7, DDA_CTRL_ANALOG_OUT_7, DDA_CTRL_SOURCE_7);
}

void dda08_Remove (void* ptr)
{
	MCCdevPtr dev = ptr;
	dda08Ptr dda = dev->device;
	int i;
	if(dda->panel)
	{
		devPanel_Remove(dda->panel);
		DiscardPanel(dda->panel);
	}
	for(i = 0; i < 8; i++)
		source_Remove (dda->Achannels[i]->port.analogueIOport.IO.source);
	free(dda);
}

void dda08Operate (int menubar, int menuItem, void *callbackData, int panel)
{
	MCCdevPtr dev = callbackData;
	dda08Ptr dda = dev->device;
	acqchanPtr acqchan;
	int i, x = 0, m;
	if(!dda->panel)
	{
		dda->panel = LoadPanel(utilG.p, "DDA08u.uir", DDA_CTRL);
		SetCtrlAttribute (dda->panel, DDA_CTRL_RANGE_0, ATTR_CALLBACK_DATA, &dda->Achannels[0]->port.analogueIOport.range);
		SetCtrlAttribute (dda->panel, DDA_CTRL_RANGE_1, ATTR_CALLBACK_DATA, &dda->Achannels[1]->port.analogueIOport.range);
		SetCtrlAttribute (dda->panel, DDA_CTRL_RANGE_2, ATTR_CALLBACK_DATA, &dda->Achannels[2]->port.analogueIOport.range);
		SetCtrlAttribute (dda->panel, DDA_CTRL_RANGE_3, ATTR_CALLBACK_DATA, &dda->Achannels[3]->port.analogueIOport.range);
		SetCtrlAttribute (dda->panel, DDA_CTRL_RANGE_4, ATTR_CALLBACK_DATA, &dda->Achannels[4]->port.analogueIOport.range);
		SetCtrlAttribute (dda->panel, DDA_CTRL_RANGE_5, ATTR_CALLBACK_DATA, &dda->Achannels[5]->port.analogueIOport.range);
		SetCtrlAttribute (dda->panel, DDA_CTRL_RANGE_6, ATTR_CALLBACK_DATA, &dda->Achannels[6]->port.analogueIOport.range);
		SetCtrlAttribute (dda->panel, DDA_CTRL_RANGE_7, ATTR_CALLBACK_DATA, &dda->Achannels[7]->port.analogueIOport.range);
		
		SetCtrlAttribute (dda->panel, DDA_CTRL_ANALOG_OUT_0, ATTR_CALLBACK_DATA, dda->Achannels[0]->port.analogueIOport.IO.source);
		SetCtrlAttribute (dda->panel, DDA_CTRL_ANALOG_OUT_1, ATTR_CALLBACK_DATA, dda->Achannels[1]->port.analogueIOport.IO.source);
		SetCtrlAttribute (dda->panel, DDA_CTRL_ANALOG_OUT_2, ATTR_CALLBACK_DATA, dda->Achannels[2]->port.analogueIOport.IO.source);
		SetCtrlAttribute (dda->panel, DDA_CTRL_ANALOG_OUT_3, ATTR_CALLBACK_DATA, dda->Achannels[3]->port.analogueIOport.IO.source);
		SetCtrlAttribute (dda->panel, DDA_CTRL_ANALOG_OUT_4, ATTR_CALLBACK_DATA, dda->Achannels[4]->port.analogueIOport.IO.source);
		SetCtrlAttribute (dda->panel, DDA_CTRL_ANALOG_OUT_5, ATTR_CALLBACK_DATA, dda->Achannels[5]->port.analogueIOport.IO.source);
		SetCtrlAttribute (dda->panel, DDA_CTRL_ANALOG_OUT_6, ATTR_CALLBACK_DATA, dda->Achannels[6]->port.analogueIOport.IO.source);
		SetCtrlAttribute (dda->panel, DDA_CTRL_ANALOG_OUT_7, ATTR_CALLBACK_DATA, dda->Achannels[7]->port.analogueIOport.IO.source);
		
		SetCtrlAttribute (dda->panel, DDA_CTRL_SOURCE_0, ATTR_CALLBACK_DATA, dda->Achannels[0]->port.analogueIOport.IO.source);
		SetCtrlAttribute (dda->panel, DDA_CTRL_SOURCE_1, ATTR_CALLBACK_DATA, dda->Achannels[1]->port.analogueIOport.IO.source);
		SetCtrlAttribute (dda->panel, DDA_CTRL_SOURCE_2, ATTR_CALLBACK_DATA, dda->Achannels[2]->port.analogueIOport.IO.source);
		SetCtrlAttribute (dda->panel, DDA_CTRL_SOURCE_3, ATTR_CALLBACK_DATA, dda->Achannels[3]->port.analogueIOport.IO.source);
		SetCtrlAttribute (dda->panel, DDA_CTRL_SOURCE_4, ATTR_CALLBACK_DATA, dda->Achannels[4]->port.analogueIOport.IO.source);
		SetCtrlAttribute (dda->panel, DDA_CTRL_SOURCE_5, ATTR_CALLBACK_DATA, dda->Achannels[5]->port.analogueIOport.IO.source);
		SetCtrlAttribute (dda->panel, DDA_CTRL_SOURCE_6, ATTR_CALLBACK_DATA, dda->Achannels[6]->port.analogueIOport.IO.source);
		SetCtrlAttribute (dda->panel, DDA_CTRL_SOURCE_7, ATTR_CALLBACK_DATA, dda->Achannels[7]->port.analogueIOport.IO.source);
		
		SetCtrlAttribute (dda->panel, DDA_CTRL_OUT_ON_0, ATTR_CALLBACK_DATA, dda->buttons[0]);
		SetCtrlAttribute (dda->panel, DDA_CTRL_OUT_ON_1, ATTR_CALLBACK_DATA, dda->buttons[1]);
		SetCtrlAttribute (dda->panel, DDA_CTRL_OUT_ON_2, ATTR_CALLBACK_DATA, dda->buttons[2]);
		SetCtrlAttribute (dda->panel, DDA_CTRL_OUT_ON_3, ATTR_CALLBACK_DATA, dda->buttons[3]);
		SetCtrlAttribute (dda->panel, DDA_CTRL_OUT_ON_4, ATTR_CALLBACK_DATA, dda->buttons[4]);
		SetCtrlAttribute (dda->panel, DDA_CTRL_OUT_ON_5, ATTR_CALLBACK_DATA, dda->buttons[5]);
		SetCtrlAttribute (dda->panel, DDA_CTRL_OUT_ON_6, ATTR_CALLBACK_DATA, dda->buttons[6]);
		SetCtrlAttribute (dda->panel, DDA_CTRL_OUT_ON_7, ATTR_CALLBACK_DATA, dda->buttons[7]);
		
	}
	DisplayPanel(dda->panel);
}

void dda08_Save (MCCdevPtr dev)
{
	int i;
	dda08Ptr dda = dev->device;
	for(i = 0; i < 8; i++)
		port_Save(dda->Achannels[i]);
}

void dda08_Load (MCCdevPtr dev)
{
	int i;
	dda08Ptr dda = dev->device;
	for(i = 0; i < 8; i++)
	{
		port_Load(dev, dda->Achannels[i]);
		dda->Achannels[i]->port.analogueIOport.IO.source->max = 10;
		dda->Achannels[i]->port.analogueIOport.IO.source->min = -10;
		dda->Achannels[i]->port.analogueIOport.IO.source->ranges.range[0] = range_Create(10, -10, 0.000305);
	}
}

void dda08_Init(void)
{
	MCCdevTypePtr devType = malloc(sizeof(MCCdevTypeItem));
	if(utilG.acq.status != ACQ_NONE)
	{
		util_ChangeInitMessage ("pci-dda08...");
		if(devType)
		{
			Fmt(devType->id, DDA08_ID);
			Fmt(devType->label, "pci dda08");
			devType->CreateDevice 	= dda08_Create;
			devType->RemoveDevice 	= dda08_Remove;
			devType->OperateDevice	= dda08Operate;
			devType->UpdateReadings	= NULL;
			devType->SaveDevice		= dda08_Save;
			devType->LoadDevice		= dda08_Load;
			boards_DevTypeList_AddDev(devType);
		}
	}
}

