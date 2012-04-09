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

static listType boards_DevList, boards_DevTypeList;
struct{
	int saveMenu, loadMenu;
}boards_util;

/*****************************************************/
void 		source_GetRange(sourcePtr src, int rangeInt);
void 		ReadAnalogue (acqchanPtr acqchan);
void		ReadAnalogueOut (acqchanPtr acqchan);
void		SetAnalogue (sourcePtr src);		  

void 		boards_DevList_SaveItem(MCCdevPtr dev);
int 		boards_DevList_LoadItem(void);
MCCdevPtr 	boards_DevList_FindItemByID(char *id);
void 		boards_DevTypeList_AddDev(MCCdevTypePtr devType);
static 		MCCdevTypePtr boards_getDevice(char *BoardName);

portPtr 	create_Port(void *dev, char *name, int type, int direction, GetReadingPtr GetReading,  ...);
void 		port_Load (void *dev, portPtr port);
void 		port_Save (portPtr port);

void 		boards_Load(void *dev, void (*load) (MCCdevPtr));
void 		boards_Save(void *dev, void (*save) (MCCdevPtr));
void 		boards_MenuCallback(int menubar, int menuItem, void *callbackData, int panel);
MCCdevPtr 	boards_Create (MCCdevTypePtr devType, int BoardNum);   
void 		init_MCCdevices (void);

/*****************************************************/
void source_GetRange(sourcePtr src, int rangeInt)
{
	double range;
	switch(rangeInt)
	{
		case BIP10VOLTS:	range = 10;break;
		case BIP5VOLTS:		range = 5;break;
		case BIP4VOLTS:		range = 4;break;
		case BIP2PT5VOLTS:	range = 2.5;break;
		case BIP2VOLTS:		range = 2;break;
		case BIP1PT25VOLTS:	range = 1.25;break;
		case BIP1VOLTS:		range = 1;break;
		case BIPPT625VOLTS:	range = .625;break;
		case BIPPT5VOLTS:	range = .5;break;
		case BIPPT25VOLTS:	range = .25;break;
		case BIPPT2VOLTS:	range = .2;break;
		case BIPPT1VOLTS:	range = .1;break;
		case BIPPT05VOLTS:	range = .05;break;
		case BIPPT01VOLTS:	range = .01;break;
		case BIPPT005VOLTS:	range = .005;break;
		case BIP1PT67VOLTS:	range = 1.67;break;
	}
	src->max = range;
	src->min = range * -1;
}
/**************************vv I/O functions vv***************************
int cbFromEngUnits	(int BoardNum, int Range, float EngUnits, 			unsigned short *DataVal)
int cbToEngUnits	(int BoardNum, int Range, unsigned short DataVal, 	float *EngUnits)
*/

void ReadAnalogue (acqchanPtr acqchan)
{
	portPtr up = acqchan->upLvl;
	MCCdevPtr dev = acqchan->dev;
	float temp;
	unsigned short reading;
	int j;
	double average = 0;
	for( j=0; j < up->averaging; j++)
	{
		cbAIn (dev->BoardNum, 	up->port.analogueIOport.channel,
							up->port.analogueIOport.range, 
							&reading);
		cbToEngUnits (dev->BoardNum, up->port.analogueIOport.range, reading, &temp);
		acqchan->reading = (double)temp;
		average += acqchan->reading;
	}
	acqchan->reading = average/(up->averaging);
	acqchan->newreading = TRUE;
}

void ReadAnalogueOut (acqchanPtr acqchan)
{
	portPtr up = acqchan->upLvl;
	MCCdevPtr dev = acqchan->dev;
	sourcePtr src = up->port.analogueIOport.IO.source;
	float temp;
	unsigned short reading;
	cbFromEngUnits(dev->BoardNum, up->port.analogueIOport.range, src->biaslevel, &reading);
	cbToEngUnits (dev->BoardNum, up->port.analogueIOport.range, reading, &temp);
	acqchan->reading = (double)temp;
	acqchan->newreading = TRUE;
}

void SetAnalogue (sourcePtr src)
{
	portPtr port = src->acqchan->upLvl;
	MCCdevPtr dev = src->acqchan->dev;
	unsigned short DataVal;
	cbFromEngUnits (dev->BoardNum, port->port.analogueIOport.range, port->port.analogueIOport.IO.source->biaslevel, &DataVal);
	cbAOut(dev->BoardNum, port->port.analogueIOport.channel, port->port.analogueIOport.range, DataVal);
	util_Delay(src->segments[src->seg]->delay);
		
}

