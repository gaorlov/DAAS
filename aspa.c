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
#include "aspa.h"
#include "aspau.h"

#define TRUE 1
#define FALSE 0
#define ASPA_ID "HEWLETT_PACKARD"

/*******************************************************************/

void	*aspa_Create (gpibioPtr dev);
void	aspa_Init (void);


/*******************************************************************/

void *aspa_Create (gpibioPtr dev)
{
	return dev;
}

void aspa_Init(void)
{
	devTypePtr devType;
	if(utilG.acq.status != ACQ_NONE)
	{
		util_ChangeInitMessage("agilent asp...");
		devType = malloc(sizeof(devTypeItem));
		if(devType)
		{
			Fmt(devType->label, "agilent asp");
			Fmt(devType->id, ASPA_ID);
			devType->CreateDevice 	= aspa_Create;
			devType->RemoveDevice	= NULL;//aspa_Remove;
			devType->InitDevice 	= NULL;//aspa_InitIO;
			devType->OperateDevice	= NULL;//aspaOperate;
			devType->UpdateReadings	= NULL;//aspa_UpdateReadings;
			devType->SaveDevice		= NULL;//aspa_Save;
			devType->LoadDevice		= NULL;//aspa_Load;
		}
	}
}
