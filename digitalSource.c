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
#include "digitalSourceu.h"

#define FALSE 0
#define TRUE 1

void	boards_DigitalSourceInit				(portPtr port);
int		boards_DigitalSourceControlCallback		(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int		boards_DigitalSourcePanelCallback		(int panel, int event, void *callbackData, int eventData1, int eventData2);
int		boards_DigitalSourceFeedbakCallback		(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void	boards_DigitalSourceUpdate				(int panel, void *ptr);

void boards_DigitalSourceInit (portPtr port)
{
	int i;
	if(!port->port.digitalIOport.measPanel)
	{
		port->port.digitalIOport.measPanel = LoadPanel(utilG.p, "digitalSourceu.uir", DIGITALSRC);
		SetPanelAttribute (port->port.digitalIOport.measPanel, ATTR_WIDTH, 50 * port->port.digitalIOport.bits + 90);
		SetPanelAttribute (port->port.digitalIOport.measPanel, ATTR_HEIGHT, 100);
		for(i = 0; i < port->port.digitalIOport.bits; i ++)
		{
			char name[10];
			Fmt (name, "bit %i", i);
			port->port.digitalIOport.bitarr[i].panel = LoadPanel(port->port.digitalIOport.measPanel, "digitalSourceu.uir", BITPANEL);
			SetPanelPos (port->port.digitalIOport.bitarr[i].panel, 0, 50 * i + 90);
			SetCtrlAttribute (port->port.digitalIOport.bitarr[i].panel, BITPANEL_BIT, ATTR_LABEL_TEXT, name);
			
			SetCtrlAttribute (port->port.digitalIOport.bitarr[i].panel, BITPANEL_BIT, ATTR_CALLBACK_DATA, &port->port.digitalIOport.bitarr[i]);
			SetPanelAttribute (port->port.digitalIOport.bitarr[i].panel, ATTR_CALLBACK_DATA, port);
			DisplayPanel (port->port.digitalIOport.bitarr[i].panel);
		}
	}
	SetCtrlVal (port->port.digitalIOport.measPanel, DIGITALSRC_RETPORT, port->port.digitalIOport.returnport);
	SetCtrlAttribute (port->port.digitalIOport.measPanel, DIGITALSRC_RETPORT, ATTR_CALLBACK_DATA, port);
	SetPanelAttribute (port->port.digitalIOport.measPanel, ATTR_CALLBACK_DATA, port);
	devPanel_Add(port->port.digitalIOport.measPanel, port, boards_DigitalSourceUpdate);
	DisplayPanel(port->port.digitalIOport.measPanel);
}

int boards_DigitalSourceControlCallback (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	double temp;
	if (control == BITPANEL_BIT && event == EVENT_COMMIT)
	{
		struct{int bitnum, val, panel; portPtr port;}*bit = callbackData;
		portPtr port = bit->port;
		int i;
		GetCtrlVal (panel, control, &bit->val);
		port->port.digitalIOport.IO.source->biaslevel = 0;
		for(i = 0; i < port->port.digitalIOport.bits; i ++)
			port->port.digitalIOport.IO.source->biaslevel += port->port.digitalIOport.bitarr[i].val * pow(2.0, (double)i);
		port->port.digitalIOport.IO.source->SetLevel(port->port.digitalIOport.IO.source);
		temp = port->port.digitalIOport.IO.source->biaslevel;
		temp;
	}
	return 0;
}

int boards_DigitalSourceFeedbakCallback (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	if (control == DIGITALSRC_RETPORT && event == EVENT_COMMIT)
	{
		portPtr port = callbackData;
		GetCtrlVal (panel, control, &port->port.digitalIOport.returnport);
	}
	return 0;
}

int boards_DigitalSourcePanelCallback (int panel, int event, void *callbackData, int eventData1, int eventData2)
{
	portPtr port = callbackData;
	if ((event == EVENT_KEYPRESS && eventData1 == VAL_ESC_VKEY) || event == EVENT_RIGHT_DOUBLE_CLICK)
	{
		devPanel_Remove (port->port.digitalIOport.measPanel);
		HidePanel(port->port.digitalIOport.measPanel);
	}
	return 0; 
}

void boards_DigitalSourceUpdate (int panel, void *ptr)
{
	portPtr port = ptr;
	int i;
	for (i = 0; i < port->port.digitalIOport.bits; i++)
	{
		SetCtrlVal (port->port.digitalIOport.bitarr[i].panel, BITPANEL_BIT, ((int)(port->port.digitalIOport.IO.source->biaslevel / pow(2.0, (double)i)) % 2));
		SetCtrlVal (port->port.digitalIOport.bitarr[i].panel, BITPANEL_LED, ((int)(port->port.digitalIOport.IO.source->biaslevel / pow(2.0, (double)i)) % 2));
	}
}