void ReadDigital (acqchanPtr acqchan)
{
	//int cbDIn(int BoardNum, int PortNum, unsigned short *DataValue)
	portPtr port = acqchan->upLvl;
	MCCdevPtr dev = acqchan->dev;
	unsigned short DataVal;
	cbDIn (dev->BoardNum, port->port.digitalIOport.port, &DataVal);
	
	acqchan->reading = (double)DataVal;
	acqchan->newreading = TRUE;
}

void ReadDigitalOut (acqchanPtr acqchan)
{
	acqchan->newreading = TRUE;
}

void SetDigital (sourcePtr src)
{
	portPtr port = src->acqchan->upLvl;
	MCCdevPtr dev = src->acqchan->dev;
	
	cbDOut(dev->BoardNum, port->port.digitalIOport.port, (unsigned short)src->biaslevel);
	util_Delay (src->segments[src->seg]->delay);
}
/**************************^^ I/O functions ^^***************************/

/**************************vv devList & devTypeList vv***************************/

void boards_DevList_SaveItem(MCCdevPtr dev)
{
	MCCdevTypePtr devType = dev->devType;
	FmtFile(fileHandle.analysis, "Board ID  : %s\n", devType->id);
	if(devType->SaveDevice)devType->SaveDevice(dev);
}

int boards_DevList_LoadItem(void)
{
	char id[30];
	int boardnum, i;
	MCCdevTypePtr devType;
	MCCdevPtr dev;
	ScanFile(fileHandle.analysis, "Board ID  : %s", id);
	devType = boards_getDevice(id);
	dev = boards_DevList_FindItemByID(id);
	if(devType && dev)
	{   
		devType = dev->devType;
		boardnum = dev->BoardNum;
		DiscardMenuItem (acquire_GetMenuBar(), dev->menuitem_id);
		if(devType->RemoveDevice) devType->RemoveDevice(dev);
		i = list_FindItem(boards_DevList, dev);
		list_RemoveItem(&boards_DevList, i, TRUE);
		dev = boards_Create(devType, boardnum);
		if(devType->CreateDevice)devType->CreateDevice(dev);
		if(devType->LoadDevice)devType->LoadDevice(dev);
		return 1;
	}
	else
	{	
		util_MessagePopup("PCI configuration load error:", "the device (%s) is not online or the file is of the wrong format", id);
		return 0;
	}
}

MCCdevPtr boards_DevList_FindItemByID(char *id)
{
	int i = 0;
	nodePtr node;
	MCCdevPtr dev;
	MCCdevTypePtr devType;
	do{
		node = list_GetNode (boards_DevList, i);
		i++;
		if(node)
		{	
			dev = node->item;
			devType = dev->devType;
		}
		else
			dev = NULL;
	}while(node && strcmp(devType->id, id));
	return dev;
}

void boards_DevTypeList_AddDev(MCCdevTypePtr devType)
{
	list_AddItem (&boards_DevTypeList, devType);
}

static MCCdevTypePtr boards_getDevice(char *BoardName)
{
	int i = 0;
	nodePtr node;
	MCCdevTypePtr devType;
	do{
		node = list_GetNode (boards_DevTypeList, i);
		i++;
		if(node)
		{
			devType = node->item;
		}
		else
			devType = NULL;
	}while(node && strcmp(devType->id, BoardName));
	return devType;
}
/**************************^^ devList & devTypeList ^^ ***************************/

/**************************vv ports vv***************************/
	/*
			notes for create_Port:
	
	if(type == ANALOGUE && direction == OUT_PORT)
		list = int channel, int range;
		
	if(type == ANALOGUE && direction == IN_PORT)	
		list = SetLevelPtr SetLevel, int channel, int range;
		
	if(type == DIGITAL && direction == OUT_PORT)
		list = int port, int devtype, int bits;
		
	if(type == DIGITAL && direction == IN_PORT)	
		list = SetLevelPtr SetLevel, int port, int devtype, int bits;

		//*/
