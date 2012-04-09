#include "toolbox.h"
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
#include "HP4156.h"
#include "HP4156u.h"

#define TRUE 1
#define FALSE 0
#define HP4156_ID "HEWLETT-PACKARD"//"Stanford_Research_Systems"

typedef enum{CURRENT_VOLTAGE, VOLTAGE_CURRENT}smuSettingType;

typedef struct RangeRingItem{
	rangePtr range;
	struct {int CtrlIndex, CtrlVal;}CtrlValIndex;
	char *CtrlTitle;
	double compliance;
}RangeRingType;
typedef RangeRingType *RangeRingPtr;

typedef struct RangeRingList{
	RangeRingPtr (*GetRangeFromIndex) (struct RangeRingList rangeRingList, int index);
	RangeRingPtr (*GetRangeFromValue) (struct RangeRingList rangeRingList, int value);
	RangeRingPtr *RangeRingItems;
	rangePtr *ranges;
	int numberOfItems;
}RangeRingList;
typedef RangeRingList *RangeRingListPtr;

typedef struct{
	struct {sourcePtr src;  int range; RangeRingList srcRange; char setupStr[20];}source;
	struct {acqchanPtr acqchan; int range; RangeRingList measRange; char setupStr[20]; double compliance[2];}measure;
	RangeRingList currRange, voltRange;
	int panel, chanNum, on;
	char title[20];
}portType;

typedef struct{
	portType port[4];
	sourcePtr vsu[2];
	acqchanPtr vmu[2];
	double sit, fit;
	int menu, sli;
	RangeRingList currentRanges, voltageRanges, vmuRanges;
}hp4156Type;
typedef hp4156Type *hp4156Ptr;

/*******************************************************************/
RangeRingPtr	rangeRing_GetRRind		(RangeRingList rangeRingList, int index);
RangeRingPtr	rangeRing_GetRRval		(RangeRingList rangeRingList, int value);
RangeRingPtr	rangeRing_Create		(char *CtrlTitle, int CtrlIndex, int CtrlVal, double maxVal, double minVal, double resolution);
RangeRingList	rangeRingList_Init		(int numOfItems);
void			hp4156_SetRanges		(hp4156Ptr spa);

