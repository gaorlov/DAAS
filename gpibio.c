#include <utility.h>
#include <ansi_c.h>
#include <formatio.h>
#include <userint.h>
#include <gpib.h>
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
#include "curveop.h"
#include "acquire.h"
#include "acquireu.h"
#include "gpibio.h"
#include "gpibiou.h"
#include "source.h"

#define FALSE 0
#define TRUE 1

#define MAX_INSTRUMENTS 32
#define MAX_INSTRUCTIONS_SAVED 102

static gpibioType ifc;

static listType devList, devTypeList, devPanelList;
static int devListp = 0, nDevsConnected;
static /*unsigned*/ short devsConnected[32];

void gpibPrint(gpibioPtr dev, const char *format, ...);
void charRecognition(char type, void *arg, void *buffer);
double gpib_GetDoubleVal(gpibioPtr dev, char *msg);
void gpib_GetCharVal(gpibioPtr dev, char *msg, char *ret);
int  gpib_GetIntVal(gpibioPtr dev, char *msg);

static char *dev_Status (gpibioPtr dev);
static int  dev_DuplicateAddr (unsigned short paddr, unsigned short saddr);
static int  dev_OnBus (unsigned short paddr, unsigned short saddr);
static void dev_CheckforGPIBErrors (gpibioPtr dev);
static void dev_UpdatePanel (gpibioPtr dev);
void        dev_StoreCommand (gpibioPtr dev, char *cmd);
void        dev_StoreResponse (gpibioPtr dev, char *rsp);
void OperateDevCallback (int menubar, int menuItem, void *callbackData, int panel);

static void ifc_StoreCommand (char *cmd);
static void ifc_StoreResponse (char *rsp);
static void ifc_UpdatePanel (void);
static void ifc_InitInterface (void);
static void ifc_Init (void);
static void IFCCallback (int menubar,  int menuItem, void *callbackData, int panel);

static char     *devList_Item (gpibioPtr dev);
static gpibioPtr devList_GetItem (int i);
static gpibioPtr devList_GetSelection(int panel);
static void      devList_UpdatePanel (void);
static void      devList_SaveItem (gpibioPtr dev);
static void      devList_LoadItem (void);
static void      SetupCallback (int menubar, int menuItem, void *callbackData, int panel);

void       devTypeList_AddItem (devTypePtr devType);
static devTypePtr devTypeList_GetItem (int i);

static int       InitGenericDevice (gpibioPtr dev);

void devPanel_UpdateReadings(void);
void devPanel_Add (int panel, void *ptr, UpdateReadingsPtr UpdateReadings);
void devPanel_Remove (int panel);

void        gpibio_Init (void);
void        gpibio_Exit (void);
int         gpibio_Addr (gpibioPtr dev);
int         gpibio_Clear (gpibioPtr dev);
int         gpibio_Local (int done, gpibioPtr dev);
int         gpibio_Remote (gpibioPtr dev);
int         gpibio_Out (gpibioPtr dev, char *cmd);
int         gpibio_In (gpibioPtr dev, char *msg_in );
int         gpibio_SRQ (gpibioPtr dev);
int         gpibio_GetStatusByte (gpibioPtr dev, short *status);
gpibioPtr   gpibio_CreateDevice (int id, devTypePtr devType, char *label, unsigned short paddr,
                            unsigned short saddr, int logio);
void        gpibio_ScanforConnectedDevs (void);
int         gpibio_DeviceMatch (gpibioPtr dev, char *cmd, char *id);


/**********************************************************************/
void gpibPrint(gpibioPtr dev, const char *format, ...)
{
	va_list List;
	char buffer[260] = "", tempstr[90];
	int i, length = StringLength(format), cond;
	void *arg;
	
	va_start(List, format);
	for (i = 0; i<length; i++)
	{
		strcpy(tempstr , "");
		if(format[i] == '%')
		{
			i++;
			arg = va_arg(List, void*);
			charRecognition(format[i], arg, buffer);
		}
		else if (format[i] == '[') // syntax:[y]:[z]. if((int) x){y}else{z}. can't nest [] ifs.
		{						   // x does not get passed by reference. it is an int, so pass it as such, damn it!
			cond = va_arg(List, int);
			i++;
			while(format[i] != ']')
			{
				if(format[i] == '%')
				{
					i++;
					arg = va_arg(List, void*);
					if(cond)
						charRecognition(format[i], arg, tempstr);
				}
				else
					Fmt(tempstr, "%s%c", tempstr, format[i]);
				i++;
			}
			if(cond)
				Fmt(buffer, "%s%s", buffer, tempstr);
			i++;
			strcpy(tempstr , "");
			if(format[i] == ':')
			{
				i++;
				if(format[i] == '[')
				{
					i++;
					while(format[i] != ']')
					{
						if(format[i] == '%')
						{
							i++;
							arg = va_arg(List, void*);
							if(!cond)
								charRecognition(format[i], arg, tempstr);
						}
						else
							Fmt(tempstr, "%s%c", tempstr, format[i]);
						i++;
					}
				}
			}
			else
				i--;
			if(!cond)
				Fmt(buffer, "%s%s", buffer, tempstr);
		}
		else
			Fmt(buffer, "%s%c", buffer, format[i]);
	}
	va_end(List);
	gpibio_Out(dev, buffer);
}

void charRecognition(char type, void *arg, void *buffer)
{
	int intType, *intPtr, cond;
	char charType, *string;
	double doubleType, *doublePtr;
	switch(type)
	{
		case 'i':
			intPtr = arg;
			intType = *intPtr;
			Fmt(buffer, "%s%i", buffer, intType);
			break;
		case 'f':
			doublePtr = arg;
			doubleType = *doublePtr;
			Fmt(buffer, "%s%f", buffer, doubleType);
			break;
		case 's':
			string = arg;
			Fmt(buffer, "%s%s", buffer, string);
			break;
		case 'c':
			string = arg;
			charType = *string;
			Fmt(buffer, "%s%c", buffer, charType);
			break;
	}
}