portPtr create_Port(void *dev, char *name, int type, int direction, GetReadingPtr GetReading,  ...)
{
	va_list list;
	SetLevelPtr SetLevel;
	MCCdevPtr devP = dev;
	int arg;
	portPtr port = malloc(sizeof(portType));
	
	va_start(list, GetReading);
	
	port->measPanel = 0;
	port->control = 0;
	port->type = type;
	port->averaging = 1;
	if(type == ANALOGUE)
	{
		if(direction == IN_PORT)
		{
			port->port.analogueIOport.IO.acqchan = acqchan_Create (name, dev, GetReading);
			port->direction = direction;
			arg = va_arg(list, int);
			port->port.analogueIOport.channel = arg;
			arg = va_arg(list, int);
			port->port.analogueIOport.range = arg;
			port->port.analogueIOport.IO.acqchan->upLvl = port;
		}
		else
		{
			SetLevel = va_arg(list, SetLevelPtr);
			port->port.analogueIOport.IO.source = source_Create(name, dev, SetLevel, GetReading);
			port->direction = direction;
			arg = va_arg(list, int);
			port->port.analogueIOport.channel = arg;
			arg = va_arg(list, int);
			port->port.analogueIOport.range = arg;
			port->port.analogueIOport.IO.source->acqchan->upLvl = port;
		}
	}
	else
	{
		port->port.digitalIOport.measPanel = 0;
		if(direction == IN_PORT)
		{
			port->port.digitalIOport.IO.acqchan = acqchan_Create(name, dev, GetReading);
			port->direction = direction;
			arg = va_arg(list, int);
			port->port.digitalIOport.port = arg;
			cbDConfigPort(devP->BoardNum, arg, DIGITALIN);
			arg = va_arg(list, int);
			port->port.digitalIOport.devtype = arg;
			arg = va_arg(list, int);
			port->port.digitalIOport.bits = arg;
			port->port.digitalIOport.returnport = 0;
			port->port.digitalIOport.IO.acqchan->upLvl = port;
		}
		else
		{
			SetLevel = va_arg(list, SetLevelPtr);
			port->port.digitalIOport.IO.source = source_Create (name, dev, SetLevel, GetReading);
			port->direction = direction;
			arg = va_arg(list, int);
			port->port.digitalIOport.port = arg;
			cbDConfigPort(devP->BoardNum, arg, DIGITALOUT);
			arg = va_arg(list, int);
			port->port.digitalIOport.devtype = arg;
			arg = va_arg(list, int);
			port->port.digitalIOport.bits = arg;
			if(arg)
			{
				int i;
				port->port.digitalIOport.bitarr = calloc(arg, sizeof(struct{int a; int b; int c; portPtr d;}));
				for (i = 0; i < arg; i ++)
				{
					port->port.digitalIOport.bitarr[i].bitnum = i;
					port->port.digitalIOport.bitarr[i].panel = 0;
					port->port.digitalIOport.bitarr[i].port = port;
					port->port.digitalIOport.bitarr[i].val = 0;
				}
			}
			port->port.digitalIOport.returnport = 0;
			port->port.digitalIOport.IO.source->acqchan->upLvl = port;
		}
	}
	return port;
}

void port_Save (portPtr port)
{
	if(port->type == ANALOGUE)
	{
		if(port->direction == IN_PORT)
		{
			acqchan_Save(port->port.analogueIOport.IO.acqchan);
			FmtFile(fileHandle.analysis, "%i, %i ", 	port->port.analogueIOport.channel, 
														port->port.analogueIOport.range);
		}
		if(port->direction == OUT_PORT)
		{
			source_Save(port->port.analogueIOport.IO.source);
			FmtFile(fileHandle.analysis, "%i, %i, %f ", 	port->port.analogueIOport.channel, 
														port->port.analogueIOport.range,
														port->port.analogueIOport.IO.source->biaslevel);
		}
	}
	else if(port->type == DIGITAL)
	{
		if(port->direction == IN_PORT)
		{
			acqchan_Save(port->port.digitalIOport.IO.acqchan);
			FmtFile(fileHandle.analysis, "%i, %i, %i \n",port->port.digitalIOport.devtype,
														port->port.digitalIOport.port,
														port->port.digitalIOport.bits);
		}
		if(port->direction == OUT_PORT)
		{
			int i = 0;
			source_Save(port->port.digitalIOport.IO.source);
			FmtFile(fileHandle.analysis, "%i, %i, %i ",port->port.digitalIOport.devtype,
														port->port.digitalIOport.port,
														port->port.digitalIOport.bits);
			for(i; i < port->port.digitalIOport.bits; i ++)
				FmtFile(fileHandle.analysis, ", %i ", port->port.digitalIOport.bitarr[i].val);
			FmtFile(fileHandle.analysis, "\n");
		}
	}
}

