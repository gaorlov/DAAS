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
#include "das-1602.h"
#include "das-1602u.h"

#define TRUE 1
#define FALSE 0
#define DAS1602_ID "PCI-DAS1602/16"

typedef struct{
	portPtr Achannels[10];
	int panel, menu;
}das1602Type;
typedef das1602Type *das1602Ptr;
/***********************************************************************************/
int  das1602_PanelCallback 			(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  das1602_ControlCallback 		(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void das1602_MenuCallback			(int menbar, int menuItem, void *callbackData, int panel);
int  das1602_MeasureControlCallback (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  das1602_IndexFromRange 		(int val);

void das1602_UpdateMeasurePanel(portPtr port);

void das1602_Create		(MCCdevPtr dev);
void das1602_Remove			(void* ptr);
int  das1602_InitIO			(MCCdevPtr dev);
void das1602Operate			(int menubar, int menuItem, void *callbackData, int panel);
void das1602_UpdateReadings (int panel, void *ptr);
void das1602_Save			(MCCdevPtr dev);
void das1602_Load			(MCCdevPtr dev);
void das1602_Init			(void);





/***********************************************************************************/
int das1602_PanelCallback (int panel, int event, void *callbackData, int eventData1, int eventData2)
{
	das1602Ptr das = callbackData;
	if((event == EVENT_KEYPRESS && eventData1 == VAL_ESC_VKEY) || event == EVENT_RIGHT_DOUBLE_CLICK)
	{
		MCCdevPtr dev = das->Achannels[0]->port.analogueIOport.IO.acqchan->dev;
		int i;
		for (i = 0; i < 8; i ++)
			if(das->Achannels[i]->measPanel)
				HidePanel(das->Achannels[i]->measPanel);
		devPanel_Remove(panel);
		HidePanel(panel);
		SetMenuBarAttribute (acquire_GetMenuBar(), dev->menuitem_id, ATTR_DIMMED, FALSE);
	}
	return 0;
}

int das1602_ControlCallback (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	portPtr port = callbackData;
	switch (control)
	{
		case DAS_CTRL_RANGE:
			if(event == EVENT_COMMIT && port)
				GetCtrlVal(panel, control, &port->port.analogueIOport.range);
			break;
		case DAS_CTRL_INPUT:
			if(event == EVENT_COMMIT && port)
			{
				int i;
				MCCdevPtr dev = port->port.analogueIOport.IO.acqchan->dev;
				das1602Ptr das = dev->device;
				GetCtrlVal(panel, control, &i);
				port->control = 0;
				if(i != -1)
				{
					das->Achannels[i]->control = control;
					SetCtrlAttribute (panel, control, ATTR_CALLBACK_DATA, das->Achannels[i]);
					SetCtrlAttribute (panel, DAS_CTRL_RANGE, ATTR_CALLBACK_DATA, das->Achannels[i]);
					SetCtrlAttribute (panel, DAS_CTRL_ACQ, ATTR_CALLBACK_DATA, das->Achannels[i]);
					SetCtrlIndex (panel, DAS_CTRL_RANGE, das1602_IndexFromRange(das->Achannels[i]->port.analogueIOport.range));
					SetCtrlVal (panel, DAS_CTRL_ACQ, das->Achannels[i]->port.analogueIOport.IO.acqchan->acquire);
				}
				SetCtrlAttribute (panel, DAS_CTRL_RANGE, ATTR_DIMMED, (i == -1));
				SetCtrlAttribute (panel, DAS_CTRL_ACQ, ATTR_DIMMED, (i == -1));
				if(i == -1)SetCtrlVal (panel, DAS_CTRL_ACQ, 0);
			}
			break;
		case DAS_CTRL_ACQ:
		if(event == EVENT_COMMIT && port && utilG.acq.status != ACQ_BUSY)
		{
			int i;
			MCCdevPtr dev = port->port.analogueIOport.IO.acqchan->dev;
			das1602Ptr das = dev->device;
			GetCtrlIndex (panel, DAS_CTRL_INPUT, &i);
			if(i)
			{
				GetCtrlVal (panel, control, &port->port.analogueIOport.IO.acqchan->acquire);
					if (port->port.analogueIOport.IO.acqchan->acquire)
					acqchanlist_AddChannel(port->port.analogueIOport.IO.acqchan);
				else
					acqchanlist_RemoveChannel(port->port.analogueIOport.IO.acqchan);
				SetMenuBarAttribute (das->menu, port->menuitem_id, ATTR_CHECKED, port->port.analogueIOport.IO.acqchan->acquire);
			}
		else if(event == EVENT_COMMIT && utilG.acq.status == ACQ_BUSY)
			SetCtrlVal (panel, control, port->port.analogueIOport.IO.acqchan->acquire);
			
		}
		break;
	}
	if(event == EVENT_COMMIT && port)
		das1602_UpdateMeasurePanel (port);
	return 0;
}

void das1602_MenuCallback(int menbar, int menuItem, void *callbackData, int panel)
{
	switch (menuItem)
	{
		case DASMENU_MEAS_IN_0:
		case DASMENU_MEAS_IN_1:
		case DASMENU_MEAS_IN_2:
		case DASMENU_MEAS_IN_3:
		case DASMENU_MEAS_IN_4:
		case DASMENU_MEAS_IN_5:
		case DASMENU_MEAS_IN_6:
		case DASMENU_MEAS_IN_7:
			if(utilG.acq.status != ACQ_BUSY){
				portPtr port = callbackData;
				MCCdevPtr dev = port->port.analogueIOport.IO.acqchan->dev;
				das1602Ptr das = dev->device;
				port->menuitem_id = menuItem;
				port->measPanel = port->measPanel? port->measPanel : LoadPanel(utilG.p, "DAS-1602u.uir", MEASURE);
				SetCtrlAttribute (port->measPanel, MEASURE_RANGE, ATTR_CALLBACK_DATA, port->port.analogueIOport.IO.acqchan);
				SetCtrlAttribute (port->measPanel, MEASURE_LABEL, ATTR_CALLBACK_DATA, port->port.analogueIOport.IO.acqchan);
				SetCtrlAttribute (port->measPanel, MEASURE_COEFF, ATTR_CALLBACK_DATA, port->port.analogueIOport.IO.acqchan);
				SetCtrlAttribute (port->measPanel, MEASURE_NOTE, ATTR_CALLBACK_DATA, port->port.analogueIOport.IO.acqchan);
				SetCtrlAttribute (port->measPanel, MEASURE_ACQ, ATTR_CALLBACK_DATA, port->port.analogueIOport.IO.acqchan);
				SetPanelAttribute (port->measPanel, ATTR_TITLE, port->port.analogueIOport.IO.acqchan->channel->label);
				SetCtrlVal (port->measPanel, MEASURE_LABEL, port->port.analogueIOport.IO.acqchan->channel->label);
				SetCtrlVal (port->measPanel, MEASURE_RANGE, port->port.analogueIOport.range);
				SetCtrlVal (port->measPanel, MEASURE_COEFF, port->port.analogueIOport.IO.acqchan->coeff);
				SetCtrlVal (port->measPanel, MEASURE_NOTE, port->port.analogueIOport.IO.acqchan->note);
				SetCtrlVal (port->measPanel, MEASURE_ACQ, port->port.analogueIOport.IO.acqchan->acquire);
				DisplayPanel (port->measPanel);
			}
			break;
		case DASMENU_SOURCE_DAC1:
		case DASMENU_SOURCE_DAC2:
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

int  das1602_MeasureControlCallback (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	acqchanPtr acqchan = callbackData;
	switch (control)
	{
		case MEASURE_RANGE:
			if (event == EVENT_COMMIT)
			{
				portPtr port = acqchan->upLvl;
				GetCtrlVal (panel, control, &port->port.analogueIOport.range);
			}
			break;
		case MEASURE_LABEL:
			if (event == EVENT_COMMIT)
			{
				portPtr port = acqchan->upLvl;
				MCCdevPtr dev = acqchan->dev;
				das1602Ptr das = dev->device;
				GetCtrlVal(panel, control, acqchan->channel->label);
				acqchanlist_ReplaceChannel(acqchan);
				SetPanelAttribute(panel, ATTR_TITLE, acqchan->channel->label);
				SetMenuBarAttribute (das->menu, port->menuitem_id, ATTR_ITEM_NAME,
									 acqchan->channel->label);
			}
			break;
		case MEASURE_COEFF:
			if (event == EVENT_COMMIT)
				GetCtrlVal(panel, control, &acqchan->coeff);
			break;
		case MEASURE_ACQ:
			if (event == EVENT_COMMIT && utilG.acq.status != ACQ_BUSY)
			{
				portPtr port = acqchan->upLvl;
				MCCdevPtr dev = acqchan->dev;
				das1602Ptr das = dev->device;
				GetCtrlVal(panel, control, &acqchan->acquire);
				if (acqchan->acquire) acqchanlist_AddChannel(acqchan);
				else acqchanlist_RemoveChannel(acqchan);
				SetMenuBarAttribute (das->menu, port->menuitem_id, ATTR_CHECKED, acqchan->acquire);
			}
			else if(event == EVENT_COMMIT && utilG.acq.status == ACQ_BUSY)
				SetCtrlVal (panel, control, acqchan->acquire);
			break;
	}
	return 0;
}

int  das1602_IndexFromRange (int val)
{
	int range;
	switch(val)
	{
		case BIP10VOLTS:	range = 0;break;
		case BIP5VOLTS:		range = 1;break;
		case BIP2PT5VOLTS:	range = 2;break;
		case BIP1PT25VOLTS:	range = 3;break;
	}
	return range;
}

void das1602_UpdateMeasurePanel(portPtr port)
{
	if(port->measPanel)
	{
		SetCtrlVal (port->measPanel, MEASURE_RANGE, port->port.analogueIOport.range);
		SetCtrlVal (port->measPanel, MEASURE_LABEL, port->port.analogueIOport.IO.acqchan->channel->label);
		SetCtrlVal (port->measPanel, MEASURE_COEFF, port->port.analogueIOport.IO.acqchan->coeff);
		SetCtrlVal (port->measPanel, MEASURE_ACQ, port->port.analogueIOport.IO.acqchan->acquire);
	}
}

/***********************************************************************************/
void das1602_Create (MCCdevPtr dev)
{
	int i;
	char name[50];
	das1602Ptr das = malloc(sizeof(das1602Type));
	
	dev->device = das;
	das->panel = 0;
	for(i = 0; i < 8; i++)
	{
		//portPtr create_Port(*dev, *name, type, direction, GetReading, channel, range)
		Fmt (name, "PCI-DAS1602 analog in %i", i);
		das->Achannels[i] = create_Port(dev, name, ANALOGUE, IN_PORT, ReadAnalogue, i, BIP10VOLTS);
		das->Achannels[i]->control = 0;
	}
	for(i = 0; i < 2; i++)
	{
		//portPtr create_Port(*dev, *name, type, direction, GetReading, SetLevel, channel, range)
		Fmt (name, "PCI-DAS1602 analog out %i", i);
		das->Achannels[i+8] = create_Port(dev, name, ANALOGUE, OUT_PORT, ReadAnalogueOut, SetAnalogue, i, BIP10VOLTS);
		das->Achannels[i+8]->port.analogueIOport.IO.source->max = 10;
		das->Achannels[i+8]->port.analogueIOport.IO.source->min = -10;
		das->Achannels[i+8]->port.analogueIOport.IO.source->ranges.temprange[0] = range_Create(10, -10, 0.000305);
	}
	das->Achannels[0]->menuitem_id = DASMENU_MEAS_IN_0;
	das->Achannels[1]->menuitem_id = DASMENU_MEAS_IN_1;
	das->Achannels[2]->menuitem_id = DASMENU_MEAS_IN_2;
	das->Achannels[3]->menuitem_id = DASMENU_MEAS_IN_3;
	das->Achannels[4]->menuitem_id = DASMENU_MEAS_IN_4;
	das->Achannels[5]->menuitem_id = DASMENU_MEAS_IN_5;
	das->Achannels[6]->menuitem_id = DASMENU_MEAS_IN_6;
	das->Achannels[7]->menuitem_id = DASMENU_MEAS_IN_7;
}

void das1602_Remove (void* ptr)
{
	MCCdevPtr dev = ptr;
	das1602Ptr das = dev->device;
	int i;
	if(das->panel)
	{
		devPanel_Remove(das->panel);
		DiscardPanel(das->panel);
	}
	for(i = 0; i < 8; i++)
		acqchan_Remove(das->Achannels[i]->port.analogueIOport.IO.acqchan);
	for(i = 8; i < 10; i++)
		source_Remove (das->Achannels[i]->port.analogueIOport.IO.source);
	free(das);
}

void das1602Operate (int menubar, int menuItem, void *callbackData, int panel)
{
	MCCdevPtr dev = callbackData;
	das1602Ptr das = dev->device;
	acqchanPtr acqchan;
	int i, x = 0, m;
	if(!das->panel)
	{
		das->panel = LoadPanel(utilG.p, "DAS-1602u.uir", DAS_CTRL);
		das->menu = LoadMenuBar(das->panel, "DAS-1602u.uir", DASMENU);
		SetPanelMenuBar(das->panel, das->menu);
		for(i = 0; i < 8; i ++)
		{
			SetMenuBarAttribute (das->menu, das->Achannels[i]->menuitem_id, ATTR_CALLBACK_DATA, das->Achannels[i]);
			SetMenuBarAttribute (das->menu, das->Achannels[i]->menuitem_id, ATTR_ITEM_NAME, das->Achannels[i]->port.analogueIOport.IO.acqchan->channel->label);
			SetMenuBarAttribute (das->menu, das->Achannels[i]->menuitem_id, ATTR_CHECKED, das->Achannels[i]->port.analogueIOport.IO.acqchan->acquire);
		}
		SetMenuBarAttribute (das->menu, DASMENU_SOURCE_DAC1, ATTR_CALLBACK_DATA, das->Achannels[8]->port.analogueIOport.IO.source);
		SetMenuBarAttribute (das->menu, DASMENU_SOURCE_DAC2, ATTR_CALLBACK_DATA, das->Achannels[9]->port.analogueIOport.IO.source);
		SetMenuBarAttribute (das->menu, DASMENU_SOURCE_DAC1, ATTR_ITEM_NAME, das->Achannels[8]->port.analogueIOport.IO.source->acqchan->channel->label);
		SetMenuBarAttribute (das->menu, DASMENU_SOURCE_DAC2, ATTR_ITEM_NAME, das->Achannels[9]->port.analogueIOport.IO.source->acqchan->channel->label);
		SetCtrlAttribute (das->panel, DAS_CTRL_RANGE, ATTR_DIMMED, 1);
		SetCtrlAttribute (das->panel, DAS_CTRL_ACQ, ATTR_DIMMED, 1);
		
		SetCtrlAttribute (das->panel, DAS_CTRL_INPUT, ATTR_CALLBACK_DATA, das->Achannels[0]);
		SetPanelAttribute (das->panel, ATTR_CALLBACK_DATA, das);
	}
	SetMenuBarAttribute(menubar, menuItem, ATTR_DIMMED, 1);
	devPanel_Add (das->panel, dev, das1602_UpdateReadings);
	DisplayPanel(das->panel);
}

void das1602_UpdateReadings (int panel, void *ptr)
{
	int i;
	MCCdevPtr dev = ptr;
	das1602Ptr das = dev->device;
	acqchanPtr acqchan;
	for(i = 0; i < 8; i++)
	{
		acqchan = das->Achannels[i]->port.analogueIOport.IO.acqchan;
		ReadAnalogue(acqchan);
		if(das->Achannels[i]->control)	
		{
			SetCtrlVal (das->panel, DAS_CTRL_ANALOGUE_IN, acqchan->reading);
			SetCtrlVal (das->panel, DAS_CTRL_RANGE, das->Achannels[i]->port.analogueIOport.range);
			SetCtrlVal (das->panel, DAS_CTRL_ACQ, das->Achannels[i]->port.analogueIOport.IO.acqchan->acquire);
		}
	}
}

void das1602_Save (MCCdevPtr dev)
{
	int i;
	das1602Ptr das = dev->device;
	for(i = 0; i < 10; i++)
		port_Save(das->Achannels[i]);
}

void das1602_Load (MCCdevPtr dev)
{
	int i;
	das1602Ptr das = dev->device;
	for(i = 0; i < 10; i++)
		port_Load(dev, das->Achannels[i]);
	for(i = 8; i < 10; i++)
	{
		das->Achannels[i]->port.analogueIOport.IO.source->max = 10;
		das->Achannels[i]->port.analogueIOport.IO.source->min = -10;
		das->Achannels[i]->port.analogueIOport.IO.source->ranges.range[0] = range_Create(10, -10, 0.000305);
	}
}

void das1602_Init(void)
{
	MCCdevTypePtr devType = malloc(sizeof(MCCdevTypeItem));
	if(utilG.acq.status != ACQ_NONE)
	{
		util_ChangeInitMessage ("pci-das1602...");
		if(devType)
		{
			Fmt(devType->id, DAS1602_ID);
			Fmt(devType->label, "pci das1602");
			devType->CreateDevice 	= das1602_Create;
			devType->RemoveDevice 	= das1602_Remove;
			devType->OperateDevice	= das1602Operate;
			devType->UpdateReadings	= das1602_UpdateReadings;
			devType->SaveDevice		= das1602_Save;
			devType->LoadDevice		= das1602_Load;
			boards_DevTypeList_AddDev(devType);
		}
	}
}