double gpib_GetDoubleVal(gpibioPtr dev, char *msg)
{
    char cmd[256];
	double doubVal;
	gpibio_Out (dev, msg);
    gpibio_In (dev, cmd);
    Scan (cmd, "%s>%f", &doubVal);
    return doubVal;
}
void gpib_GetCharVal(gpibioPtr dev, char *msg, char *ret)
{
	gpibio_Out (dev, msg);
    gpibio_In (dev, ret);
	ret;
}
int gpib_GetIntVal(gpibioPtr dev, char *msg)
{
	char cmd[256];
	int intVal;
    gpibio_Out (dev, msg);
    gpibio_In (dev, cmd);
    Scan (cmd, "%s>%i", &intVal);
    return intVal;
}

/**********************************************************************/
void gpibio_Init (void)
{
    devTypePtr devType;
    if (utilG.acq.status != ACQ_NONE) {

        util_ChangeInitMessage ("GPIB and connected instruments...");
        ifc_Init();
        InstallMenuCallback (acquire_GetMenuBar(), ACQMENUS_GPIB_SETUP_SETUP, SetupCallback, 0);

        gpibio_ScanforConnectedDevs();
        devType = malloc (sizeof (devTypeItem));
        if (devType) {
            Fmt (devType->label, "Generic Instrument");
            Fmt (devType->id, "");
            devType->CreateDevice = NULL;
            devType->InitDevice = InitGenericDevice;
            devType->OperateDevice = OperateDevCallback;
            devType->UpdateReadings = NULL;
            devType->LoadDevice = NULL;
            devType->SaveDevice = NULL;
            devType->RemoveDevice = NULL;
            devTypeList_AddItem (devType);
        }
    }
}

void gpibio_ScanforConnectedDevs (void)
{
    /*unsigned short*/ short devs[32];
    int i;

    for (i = 0; i < 31; i++) { devs[i] = i; devsConnected[i] = 0;}
    devs[i] = NOADDR;

    ibonl (0, 0);
    SendIFC (0);
    FindLstn (0, devs, devsConnected, 32); nDevsConnected = ibcntl-1;
    dev_CheckforGPIBErrors (&ifc);
}

int gpibio_DeviceMatch (gpibioPtr dev, char *cmd, char *id)
{
    char buffer[260];
    unsigned short addr = gpibio_Addr(dev);

    Send (0, addr, cmd, (unsigned long)StringLength (cmd), NLend);
    Receive (0, addr, buffer, (unsigned long)260, STOPend);
    if ((ibsta & ERR) || (FindPattern (buffer, 0, StringLength (buffer), id, 0, 0) == -1)) {
        Fmt (buffer, "%s not located at address (%i[b2], %i[b2])",
             dev->label, dev->paddr, dev->saddr);
        MessagePopup ("GPIB Add instrument message", buffer);
        return FALSE;
    }
    return TRUE;
}

void gpibio_Exit (void)
{
    int i;
    gpibioPtr dev;
    devTypePtr devType;

    if (utilG.acq.status != ACQ_NONE) {
        
        DiscardPanel (ifc.panel);

        for (i = 0; i < devList.nItems; i++) {
            dev = devList_GetItem (i);
            if (dev->device) {
                gpibio_Local (TRUE, dev);
                devType = devTypeList_GetItem (dev->id);
                if (devType->RemoveDevice) devType->RemoveDevice (dev->device);
            }

            
            DiscardPanel (dev->panel);
            free (dev);
        }

        list_RemoveAllItems (&devList, FALSE);
        list_RemoveAllItems (&devTypeList, TRUE);
    }
    acquire_Exit();
}

gpibioPtr gpibio_CreateDevice (int id, devTypePtr devType, char *label, unsigned short paddr,
                            unsigned short saddr, int logio)
{
    gpibioPtr dev;

    dev = malloc (sizeof (gpibioType));
    if (!dev) {
        util_OutofMemory ("GPIB Instrument Message");
        return NULL;
    }

    dev->id = id;
    Fmt (dev->label, label);
    dev->paddr = paddr;
    dev->saddr = saddr;
    dev->logio = logio;
    dev->nIO = 0;
    dev->device = NULL;
	dev->iPanel = 0;
	dev->devType = devType;

    if (!devType->InitDevice(dev)) {
        free (dev);
        return NULL;
    }

    dev->menuitem_id = NewMenuItem (acquire_GetMenuBar(), ACQMENUS_GPIB,
                                        dev->label, ACQMENUS_GPIB_SEP_1, 0,
                                        devType->OperateDevice, dev);
    list_AddItem(&devList, dev);

    dev->UpdateReadings = devType->UpdateReadings;

    dev->panel = LoadPanel (0, "gpibiou.uir", DEV);
    
    SetPanelPos (dev->panel, VAL_AUTO_CENTER, VAL_AUTO_CENTER);
    util_InitClose (dev->panel, DEV_CLOSE, FALSE);

    SetCtrlAttribute (dev->panel, DEV_LABEL, ATTR_CALLBACK_DATA, dev);
    SetCtrlAttribute (dev->panel, DEV_COMMAND, ATTR_CALLBACK_DATA, dev);
    SetCtrlAttribute (dev->panel, DEV_LOGIO, ATTR_CALLBACK_DATA, dev);
    SetCtrlAttribute (dev->panel, DEV_DISABLE, ATTR_CALLBACK_DATA, dev);
    SetCtrlAttribute (dev->panel, DEV_CLEAR, ATTR_CALLBACK_DATA, dev);

    return dev;
}

int gpibio_GetStatusByte (gpibioPtr dev, /*unsigned*/ short *status)
{
    char msg[80];
    if (ifc.status == DEV_REMOTE) {
        if (dev->status == DEV_REMOTE) {
            ReadStatusByte (0, gpibio_Addr(dev), status);
            dev_CheckforGPIBErrors(dev);
        }
        else *status = 0;
        if (ifc.logio || dev->logio) {
            Fmt (msg, "READ STATUS BYTE");
            ifc_StoreCommand(msg);
            dev_StoreCommand (dev, msg);
            Fmt (msg, "%s[a]<: %i", *status);
            ifc_StoreResponse (msg);
            dev_StoreResponse (dev, msg);
        }
    }
    return 0;
}