void port_Load (void *dev, portPtr port)
{
	if(dev)
	{
		if(port->type == ANALOGUE)
		{
			if(port->direction == IN_PORT)
			{										 
				acqchan_Load(dev, port->port.analogueIOport.IO.acqchan);
				ScanFile(fileHandle.analysis, "%i, %i ",	&port->port.analogueIOport.channel, 
															&port->port.analogueIOport.range);
				
			}
			if(port->direction == OUT_PORT)
			{
				source_Load (dev, port->port.analogueIOport.IO.source);
				ScanFile(fileHandle.analysis, "%i, %i, %f ",	&port->port.analogueIOport.channel, 
																&port->port.analogueIOport.range,
																&port->port.analogueIOport.IO.source->biaslevel);
				
			}
		}
		if(port->type == DIGITAL)
		{
			if(port->direction == IN_PORT)
			{
				acqchan_Load(dev, port->port.digitalIOport.IO.acqchan);
				ScanFile(fileHandle.analysis, "%i, %i, %is ",	&port->port.digitalIOport.devtype,
																&port->port.digitalIOport.port,
																&port->port.digitalIOport.bits);
				
			}
			if(port->direction == OUT_PORT)
			{
				int i = 0, temp;
				source_Load (dev, port->port.digitalIOport.IO.source);
				ScanFile(fileHandle.analysis, "%i, %i, %i, ",	&port->port.digitalIOport.devtype,
															&port->port.digitalIOport.port,
															&port->port.digitalIOport.bits);
				if(port->port.digitalIOport.measPanel)
				{	
					DiscardPanel(port->port.digitalIOport.measPanel);
					port->port.digitalIOport.measPanel = 0;
				}
				port->port.digitalIOport.IO.source->biaslevel = 0;
				boards_DigitalSourceInit(port);
				for(i; i < port->port.digitalIOport.bits; i ++)
				{
					
					ScanFile(fileHandle.analysis, "%i, ", &temp);
					port->port.digitalIOport.IO.source->biaslevel += (temp << i);
					port->port.digitalIOport.bitarr[i].val = temp;
					//boards_DigitalSourceControlCallback (port->port.digitalIOport.bitarr[i].panel, BITPANEL_BIT, EVENT_COMMIT, &port->port.digitalIOport.bitarr[i], 0,0);
				}
				port->port.digitalIOport.IO.source->SetLevel(port->port.digitalIOport.IO.source);
				//NOTE: this function sets the digital source to the loaded value							 
			}
		}												   
	}
}




/**************************^^ ports ^^ ***************************/

/**************************vv boards vv ***************************/
void boards_Save(void *dev, void (*save) (MCCdevPtr))
{
	char* path = alloca(sizeof(char)* 20);
	if(FileSelectPopup ("", "*.mcs", "", "", VAL_SAVE_BUTTON, 0, 1, 1, 0, path))
	{
		fileHandle.analysis = OpenFile (path, VAL_WRITE_ONLY, VAL_OPEN_AS_IS, VAL_ASCII);
		save(dev);
	}
}

void boards_Load(void *dev, void (*load) (MCCdevPtr))
{
	char* path = alloca(sizeof(char)* 20);
	if(FileSelectPopup ("", "*.mcs", "", "", VAL_LOAD_BUTTON, 0, 1, 1, 0, path))
	{
		fileHandle.analysis = OpenFile (path, VAL_READ_ONLY, VAL_OPEN_AS_IS, VAL_ASCII);
		load(dev);
	}
}

void boards_MenuCallback(int menubar, int menuItem, void *callbackData, int panel)
{
	char id[30], pathname [260];
	int i, file, nDevs;
	nodePtr node;
	MCCdevPtr dev;
	MCCdevTypePtr devType;
	if (menuItem == boards_util.saveMenu)		/*save PCI source setup*/
	{
		file = FileSelectPopup ("", "*.pcicfg", "*.pcicfg",
											   "Save PCI Device Configuration",
											   VAL_SAVE_BUTTON, 0, 0, 1, 0, pathname);
		if(file)
		{
			fileHandle.analysis = OpenFile (pathname, VAL_WRITE_ONLY, VAL_TRUNCATE, VAL_ASCII);
			if(fileHandle.analysis != -1)
			{
				FmtFile (fileHandle.analysis, "%s<#DAASPCIDEVSETUP: %i\n", boards_DevList.nItems);
				for(i = 0; i < boards_DevList.nItems; i++)
				{
					node = list_GetNode(boards_DevList, i);
					if(node && node->item)
					{
						dev = node->item;
						boards_DevList_SaveItem(dev);
					}
				}
				FmtFile (fileHandle.analysis, "#ENDSETUP\n");
				CloseFile(fileHandle.analysis);
			}
		}
	}
	if (menuItem == boards_util.loadMenu)		/*load PCI source setup*/
	{
		file = FileSelectPopup ("", "*.pcicfg", "*.pcicfg",
											   "Load PCI Device Configuration",
											   VAL_LOAD_BUTTON, 0, 0, 1, 0, pathname);
		if(file)
		{
			fileHandle.analysis = OpenFile (pathname, VAL_READ_ONLY, VAL_OPEN_AS_IS, VAL_ASCII);
			if(fileHandle.analysis != -1)
			{
				ScanFile (fileHandle.analysis, "%s[xt58]%i%s[dw1]", id, &nDevs);
          		if (CompareBytes (id, 0, "#DAASPCIDEVSETUP", 0, 16, 0) != 0) 
				{
              		util_MessagePopup ("Load DAAS Instrument Setup Message", "Wrong file type found...process aborted");
              		CloseFile(fileHandle.analysis);
          		}
				else
				{
					for (i = 0; i < nDevs; i++) 
					{
						if(!boards_DevList_LoadItem())
							i = nDevs;
					}
					CloseFile(fileHandle.analysis);
				}
			}
		}
	}
}

