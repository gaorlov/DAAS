#include <utility.h>
#include <ansi_c.h>
#include <formatio.h>
#include <userint.h>
#include <rs232.h>
#include <stdarg.h>

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
#include "rs232util.h"
#include "rs232u.h"

#define FALSE 0
#define TRUE 1

#define NUM_OF_COM_PORTS 2

struct{int setup, adddev;}panels;
static listType rsDevList, rsDevTypeList, comList;

/******************************************************************************/
int rs232Write(rs232Ptr dev, char *format, ...);
int rs232Read (rs232Ptr dev, char *format, ...);



void rsSetupCallback (int menubar, int menuItem, void *callbackData, int panel);
int rs232SetupControlCallback (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int AddDevCallback (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);



void rsDevType_Update(void);
void rsDevTypeItem_AddItem(rsDevTypePtr devType);
static rsDevTypePtr rsDevTypeItem_GetItem(int i);
static rsDevTypePtr rsDevTypeItem_GetItemByID(char *id);

void rsDevList_Update(void);
static char *rsDevList_Item (rs232Ptr dev);
void rsDevList_AddItem(rs232Ptr dev);
void rsDevList_RemoveItem(rs232Ptr dev, int i);
void rsDevList_SaveItem (rs232Ptr dev);
int  rsDevList_LoadItem (void);		   
static rs232Ptr rsDevList_GetItem(int i);
static rs232Ptr rsDevList_GetItemByID(char *id);

void comPort_Update(comPtr com);
static comPtr comPort_FindItem(int ind);
static comPtr comPort_GetItem(int i);
static comPtr create_com(int port, int baud, int parity, int data, int stop, int inq, int outq);



int GenInstControlCallback (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
rs232Ptr rs232_Create(rsDevTypePtr devType, char *label, comPtr com, int logio);
void rs232_Remove (void* dev);
void rs232Operate (int menubar, int menuItem, void* callbackData, int panel);
int  rs232_InitIO (rs232Ptr dev);
void rs232_Init (void); 



/******************************************************************************/
int rs232Write(rs232Ptr dev, char *format, ...)
{
	va_list list;
	int i;
	char *msg = calloc(dev->COM->outQ, sizeof(char));
	double del;
	va_start(list, format);
	msg = util_formatParse(format, &list, msg, 0);
	i = StringLength(msg);
	del = (i*((sizeof(char)) + dev->COM->data + dev->COM->parity + dev->COM->stop))/(double)dev->COM->baud;
	if(i)
	{
		i = ComWrt (dev->COM->port, msg, StringLength(msg));
		Delay(del);
		return i;
	}
	else
		return 0;
}

int rs232Read (rs232Ptr dev, char *format, ...)
{
	int read=0, i;
	va_list list;
	char *msg = malloc(sizeof(char) * dev->COM->inQ);
	double del;
	for (i = 0; i < dev->COM->outQ; i++)
		msg[i] = 0;
	va_start(list, format);
	FlushInQ(dev->COM->port);
	read = ComRdTerm (dev->COM->port, msg, dev->COM->inQ, 13);
	Fmt(msg, "%s;", msg);
	util_formatParseRead(format, &list, msg, 0);
	if(!read)
	{
		util_MessagePopup("RS232 communication error:", "the device (%s) is not responding", dev->label);
		if(dev->iPanel)
		{
			devPanel_Remove(dev->iPanel);
			SetPanelAttribute (dev->iPanel, ATTR_DIMMED, 1);
		}
	}
	return read;
}



/******************************************************************************/


void rsSetupCallback (int menubar, int menuItem, void *callbackData, int panel)
{
	panels.setup = panels.setup? panels.setup : LoadPanel(utilG.p, "rs232u.uir", SETUP);
	DisplayPanel(panels.setup);
}

int rs232SetupControlCallback (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	int i, p, file, nDevs;
	rs232Ptr dev;
	rsDevTypePtr devType;
	char id[30], pathname[260];
	switch (control)
	{
		case SETUP_ADDCOMDEV:
			if(event == EVENT_COMMIT)
			{
				panels.adddev = panels.adddev? panels.adddev : LoadPanel(utilG.p, "rs232u.uir", ADDPANEL);
				rsDevType_Update();
				rsDevList_Update();
				DisplayPanel(panels.adddev);
			}
			break;
		case SETUP_SAVESETUP:
			if(event == EVENT_COMMIT)
			{
				file = FileSelectPopup ("", "*.rscfg", "*.rscfg",
											   "Save rs232 Device Configuration",
											   VAL_SAVE_BUTTON, 0, 0, 1, 0, pathname);
				if(file)
				{
					fileHandle.analysis = OpenFile (pathname, VAL_WRITE_ONLY, VAL_TRUNCATE, VAL_ASCII);
					if(fileHandle.analysis != -1)
					{
						FmtFile (fileHandle.analysis, "%s<#DAASCOMDEVSETUP: %i\n", rsDevList.nItems);
						for(i = 0; i < rsDevList.nItems; i++)
						{
							dev = rsDevList_GetItem(i);
							rsDevList_SaveItem(dev);
						}
						FmtFile (fileHandle.analysis, "#ENDSETUP\n");
						CloseFile(fileHandle.analysis);
					}
				}
			}
			break;
		case SETUP_LOADSETUP:
			if(event == EVENT_COMMIT)
			{
				file = FileSelectPopup ("", "*.rscfg", "*.rscfg",
											   "Load rs232 Device Configuration",
											   VAL_LOAD_BUTTON, 0, 0, 1, 0, pathname);
				if(file)
				{
					fileHandle.analysis = OpenFile (pathname, VAL_READ_ONLY, VAL_OPEN_AS_IS, VAL_ASCII);
					if(fileHandle.analysis != -1)
					{
						ScanFile (fileHandle.analysis, "%s[xt58]%i%s[dw1]", id, &nDevs);
            			if (CompareBytes (id, 0, "#DAASCOMDEVSETUP", 0, 16, 0) != 0) {
                			util_MessagePopup ("Load DAAS Instrument Setup Message", "Wrong file type found...process aborted");
                			CloseFile(fileHandle.analysis);
                			return 0;
            			}
						for (i = 0; i < nDevs; i++) 
						{
							if(!rsDevList_LoadItem())
								i = nDevs;
						}
			            CloseFile(fileHandle.analysis);
					}
				}
				rsDevList_Update();
			}
			break;
		case SETUP_LIST:
			if(event == EVENT_COMMIT)
			{
				GetCtrlVal (panel, control, id);
				GetIndexFromValue(panel, control, &i, id);
				dev = rsDevList_GetItemByID(id);
				devType = dev->devType;
				if(devType->OperateDevice)
					devType->OperateDevice(acquire_GetMenuBar(), dev->menuitem_id, dev, acqG.p.setup);
			}
			if(event == EVENT_KEYPRESS && (eventData1 == VAL_BACKSPACE_VKEY || eventData1 == VAL_FWD_DELETE_VKEY))
			{
				if(((utilG.acq.status == ACQ_BUSY || utilG.acq.status == ACQ_PAUSED)  && ConfirmPopup("Remove device:", "This will stop acquisition. Are you sure?")) || (utilG.acq.status != ACQ_BUSY && utilG.acq.status != ACQ_PAUSED))
				{
					if(utilG.acq.status == ACQ_BUSY || utilG.acq.status == ACQ_PAUSED)utilG.acq.status = ACQ_TERMINATE;
					GetNumListItems(panel, control, &i);
					if(i)
					{
						GetCtrlVal (panel, control, id);
						GetIndexFromValue(panel, control, &i, id);
						dev = rsDevList_GetItemByID(id);
						devType = dev->devType;
						if(devType->RemoveDevice)
						{
							DiscardMenuItem (acquire_GetMenuBar(), dev->menuitem_id);
							devPanel_Remove(dev->iPanel);
							devType->RemoveDevice(dev);
							DeleteListItem(panel, control, i, 1);
							rsDevList_RemoveItem(dev, dev->id);
						}
						updateGraphSource();
						rsDevList_Update();
					}
				}
			}
			break;
	}
	return 0;
}
int AddDevCallback (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	rs232Ptr dev;
	rsDevTypePtr devType;
	comPtr comP;
	int i, port;
	double f;
	char label[30], id[20];
	switch(control)
	{
		case ADDPANEL_INSTRUMENT:
			if(event == EVENT_COMMIT)
			{
				GetCtrlVal(panel, control, id);
				devType = rsDevTypeItem_GetItemByID(id);
				if(devType)
					SetCtrlVal(panel, ADDPANEL_LABEL, devType->label);
			}			
			break;
		case ADDPANEL_ADDINSTR:
			if(event == EVENT_COMMIT)
			{
				GetNumListItems (panel, ADDPANEL_COMPORT, &port);
				if(port)
				{
					GetCtrlVal(panel, ADDPANEL_INSTRUMENT, id);
					GetCtrlVal(panel, ADDPANEL_COMPORT, &port);
					comP = comPort_FindItem(port);
					GetCtrlVal(panel, ADDPANEL_LABEL, label);
					GetCtrlVal(panel, ADDPANEL_BAUD, &comP->baud);
					GetCtrlVal(panel, ADDPANEL_DATA, &comP->data);
					GetCtrlVal(panel, ADDPANEL_INQ, &comP->inQ);
					GetCtrlVal(panel, ADDPANEL_OUTQ, &comP->outQ);
					GetCtrlVal(panel, ADDPANEL_PARITY, &comP->parity);
					GetCtrlVal(panel, ADDPANEL_STOP, &comP->stop);
					devType = rsDevTypeItem_GetItemByID(id); 
					if(devType)
					{
						dev = rs232_Create(devType, label, comP, FALSE);
						if(devType->CreateDevice)devType->CreateDevice(dev);
						rsDevList_Update();
						rsDevType_Update();
						comPort_Update(comP);
							
					}
				}
				else
					util_MessagePopup("Error:", "There are no open com ports.");
				HidePanel(panel);
			}
			break;
		case ADDPANEL_COMPORT:
			if(event == EVENT_COMMIT)
			{
				GetCtrlVal(panel, control, &i);
				comP = comPort_FindItem(i);
				comPort_Update(comP);
			}
			break;
		case ADDPANEL_PARITY:
		case ADDPANEL_BAUD:
		case ADDPANEL_DATA:
		case ADDPANEL_STOP:
		case ADDPANEL_OUTQ:
		case ADDPANEL_INQ:
			if(event == EVENT_COMMIT)
			{
				GetNumListItems (panel, ADDPANEL_COMPORT, &port);
				if(port)
				{
					GetCtrlVal(panel, ADDPANEL_COMPORT, &port);
					comP = comPort_FindItem(port);				
					GetCtrlVal(panel, ADDPANEL_BAUD, &comP->baud);
					GetCtrlVal(panel, ADDPANEL_DATA, &comP->data);
					GetCtrlVal(panel, ADDPANEL_INQ, &comP->inQ);
					GetCtrlVal(panel, ADDPANEL_OUTQ, &comP->outQ);
					GetCtrlVal(panel, ADDPANEL_PARITY, &comP->parity);
					GetCtrlVal(panel, ADDPANEL_STOP, &comP->stop);
				}
			}
			break;
	}
	return 0;
}


/******************************************************************************/ 

void rsDevType_Update()
{
	rsDevTypePtr devType;
	comPtr com;
	int i;
	char name[3], id[30];
	DeleteListItem(panels.adddev, ADDPANEL_COMPORT, 0, -1);
	DeleteListItem (panels.adddev, ADDPANEL_INSTRUMENT, 0, -1);
	for(i = 0; i < rsDevTypeList.nItems; i++)
	{
		devType = rsDevTypeItem_GetItem(i);
		InsertListItem (panels.adddev, ADDPANEL_INSTRUMENT, 0, devType->label, devType->id);
	}
	GetCtrlVal(panels.adddev, ADDPANEL_INSTRUMENT, id);
	devType = rsDevTypeItem_GetItemByID(id);
	SetCtrlVal(panels.adddev, ADDPANEL_LABEL, devType->label);
	for(i = 0; i < comList.nItems; i++)
	{
		com = comPort_GetItem(i);
		if(!com->inUse)
		{
			Fmt(name, "%i", com->port);
			InsertListItem(panels.adddev, ADDPANEL_COMPORT, -1, name, com->port);
		}
	}
	GetNumListItems (panels.adddev, ADDPANEL_COMPORT, &i);
	SetCtrlAttribute(panels.adddev, ADDPANEL_PARITY, ATTR_DIMMED, !i);
	SetCtrlAttribute(panels.adddev, ADDPANEL_BAUD, ATTR_DIMMED, !i);
	SetCtrlAttribute(panels.adddev, ADDPANEL_DATA, ATTR_DIMMED, !i);
	SetCtrlAttribute(panels.adddev, ADDPANEL_STOP, ATTR_DIMMED, !i);
	SetCtrlAttribute(panels.adddev, ADDPANEL_OUTQ, ATTR_DIMMED, !i);
	SetCtrlAttribute(panels.adddev, ADDPANEL_INQ, ATTR_DIMMED, !i);
}


void rsDevTypeItem_AddItem(rsDevTypePtr devType)
{
	list_AddItem(&rsDevTypeList, devType);
}

static rsDevTypePtr rsDevTypeItem_GetItem (int i)
{
	nodePtr node;
	node = list_GetNode(rsDevTypeList, i);
	if(node)
		return node->item;
	else
		return NULL;
}

static rsDevTypePtr rsDevTypeItem_GetItemByID(char *id)
{
	int i = 0;
	nodePtr node;
	rsDevTypePtr devType;
	do{
		node = list_GetNode (rsDevTypeList, i);
		i++;
		if(node)
			devType = node->item;
		else
			devType = NULL;
	}while(node && strcmp(devType->id, id));
	return devType;
}

void rsDevList_Update()
{
	int i;
	rs232Ptr dev;
	rsDevTypePtr devType;
	DeleteListItem(panels.setup, SETUP_LIST, 0, -1);
	for(i = 0; i < rsDevList.nItems; i++)
	{
		dev = rsDevList_GetItem(i);
		devType = dev->devType;
		InsertListItem(panels.setup, SETUP_LIST, -1, rsDevList_Item(dev), devType->id);
	}
}

static char *rsDevList_Item (rs232Ptr dev)
{
    char *item, logio[5];

    item = "                                                                  ";
    if (dev->logio) Fmt (logio, "Yes"); else Fmt (logio, "No");
    if (StringLength (dev->label) > 30) Fmt(item, "%s<%s[w23]...", dev->label);
        else Fmt (item, dev->label);
    Fmt (item, "%s[a]<\033p%ic%i", 300, dev->COM->port);
    Fmt (item, "%s[a]<\033p%ic%s", 445, logio);
    return item;
}

void rsDevList_AddItem(rs232Ptr dev)
{
	list_AddItem(&rsDevList, dev);
}

void rsDevList_RemoveItem(rs232Ptr dev, int i)
{
	rsDevTypePtr devType;

	dev->COM->inUse = 0;
	dev->COM->device = NULL;
	if(dev->iPanel)DiscardPanel(dev->iPanel);
	devType = dev->devType;
	if(devType->RemoveDevice) devType->RemoveDevice(dev);
	i = list_FindItem(rsDevList, dev);
	list_RemoveItem(&rsDevList, i, TRUE);
}

void rsDevList_SaveItem (rs232Ptr dev)
{
	rsDevTypePtr devType = dev->devType;
	FmtFile(fileHandle.analysis, "Device ID :%s\n", devType->id);
	FmtFile(fileHandle.analysis, "Label     :%s;\n", dev->label);
	FmtFile(fileHandle.analysis, "Log IO    :%i\n", dev->logio);
	FmtFile(fileHandle.analysis, "COM port  :%i\n", dev->COM->port);
	FmtFile(fileHandle.analysis, "COM parity:%i\n", dev->COM->parity);
	FmtFile(fileHandle.analysis, "COM baud  :%i\n", dev->COM->baud);
	FmtFile(fileHandle.analysis, "COM data  :%i\n", dev->COM->data);
	FmtFile(fileHandle.analysis, "COM stop  :%i\n", dev->COM->stop);
	FmtFile(fileHandle.analysis, "COM outq  :%i\n", dev->COM->outQ);
	FmtFile(fileHandle.analysis, "COM inq   :%i\n", dev->COM->inQ);
	if(devType->SaveDevice)devType->SaveDevice(dev);
}

int rsDevList_LoadItem (void)
{
	rs232Ptr dev;
	rsDevTypePtr devType;
	comPtr com;
	char msg[260];
	int logio, port, parity, baud, data, stop, outQ, inQ;
	ScanFile(fileHandle.analysis, "Device ID :%s", msg);
	devType = rsDevTypeItem_GetItemByID(msg);
	if(devType)
	{
		ScanFile(fileHandle.analysis, "Label     :%s[t59]", msg);
		ScanFile(fileHandle.analysis, "Log IO    :%i", &logio);
		ScanFile(fileHandle.analysis, "COM port  :%i", &port);
		ScanFile(fileHandle.analysis, "COM parity:%i", &parity);
		ScanFile(fileHandle.analysis, "COM baud  :%i", &baud);
		ScanFile(fileHandle.analysis, "COM data  :%i", &data);
		ScanFile(fileHandle.analysis, "COM stop  :%i", &stop);
		ScanFile(fileHandle.analysis, "COM outq  :%i", &outQ);
		ScanFile(fileHandle.analysis, "COM inq   :%i", &inQ);
		com = comPort_FindItem(port);
		com->port = port;
		com->baud = baud;
		com->parity = parity;
		com->data = data;
		com->stop = stop;
		com->inQ  = inQ;
		com->outQ = outQ;
		dev = rs232_Create(devType, msg, com, logio);
		if(devType->CreateDevice)devType->CreateDevice(dev);
		if(devType->LoadDevice)devType->LoadDevice(dev);
		return 1;
	}
	else
	{
		util_MessagePopup("com port configuration load error:", "specified device (%s) drivers could not be found.", msg);
		return 0;
	}
}

static rs232Ptr rsDevList_GetItem(int i)
{
	nodePtr node;
	node = list_GetNode(rsDevList, i);
	if(node)
		return node->item;
	else
		return NULL;
}

static rs232Ptr rsDevList_GetItemByID(char *id)
{
	int i = 0;
	nodePtr node;
	rs232Ptr dev;
	rsDevTypePtr devType;
	do{
		node = list_GetNode (rsDevList, i);
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

void comPort_Update(comPtr com)
{
	int i;
	comPtr tempCom = NULL;
	SetCtrlIndex (panels.adddev, ADDPANEL_PARITY, com->parity);
	SetCtrlIndex (panels.adddev, ADDPANEL_DATA, com->data - 5);
	SetCtrlIndex (panels.adddev, ADDPANEL_STOP, com->stop-1);
	SetCtrlVal(panels.adddev, ADDPANEL_OUTQ, com->outQ);
	SetCtrlVal(panels.adddev, ADDPANEL_INQ, com->inQ);
	switch(com->baud)
	{		
		case 110: i = 0;break;
		case 300: i = 1;break;
		case 600: i = 2;break;
		case 1200: i = 3;break;
		case 2400: i = 4;break;
		case 4800: i = 5;break;
		case 9600: i = 6;break;
		case 14400: i = 7;break;
		case 19200: i = 8;break;
		case 28800: i = 9;break;
		case 38400: i = 10;break;
		case 56000: i = 11;break;
		case 57600: i = 12;break;
		case 115200: i = 13;break;
		case 128000: i = 14;break;
		case 256000: i = 15;break;
	}
	SetCtrlIndex (panels.adddev, ADDPANEL_BAUD, i);
}

static comPtr comPort_FindItem(int ind)
{
	int i;
	nodePtr node;
	comPtr com, retCom = NULL;
	for(i = 0; i < comList.nItems; i++)
	{
		node = list_GetNode(comList, i);
		com = node->item;
		if(com->port == ind){ i = comList.nItems; retCom = com;}
	}
	return retCom;
}

static comPtr comPort_GetItem(int i)
{
	nodePtr node;
	node = list_GetNode(comList, i);
	if(node)
		return node->item;
	else
		return NULL;
}

static comPtr create_com(int port, int baud, int parity, int data, int stop, int inq, int outq)
{
	int status;
	comPtr com = malloc(sizeof(comType));
	
	com->port = port;
	com->baud = baud;
	com->parity = parity;
	com->data = data;
	com->stop = stop;
	com->inQ  = inq;
	com->outQ = outq;
	com->inUse = 0;
	com->device = NULL;
	Fmt(com->name, "COM%i", com->port);
	status = OpenComConfig(com->port, com->name, com->baud, com->parity, com->data, com->stop, com->inQ, com->outQ);
	if(status >= 0)	
		return com;
	else
	{
		util_MessagePopup("Com port open error", "Com port %i cannot be opened.", &com->port);
		free(com);
		return NULL;
	}
}

/******************************************************************************/ 

int GenInstControlCallback (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	switch (control)
	{
		case GEN_INST_SEND:
			if(event == EVENT_COMMIT)
			{
				rs232Ptr dev = callbackData;
				char msg[1024] = " ";
				int term;
				GetCtrlVal(panel, GEN_INST_SENDBOX, msg);
				GetCtrlVal(panel, GEN_INST_TERM, &term);
				SetPanelAttribute(panel, ATTR_DIMMED, 1);
				ProcessSystemEvents();
				rs232Write(dev, "%s[\r][\n]", msg, !term, term);
				SetPanelAttribute(panel, ATTR_DIMMED, 0);
			}
			break;
		case GEN_INST_POLL:
			if(event == EVENT_COMMIT)
			{
				rs232Ptr dev = callbackData;
				char msg[1024] = " ";
				int term;
				GetCtrlVal(panel, GEN_INST_SENDBOX, msg);
				GetCtrlVal(panel, GEN_INST_TERM, &term);
				SetPanelAttribute(panel, ATTR_DIMMED, 1);
				ProcessSystemEvents();
				rs232Write(dev, "%s[\r][\n]", msg, !term, term);
				ComRdTerm(dev->COM->port, msg, dev->COM->outQ, 13);
				Fmt(msg, "%s\n", msg);
				SetPanelAttribute(panel, ATTR_DIMMED, 0);
				SetCtrlVal(panel, GEN_INST_READBOX, msg);	
				
			}
			break;
		case GEN_INST_READ:
			if(event == EVENT_COMMIT)
			{
				rs232Ptr dev = callbackData;
				char msg[1024] = " ";
				SetPanelAttribute(panel, ATTR_DIMMED, 1);
				ProcessSystemEvents();
				ComRdTerm(dev->COM->port, msg, dev->COM->outQ, 13);
				Fmt(msg, "%s\n", msg);
				SetPanelAttribute(panel, ATTR_DIMMED, 0);
				SetCtrlVal(panel, GEN_INST_READBOX, msg);	
			}
			break;
		case GEN_INST_CLEAR:
			if(event == EVENT_COMMIT)
				ResetTextBox (panel, GEN_INST_READBOX, "");
			break;
			
	}
	return 0;
}

rs232Ptr rs232_Create(rsDevTypePtr devType, char *label, comPtr com, int logio)
{
	rs232Ptr dev = malloc(sizeof(rs232Type));
	
	Fmt(dev->label, "%s", label);
	dev->devType = devType;
	dev->COM = com;
	dev->COM->inUse = 1;
	dev->logio = logio;
	dev->device = NULL;
	dev->COM->device = dev;
	dev->iPanel = 0;
	dev->menuitem_id = NewMenuItem (acquire_GetMenuBar(), ACQMENUS_GPIB,
                                        dev->label, ACQMENUS_GPIB_SEP_1, 0,
                                        devType->OperateDevice, dev);
    rsDevList_AddItem(dev);
	if (!devType->InitDevice(dev)) {
        free (dev);
        return NULL;
    }
    if(devType->UpdateReadings) dev->UpdateReadings = devType->UpdateReadings;
	if(devType->OperateDevice)  dev->OperateDevice  = devType->OperateDevice;
	return dev;
}

void rs232_Remove (void* dev) 
{
	rs232Ptr tempDev = dev;
	free(tempDev);
}

void rs232Operate (int menubar, int menuItem, void* callbackData, int panel)
{
	rs232Ptr dev = callbackData;
	dev->iPanel = dev->iPanel? dev->iPanel : LoadPanel(utilG.p, "rs232u.uir", GEN_INST);
	
	SetCtrlAttribute(dev->iPanel, GEN_INST_SEND, ATTR_CALLBACK_DATA, dev);
	SetCtrlAttribute(dev->iPanel, GEN_INST_POLL, ATTR_CALLBACK_DATA, dev);
	SetCtrlAttribute(dev->iPanel, GEN_INST_READ, ATTR_CALLBACK_DATA, dev);
	DisplayPanel(dev->iPanel);
}

int rs232_InitIO (rs232Ptr dev)
{
	return TRUE;
}

void rs232_Init (void)
{
	rsDevTypePtr devType;
	int i;
	comPtr com;
	panels.adddev = 0;
	panels.setup = 0;
	if(utilG.acq.status != ACQ_NONE)
	{
		for(i = 1; i <= NUM_OF_COM_PORTS; i++)
		{
			com = create_com(i, 9600, 0, 8, 2, 512, 512);
			if(com)
				list_AddItem(&comList, com);
		}
		util_ChangeInitMessage("rs232 devices...");
		InstallMenuCallback(acquire_GetMenuBar(), ACQMENUS_GPIB_RS232SETUP, rsSetupCallback, 0);
		devType = malloc(sizeof(rsDevTypeItem));
		if(devType)
		{
			Fmt(devType->label, "generic rs232 device");
			Fmt(devType->id, "generic");
			devType->CreateDevice = 	NULL;
			devType->RemoveDevice = 	rs232_Remove;
			devType->InitDevice = 		rs232_InitIO;
			devType->SaveDevice = 		NULL;
			devType->LoadDevice = 		NULL;
			devType->OperateDevice = 	rs232Operate;
			devType->UpdateReadings = 	NULL;
			rsDevTypeItem_AddItem(devType);
		}
	}
}