int gpibio_SRQ (gpibioPtr dev)
{
    short srq;
    char msg[80];

    srq = 0;
    if (ifc.status == DEV_REMOTE) {
        TestSRQ (0, &srq);
        dev_CheckforGPIBErrors (&ifc);
        if (ifc.logio) {
            Fmt (msg, "TEST SRQ");
            ifc_StoreCommand (msg);
            Fmt (msg, "%s[a]<: %i", srq);
            ifc_StoreResponse (msg);
        }
/*      if (srq) {
            Fmt (msg, "SRQ on %s", dev->label);
            MessagePopup ("GPIB SRQ Message", msg);
        } */
    }
    return srq;
}

int gpibio_In (gpibioPtr dev, char *msg_in)
{
    char msg[260];
	
    FillBytes (msg_in, 0, 255, 0X00);

    if (ifc.status == DEV_REMOTE) {
        if (dev->status == DEV_REMOTE) {
            Receive (0, gpibio_Addr(dev), msg_in, (unsigned long)256, STOPend);
            dev_CheckforGPIBErrors(dev);
        }
        else Fmt (msg_in, "[DISABLED]");

        if (ifc.logio || dev->logio) {
            Fmt (msg, "RECEIVE: %s", msg_in);
            ifc_StoreResponse (msg);
            dev_StoreResponse (dev, msg);
        }
    }
    return 0;
}

int gpibio_Out (gpibioPtr dev, char *cmd)
{
    char msg[80];
    unsigned short addr = gpibio_Addr(dev);

    if (ifc.status == DEV_REMOTE) {
        if (dev->status == DEV_REMOTE) {
			Send(0, addr, cmd, (unsigned long)StringLength(cmd), NLend);
    		dev_CheckforGPIBErrors(dev);
        }

        if (ifc.logio || dev->logio) {
            Fmt (msg, "SEND: %s", cmd);
            ifc_StoreCommand (msg);
            dev_StoreCommand (dev, msg);
        }
    }
    return 0;
}

int gpibio_Local (int done, gpibioPtr dev)
{
    Addr4882_t addr[3];
    char msg[80];

    addr[0] = gpibio_Addr(dev);
    addr[1] = NOADDR;
    if (ifc.status == DEV_REMOTE) {
        EnableLocal (0, addr);
        dev->status = DEV_LOCAL;
        if (!done) {
            dev_CheckforGPIBErrors(dev);
            if (ifc.logio || dev->logio) {
                Fmt (msg, "LOCAL : %i", addr[0]);
                ifc_StoreCommand (msg);
                dev_StoreCommand(dev, msg);
            }
        }
    }
    return 0;
}

int gpibio_Remote (gpibioPtr dev)
{
    Addr4882_t addr[3];
    char msg[260];

    addr[0] = gpibio_Addr(dev);
    addr[1] = NOADDR;

    if (ifc.status == DEV_REMOTE) {
        EnableRemote (0, addr);
        dev->status = DEV_REMOTE;
        dev_CheckforGPIBErrors(dev);
        if (ifc.logio || dev->logio) {
            Fmt (msg, "REMOTE : %i", addr[0]);
            ifc_StoreCommand (msg);
            dev_StoreCommand(dev, msg);
        }
    }
    return 0;
}

int gpibio_Clear (gpibioPtr dev)
{
    char msg[260];
    if (ifc.status == DEV_REMOTE) {
        DevClear (0, gpibio_Addr(dev));
        dev_CheckforGPIBErrors(dev);
        if (ifc.logio || dev->logio) {
            Fmt (msg, "CLEAR : %i", gpibio_Addr(dev));
            ifc_StoreCommand (msg);
            dev_StoreCommand (dev, msg);
        }
    }
    return 0;
}

int gpibio_Addr (gpibioPtr dev)
{
    return MakeAddr (dev->paddr, dev->saddr);
}

/**************************************************************/

void devPanel_UpdateReadings(void)
{
    int i;
    nodePtr node;
    UpdateReadingsItemPtr item;
    for (i = 0; i < devPanelList.nItems; i++) {
        node = list_GetNode (devPanelList, i);
        item = node->item; item->UpdateReadings(item->panel, item->ptr);
    }
    expG.acqstatus = utilG.acq.status;
}

void devPanel_Add (int panel, void *ptr, UpdateReadingsPtr UpdateReadings)
{
    UpdateReadingsItemPtr item;

    item = malloc (sizeof(UpdateReadingsItem));
    item->panel = panel;
    item->ptr = ptr;
    item->UpdateReadings = UpdateReadings;
    list_AddItem (&devPanelList, item);
}

void devPanel_Remove (int panel)
{
    nodePtr node;
    UpdateReadingsItemPtr item;
    int i;

    if (devPanelList.nItems) { /* 0 if GPIB Error occured */
        i = 0;
        node = devPanelList.first;
        item = node->item;
        while ((item->panel != panel) && (node->next)) {
            node = node->next; item = node->item; i++;
        }
        if (item->panel == panel)
            list_RemoveItem (&devPanelList, i, TRUE);
    }
}

static int InitGenericDevice (gpibioPtr dev)
{
    gpibio_Remote (dev);
    return TRUE;
}

int  LoadDeviceSetupCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int filestatus, i, nDevs;
    char path[260], info[260];
    gpibioPtr dev;

    if (event == EVENT_COMMIT) {
        filestatus = FileSelectPopup ("", "*.cfg", "",
                                      "Load DAAS Instrument Setup",
                                      VAL_LOAD_BUTTON, 0, 1, 1, 0, path);
        if (filestatus == VAL_EXISTING_FILE_SELECTED) {
            fileHandle.analysis = util_OpenFile (path, FILE_READ, FALSE);
            ScanFile (fileHandle.analysis, "%s[xt58]%i%s[dw1]", info, &nDevs);
            if (CompareBytes (info, 0, "#DAASDEVICESETUP", 0, 16, 0) != 0) {
                MessagePopup ("Load DAAS Instrument Setup Message", "Wrong file type found...process aborted");
                util_CloseFile();
                return 0;
            }

            for (i = 0; i < nDevs; i++) devList_LoadItem();
            ReadLine (fileHandle.analysis, info, 255);
            util_CloseFile();
            devList_UpdatePanel();
        }
    }
    return 0;
}