int		hp4156_ControlCallback			(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void	hp4156_MenuCallback				(int menubar, int menuItem, void *callbackData, int panel);
int		hp4156_MeasureControlCallback	(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int		hp4156_UnivControlCallback		(int panel, int control, int event, void *callbackData, int eventData1, int eventDaa2);
void	hp4156_UpdatePanel				(int panel, portType *port);
void	hp4156_UpdateMeasurePanel		(acqchanPtr acqchan);
void	hp4156_UpdateRanges				(int panel, int control, RangeRingList rangelist);


void	hp4156_UpdateSettings	(portType *port);
void 	hp4156_SetLevel			(sourcePtr src);
void	hp4156_SetLevelVSU0		(sourcePtr src);
void	hp4156_SetLevelVSU1		(sourcePtr src);
void	hp4156_GetReadingVMU0	(acqchanPtr acqchan);
void	hp4156_GetReadingVMU1	(acqchanPtr acqchan);
void	hp4156_GetReadings		(gpibioPtr dev, int portNumber);
void 	hp4156_GetReadingSrc	(acqchanPtr acqchan);
void 	hp4156_GetReadingMeas	(acqchanPtr acqchan);

void	*hp4156_Create			(gpibioPtr dev);
void	hp4156_Remove			(void *ptr);
int		hp4156_InitIO			(gpibioPtr dev);
void	hp4156Operate			(int mennubar, int menuItem, void *callbackData, int panel);
void	hp4156_UpdateReadings	(int panel, void *ptr);
void	hp4156_Save				(gpibioPtr dev);
void	hp4156_Load				(gpibioPtr dev);
void	hp4156_Init				(void);




/*******************************************************************/
RangeRingPtr rangeRing_GetRRind (RangeRingList rangeRingList, int index)
{
	int i = 0;
	while(rangeRingList.RangeRingItems[i] && rangeRingList.RangeRingItems[i]->CtrlValIndex.CtrlIndex != index)
		i++;
	return rangeRingList.RangeRingItems[i];
}

RangeRingPtr rangeRing_GetRRval (RangeRingList rangeRingList, int value)
{
	int i = 0;
	while(rangeRingList.RangeRingItems[i] && rangeRingList.RangeRingItems[i]->CtrlValIndex.CtrlVal != value)
		i++;
	return rangeRingList.RangeRingItems[i];
}

RangeRingPtr rangeRing_Create (char *CtrlTitle, int CtrlIndex, int CtrlVal, double maxVal, double minVal, double resolution)
{
	RangeRingPtr range = malloc(sizeof(RangeRingType));
	range->range = range_Create(maxVal, minVal, resolution);
	range->CtrlTitle = CtrlTitle;
	range->CtrlValIndex.CtrlIndex = CtrlIndex;
	range->CtrlValIndex.CtrlVal = CtrlVal;
	range->compliance = maxVal;
	return range;
}

RangeRingList rangeRingList_Init (int numOfItems)
{
	RangeRingList rangeRingList;
	rangeRingList.RangeRingItems    = calloc(numOfItems, sizeof(RangeRingType));
	rangeRingList.GetRangeFromIndex = rangeRing_GetRRind;
	rangeRingList.GetRangeFromValue = rangeRing_GetRRval;
	rangeRingList.numberOfItems		= numOfItems;
	rangeRingList.ranges			= calloc(numOfItems, sizeof(rangeType));
	return rangeRingList;

}

void hp4156_SetRanges(hp4156Ptr spa)
{
	int i;
//rangeRing_Create (*CtrlTitle, CtrlIndex, CtrlVal, maxVal, minVal, resolution)
	spa->currentRanges.RangeRingItems[0] = rangeRing_Create ("10 pA",	0,	9,	1E-11,	-1E-11,	1E-14);
	spa->currentRanges.RangeRingItems[1] = rangeRing_Create ("100 pA",	1,	10,	1E-10,	-1E-10,	1E-14);
	spa->currentRanges.RangeRingItems[2] = rangeRing_Create ("1 nA",	2,	11,	1E-9,	-1E-9,	1E-13);
	spa->currentRanges.RangeRingItems[3] = rangeRing_Create ("10 nA",	3,	12,	1E-8,	-1E-8,	1E-12);
	spa->currentRanges.RangeRingItems[4] = rangeRing_Create ("100 nA",	4,	13,	1E-7,	-1E-7,	1E-11);
	spa->currentRanges.RangeRingItems[5] = rangeRing_Create ("1 uA",	5,	14,	1E-6,	-1E-6,	1E-10);
	spa->currentRanges.RangeRingItems[6] = rangeRing_Create ("10 uA",	6,	15,	1E-5,	-1E-5,	1E-9);
	spa->currentRanges.RangeRingItems[7] = rangeRing_Create ("100 uA",	7,	16,	1E-4,	-1E-4,	1E-8);
	spa->currentRanges.RangeRingItems[8] = rangeRing_Create ("1 mA",	8,	17,	1E-3,	-1E-3,	1E-7);
	spa->currentRanges.RangeRingItems[9] = rangeRing_Create ("10 mA",	9,	18,	1E-2,	-1E-2,	1E-6);
	spa->currentRanges.RangeRingItems[10] = rangeRing_Create ("100 mA",	10,	19,	1E-1,	-1E-1,	1E-5);
	spa->currentRanges.RangeRingItems[11] = rangeRing_Create ("auto",	11,	0,	0,		0,		0);
	spa->currentRanges.RangeRingItems[11]->range = NULL;
	
	spa->voltageRanges.RangeRingItems[0] = rangeRing_Create ("2 V",		0,	11,	2,		-2,		1E-4);
	spa->voltageRanges.RangeRingItems[1] = rangeRing_Create ("20 V",	1,	12,	20,		-20,	1E-3);
	spa->voltageRanges.RangeRingItems[2] = rangeRing_Create ("40 V",	2,	13,	40,		-40,	2E-3);
	spa->voltageRanges.RangeRingItems[3] = rangeRing_Create ("100 V",	3,	14,	100,	-100,	5E-3);
	spa->voltageRanges.RangeRingItems[4] = rangeRing_Create ("auto",	4,	0,	0,		0,		0);
	spa->voltageRanges.RangeRingItems[4]->range = NULL;
	
	spa->vmuRanges.RangeRingItems[0] = rangeRing_Create ("2 V",		0,	11,	2,		-2,		1E-4);
	spa->vmuRanges.RangeRingItems[1] = rangeRing_Create ("20 V",	1,	12,	20,		-20,	1E-3);
	
	for(i = 0; i < spa->currentRanges.numberOfItems; i ++)
		spa->currentRanges.ranges[i] = spa->currentRanges.RangeRingItems[i]->range;
	for(i = 0; i < spa->voltageRanges.numberOfItems; i ++)
		spa->voltageRanges.ranges[i] = spa->voltageRanges.RangeRingItems[i]->range;
}

/*******************************************************************/
int hp4156_PanelCallback (int panel, int event, void *callbackData, int eventData1, int eventData2)
{
	gpibioPtr dev = callbackData;
	if ((event == EVENT_KEYPRESS && eventData1 == VAL_ESC_VKEY) || (event == EVENT_RIGHT_DOUBLE_CLICK))
	{
		int i;
		hp4156Ptr spa= dev->device;
		for ( i = 0 ; i < 4; i ++)
			if(spa->port[i].measure.acqchan->p)
				HidePanel (spa->port[i].measure.acqchan->p);
		if(spa->vmu[0]->p) HidePanel (spa->vmu[0]->p);
		if(spa->vmu[1]->p) HidePanel (spa->vmu[1]->p);
		HidePanel (dev->iPanel);
		SetMenuBarAttribute (acquire_GetMenuBar(), dev->menuitem_id, ATTR_DIMMED, FALSE);
	}
	return 0;
}
int	hp4156_ControlCallback	(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	portType *port = callbackData;
	switch (control)
	{
		case HP4156PORT_SELECT:
			if (event == EVENT_COMMIT)
			{
				int i;
				char *comp = "";
				GetCtrlVal (panel, control, &port->on);
				SetCtrlAttribute (panel, HP4156PORT_SOURCE, ATTR_DIMMED, !port->on);
				SetCtrlAttribute (panel, HP4156PORT_DISPLAY, ATTR_DIMMED, !port->on);
				SetCtrlAttribute (panel, HP4156PORT_MEASURE, ATTR_DIMMED, !port->on);
				SetCtrlAttribute (panel, HP4156PORT_DISPLAY_2, ATTR_DIMMED, !port->on);
				SetCtrlAttribute (panel, HP4156PORT_SRCRANGE, ATTR_DIMMED, !port->on);
				SetCtrlAttribute (panel, HP4156PORT_MEASRANGE, ATTR_DIMMED, !port->on);
				SetCtrlAttribute (panel, HP4156PORT_COMPLIANCE,	ATTR_DIMMED, !port->on);
				if (port->on == 1)
				{
					RangeRingPtr curRange;
					Fmt (port->measure.acqchan->channel->label, "hp4156 measure %i- voltage", port->chanNum);
					Fmt (port->source.src->acqchan->channel->label, "hp4156 source %i- current", port->chanNum);
					port->measure.measRange = port->voltRange;
					port->source.srcRange   = port->currRange;
					port->source.src->max 	= 0.1;
					port->source.src->min 	= -0.1;
					hp4156_UpdatePanel (panel, port);
					SetCtrlIndex (panel, HP4156PORT_MEASRANGE, port->measure.measRange.numberOfItems-1);
					SetCtrlIndex (panel, HP4156PORT_SRCRANGE, port->source.srcRange.numberOfItems-1);
					SetCtrlVal (panel, HP4156PORT_COMPLIANCE, port->measure.measRange.GetRangeFromValue(port->measure.measRange, 0)->compliance);
					curRange = (port->source.srcRange.GetRangeFromValue(port->source.srcRange, 0));
					Fmt(port->measure.setupStr, "TV?");
					Fmt(port->source.setupStr, "DI");
					comp = "V";
					if(curRange->range)
					{
						port->source.src->ranges.temprange[0] = curRange->range;
						port->source.src->ranges.autoscale = 0;
					}
					else
					{
						port->source.src->ranges.temprange = port->source.srcRange.ranges;
						port->source.src->ranges.autoscale = 1;
					}
					hp4156_UpdateMeasurePanel (port->measure.acqchan);
					if(port->source.src->panel)source_UpdatePanel(port->source.src->panel, port->source.src);
					CallCtrlCallback (port->source.src->panel, SOURCE_LABEL, EVENT_COMMIT, 0, 0, &i);
					if(port->measure.acqchan->p)
					{
						hp4156_UpdateRanges (port->measure.acqchan->p, MEASURE_RANGE, port->measure.measRange);
						SetCtrlIndex (port->measure.acqchan->p, MEASURE_RANGE, port->measure.measRange.numberOfItems-1);
					}
				}
				if (port->on == 2)
				{
					RangeRingPtr curRange;
					Fmt (port->measure.acqchan->channel->label, "hp4156 measure %i- current", port->chanNum);
					Fmt (port->source.src->acqchan->channel->label, "hp4156 source %i- voltage", port->chanNum);
					port->measure.measRange = port->currRange;
					port->source.srcRange   = port->voltRange;
					port->source.src->max 	= 100;
					port->source.src->min 	= -100;
					hp4156_UpdatePanel (panel, port);
					SetCtrlIndex (panel, HP4156PORT_MEASRANGE, port->measure.measRange.numberOfItems-1);
					SetCtrlIndex (panel, HP4156PORT_SRCRANGE, port->source.srcRange.numberOfItems-1);
					SetCtrlVal (panel, HP4156PORT_COMPLIANCE, port->measure.measRange.GetRangeFromValue(port->measure.measRange, 0)->compliance);
					curRange = (port->source.srcRange.GetRangeFromValue(port->source.srcRange, 0));
					Fmt(port->measure.setupStr, "TI?");
					Fmt(port->source.setupStr, "DV");
					comp = "A";
					if(curRange->range)
					{
						port->source.src->ranges.temprange[0] = curRange->range;
						port->source.src->ranges.autoscale = 0;
					}
					else
					{
						port->source.src->ranges.temprange = port->source.srcRange.ranges;
						port->source.src->ranges.autoscale = 1;
					}
					hp4156_UpdateMeasurePanel (port->measure.acqchan);
					if(port->source.src->panel)source_UpdatePanel(port->source.src->panel, port->source.src);
					CallCtrlCallback (port->source.src->panel, SOURCE_LABEL, EVENT_COMMIT, 0, 0, &i);
					if(port->measure.acqchan->p)
					{
						hp4156_UpdateRanges (port->measure.acqchan->p, MEASURE_RANGE, port->measure.measRange);
						SetCtrlIndex (port->measure.acqchan->p, MEASURE_RANGE, port->measure.measRange.numberOfItems-1);
					}
				}
				else
				{
					gpibPrint((gpibioPtr)port->measure.acqchan->dev, "DV %i, 0, 0\n", &port->chanNum);
				}
				if(!port->on && port->measure.acqchan->p)
					HidePanel(port->measure.acqchan->p);
				if(!port->on && port->source.src->panel)
					HidePanel(port->source.src->panel);
				SetCtrlVal (panel, HP4156PORT_UNITS, comp);
				SetCtrlAttribute(panel, HP4156PORT_DISPLAY, ATTR_MAX_VALUE, port->source.src->max);
				SetCtrlAttribute(panel, HP4156PORT_DISPLAY, ATTR_MIN_VALUE, port->source.src->min);
				port = port->on? port : NULL;
				SetCtrlAttribute (panel, HP4156PORT_COMPLIANCE,	ATTR_CALLBACK_DATA, port);
				SetCtrlAttribute (panel, HP4156PORT_MEASRANGE,	ATTR_CALLBACK_DATA, port);
				SetCtrlAttribute (panel, HP4156PORT_SRCRANGE,	ATTR_CALLBACK_DATA, port);
				SetCtrlAttribute (panel, HP4156PORT_DISPLAY, 	ATTR_CALLBACK_DATA, port);
				SetCtrlAttribute (panel, HP4156PORT_MEASURE, 	ATTR_CALLBACK_DATA, port);
				SetCtrlAttribute (panel, HP4156PORT_SOURCE, 	ATTR_CALLBACK_DATA, port);
			}
			break;
		case HP4156PORT_SOURCE:
			if (event == EVENT_COMMIT && port)
				switch(utilG.exp)
				{
					case EXP_SOURCE: source_InitPanel(port->source.src); break;
					case EXP_FLOAT : gensrc_InitPanel(port->source.src); break;
				}
			break;
		case HP4156PORT_MEASURE:
			if (event == EVENT_COMMIT && port)
			{
				acqchanPtr acqchan = port->measure.acqchan;
				hp4156_UpdateMeasurePanel (acqchan);
				DisplayPanel(acqchan->p);
				SetInputMode (acqchan->p, MEASURE_RANGE, 0);
			}
			break;
		case HP4156PORT_DISPLAY:
			if (event == EVENT_COMMIT && port)
			{
				GetCtrlVal (panel, control, &port->source.src->biaslevel);
				port->source.src->SetLevel(port->source.src);
			}
			break;
		case HP4156PORT_SRCRANGE:
			if (event == EVENT_COMMIT && port)
			{
				GetCtrlVal (panel, control, &port->source.range);
				if (port->source.range)
				{
					port->source.src->ranges.temprange[0] = (port->source.srcRange.GetRangeFromValue(port->source.srcRange, port->source.range))->range;
					port->source.src->ranges.autoscale = 0;
				}
				else
				{
					port->source.src->ranges.temprange = port->source.srcRange.ranges;
					port->source.src->ranges.autoscale = 1;
				}
				port->source.src->ranges.autoscale = !port->source.range;
				port->source.src->SetLevel(port->source.src);
			}
			break;
		case HP4156PORT_MEASRANGE:
			if (event == EVENT_COMMIT && port)
			{
				GetCtrlVal (panel, control, &port->measure.range);
				SetCtrlVal (panel, HP4156PORT_COMPLIANCE, (port->measure.measRange.GetRangeFromValue(port->measure.measRange, port->measure.range))->compliance);
				if(port->measure.range)
				{
					SetCtrlAttribute (panel, HP4156PORT_COMPLIANCE, ATTR_MAX_VALUE, port->measure.measRange.ranges[port->measure.measRange.GetRangeFromValue(port->measure.measRange, port->measure.range)->CtrlValIndex.CtrlIndex]->maxVal);
					SetCtrlAttribute (panel, HP4156PORT_COMPLIANCE, ATTR_MIN_VALUE, port->measure.measRange.ranges[port->measure.measRange.GetRangeFromValue(port->measure.measRange, port->measure.range)->CtrlValIndex.CtrlIndex]->minVal);
				}
				else
				{
					SetCtrlAttribute (panel, HP4156PORT_COMPLIANCE, ATTR_MAX_VALUE, port->measure.measRange.ranges[port->measure.measRange.numberOfItems - 2]->maxVal);
					SetCtrlAttribute (panel, HP4156PORT_COMPLIANCE, ATTR_MAX_VALUE, port->measure.measRange.ranges[port->measure.measRange.numberOfItems - 2]->maxVal);
				}
				if(port->measure.acqchan->p)
					SetCtrlIndex (port->measure.acqchan->p, MEASURE_RANGE, port->measure.measRange.GetRangeFromValue(port->measure.measRange, port->measure.range)->CtrlValIndex.CtrlIndex);
			}
			break;
		case HP4156PORT_COMPLIANCE:
			if (event == EVENT_COMMIT && port)
			{
				GetCtrlVal (panel, HP4156PORT_MEASRANGE, &port->measure.range);
				GetCtrlVal (panel, control, &(port->measure.measRange.GetRangeFromValue(port->measure.measRange, port->measure.range))->compliance);
				port->source.src->SetLevel(port->source.src);
			}
			break;
	}
	return 0;
}

void hp4156_MenuCallback (int menubar, int menuItem, void *callbackData, int panel)
{
	switch(menuItem)
	{
		case HP4156MENU_VSUS_VSU_1:
		case HP4156MENU_VSUS_VSU_2:
			{
				sourcePtr src = callbackData;
				switch(utilG.exp)
				{
					case EXP_SOURCE: source_InitPanel(src); break;
					case EXP_FLOAT : gensrc_InitPanel(src); break;
				}
			}
			break;
		case HP4156MENU_VMUS_VMU_1:
		case HP4156MENU_VMUS_VMU_2:
			{
				acqchanPtr acqchan = callbackData;
				gpibioPtr dev = acqchan->dev;
				hp4156Ptr spa = dev->device;
				hp4156_UpdateMeasurePanel (acqchan);
				DisplayPanel(acqchan->p);
				hp4156_UpdateRanges (acqchan->p, MEASURE_RANGE, spa->vmuRanges);
				SetInputMode (acqchan->p, MEASURE_RANGE, 1);
			}
			break;
		
	}
}

int hp4156_MeasureControlCallback (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	acqchanPtr acqchan = callbackData;
	
	if (control == MEASURE_LABEL && event == EVENT_COMMIT)
		GetCtrlVal (panel, control, acqchan->channel->label);
	
	if (control == MEASURE_COEFF && event == EVENT_COMMIT)
		GetCtrlVal (panel, control, &acqchan->coeff);
	
	if (control == MEASURE_ACQ && event == EVENT_COMMIT)
	{
		if(utilG.acq.status != ACQ_BUSY)
		{
			GetCtrlVal (panel, control, &acqchan->acquire);
			if (acqchan->acquire)	acqchanlist_AddChannel (acqchan);
			else	acqchanlist_RemoveChannel (acqchan);
		}
		else
			SetCtrlVal (panel, control, acqchan->acquire);
	}
	if (control == MEASURE_RANGE && event == EVENT_COMMIT)
	{
		portType *port = acqchan->upLvl;
		GetCtrlVal (panel, control, &port->measure.range);
	}
	if(event == EVENT_COMMIT)
		hp4156_UpdateMeasurePanel (acqchan);
	return 0;
}

int hp4156_UnivControlCallback (int panel, int control, int event, void *callbackData, int eventData1, int eventDaa2)
{
	gpibioPtr dev = callbackData;
	hp4156Ptr spa = dev->device;
	switch (control)
	{
		case HP4156_INTEGR_FAST:
			if (event == EVENT_COMMIT)
			{
				GetCtrlVal (panel, control, &spa->fit);
				gpibPrint(dev, "SIT 1, %f\n", &spa->fit);
			}
			break;
		case HP4156_INTEGR_SLOW:
			if (event == EVENT_COMMIT)
			{
				GetCtrlVal (panel, control, &spa->sit);
				gpibPrint(dev, "SIT 3, %f\n", &spa->sit);	   
			}
			break;
		case HP4156_SLI:
			if (event == EVENT_COMMIT)
			{
				GetCtrlVal (panel, control, &spa->sli);
				gpibPrint(dev, "SLI %i\n", &spa->sli);
			}
			break;
	}
	return 0;
}

void hp4156_UpdatePanel (int panel, portType *port)
{
	hp4156_UpdateRanges (panel, HP4156PORT_SRCRANGE,  port->source.srcRange);
	hp4156_UpdateRanges (panel, HP4156PORT_MEASRANGE, port->measure.measRange);
}

void hp4156_UpdateMeasurePanel (acqchanPtr acqchan)
{
	if(acqchan->p)
	{
		SetPanelAttribute (acqchan->p, ATTR_TITLE, acqchan->channel->label);
		SetCtrlVal (acqchan->p, MEASURE_LABEL, acqchan->channel->label);
		SetCtrlVal (acqchan->p, MEASURE_COEFF, acqchan->coeff);
		SetCtrlVal (acqchan->p, MEASURE_NOTE, acqchan->note);
		SetCtrlVal (acqchan->p, MEASURE_ACQ, acqchan->acquire);
		if(acqchan->menuitem_id)
		{
			gpibioPtr dev = acqchan->dev;
			hp4156Ptr spa = dev->device;
			SetMenuBarAttribute (spa->menu, acqchan->menuitem_id, ATTR_CHECKED, acqchan->acquire);
		}
	}
	else
	{
		acqchan->p = LoadPanel (utilG.p, "HP4156u.uir", MEASURE);
		SetPanelAttribute (acqchan->p, ATTR_TITLE, acqchan->channel->label);
		SetCtrlAttribute (acqchan->p, MEASURE_RANGE, ATTR_CALLBACK_DATA, acqchan);
		SetCtrlAttribute (acqchan->p, MEASURE_LABEL, ATTR_CALLBACK_DATA, acqchan);
		SetCtrlAttribute (acqchan->p, MEASURE_COEFF, ATTR_CALLBACK_DATA, acqchan);
		SetCtrlAttribute (acqchan->p, MEASURE_NOTE, ATTR_CALLBACK_DATA, acqchan);
		SetCtrlAttribute (acqchan->p, MEASURE_ACQ, ATTR_CALLBACK_DATA, acqchan);
		hp4156_UpdateMeasurePanel (acqchan);
	}
}

void hp4156_UpdateRanges (int panel, int control, RangeRingList rangelist)
{
	int i;
	DeleteListItem(panel, control, 0, -1);
	for (i = 0; i < rangelist.numberOfItems; i ++)
		InsertListItem (panel, control, rangelist.RangeRingItems[i]->CtrlValIndex.CtrlIndex, rangelist.RangeRingItems[i]->CtrlTitle, rangelist.RangeRingItems[i]->CtrlValIndex.CtrlVal);
}

/*******************************************************************/
/*
4156 IO operations:

CMM chnum, measmode - Sets measurement mode
DI chnum, range, current, Vcomp - Current output command
DV chnum, range, voltage, Icomp - Voltage output command.
RI chnum, range - Current read command
RV chnum, range - Voltage read command
XE - triggers performance of measurement
RMD? [count] - reads the data in the output buffer. count specifies number of return parameters.


//*/

void hp4156_SetLevel (sourcePtr src)
{
	portType *port = src->acqchan->upLvl; 
	gpibioPtr dev = src->acqchan->dev;
	gpibPrint(dev, "%s %i, %i, %f, %f\n", port->source.setupStr, &port->chanNum, &port->source.range, &src->biaslevel, &port->measure.measRange.GetRangeFromValue(port->measure.measRange, port->measure.range)->compliance);
	util_Delay(src->segments[src->seg]->delay);//*/
}

void hp4156_SetLevelVSU0 (sourcePtr src)
{
	gpibPrint((gpibioPtr)src->acqchan->dev, "DV 21, 12, %f\n", &src->biaslevel);
	util_Delay (src->segments[src->seg]->delay);
}

void hp4156_SetLevelVSU1 (sourcePtr src)
{
	gpibPrint((gpibioPtr)src->acqchan->dev, "DV 21, 12, %f\n", &src->biaslevel);
	util_Delay (src->segments[src->seg]->delay);
}

void hp4156_GetReadingVMU0 (acqchanPtr acqchan)
{
	acqchan->newreading = TRUE;
}

void hp4156_GetReadingVMU1 (acqchanPtr acqchan)
{
	acqchan->newreading = TRUE;
}

void hp4156_GetReadings (gpibioPtr dev, int portNumber)
{
	hp4156Ptr spa = dev->device;
	spa->port[portNumber].source.src->acqchan->GetReading (spa->port[portNumber].source.src->acqchan);
	spa->port[portNumber].measure.acqchan->GetReading (spa->port[portNumber].measure.acqchan);
}

void hp4156_GetReadingSrc (acqchanPtr acqchan)
{
	portType *port = acqchan->upLvl;
	acqchan->reading = port->source.src->biaslevel;
	acqchan->newreading = TRUE;
}

void hp4156_GetReadingMeas (acqchanPtr acqchan)
{
	char msg[260];
	portType *port = acqchan->upLvl;
	gpibioPtr dev = acqchan->dev;
	Fmt(msg, "%s %i, %i\n",port->measure.setupStr, port->chanNum, port->measure.range);
	acqchan->reading = gpib_GetDoubleVal (dev, msg);
	acqchan->newreading = TRUE;
}

/*******************************************************************/

void *hp4156_Create (gpibioPtr dev)
{
	hp4156Ptr spa = malloc(sizeof(hp4156Type));
	char name[30];
	int i;
	
	dev->device = spa;
	spa->voltageRanges = rangeRingList_Init (5);
	spa->currentRanges = rangeRingList_Init (12);
	spa->vmuRanges     = rangeRingList_Init (2);
	hp4156_SetRanges(spa);
	for(i = 0; i < 4; i++)
	{
		Fmt (name, "hp4156 source %i", i+1);
		spa->port[i].source.src = source_Create (name, dev, hp4156_SetLevel, hp4156_GetReadingSrc);
		Fmt (name, "hp4156 measure %i", i+1);
		spa->port[i].measure.acqchan = acqchan_Create (name, dev, hp4156_GetReadingMeas);
		spa->port[i].measure.acqchan->upLvl = &spa->port[i];
		spa->port[i].source.src->acqchan->upLvl = &spa->port[i];
		spa->port[i].source.src->biaslevel = 0;
		Fmt (spa->port[i].title, "smu %i", i+1);
		spa->port[i].panel = 0;
		spa->port[i].voltRange = spa->voltageRanges;
		spa->port[i].currRange = spa->currentRanges;
		spa->port[i].chanNum = i+1;
		spa->port[i].measure.compliance[0] = 0;
		spa->port[i].measure.compliance[1] = 0;
		spa->port[i].on = 0;
		spa->port[i].measure.range = 0;
		spa->port[i].source.range = 0;
	}
	spa->vsu[0] = source_Create ("hp4156 vsu 1", dev, hp4156_SetLevelVSU0, hp4156_GetReadingSrc);
	spa->vsu[1] = source_Create ("hp4156 vsu 2", dev, hp4156_SetLevelVSU1, hp4156_GetReadingSrc);
	spa->vsu[0]->ranges.temprange = spa->voltageRanges.ranges;
	spa->vsu[0]->ranges.autoscale = 1;
	spa->vsu[1]->ranges.temprange = spa->voltageRanges.ranges;
	spa->vsu[1]->ranges.autoscale = 1;
	spa->vsu[0]->max = 100;
	spa->vsu[0]->min = -100;
	spa->vsu[1]->max = 100;
	spa->vsu[1]->min = -100;
	
	spa->vmu[0] = acqchan_Create ("hp4156 vmu 1", dev, hp4156_GetReadingVMU0);
	spa->vmu[1] = acqchan_Create ("hp4156 vmu 2", dev, hp4156_GetReadingVMU1);
	spa->vsu[0]->menuitem_id = HP4156MENU_VSUS_VSU_1;
	spa->vsu[1]->menuitem_id = HP4156MENU_VSUS_VSU_2;
	spa->vmu[0]->menuitem_id = HP4156MENU_VMUS_VMU_1;
	spa->vmu[1]->menuitem_id = HP4156MENU_VMUS_VMU_2;
	spa->sit = 0.04;
	spa->fit = 0.0005;
	spa->sli = 1;
	return spa;
}

void hp4156_Remove (void *ptr)
{
	int i;
	hp4156Ptr spa = ptr;
	for (i = 0; i < 4; i ++)
	{
		source_Remove  (spa->port[i].source.src);
		acqchan_Remove (spa->port[i].measure.acqchan);
	}
	source_Remove  (spa->vsu[0]);
	source_Remove (spa->vsu[1]);
	acqchan_Remove  (spa->vmu[0]);
	acqchan_Remove (spa->vmu[1]);
	free(spa);
}

int hp4156_InitIO (gpibioPtr dev)
{
	int i;
	gpibio_Remote(dev);
	if(gpibio_DeviceMatch(dev, "*IDN?", HP4156_ID))
	{
		gpibPrint(dev, "*RST\n");
		gpibPrint(dev, "US\n");
		gpibPrint(dev, "FMT 2\n");
		gpibPrint(dev, "AV 1\n");
		gpibPrint(dev, "SIT 1, 0.0005\n");
		gpibPrint(dev, "SIT 3, 0.04\n");
		gpibPrint(dev, "SLI 1\n");
		gpibPrint(dev, "FL 0\n");
		gpibPrint(dev, "CN\n");
		for ( i = 0; i < 4 ; i ++)
			gpibPrint(dev, "DV %i, 0\n", &i);
		return TRUE;
	}
	return FALSE;
}

void hp4156Operate (int menubar, int menuItem, void *callbackData, int panel)
{
	gpibioPtr dev = callbackData;
	hp4156Ptr spa = dev->device;
	if(!dev->iPanel)
	{
		int i;
		dev->iPanel = LoadPanel (utilG.p, "HP4156u.uir", HP4156);
		spa->menu = LoadMenuBar (dev->iPanel, "HP4156u.uir", HP4156MENU);
		SetPanelMenuBar (dev->iPanel, spa->menu);
		SetPanelAttribute (dev->iPanel, ATTR_WIDTH, 1000);
		SetPanelAttribute (dev->iPanel, ATTR_HEIGHT, 280);
		SetPanelAttribute (dev->iPanel, ATTR_CALLBACK_DATA, dev);
		SetPanelAttribute (dev->iPanel, ATTR_TITLE, dev->label);
		SetCtrlAttribute (dev->iPanel, HP4156_INTEGR_FAST, ATTR_CALLBACK_DATA, dev);
		SetCtrlAttribute (dev->iPanel, HP4156_INTEGR_SLOW, ATTR_CALLBACK_DATA, dev);
		SetCtrlAttribute (dev->iPanel, HP4156_SLI, ATTR_CALLBACK_DATA, dev);
		SetMenuBarAttribute (spa->menu, spa->vsu[0]->menuitem_id, ATTR_CALLBACK_DATA, spa->vsu[0]);
		SetMenuBarAttribute (spa->menu, spa->vsu[1]->menuitem_id, ATTR_CALLBACK_DATA, spa->vsu[1]);
		SetMenuBarAttribute (spa->menu, spa->vmu[0]->menuitem_id, ATTR_CALLBACK_DATA, spa->vmu[0]);
		SetMenuBarAttribute (spa->menu, spa->vmu[1]->menuitem_id, ATTR_CALLBACK_DATA, spa->vmu[1]);
		SetMenuBarAttribute (spa->menu, spa->vmu[0]->menuitem_id, ATTR_CHECKED, spa->vmu[0]->acquire);
		SetMenuBarAttribute (spa->menu, spa->vmu[1]->menuitem_id, ATTR_CHECKED, spa->vmu[1]->acquire);
		SetCtrlVal (dev->iPanel, HP4156_INTEGR_FAST, spa->fit);
		SetCtrlVal (dev->iPanel, HP4156_INTEGR_SLOW, spa->sit);
		SetCtrlVal (dev->iPanel, HP4156_SLI, spa->sli);
		for (i = 0; i < 4; i++)
		{
			spa->port[i].panel = LoadPanel (dev->iPanel, "HP4156u.uir", HP4156PORT);
			SetPanelPos (spa->port[i].panel, 17, (i*(250)));
			SetCtrlVal (spa->port[i].panel, HP4156PORT_TITLE, spa->port[i].title);
			SetCtrlAttribute (spa->port[i].panel, HP4156PORT_SOURCE, ATTR_DIMMED, 1);
			SetCtrlAttribute (spa->port[i].panel, HP4156PORT_MEASURE, ATTR_DIMMED, 1);
			SetCtrlAttribute (spa->port[i].panel, HP4156PORT_DISPLAY, ATTR_DIMMED, 1);
			SetCtrlAttribute (spa->port[i].panel, HP4156PORT_DISPLAY_2, ATTR_DIMMED, 1);
			SetCtrlAttribute (spa->port[i].panel, HP4156PORT_SRCRANGE, ATTR_DIMMED, 1);
			SetCtrlAttribute (spa->port[i].panel, HP4156PORT_MEASRANGE, ATTR_DIMMED, 1);
			SetCtrlAttribute (spa->port[i].panel, HP4156PORT_COMPLIANCE, ATTR_DIMMED, 1);
			SetCtrlAttribute (spa->port[i].panel, HP4156PORT_SELECT, ATTR_CALLBACK_DATA, &spa->port[i]);
			if(spa->port[i].on)
				SetCtrlIndex (spa->port[i].panel, HP4156PORT_SELECT, spa->port[i].on-1);
			else
				SetCtrlIndex (spa->port[i].panel, HP4156PORT_SELECT, 2);
			hp4156_ControlCallback (spa->port[i].panel, HP4156PORT_SELECT, EVENT_COMMIT, &spa->port[i], 0, 0);
			SetPanelAttribute (spa->port[i].panel, ATTR_CALLBACK_DATA, dev);
			DisplayPanel (spa->port[i].panel);
		}
	}
	SetMenuBarAttribute(menubar, menuItem, ATTR_DIMMED, 1);
	devPanel_Add (dev->iPanel, dev, hp4156_UpdateReadings);
	DisplayPanel (dev->iPanel);
}

void hp4156_UpdateReadings (int panel, void *ptr)
{
	int i;
	gpibioPtr dev = ptr;
	hp4156Ptr spa = dev->device;
	for(i = 0; i < 4; i ++)
	{
		if(spa->port[i].on)
		{
			hp4156_GetReadings (dev, i);
			SetCtrlVal (spa->port[i].panel, HP4156PORT_DISPLAY_2, spa->port[i].measure.acqchan->reading);
			SetCtrlVal (spa->port[i].panel, HP4156PORT_DISPLAY, spa->port[i].source.src->acqchan->reading);
		}
	}
	
}
void hp4156_Save(gpibioPtr dev)
{
	int i;
	hp4156Ptr spa = dev->device;
	for (i = 0; i < 4; i++)
	{
		FmtFile (fileHandle.analysis, "port status :%i\n", spa->port[i].on);
		source_Save  (spa->port[i].source.src);
		acqchan_Save (spa->port[i].measure.acqchan);
	}
	source_Save (spa->vsu[0]);
	source_Save (spa->vsu[1]);
	acqchan_Save (spa->vmu[0]);
	acqchan_Save (spa->vmu[1]);
}

void hp4156_Load(gpibioPtr dev)
{   
	int i;
	hp4156Ptr spa = dev->device;
	for (i = 0; i < 4; i++)
	{
		ScanFile (fileHandle.analysis, "port status :%i\n", &spa->port[i].on);
		source_Load  (dev, spa->port[i].source.src);
		acqchan_Load (dev, spa->port[i].measure.acqchan);
	}
	source_Load (dev, spa->vsu[0]);
	source_Load (dev, spa->vsu[1]);
	acqchan_Load (dev, spa->vmu[0]);
	acqchan_Load (dev, spa->vmu[1]);
}

void hp4156_Init(void)
{
	devTypePtr devType;
	if(utilG.acq.status != ACQ_NONE)
	{
		util_ChangeInitMessage("hp4156 spa...");
		devType = malloc(sizeof(devTypeItem));
		if(devType)
		{
			Fmt(devType->label, "hp4156 semicouductor parameter analyzer");
			Fmt(devType->id, HP4156_ID);
			devType->CreateDevice 	= hp4156_Create;
			devType->RemoveDevice	= hp4156_Remove;
			devType->InitDevice 	= hp4156_InitIO;
			devType->OperateDevice	= hp4156Operate;
			devType->UpdateReadings	= hp4156_UpdateReadings;
			devType->SaveDevice		= hp4156_Save;
			devType->LoadDevice		= hp4156_Load;
			devTypeList_AddItem (devType);
		}
	}
}