MCCdevPtr boards_Create (MCCdevTypePtr devType, int BoardNum)
{
	MCCdevPtr dev = malloc(sizeof(MCCdevType));
	if(dev)
	{
		dev->devType = devType;
		dev->name = malloc(sizeof(devType->label) * StringLength(devType->label));
		Fmt(dev->name, "%s", devType->label);
		dev->BoardNum = BoardNum;
		cbGetConfig (BOARDINFO, BoardNum, 0, BIBASEADR, &dev->baseAddr);
		dev->menuitem_id = NewMenuItem (acquire_GetMenuBar(), ACQMENUS_GPIB,
                                        dev->name, ACQMENUS_GPIB_SEP_1, 0,
                                        devType->OperateDevice, dev);
		list_AddItem(&boards_DevList, dev);
		return dev;
	}
	else
		return NULL;
}

void init_MCCdevices (void)
{
	int i, con;
	char name[30];
	MCCdevPtr dev;
	MCCdevTypePtr devType;
	util_ChangeInitMessage("PCI devices...");
	for(i = 0; i < 100; i++)
	{
		cbGetBoardName(i, name);
		if(strcmp(name, ""))
		{
			con = 0;
			devType = boards_getDevice(name);
			if(devType)
			{   //*
				con = 1;
				dev = boards_Create(devType, i);
				if(devType->CreateDevice)devType->CreateDevice(dev);
			}
			util_MessagePopup("MCC device found:", "The device found is:%s. Device [not ]connected.", name, !con);
		}
	}
	if(boards_DevList.nItems)
	{
		boards_util.saveMenu = NewMenuItem (acquire_GetMenuBar(), ACQMENUS_GPIB_PCIDEV_SUBMENU,
					"save setup", ACQMENUS_GPIB_PCIDEV_PCI_SEP,
					0, boards_MenuCallback, NULL);
		boards_util.loadMenu = NewMenuItem (acquire_GetMenuBar(), ACQMENUS_GPIB_PCIDEV_SUBMENU,
					"load setup", ACQMENUS_GPIB_PCIDEV_PCI_SEP,
					0, boards_MenuCallback, NULL);
	}
	else
	{
		if(acqG.p.setup)
		{
			DiscardMenuItem (acquire_GetMenuBar(), ACQMENUS_GPIB_PCIDEV);
			DiscardMenuItem (acquire_GetMenuBar(), ACQMENUS_GPIB_SEP_3);
		}
	}
}
/*
value to index conversion for range list
	case BIP10VOLTS:	range = 0;break;
	case BIP5VOLTS:		range = 1;break;
	case BIP4VOLTS:		range = 2;break;
	case BIP2PT5VOLTS:	range = 3;break;
	case BIP2VOLTS:		range = 4;break;
	case BIP1PT25VOLTS:	range = 5;break;
	case BIP1VOLTS:		range = 6;break;
	case BIPPT625VOLTS:	range = 7;break;
	case BIPPT5VOLTS:	range = 8;break;
	case BIPPT25VOLTS:	range = 9;break;
	case BIPPT2VOLTS:	range = 10;break;
	case BIPPT1VOLTS:	range = 11;break;
	case BIPPT05VOLTS:	range = 12;break;
	case BIPPT01VOLTS:	range = 13;break;
	case BIPPT005VOLTS:	range = 14;break;
	case BIP1PT67VOLTS:	range = 15;break;
//*/