int  SaveDeviceSetupCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int filestatus, i;
    char path[260];
    gpibioPtr dev;

    if (event == EVENT_COMMIT) {
        filestatus = FileSelectPopup ("", "*.cfg", "",
                                      "Save DAAS Instrument Setup",
                                      VAL_SAVE_BUTTON, 0, 1, 1, 1, path);
		if(filestatus)
		{
        	fileHandle.analysis = util_OpenFile (path, FILE_WRITE, FALSE);
        	FmtFile (fileHandle.analysis, "%s<#DAASDEVICESETUP: %i\n", devList.nItems);
        	for (i = 0; i < devList.nItems; i++) {
				dev = devList_GetItem (i);
				devList_SaveItem (dev);
			}
			FmtFile (fileHandle.analysis, "#ENDSETUP\n");
			util_CloseFile();
		}
    }
    return 0;
}

int  AddDeviceControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int i, index;
    unsigned short addr;
    char label[50], item[30];
    devTypePtr devType;
    gpibioPtr dev;

    switch (control) {
        case ADDDEV_LIST:
            if (event == EVENT_VAL_CHANGED) {
                GetCtrlIndex (panel, control, &i);
                GetLabelFromIndex (panel, control, i, label);
                SetCtrlVal (panel, ADDDEV_LABEL, label);
            }
            break;
        case ADDDEV_SCAN:
            if (event == EVENT_COMMIT) {
                SetMouseCursor (VAL_HOUR_GLASS_CURSOR);
                gpibio_ScanforConnectedDevs();
                ClearListCtrl (panel, ADDDEV_ADDRS);
                for (i = 1; i <= nDevsConnected; i++) {
                    Fmt (item, "%i, %i", GetPAD (devsConnected[i]), GetSAD (devsConnected[i]));
                    InsertListItem (panel, ADDDEV_ADDRS, -1, item, devsConnected[i]);
                }

                for (i = 0; i < devList.nItems; i++) {
                    dev = devList_GetItem (i);
                    GetIndexFromValue (panel, ADDDEV_ADDRS, &index, gpibio_Addr(dev));
                    DeleteListItem (panel, ADDDEV_ADDRS, index, 1);
                }
                SetMouseCursor (VAL_DEFAULT_CURSOR);
            }
            break;
        case ADDDEV_CONNECT:
            if (event == EVENT_COMMIT) {
				GetNumListItems (panel, ADDDEV_ADDRS, &i);
				if(i)
				{
					GetCtrlVal (panel, ADDDEV_ADDRS, &addr);
                	GetCtrlVal (panel, ADDDEV_LABEL, label);
                	GetCtrlVal (panel, ADDDEV_LIST, &i);
                	devType = devTypeList_GetItem (i);
                	dev = gpibio_CreateDevice (i, devType, label, GetPAD (addr), GetSAD(addr), FALSE);
                	if (dev) {
                    	if (devType->CreateDevice) devType->CreateDevice(dev);
                    	GetCtrlIndex (panel, ADDDEV_ADDRS, &i);
                    	DeleteListItem (panel, ADDDEV_ADDRS, i, 1);
                	}
                	devList_UpdatePanel();
					HidePanel(panel);
				}
            }
            break;
    }
    return 0;
}

int  AddDeviceCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int p, i, index;
    devTypePtr devType;
    gpibioPtr dev;
    char item[50];

    if (event == EVENT_COMMIT) {
        p = LoadPanel (0, "gpibiou.uir", ADDDEV);
        
        SetPanelPos (p, VAL_AUTO_CENTER, VAL_AUTO_CENTER);

        util_InitClose (p, ADDDEV_CLOSE, FALSE);

        ClearListCtrl (p, ADDDEV_LIST);
        for (i = 0; i < devTypeList.nItems; i++) {
            devType = devTypeList_GetItem (i);
            InsertListItem (p, ADDDEV_LIST, 0, devType->label, i);
            if (i == 0) InsertListItem (p, ADDDEV_LIST, 0, "\033m-", i);
        }
        SetCtrlIndex (p, ADDDEV_LIST, 0);
        SetCtrlVal (p, ADDDEV_LABEL, devType->label);

        ClearListCtrl (p, ADDDEV_ADDRS);
        for (i = 1; i <= nDevsConnected; i++) {
            Fmt (item, "%i, %i", GetPAD (devsConnected[i]), GetSAD (devsConnected[i]));
            InsertListItem (p, ADDDEV_ADDRS, -1, item, devsConnected[i]);
        }

        for (i = 0; i < devList.nItems; i++) {
            dev = devList_GetItem (i);
            GetIndexFromValue (p, ADDDEV_ADDRS, &index, gpibio_Addr(dev));
            DeleteListItem (p, ADDDEV_ADDRS, index, 1);
        }

        SetInputMode (p, ADDDEV_SCAN, /*!devPanelList.nItems && */!util_TakingData());
        InstallPopup (p);
    }
    return 0;
}

void devTypeList_AddItem (devTypePtr devType)
{
    list_AddItem (&devTypeList, devType);
}

static devTypePtr devTypeList_GetItem (int i)
{
    nodePtr node;
    node = list_GetNode (devTypeList, i);
    return node->item;
}

int  DevListCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    gpibioPtr dev;
	devTypePtr devType;
	char name[60] = "";
	int i;

    if (event == EVENT_KEYPRESS && eventData1 == VAL_ENTER_VKEY) {
        dev = devList_GetSelection(panel);
        dev_UpdatePanel (dev);
        InstallPopup (dev->panel);
    }
	if (event == EVENT_LEFT_DOUBLE_CLICK)
	{
		GetCtrlIndex(panel, control, &i);
		dev = devList_GetItem (i);
		devType = dev->devType;
		devType->OperateDevice (acquire_GetMenuBar(), dev->menuitem_id, dev, acqG.p.setup);
	}
	//*
	if (event == EVENT_KEYPRESS && (eventData1 == VAL_BACKSPACE_VKEY || eventData1 == VAL_FWD_DELETE_VKEY))
	{
        if(((utilG.acq.status == ACQ_BUSY || utilG.acq.status == ACQ_PAUSED)  && ConfirmPopup("Remove device:", "This will stop acquisition. Are you sure?")) || (utilG.acq.status != ACQ_BUSY && utilG.acq.status != ACQ_PAUSED))
		{
			if(utilG.acq.status == ACQ_BUSY || utilG.acq.status == ACQ_PAUSED)utilG.acq.status = ACQ_TERMINATE;
			GetCtrlIndex(panel, control, &i);
			dev = devList_GetItem (i);
			if(dev->iPanel){devPanel_Remove(dev->iPanel);DiscardPanel(dev->iPanel);}
			DiscardMenuItem (acquire_GetMenuBar(), dev->menuitem_id);
			devType = devTypeList_GetItem(dev->id);
			if(devType->RemoveDevice) devType->RemoveDevice(dev->device);
			list_RemoveItem(&devList, i, TRUE);
			updateGraphSource();
			devList_UpdatePanel();		
		}
	}//*/
    return 0;
}

