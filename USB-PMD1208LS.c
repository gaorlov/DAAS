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
#include "digitalSource.h"
#include "USB-PMD1208LS.h"
#include "USB-1208LSu.h"


#define TRUE 1
#define FALSE 0
#define PMD1208LS_ID "USB-1208LS"
#define PMD1208LS_ID_ALT "PMD-1208LS"

typedef struct{
	portPtr aiPorts[4]; //  AnIn 0-3, AnOut 2,dIn 4, dOut 6;
	portPtr aoPorts[3];
	portPtr diPort, doPort;
	int panel, menu, *bits;
	rangePtr *ranges;
}usb1208lsType;
typedef usb1208lsType *usb1208lsPtr;


/*******************************************************************/
int		usb1208ls_PanelCallback				(int panel, int event, void *callbackData, int eventData1, int eventData2);
int		usb1208ls_ControlCallback 			(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int		usb1208ls_MeasureControlCallback	(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int		usb1208ls_IndexFromRange			(int val);
void    usb1208ls_MenuCallback				(int menubar, int menuItem, void *callbackData, int panel); 
void    usb1208ls_UpdateMeasurePanel		(portPtr port);
int		usb1208ls_IndexFromRange			(int val);

void	usb1208ls_ReadAnalogueOut			(acqchanPtr acqchan);
void	usb1208ls_SetLevel					(sourcePtr src);		 
void	usb1208ls_ReadFeedback				(acqchanPtr acqchan);
void	usb1208ls_ReadDigitalFeedback		(acqchanPtr acqchan);

void	usb1208ls_Create			(MCCdevPtr dev);
void	usb1208ls_Remove			(void *ptr);
void	usb1208lsOperate			(int menubar, int menuItem, void *callbackData, int panel);
void	usb1208ls_UpdateReadings	(int panel, void *ptr);
void	usb1208ls_Save				(MCCdevPtr dev);
void	usb1208ls_Load				(MCCdevPtr dev);
void	usb1208ls_Init				(void);


/*******************************************************************/

int	usb1208ls_PanelCallback (int panel, int event, void *callbackData, int eventData1, int eventData2)
{
	usb1208lsPtr pmd = callbackData;
	if((event == EVENT_KEYPRESS && eventData1 == VAL_ESC_VKEY) || event == EVENT_RIGHT_DOUBLE_CLICK)
	{
		MCCdevPtr dev = pmd->aiPorts[0]->port.analogueIOport.IO.acqchan->dev;
		int i;
		for (i = 0; i < 4; i++)
			if(pmd->aiPorts[i]->measPanel)
				HidePanel( pmd->aiPorts[i]->measPanel);
		HidePanel(panel);
		devPanel_Remove(panel);
		SetMenuBarAttribute (acquire_GetMenuBar(), dev->menuitem_id, ATTR_DIMMED, FALSE);
	}
	return 0;
}

int usb1208ls_ControlCallback (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	portPtr port = callbackData;
	switch (control)
	{
		case USB1208LS_RANGE:
			if(event == EVENT_COMMIT && port)
				GetCtrlVal(panel, control, &port->port.analogueIOport.range);
			break;
		case USB1208LS_INPUT:
			if(event == EVENT_COMMIT && port)
			{
				int i;
				MCCdevPtr dev = port->port.analogueIOport.IO.acqchan->dev;
				usb1208lsPtr pmd = dev->device;
				GetCtrlVal(panel, control, &i);
				port->control = 0;
				if(i != -1)
				{
					pmd->aiPorts[i]->control = control;
					SetCtrlAttribute (panel, control, ATTR_CALLBACK_DATA, pmd->aiPorts[i]);
					SetCtrlAttribute (panel, USB1208LS_RANGE, ATTR_CALLBACK_DATA, pmd->aiPorts[i]);
					SetCtrlAttribute (panel, USB1208LS_ACQ, ATTR_CALLBACK_DATA, pmd->aiPorts[i]);
					SetCtrlAttribute (panel, USB1208LS_NUMERIC, ATTR_CALLBACK_DATA, pmd->aiPorts[i]);
					SetCtrlVal (panel, USB1208LS_NUMERIC, pmd->aiPorts[i]->averaging);
					SetCtrlIndex (panel, USB1208LS_RANGE, usb1208ls_IndexFromRange(pmd->aiPorts[i]->port.analogueIOport.range));
					SetCtrlVal (panel, USB1208LS_ACQ, pmd->aiPorts[i]->port.analogueIOport.IO.acqchan->acquire);
				}
				SetCtrlAttribute (panel, USB1208LS_RANGE, ATTR_DIMMED, (i == -1));
				SetCtrlAttribute (panel, USB1208LS_ACQ, ATTR_DIMMED, (i == -1));
				if(i == -1)SetCtrlVal (panel, USB1208LS_ACQ, 0);
			}
			break;
		case USB1208LS_ACQ:
			if(event == EVENT_COMMIT && port && utilG.acq.status != ACQ_BUSY)
			{
				int i;
				MCCdevPtr dev = port->port.analogueIOport.IO.acqchan->dev;
				usb1208lsPtr pmd = dev->device;
				GetCtrlIndex (panel, USB1208LS_INPUT, &i);
				if(i)
				{
					GetCtrlVal (panel, control, &port->port.analogueIOport.IO.acqchan->acquire);
					if (port->port.analogueIOport.IO.acqchan->acquire)
						acqchanlist_AddChannel(port->port.analogueIOport.IO.acqchan);
					else
						acqchanlist_RemoveChannel(port->port.analogueIOport.IO.acqchan);
					SetMenuBarAttribute (pmd->menu, port->menuitem_id, ATTR_CHECKED, port->port.analogueIOport.IO.acqchan->acquire);
				}
			}
			else if(event == EVENT_COMMIT && utilG.acq.status == ACQ_BUSY)
				SetCtrlVal (panel, control, port->port.analogueIOport.IO.acqchan->acquire);
			break;
		/*case USB1208LS_DIGITAL_OUT_0:
		case USB1208LS_DIGITAL_OUT_1:
		case USB1208LS_DIGITAL_OUT_2:
		case USB1208LS_DIGITAL_OUT_3:
		case USB1208LS_DIGITAL_OUT_4:
		case USB1208LS_DIGITAL_OUT_5:
			if(event == EVENT_COMMIT && port)
			{
				MCCdevPtr dev = port->port.digitalIOport.IO.source->acqchan->dev;
				usb1208lsPtr pmd = dev->device;
				GetCtrlVal (panel, USB1208LS_DIGITAL_OUT_0, &pmd->bits[0]);
				GetCtrlVal (panel, USB1208LS_DIGITAL_OUT_1, &pmd->bits[1]);
				GetCtrlVal (panel, USB1208LS_DIGITAL_OUT_2, &pmd->bits[2]);
				GetCtrlVal (panel, USB1208LS_DIGITAL_OUT_3, &pmd->bits[3]);
				GetCtrlVal (panel, USB1208LS_DIGITAL_OUT_4, &pmd->bits[4]);
				GetCtrlVal (panel, USB1208LS_DIGITAL_OUT_5, &pmd->bits[5]);
				pmd->bits[1] *= 2;
				pmd->bits[2] *= 4;
				pmd->bits[3] *= 8;
				pmd->bits[4] *= 16;
				pmd->bits[5] *= 32;
				port->port.digitalIOport.IO.source->biaslevel = pmd->bits[0] + pmd->bits[1] + pmd->bits[2] + pmd->bits[3] + pmd->bits[4] + pmd->bits[5];
				port->port.digitalIOport.IO.source->SetLevel(port->port.digitalIOport.IO.source);
			}
			break;*/
		case USB1208LS_NUMERIC:
			if(event == EVENT_COMMIT && port)
				GetCtrlVal (panel, control, &port->averaging);
			break;
	}
	if (event == EVENT_COMMIT && port)
		usb1208ls_UpdateMeasurePanel(port);
	return 0;
}

int  usb1208ls_MeasureControlCallback (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
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
				usb1208lsPtr pmd = dev->device;
				GetCtrlVal(panel, control, acqchan->channel->label);
				acqchanlist_ReplaceChannel(acqchan);
				SetPanelAttribute(panel, ATTR_TITLE, acqchan->channel->label);
				SetMenuBarAttribute (pmd->menu, port->menuitem_id, ATTR_ITEM_NAME,
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
				usb1208lsPtr pmd = dev->device;
				GetCtrlVal(panel, control, &acqchan->acquire);
				if (acqchan->acquire) acqchanlist_AddChannel(acqchan);
				else acqchanlist_RemoveChannel(acqchan);
				SetMenuBarAttribute (pmd->menu, port->menuitem_id, ATTR_CHECKED, acqchan->acquire);
			}
			else if(event == EVENT_COMMIT && utilG.acq.status == ACQ_BUSY)
				SetCtrlVal (panel, control, acqchan->acquire);
			break;
	}
	return 0;
}

void usb1208ls_MenuCallback(int menubar, int menuItem, void *callbackData, int panel)
{
	switch (menuItem)
	{
		case USBMENU_MEAS_IN_0:
		case USBMENU_MEAS_IN_1:
		case USBMENU_MEAS_IN_2:
		case USBMENU_MEAS_IN_3:
		
			if(utilG.acq.status != ACQ_BUSY){
				portPtr port = callbackData;
				MCCdevPtr dev = port->port.analogueIOport.IO.acqchan->dev;
				usb1208lsPtr pmd = dev->device;
				port->measPanel = port->measPanel? port->measPanel : LoadPanel(utilG.p, "USB-1208LSu.uir", MEASURE);
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
		case USBMENU_SOURCE_DAC1:
		case USBMENU_SOURCE_DAC2:
		case USBMENU_SOURCE_DAC3:
			{
				sourcePtr src = callbackData;
				switch(utilG.exp)
				{
					case EXP_SOURCE: source_InitPanel(src); break;
					case EXP_FLOAT : gensrc_InitPanel(src); break;
				}
			}
			break;
		case USBMENU_SOURCE_DAC4:
			{
				portPtr port = callbackData;
				boards_DigitalSourceInit(port);
			}
			break;
		case USBMENU_FILE_SAVE:
			boards_Save(callbackData, usb1208ls_Save);
			break;
		case USBMENU_FILE_LOAD:
			boards_Load(callbackData, usb1208ls_Load);
			break;
	}
}

void usb1208ls_UpdateMeasurePanel(portPtr port)
{
	if(port->measPanel)
	{
		SetCtrlVal (port->measPanel, MEASURE_RANGE, port->port.analogueIOport.range);
		SetCtrlVal (port->measPanel, MEASURE_LABEL, port->port.analogueIOport.IO.acqchan->channel->label);
		SetCtrlVal (port->measPanel, MEASURE_COEFF, port->port.analogueIOport.IO.acqchan->coeff);
		SetCtrlVal (port->measPanel, MEASURE_ACQ, port->port.analogueIOport.IO.acqchan->acquire);
	}
}

int  usb1208ls_IndexFromRange (int val)
{
	int range;
	switch(val)
	{
		case BIP20VOLTS:	range = 0;break;
		case BIP10VOLTS:	range = 1;break;  
		case BIP5VOLTS:		range = 2;break;
		case BIP4VOLTS: 	range = 3;break;
		case BIP2PT5VOLTS:	range = 4;break;
	}
	return range;
}

/*******************************************************************/
void usb1208ls_ReadAnalogueOut (acqchanPtr acqchan)
{
	portPtr up = acqchan->upLvl;
	MCCdevPtr dev = acqchan->dev;
	sourcePtr src = up->port.analogueIOport.IO.source;
	float temp;
	unsigned short reading;
	cbFromEngUnits(dev->BoardNum, up->port.analogueIOport.range, src->biaslevel, &reading);
	cbToEngUnits (dev->BoardNum, up->port.analogueIOport.range, reading, &temp);
	acqchan->reading = (double)temp * 4;   // due to bug in MCC!!!
	acqchan->newreading = TRUE;
}
/*
void usb1208ls_SetLevel_old(sourcePtr src)
{
	MCCdevPtr dev = src->acqchan->dev;
	int i;
	usb1208lsPtr pmd = dev->device;
	if (utilG.acq.status == ACQ_BUSY)
	{
		pmd->doPort->port.digitalIOport.bitarr[0].val = 0;
		pmd->doPort->port.digitalIOport.bitarr[1].val = 0;
		pmd->doPort->port.digitalIOport.IO.source->biaslevel = 0;
		for(i = 0; i < pmd->doPort->port.digitalIOport.bits; i ++)
			pmd->doPort->port.digitalIOport.IO.source->biaslevel += pmd->doPort->port.digitalIOport.bitarr[i].val * pow(2.0, (double)i);// binary 0000 stop
		pmd->doPort->port.digitalIOport.IO.source->SetLevel(pmd->doPort->port.digitalIOport.IO.source);
		pmd->diPort->port.digitalIOport.IO.acqchan->GetReading(pmd->diPort->port.digitalIOport.IO.acqchan);
		if(pmd->panel)
		{
			SetCtrlVal (pmd->panel, USB1208LS_DIGITAL_IN_0, ((int)pmd->diPort->port.digitalIOport.IO.acqchan->reading % 4 % 2));
			SetCtrlVal (pmd->panel, USB1208LS_DIGITAL_IN_1, ((int)pmd->diPort->port.digitalIOport.IO.acqchan->reading % 4 / 2));
			ProcessSystemEvents(); //updates panel within function runtime
		}
	}
	SetAnalogue(src);
	if (utilG.acq.status == ACQ_BUSY)	
	{
		
		if(src->acqchan->reading < src->biaslevel)	//+other bits binary 0010 ramp up
		{
			pmd->doPort->port.digitalIOport.bitarr[0].val = 0;
			pmd->doPort->port.digitalIOport.bitarr[1].val = 1;
		}
		else	//binary 0001 ramp down
		{
			pmd->doPort->port.digitalIOport.bitarr[0].val = 1;
			pmd->doPort->port.digitalIOport.bitarr[1].val = 0;
		}
		pmd->doPort->port.digitalIOport.IO.source->biaslevel = 0;
		for(i = 0; i < pmd->doPort->port.digitalIOport.bits; i ++)
			pmd->doPort->port.digitalIOport.IO.source->biaslevel += pmd->doPort->port.digitalIOport.bitarr[i].val * pow(2.0, (double)i);
		pmd->doPort->port.digitalIOport.IO.source->SetLevel(pmd->doPort->port.digitalIOport.IO.source);
	}
}		 //*/


void usb1208ls_SetLevel(sourcePtr src)
{
	MCCdevPtr dev = src->acqchan->dev;
	usb1208lsPtr pmd = dev->device;
	SetAnalogue(src);
	if(src->segments[src->seg]->start > src->segments[src->seg]->stop)
	{   //set ramp down (A0 = 1 A1 = 0)
		int biaslevel = (int)(pmd->doPort->port.digitalIOport.IO.source->biaslevel);
		pmd->doPort->port.digitalIOport.bitarr[0].val = 1;
		pmd->doPort->port.digitalIOport.bitarr[1].val = 0;
		biaslevel |= 0x1; //biaslevel |= 0...000001 sets A0 to 1
		biaslevel &= 0xFFFFFFFD;//biaslevel &= 1...1111101 sets A1 to 0
		pmd->doPort->port.digitalIOport.IO.source->biaslevel = (double)biaslevel;
	}
	else
	{	//set ramp up
		int biaslevel = (int)(pmd->doPort->port.digitalIOport.IO.source->biaslevel);
		pmd->doPort->port.digitalIOport.bitarr[0].val = 0;
		pmd->doPort->port.digitalIOport.bitarr[1].val = 1;
		biaslevel |= 0x2;// biaslevel |= 0...00010 sets A1 to 1
		biaslevel &= 0xFFFFFFFE;//biaslevel &= 1...1111110 sets A0 to 0
		pmd->doPort->port.digitalIOport.IO.source->biaslevel = (double)biaslevel;
	}
	pmd->doPort->port.digitalIOport.IO.source->SetLevel(pmd->doPort->port.digitalIOport.IO.source);
}


void usb1208ls_ReadFeedback (acqchanPtr acqchan)
{
	MCCdevPtr dev = acqchan->dev;
	float temp;
	unsigned short reading;
	cbAIn (dev->BoardNum, 	0, //channel
				BIP10VOLTS, 
				&reading);
	cbToEngUnits (dev->BoardNum, BIP10VOLTS, reading, &temp);
//	acqchan->reading = src->biaslevel ;
	
	acqchan->reading = (double)temp;
	acqchan->newreading = TRUE;
/*
	acqchan->reading HAS to be the same units and magnitude as the source->biaslevel, or it has to be converted
//*/
}

void usb1208ls_ReadDigitalFeedback(acqchanPtr acqchan)
{
	MCCdevPtr dev = acqchan->dev;
	usb1208lsPtr pmd = dev->device;
	sourcePtr src = pmd->aoPorts[2]->port.analogueIOport.IO.source;
	acqchanPtr acqchanD = pmd->diPort->port.digitalIOport.IO.acqchan;
	acqchan->newreading = TRUE;
	if(src->segments[src->seg]->des_start > src->segments[src->seg]->des_stop)
	{   //if ramp down A6 is 1 until limit is reached, at which point the sweep needs to stop
		int reading;
		ReadDigital(acqchanD);
		reading = (int)acqchanD->reading;
		if(reading & 1) 
		{
			acqchan->reading = src->biaslevel - src->segments[src->seg]->error.val-1;
			if(src->segments[src->seg]->pt == (src->segments[src->seg]->points-1))
				acqchan->reading = src->biaslevel; 		//this allows the stepping to continue after the lastp point
		}											  	//in the down sweep because there the source resets the bits
													  	//thus making the "limit reached" be 1 instead of 0 again
		else									  
		{	//setting ramp to go up (A0 = 0 A1 = 1)
			int biaslevel = (int)(pmd->doPort->port.digitalIOport.IO.source->biaslevel);
			if(src->segments[src->seg]->stop && (src->segments[src->seg]->pt == (src->segments[src->seg]->points-1)))
			pmd->doPort->port.digitalIOport.bitarr[0].val = 1;
			pmd->doPort->port.digitalIOport.bitarr[1].val = 0;
			biaslevel |= 0x2; //biaslevel |= 0...0000010 sets A1 to 1
			biaslevel &= 0x0FFFFFFE;//biaslevel &= 1...1111110 sets A0 to 0
			pmd->doPort->port.digitalIOport.IO.source->biaslevel = (double)biaslevel;
			pmd->doPort->port.digitalIOport.IO.source->SetLevel(pmd->doPort->port.digitalIOport.IO.source);
			acqchan->reading = src->biaslevel;
		}
	}
	else 
	{   //if ramp is going up. once the limit is reached, A6 is set to 1 and the sweep stops
		int reading;
		ReadDigital(acqchanD);
		reading = (int)acqchanD->reading;
		if(reading & 1) 
			acqchan->reading = src->biaslevel;
		else
			acqchan->reading = src->biaslevel - src->segments[src->seg]->error.val-1;
	}
}
	
/*******************************************************************/
/*
create_Port(*dev, *name, type, direction, GetReading, port, devtype, bits;)
create_Port(*dev, *name, type, direction, GetReading, SetLevel, port, devtype, bits)
create_Port(*dev, *name, type, direction, GetReading, channel, range)
create_Port(*dev, *name, type, direction, GetReading, SetLevel, channel, range)
//*/																 
void usb1208ls_Create (MCCdevPtr dev)
{
	usb1208lsPtr pmd = malloc(sizeof(usb1208lsType));
	int i;
	char name[30];
	dev->device = pmd;
	pmd->ranges = calloc(1, sizeof(rangeType));
	pmd->bits = calloc(6, sizeof(int));
	pmd->panel = 0;
	pmd->ranges[0] = range_Create (5, 0, 5.0/1024.0);
	for (i = 0; i < 4; i ++)
	{
		Fmt(name, "analog IN %i", i);
		pmd->aiPorts[i] = create_Port (dev, name, ANALOGUE, IN_PORT, ReadAnalogue, i, BIP10VOLTS);
	}
	for (i = 0; i < 2; i ++)
	{
		Fmt(name, "analog OUT %i", i);
		pmd->aoPorts[i] = create_Port (dev, name, ANALOGUE, OUT_PORT, usb1208ls_ReadAnalogueOut, SetAnalogue, i, UNI5VOLTS);
		pmd->aoPorts[i]->port.analogueIOport.IO.source->ranges.temprange[0] = pmd->ranges[0];
		pmd->aoPorts[i]->port.analogueIOport.IO.source->min = 0;
		pmd->aoPorts[i]->port.analogueIOport.IO.source->max = 5;
	}
	pmd->aoPorts[1]->port.analogueIOport.IO.source->SetLevel = SetAnalogue;
	
	Fmt(name, "IPS100 ramp");
	pmd->aoPorts[2] = create_Port (dev, name, ANALOGUE, OUT_PORT, usb1208ls_ReadDigitalFeedback, usb1208ls_SetLevel, 0, UNI5VOLTS);
	pmd->aoPorts[2]->port.analogueIOport.IO.source->ranges.temprange[0] = pmd->ranges[0];
	pmd->aoPorts[2]->port.analogueIOport.IO.source->min = 0;
	pmd->aoPorts[2]->port.analogueIOport.IO.source->max = 5;
	pmd->aoPorts[2]->port.analogueIOport.IO.source->segments[0]->error.on = 1;
	
	Fmt(name, "digital OUT %i", i);
	pmd->doPort = create_Port (dev, name, DIGITAL, OUT_PORT, usb1208ls_ReadFeedback, SetDigital, FIRSTPORTA, 0, 8);
	Fmt(name, "digital IN %i", i);
	pmd->diPort = create_Port (dev, name, DIGITAL, IN_PORT, ReadDigital, FIRSTPORTB, 0, 0);
	pmd->aiPorts[0]->menuitem_id = USBMENU_MEAS_IN_0;
	pmd->aiPorts[1]->menuitem_id = USBMENU_MEAS_IN_1;
	pmd->aiPorts[2]->menuitem_id = USBMENU_MEAS_IN_2;
	pmd->aiPorts[3]->menuitem_id = USBMENU_MEAS_IN_3;
}

void usb1208ls_Remove(void *ptr)
{
	int i;
	MCCdevPtr dev = ptr;
	usb1208lsPtr pmd = dev->device;
	if(pmd->panel)
	{
		devPanel_Remove(pmd->panel);
		DiscardPanel(pmd->panel);
	}
	for ( i = 0; i < 4; i ++)
		acqchan_Remove(pmd->aiPorts[i]->port.analogueIOport.IO.acqchan);
	for ( i = 0; i < 3; i ++)
	{
		if(pmd->aoPorts[i]->port.analogueIOport.IO.source->panel) DiscardPanel(pmd->aoPorts[i]->port.analogueIOport.IO.source->panel);
		source_Remove (pmd->aoPorts[i]->port.analogueIOport.IO.source);
	}
	acqchan_Remove(pmd->diPort->port.digitalIOport.IO.acqchan);
	if(pmd->doPort->port.analogueIOport.IO.source->panel)DiscardPanel(pmd->doPort->port.analogueIOport.IO.source->panel);
	source_Remove (pmd->doPort->port.digitalIOport.IO.source);
	free(pmd);
}

void usb1208lsOperate (int menubar, int menuItem, void *callbackData, int panel)
{
	MCCdevPtr dev = callbackData;
	usb1208lsPtr pmd = dev->device;
	acqchanPtr acqchan;
	int i, x = 0, m;
	if(!pmd->panel)
	{
		pmd->panel = LoadPanel(utilG.p, "USB-1208LSu.uir", USB1208LS);
		pmd->menu = LoadMenuBar(pmd->panel, "USB-1208LSu.uir", USBMENU);
		SetPanelMenuBar(pmd->panel, pmd->menu);
		for(i = 0; i < 4; i ++)
		{
			SetMenuBarAttribute (pmd->menu, pmd->aiPorts[i]->menuitem_id, ATTR_CALLBACK_DATA, pmd->aiPorts[i]);
			SetMenuBarAttribute (pmd->menu, pmd->aiPorts[i]->menuitem_id, ATTR_ITEM_NAME, pmd->aiPorts[i]->port.analogueIOport.IO.acqchan->channel->label);
			SetMenuBarAttribute (pmd->menu, pmd->aiPorts[i]->menuitem_id, ATTR_CHECKED, pmd->aiPorts[i]->port.analogueIOport.IO.acqchan->acquire);
		}
		SetMenuBarAttribute (pmd->menu, USBMENU_SOURCE_DAC1, ATTR_CALLBACK_DATA, pmd->aoPorts[0]->port.analogueIOport.IO.source);
		SetMenuBarAttribute (pmd->menu, USBMENU_SOURCE_DAC2, ATTR_CALLBACK_DATA, pmd->aoPorts[1]->port.analogueIOport.IO.source);
		SetMenuBarAttribute (pmd->menu, USBMENU_SOURCE_DAC3, ATTR_CALLBACK_DATA, pmd->aoPorts[2]->port.analogueIOport.IO.source);
		SetMenuBarAttribute (pmd->menu, USBMENU_SOURCE_DAC4, ATTR_CALLBACK_DATA, pmd->doPort);
		SetMenuBarAttribute (pmd->menu, USBMENU_SOURCE_DAC1, ATTR_ITEM_NAME, pmd->aoPorts[0]->port.analogueIOport.IO.source->acqchan->channel->label);
		SetMenuBarAttribute (pmd->menu, USBMENU_SOURCE_DAC2, ATTR_ITEM_NAME, pmd->aoPorts[1]->port.analogueIOport.IO.source->acqchan->channel->label);
		SetMenuBarAttribute (pmd->menu, USBMENU_SOURCE_DAC3, ATTR_ITEM_NAME, pmd->aoPorts[2]->port.analogueIOport.IO.source->acqchan->channel->label);
		
		SetMenuBarAttribute (pmd->menu, USBMENU_FILE_SAVE, ATTR_CALLBACK_DATA, dev);
		SetMenuBarAttribute (pmd->menu, USBMENU_FILE_LOAD, ATTR_CALLBACK_DATA, dev);
		
		SetCtrlAttribute (pmd->panel, USB1208LS_RANGE, ATTR_DIMMED, 1);
		SetCtrlAttribute (pmd->panel, USB1208LS_ACQ, ATTR_DIMMED, 1);
		
		/*SetCtrlAttribute (pmd->panel, USB1208LS_DIGITAL_OUT_0, ATTR_CALLBACK_DATA, pmd->doPort);
		SetCtrlAttribute (pmd->panel, USB1208LS_DIGITAL_OUT_1, ATTR_CALLBACK_DATA, pmd->doPort);
		SetCtrlAttribute (pmd->panel, USB1208LS_DIGITAL_OUT_2, ATTR_CALLBACK_DATA, pmd->doPort);
		SetCtrlAttribute (pmd->panel, USB1208LS_DIGITAL_OUT_3, ATTR_CALLBACK_DATA, pmd->doPort);
		SetCtrlAttribute (pmd->panel, USB1208LS_DIGITAL_OUT_4, ATTR_CALLBACK_DATA, pmd->doPort);     		
		SetCtrlAttribute (pmd->panel, USB1208LS_DIGITAL_OUT_5, ATTR_CALLBACK_DATA, pmd->doPort);*/     		
		SetCtrlAttribute (pmd->panel, USB1208LS_INPUT, ATTR_CALLBACK_DATA, pmd->aiPorts[0]);
		SetPanelAttribute (pmd->panel, ATTR_CALLBACK_DATA, pmd);
	}
	SetMenuBarAttribute(menubar, menuItem, ATTR_DIMMED, 1);
	devPanel_Add (pmd->panel, dev, usb1208ls_UpdateReadings);
	DisplayPanel(pmd->panel);
}

void usb1208ls_UpdateReadings (int panel, void *ptr)
{
	int i;
	MCCdevPtr dev = ptr;
	usb1208lsPtr pmd = dev->device;
	acqchanPtr acqchan;
	for(i = 0; i < 4; i++)
	{
		acqchan = pmd->aiPorts[i]->port.analogueIOport.IO.acqchan;
		acqchan->GetReading(acqchan);
		if(pmd->aiPorts[i]->control)	
		{
			SetCtrlVal (pmd->panel, USB1208LS_ANALOGUE_IN, acqchan->reading);
			SetCtrlVal (pmd->panel, USB1208LS_RANGE, pmd->aiPorts[i]->port.analogueIOport.range);
			SetCtrlVal (pmd->panel, USB1208LS_ACQ, acqchan->acquire);
		}
	}
/*	SetCtrlVal (pmd->panel, USB1208LS_DIGITAL_OUT_0, ((int)pmd->doPort->port.digitalIOport.IO.source->biaslevel % 2));
	SetCtrlVal (pmd->panel, USB1208LS_DIGITAL_OUT_1, ((int)pmd->doPort->port.digitalIOport.IO.source->biaslevel / 2 % 2));
	SetCtrlVal (pmd->panel, USB1208LS_DIGITAL_OUT_2, ((int)pmd->doPort->port.digitalIOport.IO.source->biaslevel / 4 % 2));
	SetCtrlVal (pmd->panel, USB1208LS_DIGITAL_OUT_3, ((int)pmd->doPort->port.digitalIOport.IO.source->biaslevel / 8 % 2));
	SetCtrlVal (pmd->panel, USB1208LS_DIGITAL_OUT_4, ((int)pmd->doPort->port.digitalIOport.IO.source->biaslevel / 16 % 2));
	SetCtrlVal (pmd->panel, USB1208LS_DIGITAL_OUT_5, ((int)pmd->doPort->port.digitalIOport.IO.source->biaslevel / 32 %2));
	*/
	pmd->diPort->port.digitalIOport.IO.acqchan->GetReading(pmd->diPort->port.digitalIOport.IO.acqchan);
	SetCtrlVal (pmd->panel, USB1208LS_DIGITAL_IN_0, ((int)pmd->diPort->port.digitalIOport.IO.acqchan->reading % 4 % 2));
	//SetCtrlVal (pmd->panel, USB1208LS_DIGITAL_IN_1, ((int)pmd->diPort->port.digitalIOport.IO.acqchan->reading % 4 / 2));
}

void usb1208ls_Save (MCCdevPtr dev)
{
	int i;
	
	usb1208lsPtr pmd = dev->device;
	for ( i = 0; i < 4; i++)
		port_Save (pmd->aiPorts[i]);
	for ( i = 0; i < 3; i ++)
		port_Save (pmd->aoPorts[i]);
	port_Save (pmd->diPort);
	port_Save (pmd->doPort);
}

void usb1208ls_Load (MCCdevPtr dev)
{
	int i;
	usb1208lsPtr pmd = dev->device;
	for ( i = 0; i < 4; i++)
	{
		port_Load (dev, pmd->aiPorts[i]);
		SetMenuBarAttribute (pmd->menu, pmd->aiPorts[i]->menuitem_id, ATTR_CHECKED, pmd->aiPorts[i]->port.analogueIOport.IO.acqchan->acquire); 
	}
	for ( i = 0; i < 3; i ++)
	{	
		port_Load (dev, pmd->aoPorts[i]);
		pmd->aoPorts[i]->port.analogueIOport.IO.source->acqchan->reading = pmd->aoPorts[i]->port.analogueIOport.IO.source->biaslevel;
		usb1208ls_SetLevel(pmd->aoPorts[i]->port.analogueIOport.IO.source);
		
	}
	port_Load (dev, pmd->diPort);
	port_Load (dev, pmd->doPort);
}

void usb1208ls_Init(void)
{
	MCCdevTypePtr devType = malloc(sizeof(MCCdevTypeItem));
	MCCdevTypePtr devType_alt = malloc(sizeof(MCCdevTypeItem));
	if(utilG.acq.status != ACQ_NONE)
	{
		util_ChangeInitMessage ("usb-usb1208...");
		if(devType)
		{
			Fmt(devType->id, PMD1208LS_ID);
			Fmt(devType->label, "usb-1208ls");
			devType->CreateDevice 	= usb1208ls_Create;
			devType->RemoveDevice 	= usb1208ls_Remove;
			devType->OperateDevice	= usb1208lsOperate;
			devType->UpdateReadings	= usb1208ls_UpdateReadings;
			devType->SaveDevice		= usb1208ls_Save;
			devType->LoadDevice		= usb1208ls_Load;
			boards_DevTypeList_AddDev(devType);
		}
		if(devType_alt)
		{
			Fmt(devType_alt->id, PMD1208LS_ID_ALT);
			Fmt(devType_alt->label, "usb-1208ls");
			devType_alt->CreateDevice 	= devType->CreateDevice;
			devType_alt->RemoveDevice 	= devType->RemoveDevice;
			devType_alt->OperateDevice	= devType->OperateDevice;
			devType_alt->UpdateReadings	= devType->UpdateReadings;
			devType_alt->SaveDevice		= devType->SaveDevice;
			devType_alt->LoadDevice		= devType->LoadDevice;
			boards_DevTypeList_AddDev(devType_alt);
		}
	}
}