int  SetupPanelCallback(int panel, int event, void *callbackData, int eventData1, int eventData2)
{
    if (event == EVENT_GOT_FOCUS) devList_UpdatePanel ();
    if (((event == EVENT_KEYPRESS) && (eventData1 == VAL_ESC_VKEY)) || (event == EVENT_RIGHT_DOUBLE_CLICK))
		HidePanel (devListp);
	return 0;
}

static void SetupCallback (int menubar, int menuItem, void *callbackData, int panel)
{
    devListp = devListp? devListp:LoadPanel (utilG.p, "gpibiou.uir", SETUP);
    SetPanelPos (devListp, VAL_AUTO_CENTER, VAL_AUTO_CENTER);
    
    devList_UpdatePanel ();
    DisplayPanel (devListp);
}

static void devList_LoadItem (void)
{
    devTypePtr devType;
    gpibioPtr dev;
    int id, logio;
    unsigned short paddr, saddr;
    char label[50];

    ScanFile (fileHandle.analysis, "%s>Device ID: %i%s[dw1]", &id);
    if(id < devTypeList.nItems)
	{
		devType = devTypeList_GetItem (id);
		
    	ScanFile (fileHandle.analysis, "%s>Address  : %i[b2], %i[b2]%s[dw1]", &paddr, &saddr);
    	ScanFile (fileHandle.analysis, "%s>Label    : %s[xt59]%s[dw1]", label);
    	ScanFile (fileHandle.analysis, "%s>Log I/O  : %i%s[dw1]", &logio);

    	if (dev_OnBus (paddr, saddr) && !dev_DuplicateAddr (paddr, saddr)) {
	        dev = gpibio_CreateDevice(id, devType, label, paddr, saddr, logio);
    	    if (dev && devType->CreateDevice) devType->CreateDevice(dev);
    	} else {
        	dev = NULL;
        	MessagePopup ("Loading DAAS Instrument Setup Message",
                      "Instrument not connected to GPIB or duplicate address found");
    	}
    	if (devType->LoadDevice && dev) devType->LoadDevice (dev);
	}
	else
		util_MessagePopup ("Error loading device", "Configuration file not recognized.");
}

static void devList_SaveItem (gpibioPtr dev)
{
    devTypePtr devType;
    FmtFile (fileHandle.analysis, "Device ID: %i\n", dev->id);
    FmtFile (fileHandle.analysis, "Address  : %i, %i\n", dev->paddr, dev->saddr);
    FmtFile (fileHandle.analysis, "Label    : %s;\n", dev->label);
    FmtFile (fileHandle.analysis, "Log I/O  : %i\n", dev->logio);
    if (dev->device) {
        devType = devTypeList_GetItem(dev->id);
        devType->SaveDevice (dev);
    }
}

static void devList_UpdatePanel (void)
{
    gpibioPtr dev;
    int i;
    ClearListCtrl (devListp, SETUP_LIST);
    for (i = 0; i < devList.nItems; i++) {
        dev = devList_GetItem (i);
        InsertListItem (devListp, SETUP_LIST, -1, devList_Item(dev), i);
    }
    SetInputMode (devListp, SETUP_LIST, devList.nItems);
    SetInputMode (devListp, SETUP_SAVE, devList.nItems);
    SetInputMode (devListp, SETUP_ADD, !util_TakingData());
    SetInputMode (devListp, SETUP_LOAD, !util_TakingData());
}

static gpibioPtr devList_GetSelection(int panel)
{
    int i;
    GetCtrlIndex (panel, SETUP_LIST, &i);
    return devList_GetItem (i);
}

static gpibioPtr devList_GetItem (int i)
{
    nodePtr node;
    node = list_GetNode (devList, i);
    return node->item;
}

static char *devList_Item (gpibioPtr dev)
{
    char *item, logio[5];

    item = "                                                                  ";
    if (dev->logio) Fmt (logio, "Yes"); else Fmt (logio, "No");
    if (StringLength (dev->label) > 30) Fmt(item, "%s<%s[w23]...", dev->label);
        else Fmt (item, dev->label);
    Fmt (item, "%s[a]<\033p%ic%s", 188, dev_Status(dev));
    Fmt (item, "%s[a]<\033p%ir%i", 261, dev->paddr);
    Fmt (item, "%s[a]<\033p%ir%i", 307, dev->saddr);
    Fmt (item, "%s[a]<\033p%ic%s", 376, logio);
    return item;
}



int  ifcControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int i;
    char label1[260], label2[260];
    unsigned short paddr, saddr;
    gpibioPtr dev;

    if (event == EVENT_COMMIT) {
        switch (control) {
            case IFC_COMMAND:
                GetCtrlVal (panel, IFC_PADDR, &ifc.paddr);
                GetCtrlVal (panel, IFC_SADDR, &ifc.saddr);
                if (dev_OnBus (ifc.paddr, ifc.saddr)){
                    GetCtrlVal (panel, IFC_COMMAND, label1);
                    i = CompareStrings (label1, 0, "rcv", 0, 0);
                    if (i == 0) gpibio_In (&ifc, label2);
                    else gpibio_Out (&ifc, label1);
                }
                else
                    MessagePopup ("IFC Warning Message",
                                "Address not connected to GPIB.\n"
                                "Command ignored.");
                break;
            case IFC_ENABLESADDR:
                GetCtrlVal (panel, control, &i);
                SetInputMode (panel, IFC_SADDR, i);
                GetCtrlVal (panel, IFC_SADDR, &ifc.saddr);
                ifc.saddr *= i;
                break;
            case IFC_LOGIO:
                GetCtrlVal (panel, control, &ifc.logio);
                break;
            case IFC_DISABLE:
                GetCtrlVal (panel, control, &i);
                if (i) ifc.status = DEV_LOCAL; else ifc.status = DEV_REMOTE;
                SetMenuBarAttribute (acquire_GetMenuBar(), ACQMENUS_GPIB_SETUP_SETUP,
                                     ATTR_DIMMED, (ifc.status != DEV_REMOTE));
                for (i = 0; i < devList.nItems; i++) {
                    dev = devList_GetItem (i);
                    if (dev->menuitem_id)
                        SetMenuBarAttribute (acquire_GetMenuBar(), dev->menuitem_id,
                                             ATTR_DIMMED, (ifc.status != DEV_REMOTE));
                }
                ifc_UpdatePanel();
                break;
            case IFC_TIMEOUT:
                GetCtrlVal (panel, control, &i);
                GetLabelFromIndex (panel, control, i, label1);
                ibtmo (0, i);
                dev_CheckforGPIBErrors (&ifc);
                Fmt (label2, "TMO: %s", label1);
                ifc_StoreCommand (label2);
                break;
            case IFC_CLOSE: RemovePopup (0); break;
        }
    }
    return 0;
}

static void IFCCallback (int menubar,  int menuItem, void *callbackData, int panel)
{
    ifc_UpdatePanel();
    InstallPopup (ifc.panel);
}

static void ifc_Init (void)
{
    ifc.panel = LoadPanel (0, "gpibiou.uir", IFC);
    SetPanelPos (ifc.panel, VAL_AUTO_CENTER, VAL_AUTO_CENTER);
    

    util_InitClose (ifc.panel, IFC_CLOSE, FALSE);

    ifc.nIO = 0;
    Fmt (ifc.label, "IFC");
    ifc.paddr = 1;
    ifc.saddr = 0;
    ifc.logio = FALSE;

    ifc_InitInterface();

    InstallMenuCallback (acquire_GetMenuBar(), ACQMENUS_GPIB_SETUP_STATUS, IFCCallback, 0);
}

static void ifc_InitInterface (void)
{
    int i1, addr;
    char msg[80], dev[20];

    ifc.status = DEV_REMOTE;
    ifc.logio = TRUE;

    ibsic (0);
    if (ibsta & ERR) ifc.status = DEV_OFFLINE;
    dev_CheckforGPIBErrors (&ifc);
    Fmt (msg, "IBSIC: 0");
    ifc_StoreCommand (msg);

    if (ifc.status == DEV_REMOTE)
    {
        ibonl(0,1);
        dev_CheckforGPIBErrors (&ifc);
        Fmt (msg, "IBONL: 1");
        ifc_StoreCommand (msg);

        for (i1 = 1;i1 <= 16; i1++)
        {
            Fmt (dev, "DEV%i", i1);

            addr = ibfind(dev);
            dev_CheckforGPIBErrors (&ifc);
            Fmt (msg, "IBFIND: (%s) - addr: %i", dev, addr);
            ifc_StoreCommand (msg);

            ibonl (addr, 1);
            dev_CheckforGPIBErrors (&ifc);
            Fmt (msg, "IBONL: (%s) 1", dev);
            ifc_StoreCommand (msg);

            ibonl (addr, 0);
            dev_CheckforGPIBErrors (&ifc);
            Fmt (msg, "IBONL: (%s) 0", dev);
            ifc_StoreCommand (msg);
        }

        ibonl(0,1);
        dev_CheckforGPIBErrors (&ifc);
        Fmt (msg, "IBONL: 1");
        ifc_StoreCommand (msg);

        SendIFC(0);
        dev_CheckforGPIBErrors (&ifc);
        Fmt (msg, "SENDIFC (0)");
        ifc_StoreCommand (msg);
        ifc.logio = FALSE;
        utilG.acq.status = ACQ_DONE;
    }
}

static void ifc_UpdatePanel (void)
{
    SetCtrlVal (ifc.panel, IFC_STATUS, dev_Status (&ifc));
    SetCtrlVal (ifc.panel, IFC_LOGIO, ifc.logio);

    SetInputMode (ifc.panel, IFC_TIMEOUT, (ifc.status == DEV_REMOTE));

    SetInputMode (ifc.panel, IFC_DISABLE,
        ((ifc.status != DEV_OFFLINE) && !util_TakingData()));
    SetInputMode (ifc.panel, IFC_PADDR,
        ((ifc.status == DEV_REMOTE) && !util_TakingData()));
    SetInputMode (ifc.panel, IFC_SADDR,
        ((ifc.status == DEV_REMOTE) && !util_TakingData() && ifc.saddr));
    SetInputMode (ifc.panel, IFC_ENABLESADDR,
        ((ifc.status == DEV_REMOTE) && !util_TakingData()));
    SetInputMode (ifc.panel, IFC_COMMAND,
        ((ifc.status == DEV_REMOTE) && !util_TakingData()));
}

static void ifc_StoreResponse (char *rsp)
{
    char msg[260], ifc_msg[260];
    int nLines;

    if (ifc.logio) {
        switch (ifc.status)
        {
            case DEV_OFFLINE: Fmt (ifc_msg, "[IFC offline]"); break;
            case DEV_LOCAL:   Fmt (ifc_msg, "[IFC disabled]"); break;
            case DEV_REMOTE:  Fmt (ifc_msg, ""); break;
        }

        Fmt (msg, "%s<%i: %s %s", ifc.nIO, rsp, ifc_msg);
        InsertTextBoxLine (ifc.panel, IFC_RESPONSES, 0, msg);
        GetNumTextBoxLines (ifc.panel, IFC_RESPONSES, &nLines);
        if (nLines >= MAX_INSTRUCTIONS_SAVED) DeleteTextBoxLine (ifc.panel, IFC_RESPONSES, nLines-2);
    }
}

static void ifc_StoreCommand (char *cmd)
{
    char msg[260], ifc_msg[260];
    int nLines;

    if (ifc.logio) {
        switch (ifc.status) {
            case DEV_OFFLINE:   Fmt (ifc_msg, "[IFC offline]"); break;
            case DEV_LOCAL:     Fmt (ifc_msg, "[IFC disabled]"); break;
            case DEV_REMOTE:    Fmt (ifc_msg, ""); break;
        }

        ifc.nIO++;
        Fmt (msg, "%s<%i: %s: %s ", ifc.nIO, cmd, ifc_msg);
        InsertTextBoxLine (ifc.panel, IFC_COMMANDS, 0, msg);
        GetNumTextBoxLines (ifc.panel, IFC_COMMANDS, &nLines);
        if (nLines >= MAX_INSTRUCTIONS_SAVED) DeleteTextBoxLine (ifc.panel, IFC_COMMANDS, nLines-2);
    }
}

int  DevControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int i;
    char label1[260];
    gpibioPtr dev;

    if (event == EVENT_COMMIT) {
        dev = callbackData;
        switch (control) {
            case DEV_CLEAR: gpibio_Clear (dev); break;
            case DEV_DISABLE:
                GetCtrlVal (panel, control, &i);
                if (i) gpibio_Local (FALSE, dev); else gpibio_Remote (dev);
                SetMenuBarAttribute (acquire_GetMenuBar(), dev->menuitem_id, ATTR_DIMMED, i);
                dev_UpdatePanel (dev);
                break;
            case DEV_LOGIO:
                GetCtrlVal (panel, control, &dev->logio);
                break;
            case DEV_COMMAND:
                GetCtrlVal (panel, control, label1);
                i = CompareStrings (label1, 0, "rcv", 0, 0);
                if (i == 0) gpibio_In (dev, label1); else gpibio_Out (dev, label1);
                break;
            case DEV_LABEL:
                GetCtrlVal (panel, control, dev->label);
                Fmt (label1, "GPIB I/O: %s", dev->label);
                SetPanelAttribute (dev->panel, ATTR_TITLE, label1);
                if (dev->status != DEV_OFFLINE)
                    SetMenuBarAttribute (acquire_GetMenuBar(), dev->menuitem_id,
                                         ATTR_ITEM_NAME, dev->label);
                break;
            case DEV_CLOSE: RemovePopup (0); break;
        }
    }
    return 0;
}

void OperateDevCallback (int menubar, int menuItem, void *callbackData, int panel)
{
    gpibioPtr dev;

    dev = callbackData;
    dev_UpdatePanel (dev);
    InstallPopup (dev->panel);
}

void dev_StoreResponse (gpibioPtr dev, char *rsp)
{
    char msg[260], ifc_msg[260], p_msg[260];
    int nLines;

    if (dev->logio) {
        switch (ifc.status) {
            case DEV_OFFLINE: Fmt (ifc_msg, "[IFC offline]"); break;
            case DEV_LOCAL:   Fmt (ifc_msg, "[IFC disabled]"); break;
            case DEV_REMOTE:  Fmt (ifc_msg, ""); break;
        }

        switch (dev->status) {
            case DEV_OFFLINE: Fmt (p_msg, "[OFFLINE]"); break;
            case DEV_LOCAL:   Fmt (p_msg, "[LOCAL]"); break;
            case DEV_REMOTE:  Fmt (p_msg, ""); break;
        }

        Fmt (msg, "%s<%i: %s: %s %s %s", dev->nIO, dev->label, rsp, p_msg, ifc_msg);
        InsertTextBoxLine (dev->panel, DEV_RESPONSES, 0, msg);
        GetNumTextBoxLines (dev->panel, DEV_RESPONSES, &nLines);
        if (nLines >= MAX_INSTRUCTIONS_SAVED) DeleteTextBoxLine (dev->panel, DEV_RESPONSES, nLines-2);
    }
}

void dev_StoreCommand (gpibioPtr dev, char *cmd)
{
    char msg[260], ifc_msg[260], p_msg[260];
    int nLines;

    if (dev->logio) {
        switch (ifc.status) {
            case DEV_OFFLINE: Fmt (ifc_msg, "[IFC offline]"); break;
            case DEV_LOCAL:   Fmt (ifc_msg, "[IFC disabled]"); break;
            case DEV_REMOTE:  Fmt (ifc_msg, ""); break;
        }

        switch (dev->status) {
            case DEV_OFFLINE: Fmt (p_msg, "[OFFLINE]"); break;
            case DEV_LOCAL:   Fmt (p_msg, "[LOCAL]"); break;
            case DEV_REMOTE:  Fmt (p_msg, ""); break;
        }

        dev->nIO++;
        Fmt (msg, "%s<%i: %s: %s %s %s", dev->nIO, dev->label, cmd, p_msg, ifc_msg);
        InsertTextBoxLine (dev->panel, DEV_COMMANDS, 0, msg);
        GetNumTextBoxLines (dev->panel, DEV_COMMANDS, &nLines);
        if (nLines >= MAX_INSTRUCTIONS_SAVED) DeleteTextBoxLine (dev->panel, DEV_COMMANDS, nLines-2);
    }
}

static void dev_UpdatePanel (gpibioPtr dev)
{
    char msg[260];

    Fmt (msg, "GPIB I/O: %s", dev->label);
    SetPanelAttribute (dev->panel, ATTR_TITLE, msg);

    SetCtrlVal (dev->panel, DEV_LABEL, dev->label);

    SetInputMode (dev->panel, DEV_COMMAND,
        ((dev->status == DEV_REMOTE) && !util_TakingData()));
    SetInputMode (dev->panel, DEV_DISABLE, !util_TakingData());
    SetInputMode (dev->panel, DEV_CLEAR,
        ((dev->status == DEV_REMOTE) && !util_TakingData()));

    SetCtrlVal (dev->panel, DEV_STATUS, dev_Status(dev));
    SetCtrlVal (dev->panel, DEV_PADDR, dev->paddr);
    SetInputMode (dev->panel, DEV_SADDR, dev->saddr);
    SetCtrlVal (dev->panel, DEV_SADDR, dev->saddr);
    SetCtrlVal (dev->panel, DEV_LOGIO, dev->logio);
}

static void dev_CheckforGPIBErrors (gpibioPtr dev)
{
    char errormsg[80];
    int p, i;

    if (ibsta & ERR) {
        if (dev->panel != ifc.panel) {
            p = GetActivePanel();
            if (p == dev->panel) RemovePopup (0);
        }

        ifc.logio = TRUE;
        ifc_StoreCommand ("GPIB ERROR!");

        Fmt (errormsg, "IBSTA = H%x", ibsta);
        if (ibsta & ERR )  Fmt (errormsg, "%s[a]< ERR");
        if (ibsta & TIMO)  Fmt (errormsg, "%s[a]< TIMO");
        if (ibsta & END )  Fmt (errormsg, "%s[a]< END");
        if (ibsta & SRQI)  Fmt (errormsg, "%s[a]< SRQI");
        if (ibsta & RQS )  Fmt (errormsg, "%s[a]< RQS");
        if (ibsta & SPOLL) Fmt (errormsg, "%s[a]< SPOLL");
        if (ibsta & EVENT) Fmt (errormsg, "%s[a]< EVENT");
        if (ibsta & CMPL)  Fmt (errormsg, "%s[a]< CMPL");
        if (ibsta & LOK )  Fmt (errormsg, "%s[a]< LOK");
        if (ibsta & REM )  Fmt (errormsg, "%s[a]< REM");
        if (ibsta & CIC )  Fmt (errormsg, "%s[a]< CIC");
        if (ibsta & ATN )  Fmt (errormsg, "%s[a]< ATN");
        if (ibsta & TACS)  Fmt (errormsg, "%s[a]< TACS");
        if (ibsta & LACS)  Fmt (errormsg, "%s[a]< LACS");
        if (ibsta & DTAS)  Fmt (errormsg, "%s[a]< DTAS");
        if (ibsta & DCAS)  Fmt (errormsg, "%s[a]< DCAS");
        ifc_StoreCommand (errormsg);

        Fmt (errormsg, "IBERR = %d", iberr);
        if (iberr == EDVR) Fmt (errormsg, "%s[a]< EDVR DOS Error");
        if (iberr == ECIC) Fmt (errormsg, "%s[a]< ECIC Not CIC");
        if (iberr == ENOL) Fmt (errormsg, "%s[a]< ENOL No Listener");
        if (iberr == EADR) Fmt (errormsg, "%s[a]< EADR Address error");
        if (iberr == EARG) Fmt (errormsg, "%s[a]< EARG Invalid argument");
        if (iberr == ESAC) Fmt (errormsg, "%s[a]< ESAC Not Sys Ctrlr");
        if (iberr == EABO) Fmt (errormsg, "%s[a]< EABO Op. aborted");
        if (iberr == ENEB) Fmt (errormsg, "%s[a]< ENEB No GPIB board");
        if (iberr == EOIP) Fmt (errormsg, "%s[a]< EOIP Async I/O in prg");
        if (iberr == ECAP) Fmt (errormsg, "%s[a]< ECAP No capability");
        if (iberr == EFSO) Fmt (errormsg, "%s[a]< EFSO File sys. error");
        if (iberr == EBUS) Fmt (errormsg, "%s[a]< EBUS Command error");
        if (iberr == ESTB) Fmt (errormsg, "%s[a]< ESTB Status byte lost");
        if (iberr == ESRQ) Fmt (errormsg, "%s[a]< ESRQ SRQ stuck on");
        if (iberr == ETAB) Fmt (errormsg, "%s[a]< ETAB Table Overflow");
        ifc_StoreCommand (errormsg);

        Fmt (errormsg, "IBCNT = %i[b4]", ibcntl);
        ifc_StoreCommand (errormsg);

        /*
		if (!ConfirmPopup ("GPIB Error Message",
                           "Fatal GPIB error...continuation is not advised.\n"
                           "to continue [yes]     to end program [no]")) 
		{
            if (ifc.status == DEV_REMOTE) 
			{
                SendIFC(0);
                ibonl (0, 0);
            }
            utilG.DiscardPanels();
            exit(0);
        }
		

        ifc_UpdatePanel();
        p = GetActivePanel();
        if (p != ifc.panel) InstallPopup (ifc.panel);
		//*/
		DiscardMenuItem (acquire_GetMenuBar(), ACQMENUS_GPIB_SETUP);
		DiscardMenuItem (acquire_GetMenuBar(), ACQMENUS_GPIB_SEPARATOR);
        util_MessagePopup("GPIB error", "gpib disabled");
		list_RemoveAllItems (&devPanelList, TRUE);
        if (util_TakingData()) utilG.acq.status = ACQ_TERMINATE;
    }
}

static int dev_OnBus (unsigned short paddr, unsigned short saddr)
{
     short onbus = TRUE;
    char msg[260];

    if (ifc.status == DEV_OFFLINE) onbus = FALSE;
    else {
        ibln (0, paddr, saddr, &onbus);
        dev_CheckforGPIBErrors (&ifc);
        if (ibsta & ERR) onbus = FALSE;
    }

    if (ifc.logio) {
        Fmt (msg, "IBLN: P(%d)S(%d)=%d [%d]", paddr, saddr,
             MakeAddr (paddr, saddr), onbus);
        ifc_StoreCommand (msg);
    }
    return onbus;
}

static int dev_DuplicateAddr (unsigned short paddr, unsigned short saddr)
{
    int i, dup = FALSE;
    gpibioPtr dev;
    for (i = 0; i < devList.nItems; i++) {
        dev = devList_GetItem (i);
        if (gpibio_Addr (dev) == MakeAddr (paddr, saddr)) {
            dup = TRUE;
            break;
        }
    }
    return dup;
}

static char *dev_Status (gpibioPtr dev)
{
    char *status;
    status = "                   ";
    switch (dev->status)
    {
        case DEV_LOCAL:     Fmt (status, "Local"); break;
        case DEV_REMOTE:    Fmt (status, "Remote"); break;
        case DEV_OFFLINE:   Fmt (status, "Not on GPIB"); break;
    }
    return status;
}

