/* 
Version of 10/05/2007 

fixed double conversion  bug

DAAS4a
- added note list and note variable to acqchan data structure
*/

#include <analysis.h>
#include <utility.h>
#include <formatio.h>
#include <ansi_c.h>
#include <userint.h>

#include "util.h"
#include "utilu.h"
#include "list.h"
#include "listu.h"
#include "channel.h"
#include "acqchan.h"
#include "acqchanu.h"

#define FALSE 0
#define TRUE 1

#define RUO_RESISTANCE 0
#define RUO_TEMPERATURE 1

static struct acqchanLStruct
{
    int p;
    struct {int width, height, left, top;} mControl, mPanel, sControl, sPanel;
    acqchanPtr index, time;
}   acqchanL;

struct acqchanGStruct acqchanG;

static void     RuO_Low_Init(void);
static void     RuO_High_Init(void);
static double   RuO_Resistance2Temperature (double resistance, int tableitems, double table[274][2]);

void        acqchan_Init(void);
void        acqchan_Exit(void);
acqchanPtr  acqchan_Create (char *label, void *dev, GetReadingPtr GetReading);
void        acqchan_Remove (acqchanPtr chan);
double      acqchan_Measurement (double reading, double coeff, int conversion);
double		**acqchan_MeasurementArray (double *readings, double coeff, int conversion, int pts);
void        acqchan_DoMeasurement (acqchanPtr chan);
void        acqchan_AddGraph (acqchanPtr chan, void *item);
void        acqchan_RemoveGraph (acqchanPtr chan, void *item);
void        acqchan_UpdateReadingPanel(acqchanPtr acqchan);

void        acqchan_Save (acqchanPtr acqchan);
void        acqchan_Load (void *dev, acqchanPtr acqchan);

void        acqchan_GetIndex (acqchanPtr acqchan);
void        acqchan_GetTime (acqchanPtr acqchan);

void        acqchanlist_Dimmed (int dimmed);
acqchanPtr  acqchanlist_GetItem (int i);
acqchanPtr  acqchanlist_GetSelection (void);
acqchanPtr  acqchanlist_GetItemByTitle(char *label);
int         acqchanlist_FindItem (acqchanPtr chan);
void        acqchanlist_Copy (int panel, int control);
void        acqchanlist_AddChannel (acqchanPtr chan);
void        acqchanlist_RemoveChannel (acqchanPtr chan);
void        acqchanlist_ReplaceChannel (acqchanPtr chan);

void        acqchanlist_UpdatePanel (void);
void        acqchanlist_Display(void);

int         acqchanlist_AllocMemory (void);
void        acqchanlist_InitDataFile (char *path);
void        acqchanlist_CloseFile (void);
void        acqchanlist_GetandSaveReadings (void);
void        acqchanlist_CopytoChannelList (void);

int  DAASAcqChannelsCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


int  TimeAcqChannelsCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int time;

    if (event == EVENT_COMMIT) {
        GetCtrlVal (panel, control, &time);
        if (time) acqchanlist_AddChannel (acqchanL.time);
        else {
            if (acqchanL.time->graphs.nItems) {
                SetCtrlVal (panel, control, TRUE);
                MessagePopup ("Remove Acquisition Channel Message",
                              "Channel linked to graph");
            }
            else {
                acqchanlist_RemoveChannel(acqchanL.time);
                SetCtrlVal (panel, control, FALSE);
            }
        }
    }
    return 0;
}

int  IndexAcqChannelsCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int index;

    if (event == EVENT_COMMIT) {
        GetCtrlVal (panel, control, &index);
        if (index) acqchanlist_AddChannel (acqchanL.index);
        else {
            if (acqchanL.index->graphs.nItems) {
                SetCtrlVal (panel, control, TRUE);
                MessagePopup ("Remove Acquisition Channel Message",
                              "Channel linked to graph");
            }
            else {
                acqchanlist_RemoveChannel(acqchanL.index);
                SetCtrlVal (panel, control, FALSE);
            }
        }
    }
    return 0;
}

int  RemoveAcqChanCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    acqchanPtr chan;
    if (event == EVENT_COMMIT)
    {
        chan = acqchanlist_GetSelection();
        if (chan->graphs.nItems == 0)
        {
            if (chan == acqchanL.index) SetCtrlVal (acqchanL.p, ACQLIST_INDEX, FALSE);
            if (chan == acqchanL.time) SetCtrlVal (acqchanL.p, ACQLIST_TIME, FALSE);
            acqchanlist_RemoveChannel(chan);
            chan->acquire = FALSE;
        }
        else
            MessagePopup ("Remove Acquisition Channel Message",
                          "Channel linked to graph");
    }
    return 0;
}

int  MoveDownAcqChanCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int i, checked;
    acqchanPtr chan;
    if (event == EVENT_COMMIT)
    {
        GetCtrlIndex (panel, ACQLIST_CHANNELS, &i);
        chan = acqchanlist_GetItem(i);
        if (acqchanG.channels.last->item != chan)
        {
            IsListItemChecked (acqchanL.p, ACQLIST_CHANNELS, i, &checked);
            DeleteListItem (acqchanL.p, ACQLIST_CHANNELS, i, 1);
            list_MoveDown (acqchanG.channels, i);
            InsertListItem (acqchanL.p, ACQLIST_CHANNELS, i+1, chan->channel->label, i+1);
            CheckListItem (acqchanL.p, ACQLIST_CHANNELS, i+1, checked);
            SetCtrlIndex (acqchanL.p, ACQLIST_CHANNELS, i+1);
        }
    }
    return 0;
}

int  MoveUpAcqChanCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int i, checked;
    acqchanPtr chan;

    if (event == EVENT_COMMIT)
    {
        GetCtrlIndex (panel, ACQLIST_CHANNELS, &i);
        chan = acqchanlist_GetItem(i);
        if (acqchanG.channels.first->item != chan)
        {
            IsListItemChecked (acqchanL.p, ACQLIST_CHANNELS, i, &checked);
            DeleteListItem (acqchanL.p, ACQLIST_CHANNELS, i, 1);
            list_MoveUp (acqchanG.channels, i);
            InsertListItem (acqchanL.p, ACQLIST_CHANNELS, i-1, chan->channel->label, i-1);
            CheckListItem (acqchanL.p, ACQLIST_CHANNELS, i-1, checked);
            SetCtrlIndex (acqchanL.p, ACQLIST_CHANNELS, i-1);
        }
    }
    return 0;
}

int  SelectAcqChanCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int i, checked, width;
    acqchanPtr chan;

    switch (event) {
        case EVENT_LEFT_DOUBLE_CLICK:
            GetCtrlIndex (panel, control, &i);
            IsListItemChecked (panel, control, i, &checked);
            chan = acqchanlist_GetItem(i);
            if (chan->p && checked) {
                GetPanelAttribute (chan->p, ATTR_WIDTH, &width);
                if (width > acqchanL.mControl.width) {
                    SetCtrlAttribute (chan->p, ACQDATA_MEAS, ATTR_TOP, 0);
                    SetCtrlAttribute (chan->p, ACQDATA_MEAS, ATTR_LEFT, 0);
                    SetPanelAttribute (chan->p, ATTR_WIDTH, acqchanL.mControl.width);
                    SetPanelAttribute (chan->p, ATTR_HEIGHT, acqchanL.mControl.height);
                } else {
                    if (chan->conversion != -1) {
                        SetPanelAttribute (chan->p, ATTR_WIDTH, acqchanL.mPanel.width);
                        SetPanelAttribute (chan->p, ATTR_HEIGHT, acqchanL.mPanel.height);
                        SetCtrlAttribute (chan->p, ACQDATA_MEAS, ATTR_TOP, acqchanL.mControl.top);
                        SetCtrlAttribute (chan->p, ACQDATA_MEAS, ATTR_LEFT, acqchanL.mControl.left);
                    } else {
                        SetPanelAttribute (chan->p, ATTR_WIDTH, acqchanL.sPanel.width);
                        SetPanelAttribute (chan->p, ATTR_HEIGHT, acqchanL.sPanel.height);
                        SetCtrlAttribute (chan->p, ACQDATA_MEAS, ATTR_TOP, acqchanL.sControl.top);
                        SetCtrlAttribute (chan->p, ACQDATA_MEAS, ATTR_LEFT, acqchanL.sControl.left);
                    }
                }

            }
            break;
        case EVENT_COMMIT:
            GetCtrlIndex (panel, control, &i);
            IsListItemChecked (panel, control, i, &checked);
            chan = acqchanlist_GetItem(i);
            if ((!chan->p) && checked) {
                chan->p = LoadPanel (utilG.p, "acqchanu.uir", ACQDATA);
                
                if (chan->conversion != -1) {
                    GetPanelAttribute (chan->p, ATTR_HEIGHT, &acqchanL.mPanel.height);
                    GetPanelAttribute (chan->p, ATTR_WIDTH, &acqchanL.mPanel.width);
                    GetCtrlAttribute (chan->p, ACQDATA_MEAS, ATTR_TOP, &acqchanL.mControl.top);
                    GetCtrlAttribute (chan->p, ACQDATA_MEAS, ATTR_LEFT, &acqchanL.mControl.left);
                } else {
                    SetCtrlAttribute (chan->p, ACQDATA_CONV, ATTR_VISIBLE, FALSE);
                    GetCtrlAttribute (chan->p, ACQDATA_CONV, ATTR_LEFT, &acqchanL.sControl.left);
                    GetCtrlAttribute (chan->p, ACQDATA_CONV, ATTR_TOP, &acqchanL.sControl.top);
                    SetCtrlAttribute (chan->p, ACQDATA_MEAS, ATTR_LEFT, acqchanL.sControl.left);
                    SetCtrlAttribute (chan->p, ACQDATA_MEAS, ATTR_TOP, acqchanL.sControl.top);
                    GetCtrlAttribute (chan->p, ACQDATA_MEAS, ATTR_WIDTH, &width);

                    acqchanL.sPanel.width = acqchanL.sControl.left+width+10;
                    GetPanelAttribute (chan->p, ATTR_HEIGHT, &acqchanL.sPanel.height);
                }

                GetCtrlAttribute (chan->p, ACQDATA_MEAS, ATTR_HEIGHT, &acqchanL.mControl.height);
                GetCtrlAttribute (chan->p, ACQDATA_MEAS, ATTR_WIDTH, &acqchanL.mControl.width);

                SetPanelPos (chan->p, VAL_AUTO_CENTER, VAL_AUTO_CENTER);
                SetPanelAttribute (chan->p, ATTR_TITLE, chan->channel->label);
                SetPanelAttribute (chan->p, ATTR_CALLBACK_DATA, chan);

                SetCtrlAttribute (chan->p, ACQDATA_COEFF, ATTR_CALLBACK_DATA, chan);
                SetCtrlAttribute (chan->p, ACQDATA_CONV, ATTR_CALLBACK_DATA, chan);
                SetCtrlAttribute (chan->p, ACQDATA_NOTE, ATTR_CALLBACK_DATA, chan);

                SetCtrlVal (chan->p, ACQDATA_COEFF, chan->coeff);
                SetCtrlVal (chan->p, ACQDATA_NOTE, chan->note);
                if (chan->conversion != -1)
                    SetCtrlIndex (chan->p, ACQDATA_CONV, chan->conversion);

                SetCtrlAttribute (chan->p, ACQDATA_MEAS, ATTR_TOP, 0);
                SetCtrlAttribute (chan->p, ACQDATA_MEAS, ATTR_LEFT, 0);
                SetPanelAttribute (chan->p, ATTR_WIDTH, acqchanL.mControl.width);
                SetPanelAttribute (chan->p, ATTR_HEIGHT, acqchanL.mControl.height);

                DisplayPanel (chan->p);
            }
            else if (chan->p) {
                CheckListItem (panel, control, i, 0);
                DiscardPanel (chan->p);
                chan->p = 0;
            }
            break;
    }
    return 0;
}

int  AcqDataCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    acqchanPtr acqchan;
    int i;

    switch (control) {
        case ACQDATA_COEFF:
            if (event == EVENT_COMMIT) {
                acqchan = callbackData;
                GetCtrlVal (panel, control, &acqchan->coeff);
            }
            break;
        case ACQDATA_CONV:
            if (event == EVENT_COMMIT) {
                acqchan = callbackData;
                GetCtrlVal (panel, control, &acqchan->conversion);
            }
            break;
    }
    return 0;
}

int  AcqDataNoteCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    acqchanPtr acqchan;
    int i;

    if (event == EVENT_COMMIT) {
        acqchan = callbackData;
        GetCtrlVal (panel, control, &acqchan->note);
        if (acqchan->note) list_AddItem(&acqchanG.notes, acqchan);
        else {
            i = list_FindItem (acqchanG.notes, acqchan);
            if (i != NOT_IN_LIST) list_RemoveItem (&acqchanG.notes, i, FALSE);
        }
    }
    return 0;
}

int  AcqDataPanelCallback(int panel, int event, void *callbackData, int eventData1, int eventData2)
{
    int width;
    acqchanPtr chan = callbackData;

    if ((event == EVENT_KEYPRESS) && (eventData1 == VAL_F1_VKEY)) {
        GetPanelAttribute (panel, ATTR_WIDTH, &width);
        if (width > acqchanL.mControl.width) {
            SetCtrlAttribute (panel, ACQDATA_MEAS, ATTR_TOP, 0);
            SetCtrlAttribute (panel, ACQDATA_MEAS, ATTR_LEFT, 0);
            SetPanelAttribute (panel, ATTR_WIDTH, acqchanL.mControl.width);
            SetPanelAttribute (panel, ATTR_HEIGHT, acqchanL.mControl.height);
        } else {
            if (chan->conversion != -1) {
                SetPanelAttribute (panel, ATTR_WIDTH, acqchanL.mPanel.width);
                SetPanelAttribute (panel, ATTR_HEIGHT, acqchanL.mPanel.height);
                SetCtrlAttribute (panel, ACQDATA_MEAS, ATTR_TOP, acqchanL.mControl.top);
                SetCtrlAttribute (panel, ACQDATA_MEAS, ATTR_LEFT, acqchanL.mControl.left);
            } else {
                SetPanelAttribute (panel, ATTR_WIDTH, acqchanL.sPanel.width);
                SetPanelAttribute (panel, ATTR_HEIGHT, acqchanL.sPanel.height);
                SetCtrlAttribute (panel, ACQDATA_MEAS, ATTR_TOP, acqchanL.sControl.top);
                SetCtrlAttribute (panel, ACQDATA_MEAS, ATTR_LEFT, acqchanL.sControl.left);
            }
        }
    }
    return 0;
}


void acqchanlist_CopytoChannelList (void)
{
    acqchanPtr acqchan;
    channelPtr chan;
    int i;
    char label[256];
    double *readings;

    for (i = 0; i < acqchanG.channels.nItems; i++) {
        acqchan = acqchanlist_GetItem(i);
        chan = acqchan->channel;
        Fmt (label, chan->label);
        Fmt (chan->label, "%s<• %s", label);
        readings = (double *) calloc (utilG.acq.pt, sizeof(double));
        if(utilG.acq.pt)
			Subset1D (chan->readings, chan->pts, 0, utilG.acq.pt, readings);
        free (chan->readings);
        chan->readings = readings;
        chan->pts = utilG.acq.pt;
        channellist_AddChannel (acqchan->channel);

        acqchan->channel = channel_Create();
        Fmt (acqchan->channel->label, label);
    }
    channellist_UpdatePanel();
}

void acqchanlist_GetandSaveReadings (void)
{
    int i;
    acqchanPtr chan;

    chan = acqchanlist_GetItem(0);
    acqchan_DoMeasurement (chan);

    if (chan == acqchanL.index)
        FmtFile (fileHandle.data, "%s<%f[p0]", chan->channel->readings[utilG.acq.pt]);
    else if (chan == acqchanL.time)
        FmtFile (fileHandle.data, "%s<%f[p3]", chan->channel->readings[utilG.acq.pt]);
    else
        FmtFile (fileHandle.data, "%s<%f[e2p5]", chan->channel->readings[utilG.acq.pt]);
    chan->newreading = FALSE;

    for (i = 1; i < acqchanG.channels.nItems; i++) {
        chan = acqchanlist_GetItem (i);
        acqchan_DoMeasurement (chan);

        if (chan == acqchanL.index)
            FmtFile (fileHandle.data, "%s<,%f[p0]", chan->channel->readings[utilG.acq.pt]);
        else if (chan == acqchanL.time)
            FmtFile (fileHandle.data, "%s<,%f[p3]", chan->channel->readings[utilG.acq.pt]);
        else
            FmtFile (fileHandle.data, "%s<,%f[e2p5]", chan->channel->readings[utilG.acq.pt]);
        chan->newreading = FALSE;
    }
    WriteFile (fileHandle.data, "\n", 1);
}

void acqchanlist_CloseFile (void)
{
    FmtFile (fileHandle.data, "%s\n", "#ENDACQDATA");
    CloseFile (fileHandle.data);
}

void acqchanlist_InitDataFile (char *path)
{
    int i;
    acqchanPtr chan;

    fileHandle.data = OpenFile(path, FILE_WRITE, 0, 1);

    FmtFile (fileHandle.data, "%s\n", "#ACQDATA");

    chan = acqchanlist_GetItem (0);
    FmtFile (fileHandle.data, "%s", chan->channel->label);
    chan->newreading = FALSE;
    for (i = 1; i < acqchanG.channels.nItems; i++) {
        chan = acqchanlist_GetItem (i);
        FmtFile (fileHandle.data, ",%s", chan->channel->label);
        chan->newreading = FALSE;
    }
    WriteFile(fileHandle.data, ";\n", 2);
    acqchanG.time = Timer();
}

int acqchanlist_AllocMemory (void)
{
    int i, noErr = TRUE;
    acqchanPtr chan;

    for (i = 0; i < acqchanG.channels.nItems; i++) {
        chan = acqchanlist_GetItem (i);
        if (!channel_AllocMem (chan->channel, utilG.acq.nPts)) {
            util_OutofMemory ("Acquisition Channel Memory Allocation");
            noErr = FALSE;
            break;
        }
    }
    return noErr;
}

void acqchanlist_Dimmed (int dimmed)
{
    SetInputMode (acqchanL.p, ACQLIST_UP, !dimmed);
    SetInputMode (acqchanL.p, ACQLIST_DOWN, !dimmed);
    SetInputMode (acqchanL.p, ACQLIST_REMOVE, !dimmed);
    SetInputMode (acqchanL.p, ACQLIST_INDEX, !dimmed);
    SetInputMode (acqchanL.p, ACQLIST_TIME, !dimmed);
}

/************************************************************************/

int  DAASAcqChannelsCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    if (event == EVENT_COMMIT) {
        acqchanlist_UpdatePanel();
        DisplayPanel (acqchanL.p);
    }
    return 0;
}

void acqchanlist_UpdatePanel (void)
{
    SetInputMode (acqchanL.p, ACQLIST_UP,
                  ((acqchanG.channels.nItems > 1) && !util_TakingData()));
    SetInputMode (acqchanL.p, ACQLIST_DOWN,
                  ((acqchanG.channels.nItems > 1) && !util_TakingData()));
    SetInputMode (acqchanL.p, ACQLIST_REMOVE,
                  (acqchanG.channels.nItems && !util_TakingData()));
    SetInputMode (acqchanL.p, ACQLIST_CHANNELS, acqchanG.channels.nItems);
}

/************************************************************************/

void acqchanlist_ReplaceChannel (acqchanPtr chan)
{
    int i;
    if (chan->acquire) {
        i = list_FindItem (acqchanG.channels, chan);
        ReplaceListItem (acqchanL.p, ACQLIST_CHANNELS, i,
                         chan->channel->label, i);
    }
}

void acqchanlist_RemoveChannel (acqchanPtr chan)
{
    int i;

    i = list_FindItem (acqchanG.channels, chan);
    if (i != NOT_IN_LIST) {
        list_RemoveItem (&acqchanG.channels, i, FALSE);
        DeleteListItem (acqchanL.p, ACQLIST_CHANNELS, i, 1);
        acqchanlist_UpdatePanel();
    }
}

void acqchanlist_AddChannel (acqchanPtr chan)
{
    if (list_AddItem (&acqchanG.channels, chan)) {
        InsertListItem (acqchanL.p, ACQLIST_CHANNELS, -1, chan->channel->label,
                        acqchanG.channels.nItems-1);
        acqchanlist_UpdatePanel ();
    }
}

void acqchanlist_Copy (int panel, int control)
{
    int i;
    acqchanPtr chan;
    ClearListCtrl (panel, control);
    InsertListItem (panel, control, -1, "NONE", -1);
    for (i = 0; i < acqchanG.channels.nItems; i++) {
        chan = acqchanlist_GetItem (i);
        InsertListItem (panel, control, -1, chan->channel->label, i);
    }
}

int acqchanlist_FindItem (acqchanPtr chan)
{
    return list_FindItem (acqchanG.channels, chan);
}

acqchanPtr acqchanlist_GetSelection (void)
{
    int i;
    GetCtrlIndex (acqchanL.p, ACQLIST_CHANNELS, &i);
    return acqchanlist_GetItem(i);
}

acqchanPtr acqchanNote_GetItem (int i)
{
    nodePtr node;
    node = list_GetNode (acqchanG.notes, i);
    return node->item;
}

acqchanPtr acqchanlist_GetItem (int i)
{
    nodePtr node;
    node = list_GetNode (acqchanG.channels, i);
    return node->item;
}

acqchanPtr acqchanlist_GetItemByTitle(char *label)
{
	int i = 0;
	nodePtr node;
	acqchanPtr acqPtr;
	do{
		node = list_GetNode (acqchanG.channels, i);
		i++;
		if(node)
			acqPtr = node->item;
		else
			acqPtr = NULL;
	}while(node && strcmp(acqPtr->channel->label, label));
	return acqPtr;
}

void acqchan_GetTime (acqchanPtr acqchan)
{
    acqchan->reading = (Timer() - acqchanG.time)/60.0;
/*    (double) rand()/32767;*/
}

void acqchan_GetIndex (acqchanPtr acqchan)
{
    acqchan->reading = utilG.acq.pt;
}

/************************************************************************/

void acqchan_Load (void *dev, acqchanPtr acqchan)
{
    int acq;
    char info[256];

    if (dev) {
        ScanFile (fileHandle.analysis, "%s>%s[xt58]%i, %i, %f[x]",
                  acqchan->channel->label, &acq,
                  &acqchan->conversion, &acqchan->coeff);

        if (acq != acqchan->acquire) {
            if (acq) acqchanlist_AddChannel (acqchan);
            else acqchanlist_RemoveChannel (acqchan);
            acqchan->acquire = acq;
        }
        else if (acqchan->acquire) {
            acqchanlist_ReplaceChannel (acqchan);
            acqchan_UpdateReadingPanel (acqchan);
        }
    } else ReadLine (fileHandle.analysis, info, 255);
}

void acqchan_Save (acqchanPtr acqchan)
{
    FmtFile (fileHandle.analysis, "%s<%s: %i, %i, %f[e2p5]\n",
             acqchan->channel->label,
             acqchan->acquire, acqchan->conversion,
             acqchan->coeff);
}

void acqchan_UpdateReadingPanel(acqchanPtr acqchan)
{
    if (acqchan->p) {
        SetPanelAttribute (acqchan->p, ATTR_TITLE, acqchan->channel->label);
        SetCtrlVal (acqchan->p, ACQDATA_COEFF, acqchan->coeff);
        if (acqchan->conversion != -1)
            SetCtrlVal (acqchan->p, ACQDATA_CONV, acqchan->conversion);
    }
}

void acqchan_RemoveGraph (acqchanPtr chan, void *item)
{
    int i;

    i = list_FindItem (chan->graphs, item);
    if (i != NOT_IN_LIST) list_RemoveItem (&chan->graphs, i, FALSE);
}

void acqchan_AddGraph (acqchanPtr chan, void *item)
{
    list_AddItem (&chan->graphs, item);
}

void acqchan_DoMeasurement (acqchanPtr chan)
{
    double meas;
    if (!chan->newreading)
        chan->GetReading(chan);
    chan->reading *= chan->coeff;
    //meas = acqchan_Measurement(chan->reading, chan->coeff, chan->conversion); WHY IS IT HERE????
    meas = acqchan_Measurement(chan->reading, 1.0, chan->conversion);
    chan->channel->readings[utilG.acq.pt] = meas;
    chan->channel->readings[utilG.acq.pt] = chan->reading;
}


double acqchan_Measurement (double reading, double coeff, int conversion)
{
    double measurement = 0;
    switch (conversion)
    {
        case -1:
        case 0:
            measurement =  reading;
            break;
        case 1:
            measurement = RuO_Resistance2Temperature (reading,
                                            RUO_HIGH_TABLEITEMS, RuO_High_Table);
            break;
        case 2:
            measurement =
                RuO_Resistance2Temperature (reading,
                                            RUO_LOW_TABLEITEMS, RuO_Low_Table);
            break;
		case 3:
		{
			measurement = RuO_Resistance2Temperature (reading, 
											RUO_USR_TABLEITEMS, User_Table);//*/
		}
			break;
    }
    return measurement;
}

double **acqchan_MeasurementArray (double *readings, double coeff, int conversion, int pts)
{
	int i;
	double **measurement = malloc(sizeof(double*));
	measurement[0] = calloc(pts, sizeof(double));
    for(i = 0; i < pts; i++)
	{
		measurement[0][i] = acqchan_Measurement (readings[i], 1.0, conversion);
	}
    return measurement;
}

acqchanPtr acqchan_Create (char *label, void *dev, GetReadingPtr GetReading)
{
    acqchanPtr c;

    c = malloc (sizeof (acqchanType));
    if (!c) {util_OutofMemory("Acquisition Channel Error"); return NULL;}

    c->acquire = FALSE;
    c->conversion = 0;
    c->newreading = FALSE;
    c->note = FALSE;

    c->coeff = 1.0;
    c->reading = 0.0;

    c->channel = channel_Create ();
    if (!c->channel) {
        free (c);
        return NULL;
    }
    Fmt (c->channel->label, label);

    list_Init (&c->graphs);

    c->GetReading = GetReading;
    c->p = FALSE;
	c->menuitem_id = 0;
    c->dev = dev;

    return c;
}

void acqchan_Init (void)
{
    if (utilG.acq.status != ACQ_NONE) {
        util_ChangeInitMessage ("Channel Acquisition Utilities...");

        acqchanL.p = LoadPanel (utilG.p, "acqchanu.uir", ACQLIST);
        SetPanelPos (acqchanL.p, VAL_AUTO_CENTER, VAL_AUTO_CENTER);

        list_Init (&acqchanG.channels);
        list_Init (&acqchanG.notes);

        InstallCtrlCallback (utilG.p, BG_ACQCHANNELS, DAASAcqChannelsCallback, 0);
        acqchanL.index = acqchan_Create ("INDEX", NULL, acqchan_GetIndex);
        acqchanL.time = acqchan_Create ("Time [min]", NULL, acqchan_GetTime);

        acqchanL.mPanel.width = 0;
        acqchanL.mPanel.height = 0;
        acqchanL.mControl.width = 0;
        acqchanL.mControl.height = 0;

        RuO_High_Init();
        RuO_Low_Init();
    } else SetCtrlAttribute (utilG.p, BG_ACQCHANNELS, ATTR_VISIBLE, FALSE);
}

void acqchan_Remove (acqchanPtr chan)
{
    if (chan && chan->p) {
        DiscardPanel (chan->p);
    }
	acqchanlist_RemoveChannel(chan);
    list_RemoveAllItems (&chan->graphs, FALSE);
    if (chan->channel->readings) free (chan->channel->readings);
    free (chan->channel);
    free (chan);
}

void acqchan_Exit (void)
{
    if (utilG.acq.status != ACQ_NONE) {
        list_RemoveAllItems (&acqchanG.channels, FALSE);

        acqchan_Remove (acqchanL.index);
        acqchan_Remove (acqchanL.time);

        DiscardPanel (acqchanL.p);
    }

    channelG_Exit();
}

static double RuO_Resistance2Temperature (double resistance, int tableitems, double table[274][2])
{
    int i;
    double b, m;

    i = 2;
    while (i < tableitems)
    {
        if ((resistance >= table[i-1][RUO_RESISTANCE]) &&
            (resistance <= table[i][RUO_RESISTANCE]))
        {
            m = (table[i][RUO_TEMPERATURE] - table[i-1][RUO_TEMPERATURE])/
                (table[i][RUO_RESISTANCE] - table[i-1][RUO_RESISTANCE]);
            b = table[i-1][RUO_TEMPERATURE] - (m * table[i-1][RUO_RESISTANCE]);
            return (m * resistance) + b;
        }
        i++;
    }
    return resistance;
}

static void RuO_Low_Init (void)
{
    RuO_Low_Table[1][RUO_TEMPERATURE] = 20.0; RuO_Low_Table[1][RUO_RESISTANCE] = 1103.7;
    RuO_Low_Table[2][RUO_TEMPERATURE] = 19.0; RuO_Low_Table[2][RUO_RESISTANCE] = 1107.9;
    RuO_Low_Table[3][RUO_TEMPERATURE] = 18.0; RuO_Low_Table[3][RUO_RESISTANCE] = 1112.6;
    RuO_Low_Table[4][RUO_TEMPERATURE] = 17.0; RuO_Low_Table[4][RUO_RESISTANCE] = 1117.9;
    RuO_Low_Table[5][RUO_TEMPERATURE] = 16.0; RuO_Low_Table[5][RUO_RESISTANCE] = 1123.6;
    RuO_Low_Table[6][RUO_TEMPERATURE] = 15.0; RuO_Low_Table[6][RUO_RESISTANCE] = 1130.2;
    RuO_Low_Table[7][RUO_TEMPERATURE] = 14.0; RuO_Low_Table[7][RUO_RESISTANCE] = 1137.7;
    RuO_Low_Table[8][RUO_TEMPERATURE] = 13.0; RuO_Low_Table[8][RUO_RESISTANCE] = 1146.3;
    RuO_Low_Table[9][RUO_TEMPERATURE] = 12.0; RuO_Low_Table[9][RUO_RESISTANCE] = 1156.5;
    RuO_Low_Table[10][RUO_TEMPERATURE] = 11.0; RuO_Low_Table[10][RUO_RESISTANCE] = 1168.3;
    RuO_Low_Table[11][RUO_TEMPERATURE] = 10.0; RuO_Low_Table[11][RUO_RESISTANCE] = 1182.2;
    RuO_Low_Table[12][RUO_TEMPERATURE] = 9.5; RuO_Low_Table[12][RUO_RESISTANCE] = 1190.4;
    RuO_Low_Table[13][RUO_TEMPERATURE] = 9.0; RuO_Low_Table[13][RUO_RESISTANCE] = 1199.3;
    RuO_Low_Table[14][RUO_TEMPERATURE] = 8.5; RuO_Low_Table[14][RUO_RESISTANCE] = 1209.3;
    RuO_Low_Table[15][RUO_TEMPERATURE] = 8.0; RuO_Low_Table[15][RUO_RESISTANCE] = 1220.4;
    RuO_Low_Table[16][RUO_TEMPERATURE] = 7.5; RuO_Low_Table[16][RUO_RESISTANCE] = 1233.1;
    RuO_Low_Table[17][RUO_TEMPERATURE] = 7.0; RuO_Low_Table[17][RUO_RESISTANCE] = 1247.2;
    RuO_Low_Table[18][RUO_TEMPERATURE] = 6.5; RuO_Low_Table[18][RUO_RESISTANCE] = 1263.7;
    RuO_Low_Table[19][RUO_TEMPERATURE] = 6.0; RuO_Low_Table[19][RUO_RESISTANCE] = 1282.6;
    RuO_Low_Table[20][RUO_TEMPERATURE] = 5.5; RuO_Low_Table[20][RUO_RESISTANCE] = 1305.0;
    RuO_Low_Table[21][RUO_TEMPERATURE] = 5.0; RuO_Low_Table[21][RUO_RESISTANCE] = 1331.4;
    RuO_Low_Table[22][RUO_TEMPERATURE] = 4.8; RuO_Low_Table[22][RUO_RESISTANCE] = 1343.4;

    RuO_Low_Table[23][RUO_TEMPERATURE] = 4.6; RuO_Low_Table[23][RUO_RESISTANCE] = 1356.4;
    RuO_Low_Table[24][RUO_TEMPERATURE] = 4.4; RuO_Low_Table[24][RUO_RESISTANCE] = 1370.4;
    RuO_Low_Table[25][RUO_TEMPERATURE] = 4.2; RuO_Low_Table[25][RUO_RESISTANCE] = 1385.9;
    RuO_Low_Table[26][RUO_TEMPERATURE] = 4.0; RuO_Low_Table[26][RUO_RESISTANCE] = 1402.5;
    RuO_Low_Table[27][RUO_TEMPERATURE] = 3.8; RuO_Low_Table[27][RUO_RESISTANCE] = 1421.1;
    RuO_Low_Table[28][RUO_TEMPERATURE] = 3.6; RuO_Low_Table[28][RUO_RESISTANCE] = 1441.5;
    RuO_Low_Table[29][RUO_TEMPERATURE] = 3.4; RuO_Low_Table[29][RUO_RESISTANCE] = 1463.3;
    RuO_Low_Table[30][RUO_TEMPERATURE] = 3.2; RuO_Low_Table[30][RUO_RESISTANCE] = 1488.0;
    RuO_Low_Table[31][RUO_TEMPERATURE] = 3.0; RuO_Low_Table[31][RUO_RESISTANCE] = 1515.9;
    RuO_Low_Table[32][RUO_TEMPERATURE] = 2.8; RuO_Low_Table[32][RUO_RESISTANCE] = 1547.6;
    RuO_Low_Table[33][RUO_TEMPERATURE] = 2.6; RuO_Low_Table[33][RUO_RESISTANCE] = 1584.5;
    RuO_Low_Table[34][RUO_TEMPERATURE] = 2.4; RuO_Low_Table[34][RUO_RESISTANCE] = 1629.9;
    RuO_Low_Table[35][RUO_TEMPERATURE] = 2.2; RuO_Low_Table[35][RUO_RESISTANCE] = 1681.4;
    RuO_Low_Table[36][RUO_TEMPERATURE] = 2.0; RuO_Low_Table[36][RUO_RESISTANCE] = 1740.7;
    RuO_Low_Table[37][RUO_TEMPERATURE] = 1.9; RuO_Low_Table[37][RUO_RESISTANCE] = 1774.6;
    RuO_Low_Table[38][RUO_TEMPERATURE] = 1.8; RuO_Low_Table[38][RUO_RESISTANCE] = 1812.4;
    RuO_Low_Table[39][RUO_TEMPERATURE] = 1.7; RuO_Low_Table[39][RUO_RESISTANCE] = 1854.4;
    RuO_Low_Table[40][RUO_TEMPERATURE] = 1.6; RuO_Low_Table[40][RUO_RESISTANCE] = 1901.5;
    RuO_Low_Table[41][RUO_TEMPERATURE] = 1.5; RuO_Low_Table[41][RUO_RESISTANCE] = 1953.7;
    RuO_Low_Table[42][RUO_TEMPERATURE] = 1.4; RuO_Low_Table[42][RUO_RESISTANCE] = 2011.5;
    RuO_Low_Table[43][RUO_TEMPERATURE] = 1.3; RuO_Low_Table[43][RUO_RESISTANCE] = 2079.3;

    RuO_Low_Table[44][RUO_TEMPERATURE] = 1.2; RuO_Low_Table[44][RUO_RESISTANCE] = 2160.4;
    RuO_Low_Table[45][RUO_TEMPERATURE] = 1.1; RuO_Low_Table[45][RUO_RESISTANCE] = 2256.6;
    RuO_Low_Table[46][RUO_TEMPERATURE] = 1.0; RuO_Low_Table[46][RUO_RESISTANCE] = 2368.1;
    RuO_Low_Table[47][RUO_TEMPERATURE] = 0.9; RuO_Low_Table[47][RUO_RESISTANCE] = 2508.7;
    RuO_Low_Table[48][RUO_TEMPERATURE] = 0.8; RuO_Low_Table[48][RUO_RESISTANCE] = 2679.9;
    RuO_Low_Table[49][RUO_TEMPERATURE] = 0.7; RuO_Low_Table[49][RUO_RESISTANCE] = 2902.2;
    RuO_Low_Table[50][RUO_TEMPERATURE] = 0.6; RuO_Low_Table[50][RUO_RESISTANCE] = 3193.5;
    RuO_Low_Table[51][RUO_TEMPERATURE] = 0.5; RuO_Low_Table[51][RUO_RESISTANCE] = 3602.5;
    RuO_Low_Table[52][RUO_TEMPERATURE] = 0.45; RuO_Low_Table[52][RUO_RESISTANCE] = 3875.1;
    RuO_Low_Table[53][RUO_TEMPERATURE] = 0.40; RuO_Low_Table[53][RUO_RESISTANCE] = 4222.8;
    RuO_Low_Table[54][RUO_TEMPERATURE] = 0.35; RuO_Low_Table[54][RUO_RESISTANCE] = 4629.0;
    RuO_Low_Table[55][RUO_TEMPERATURE] = 0.30; RuO_Low_Table[55][RUO_RESISTANCE] = 5228.7;
    RuO_Low_Table[56][RUO_TEMPERATURE] = 0.25; RuO_Low_Table[56][RUO_RESISTANCE] = 6081.5;
    RuO_Low_Table[57][RUO_TEMPERATURE] = 0.20; RuO_Low_Table[57][RUO_RESISTANCE] = 7398.5;
    RuO_Low_Table[58][RUO_TEMPERATURE] = 0.15; RuO_Low_Table[58][RUO_RESISTANCE] = 9749.7;
    RuO_Low_Table[59][RUO_TEMPERATURE] = 0.10; RuO_Low_Table[59][RUO_RESISTANCE] = 15315.0;
    RuO_Low_Table[60][RUO_TEMPERATURE] = 0.09; RuO_Low_Table[60][RUO_RESISTANCE] = 17554;
    RuO_Low_Table[61][RUO_TEMPERATURE] = 0.08; RuO_Low_Table[61][RUO_RESISTANCE] = 20523;
    RuO_Low_Table[62][RUO_TEMPERATURE] = 0.07; RuO_Low_Table[62][RUO_RESISTANCE] = 23723;
    RuO_Low_Table[63][RUO_TEMPERATURE] = 0.06; RuO_Low_Table[63][RUO_RESISTANCE] = 29568;
    RuO_Low_Table[64][RUO_TEMPERATURE] = 0.05; RuO_Low_Table[64][RUO_RESISTANCE] = 37886;
}

static void RuO_High_Init (void)
{
    RuO_High_Table[1][RUO_TEMPERATURE] = 273.0; RuO_High_Table[1][RUO_RESISTANCE] = 100366.2;
    RuO_High_Table[2][RUO_TEMPERATURE] = 272.0; RuO_High_Table[2][RUO_RESISTANCE] = 100373.0;
    RuO_High_Table[3][RUO_TEMPERATURE] = 271.0; RuO_High_Table[3][RUO_RESISTANCE] = 100379.9;
    RuO_High_Table[4][RUO_TEMPERATURE] = 270.0; RuO_High_Table[4][RUO_RESISTANCE] = 100387.0;

    RuO_High_Table[5][RUO_TEMPERATURE] = 269.0; RuO_High_Table[5][RUO_RESISTANCE] = 100394.1;
    RuO_High_Table[6][RUO_TEMPERATURE] = 268.0; RuO_High_Table[6][RUO_RESISTANCE] = 100401.5;
    RuO_High_Table[7][RUO_TEMPERATURE] = 267.0; RuO_High_Table[7][RUO_RESISTANCE] = 100409.1;
    RuO_High_Table[8][RUO_TEMPERATURE] = 266.0; RuO_High_Table[8][RUO_RESISTANCE] = 100416.7;
    RuO_High_Table[9][RUO_TEMPERATURE] = 265.0; RuO_High_Table[9][RUO_RESISTANCE] = 100424.5;
    RuO_High_Table[10][RUO_TEMPERATURE] = 264.0; RuO_High_Table[10][RUO_RESISTANCE] = 100432.4;
    RuO_High_Table[11][RUO_TEMPERATURE] = 263.0; RuO_High_Table[11][RUO_RESISTANCE] = 100440.6;
    RuO_High_Table[12][RUO_TEMPERATURE] = 262.0; RuO_High_Table[12][RUO_RESISTANCE] = 100448.8;
    RuO_High_Table[13][RUO_TEMPERATURE] = 261.0; RuO_High_Table[13][RUO_RESISTANCE] = 100457.3;
    RuO_High_Table[14][RUO_TEMPERATURE] = 260.0; RuO_High_Table[14][RUO_RESISTANCE] = 100465.9;

    RuO_High_Table[15][RUO_TEMPERATURE] = 259.0; RuO_High_Table[15][RUO_RESISTANCE] = 100474.7;
    RuO_High_Table[16][RUO_TEMPERATURE] = 258.0; RuO_High_Table[16][RUO_RESISTANCE] = 100483.6;
    RuO_High_Table[17][RUO_TEMPERATURE] = 257.0; RuO_High_Table[17][RUO_RESISTANCE] = 100492.7;
    RuO_High_Table[18][RUO_TEMPERATURE] = 256.0; RuO_High_Table[18][RUO_RESISTANCE] = 100501.9;
    RuO_High_Table[19][RUO_TEMPERATURE] = 255.0; RuO_High_Table[19][RUO_RESISTANCE] = 100511.3;
    RuO_High_Table[20][RUO_TEMPERATURE] = 254.0; RuO_High_Table[20][RUO_RESISTANCE] = 100520.9;
    RuO_High_Table[21][RUO_TEMPERATURE] = 253.0; RuO_High_Table[21][RUO_RESISTANCE] = 100530.7;
    RuO_High_Table[22][RUO_TEMPERATURE] = 252.0; RuO_High_Table[22][RUO_RESISTANCE] = 100540.6;
    RuO_High_Table[23][RUO_TEMPERATURE] = 251.0; RuO_High_Table[23][RUO_RESISTANCE] = 100550.8;
    RuO_High_Table[24][RUO_TEMPERATURE] = 250.0; RuO_High_Table[24][RUO_RESISTANCE] = 100561.1;

    RuO_High_Table[25][RUO_TEMPERATURE] = 249.0; RuO_High_Table[25][RUO_RESISTANCE] = 100571.4;
    RuO_High_Table[26][RUO_TEMPERATURE] = 248.0; RuO_High_Table[26][RUO_RESISTANCE] = 100582.1;
    RuO_High_Table[27][RUO_TEMPERATURE] = 247.0; RuO_High_Table[27][RUO_RESISTANCE] = 100592.8;
    RuO_High_Table[28][RUO_TEMPERATURE] = 246.0; RuO_High_Table[28][RUO_RESISTANCE] = 100603.7;
    RuO_High_Table[29][RUO_TEMPERATURE] = 245.0; RuO_High_Table[29][RUO_RESISTANCE] = 100614.8;
    RuO_High_Table[30][RUO_TEMPERATURE] = 244.0; RuO_High_Table[30][RUO_RESISTANCE] = 100626.1;
    RuO_High_Table[31][RUO_TEMPERATURE] = 243.0; RuO_High_Table[31][RUO_RESISTANCE] = 100637.6;
    RuO_High_Table[32][RUO_TEMPERATURE] = 242.0; RuO_High_Table[32][RUO_RESISTANCE] = 100649.2;
    RuO_High_Table[33][RUO_TEMPERATURE] = 241.0; RuO_High_Table[33][RUO_RESISTANCE] = 100661.1;
    RuO_High_Table[34][RUO_TEMPERATURE] = 240.0; RuO_High_Table[34][RUO_RESISTANCE] = 100673.1;

    RuO_High_Table[35][RUO_TEMPERATURE] = 239.0; RuO_High_Table[35][RUO_RESISTANCE] = 100685.4;
    RuO_High_Table[36][RUO_TEMPERATURE] = 238.0; RuO_High_Table[36][RUO_RESISTANCE] = 100697.8;
    RuO_High_Table[37][RUO_TEMPERATURE] = 237.0; RuO_High_Table[37][RUO_RESISTANCE] = 100710.5;
    RuO_High_Table[38][RUO_TEMPERATURE] = 236.0; RuO_High_Table[38][RUO_RESISTANCE] = 100723.3;
    RuO_High_Table[39][RUO_TEMPERATURE] = 235.0; RuO_High_Table[39][RUO_RESISTANCE] = 100736.3;
    RuO_High_Table[40][RUO_TEMPERATURE] = 234.0; RuO_High_Table[40][RUO_RESISTANCE] = 100749.6;
    RuO_High_Table[41][RUO_TEMPERATURE] = 233.0; RuO_High_Table[41][RUO_RESISTANCE] = 100763.0;
    RuO_High_Table[42][RUO_TEMPERATURE] = 232.0; RuO_High_Table[42][RUO_RESISTANCE] = 100776.7;
    RuO_High_Table[43][RUO_TEMPERATURE] = 231.0; RuO_High_Table[43][RUO_RESISTANCE] = 100790.5;
    RuO_High_Table[44][RUO_TEMPERATURE] = 230.0; RuO_High_Table[44][RUO_RESISTANCE] = 100804.7;

    RuO_High_Table[45][RUO_TEMPERATURE] = 229.0; RuO_High_Table[45][RUO_RESISTANCE] = 100819.0;
    RuO_High_Table[46][RUO_TEMPERATURE] = 228.0; RuO_High_Table[46][RUO_RESISTANCE] = 100833.5;
    RuO_High_Table[47][RUO_TEMPERATURE] = 227.0; RuO_High_Table[47][RUO_RESISTANCE] = 100848.2;
    RuO_High_Table[48][RUO_TEMPERATURE] = 226.0; RuO_High_Table[48][RUO_RESISTANCE] = 100863.2;
    RuO_High_Table[49][RUO_TEMPERATURE] = 225.0; RuO_High_Table[49][RUO_RESISTANCE] = 100878.4;
    RuO_High_Table[50][RUO_TEMPERATURE] = 224.0; RuO_High_Table[50][RUO_RESISTANCE] = 100893.8;
    RuO_High_Table[51][RUO_TEMPERATURE] = 223.0; RuO_High_Table[51][RUO_RESISTANCE] = 100909.4;
    RuO_High_Table[52][RUO_TEMPERATURE] = 222.0; RuO_High_Table[52][RUO_RESISTANCE] = 100925.2;
    RuO_High_Table[53][RUO_TEMPERATURE] = 221.0; RuO_High_Table[53][RUO_RESISTANCE] = 100941.3;
    RuO_High_Table[54][RUO_TEMPERATURE] = 220.0; RuO_High_Table[54][RUO_RESISTANCE] = 100957.5;

    RuO_High_Table[55][RUO_TEMPERATURE] = 219.0; RuO_High_Table[55][RUO_RESISTANCE] = 100974.1;
    RuO_High_Table[56][RUO_TEMPERATURE] = 218.0; RuO_High_Table[56][RUO_RESISTANCE] = 100990.9;
    RuO_High_Table[57][RUO_TEMPERATURE] = 217.0; RuO_High_Table[57][RUO_RESISTANCE] = 101007.9;
    RuO_High_Table[58][RUO_TEMPERATURE] = 216.0; RuO_High_Table[58][RUO_RESISTANCE] = 101025.2;
    RuO_High_Table[59][RUO_TEMPERATURE] = 215.0; RuO_High_Table[59][RUO_RESISTANCE] = 101042.8;
    RuO_High_Table[60][RUO_TEMPERATURE] = 214.0; RuO_High_Table[60][RUO_RESISTANCE] = 101060.5;
    RuO_High_Table[61][RUO_TEMPERATURE] = 213.0; RuO_High_Table[61][RUO_RESISTANCE] = 101078.6;
    RuO_High_Table[62][RUO_TEMPERATURE] = 212.0; RuO_High_Table[62][RUO_RESISTANCE] = 101096.9;
    RuO_High_Table[63][RUO_TEMPERATURE] = 211.0; RuO_High_Table[63][RUO_RESISTANCE] = 101115.5;
    RuO_High_Table[64][RUO_TEMPERATURE] = 210.0; RuO_High_Table[64][RUO_RESISTANCE] = 101134.4;

    RuO_High_Table[65][RUO_TEMPERATURE] = 209.0; RuO_High_Table[65][RUO_RESISTANCE] = 101153.5;
    RuO_High_Table[66][RUO_TEMPERATURE] = 208.0; RuO_High_Table[66][RUO_RESISTANCE] = 101172.9;
    RuO_High_Table[67][RUO_TEMPERATURE] = 207.0; RuO_High_Table[67][RUO_RESISTANCE] = 101192.5;
    RuO_High_Table[68][RUO_TEMPERATURE] = 206.0; RuO_High_Table[68][RUO_RESISTANCE] = 101212.5;
    RuO_High_Table[69][RUO_TEMPERATURE] = 205.0; RuO_High_Table[69][RUO_RESISTANCE] = 101232.8;
    RuO_High_Table[70][RUO_TEMPERATURE] = 204.0; RuO_High_Table[70][RUO_RESISTANCE] = 101253.3;
    RuO_High_Table[71][RUO_TEMPERATURE] = 203.0; RuO_High_Table[71][RUO_RESISTANCE] = 101274.1;
    RuO_High_Table[72][RUO_TEMPERATURE] = 202.0; RuO_High_Table[72][RUO_RESISTANCE] = 101295.3;
    RuO_High_Table[73][RUO_TEMPERATURE] = 201.0; RuO_High_Table[73][RUO_RESISTANCE] = 101316.7;
    RuO_High_Table[74][RUO_TEMPERATURE] = 200.0; RuO_High_Table[74][RUO_RESISTANCE] = 101338.5;

    RuO_High_Table[75][RUO_TEMPERATURE] = 199.0; RuO_High_Table[75][RUO_RESISTANCE] = 101360.0;
    RuO_High_Table[76][RUO_TEMPERATURE] = 198.0; RuO_High_Table[76][RUO_RESISTANCE] = 101381.8;
    RuO_High_Table[77][RUO_TEMPERATURE] = 197.0; RuO_High_Table[77][RUO_RESISTANCE] = 101403.8;
    RuO_High_Table[78][RUO_TEMPERATURE] = 196.0; RuO_High_Table[78][RUO_RESISTANCE] = 101426.2;
    RuO_High_Table[79][RUO_TEMPERATURE] = 195.0; RuO_High_Table[79][RUO_RESISTANCE] = 101448.8;
    RuO_High_Table[80][RUO_TEMPERATURE] = 194.0; RuO_High_Table[80][RUO_RESISTANCE] = 101471.7;
    RuO_High_Table[81][RUO_TEMPERATURE] = 193.0; RuO_High_Table[81][RUO_RESISTANCE] = 101495.0;
    RuO_High_Table[82][RUO_TEMPERATURE] = 192.0; RuO_High_Table[82][RUO_RESISTANCE] = 101518.5;
    RuO_High_Table[83][RUO_TEMPERATURE] = 191.0; RuO_High_Table[83][RUO_RESISTANCE] = 101542.2;
    RuO_High_Table[84][RUO_TEMPERATURE] = 190.0; RuO_High_Table[84][RUO_RESISTANCE] = 101566.3;

    RuO_High_Table[85][RUO_TEMPERATURE] = 189.0; RuO_High_Table[85][RUO_RESISTANCE] = 101590.7;
    RuO_High_Table[86][RUO_TEMPERATURE] = 188.0; RuO_High_Table[86][RUO_RESISTANCE] = 101615.4;
    RuO_High_Table[87][RUO_TEMPERATURE] = 187.0; RuO_High_Table[87][RUO_RESISTANCE] = 101640.4;
    RuO_High_Table[88][RUO_TEMPERATURE] = 186.0; RuO_High_Table[88][RUO_RESISTANCE] = 101665.7;
    RuO_High_Table[89][RUO_TEMPERATURE] = 185.0; RuO_High_Table[89][RUO_RESISTANCE] = 101691.4;
    RuO_High_Table[90][RUO_TEMPERATURE] = 184.0; RuO_High_Table[90][RUO_RESISTANCE] = 101717.4;
    RuO_High_Table[91][RUO_TEMPERATURE] = 183.0; RuO_High_Table[91][RUO_RESISTANCE] = 101743.7;
    RuO_High_Table[92][RUO_TEMPERATURE] = 182.0; RuO_High_Table[92][RUO_RESISTANCE] = 101770.4;
    RuO_High_Table[93][RUO_TEMPERATURE] = 181.0; RuO_High_Table[93][RUO_RESISTANCE] = 101797.4;
    RuO_High_Table[94][RUO_TEMPERATURE] = 180.0; RuO_High_Table[94][RUO_RESISTANCE] = 101824.8;

    RuO_High_Table[95][RUO_TEMPERATURE] = 179.0; RuO_High_Table[95][RUO_RESISTANCE] = 101852.5;
    RuO_High_Table[96][RUO_TEMPERATURE] = 178.0; RuO_High_Table[96][RUO_RESISTANCE] = 101880.5;
    RuO_High_Table[97][RUO_TEMPERATURE] = 177.0; RuO_High_Table[97][RUO_RESISTANCE] = 101908.9;
    RuO_High_Table[98][RUO_TEMPERATURE] = 176.0; RuO_High_Table[98][RUO_RESISTANCE] = 101937.7;
    RuO_High_Table[99][RUO_TEMPERATURE] = 175.0; RuO_High_Table[99][RUO_RESISTANCE] = 101966.8;
    RuO_High_Table[100][RUO_TEMPERATURE] = 174.0; RuO_High_Table[100][RUO_RESISTANCE] = 101996.2;
    RuO_High_Table[101][RUO_TEMPERATURE] = 173.0; RuO_High_Table[101][RUO_RESISTANCE] = 102026.0;
    RuO_High_Table[102][RUO_TEMPERATURE] = 172.0; RuO_High_Table[102][RUO_RESISTANCE] = 102056.2;
    RuO_High_Table[103][RUO_TEMPERATURE] = 171.0; RuO_High_Table[103][RUO_RESISTANCE] = 102086.7;
    RuO_High_Table[104][RUO_TEMPERATURE] = 170.0; RuO_High_Table[104][RUO_RESISTANCE] = 102117.6;

    RuO_High_Table[105][RUO_TEMPERATURE] = 169.0; RuO_High_Table[105][RUO_RESISTANCE] = 102149.0;
    RuO_High_Table[106][RUO_TEMPERATURE] = 168.0; RuO_High_Table[106][RUO_RESISTANCE] = 102180.7;
    RuO_High_Table[107][RUO_TEMPERATURE] = 167.0; RuO_High_Table[107][RUO_RESISTANCE] = 102212.8;
    RuO_High_Table[108][RUO_TEMPERATURE] = 166.0; RuO_High_Table[108][RUO_RESISTANCE] = 102245.3;
    RuO_High_Table[109][RUO_TEMPERATURE] = 165.0; RuO_High_Table[109][RUO_RESISTANCE] = 102278.3;
    RuO_High_Table[110][RUO_TEMPERATURE] = 164.0; RuO_High_Table[110][RUO_RESISTANCE] = 102311.7;
    RuO_High_Table[111][RUO_TEMPERATURE] = 163.0; RuO_High_Table[111][RUO_RESISTANCE] = 102345.5;
    RuO_High_Table[112][RUO_TEMPERATURE] = 162.0; RuO_High_Table[112][RUO_RESISTANCE] = 102379.8;
    RuO_High_Table[113][RUO_TEMPERATURE] = 161.0; RuO_High_Table[113][RUO_RESISTANCE] = 102414.6;
    RuO_High_Table[114][RUO_TEMPERATURE] = 160.0; RuO_High_Table[114][RUO_RESISTANCE] = 102449.7;

    RuO_High_Table[115][RUO_TEMPERATURE] = 159.0; RuO_High_Table[115][RUO_RESISTANCE] = 102485.4;
    RuO_High_Table[116][RUO_TEMPERATURE] = 158.0; RuO_High_Table[116][RUO_RESISTANCE] = 102521.5;
    RuO_High_Table[117][RUO_TEMPERATURE] = 157.0; RuO_High_Table[117][RUO_RESISTANCE] = 102558.0;
    RuO_High_Table[118][RUO_TEMPERATURE] = 156.0; RuO_High_Table[118][RUO_RESISTANCE] = 102595.1;
    RuO_High_Table[119][RUO_TEMPERATURE] = 155.0; RuO_High_Table[119][RUO_RESISTANCE] = 102632.7;
    RuO_High_Table[120][RUO_TEMPERATURE] = 154.0; RuO_High_Table[120][RUO_RESISTANCE] = 102670.8;
    RuO_High_Table[121][RUO_TEMPERATURE] = 153.0; RuO_High_Table[121][RUO_RESISTANCE] = 102709.4;
    RuO_High_Table[122][RUO_TEMPERATURE] = 152.0; RuO_High_Table[122][RUO_RESISTANCE] = 102748.5;
    RuO_High_Table[123][RUO_TEMPERATURE] = 151.0; RuO_High_Table[123][RUO_RESISTANCE] = 102788.1;
    RuO_High_Table[124][RUO_TEMPERATURE] = 150.0; RuO_High_Table[124][RUO_RESISTANCE] = 102828.3;

    RuO_High_Table[125][RUO_TEMPERATURE] = 149.0; RuO_High_Table[125][RUO_RESISTANCE] = 102868.4;
    RuO_High_Table[126][RUO_TEMPERATURE] = 148.0; RuO_High_Table[126][RUO_RESISTANCE] = 102908.8;
    RuO_High_Table[127][RUO_TEMPERATURE] = 147.0; RuO_High_Table[127][RUO_RESISTANCE] = 102949.8;
    RuO_High_Table[128][RUO_TEMPERATURE] = 146.0; RuO_High_Table[128][RUO_RESISTANCE] = 102991.3;
    RuO_High_Table[129][RUO_TEMPERATURE] = 145.0; RuO_High_Table[129][RUO_RESISTANCE] = 103033.3;
    RuO_High_Table[130][RUO_TEMPERATURE] = 144.0; RuO_High_Table[130][RUO_RESISTANCE] = 103075.8;
    RuO_High_Table[131][RUO_TEMPERATURE] = 143.0; RuO_High_Table[131][RUO_RESISTANCE] = 103118.8;
    RuO_High_Table[132][RUO_TEMPERATURE] = 142.0; RuO_High_Table[132][RUO_RESISTANCE] = 103162.5;
    RuO_High_Table[133][RUO_TEMPERATURE] = 141.0; RuO_High_Table[133][RUO_RESISTANCE] = 103206.6;
    RuO_High_Table[134][RUO_TEMPERATURE] = 140.0; RuO_High_Table[134][RUO_RESISTANCE] = 103251.4;

    RuO_High_Table[135][RUO_TEMPERATURE] = 139.0; RuO_High_Table[135][RUO_RESISTANCE] = 103296.7;
    RuO_High_Table[136][RUO_TEMPERATURE] = 138.0; RuO_High_Table[136][RUO_RESISTANCE] = 103342.5;
    RuO_High_Table[137][RUO_TEMPERATURE] = 137.0; RuO_High_Table[137][RUO_RESISTANCE] = 103389.0;
    RuO_High_Table[138][RUO_TEMPERATURE] = 136.0; RuO_High_Table[138][RUO_RESISTANCE] = 103436.1;
    RuO_High_Table[139][RUO_TEMPERATURE] = 135.0; RuO_High_Table[139][RUO_RESISTANCE] = 103483.8;
    RuO_High_Table[140][RUO_TEMPERATURE] = 134.0; RuO_High_Table[140][RUO_RESISTANCE] = 103532.1;
    RuO_High_Table[141][RUO_TEMPERATURE] = 133.0; RuO_High_Table[141][RUO_RESISTANCE] = 103581.1;
    RuO_High_Table[142][RUO_TEMPERATURE] = 132.0; RuO_High_Table[142][RUO_RESISTANCE] = 103630.7;
    RuO_High_Table[143][RUO_TEMPERATURE] = 131.0; RuO_High_Table[143][RUO_RESISTANCE] = 103681.0;
    RuO_High_Table[144][RUO_TEMPERATURE] = 130.0; RuO_High_Table[144][RUO_RESISTANCE] = 103731.9;

    RuO_High_Table[145][RUO_TEMPERATURE] = 129.0; RuO_High_Table[145][RUO_RESISTANCE] = 103783.6;
    RuO_High_Table[146][RUO_TEMPERATURE] = 128.0; RuO_High_Table[146][RUO_RESISTANCE] = 103836.0;
    RuO_High_Table[147][RUO_TEMPERATURE] = 127.0; RuO_High_Table[147][RUO_RESISTANCE] = 103889.1;
    RuO_High_Table[148][RUO_TEMPERATURE] = 126.0; RuO_High_Table[148][RUO_RESISTANCE] = 103942.9;
    RuO_High_Table[149][RUO_TEMPERATURE] = 125.0; RuO_High_Table[149][RUO_RESISTANCE] = 103997.4;
    RuO_High_Table[150][RUO_TEMPERATURE] = 124.0; RuO_High_Table[150][RUO_RESISTANCE] = 104052.6;
    RuO_High_Table[151][RUO_TEMPERATURE] = 123.0; RuO_High_Table[151][RUO_RESISTANCE] = 104108.5;
    RuO_High_Table[152][RUO_TEMPERATURE] = 122.0; RuO_High_Table[152][RUO_RESISTANCE] = 104165.3;
    RuO_High_Table[153][RUO_TEMPERATURE] = 121.0; RuO_High_Table[153][RUO_RESISTANCE] = 104222.8;
    RuO_High_Table[154][RUO_TEMPERATURE] = 120.0; RuO_High_Table[154][RUO_RESISTANCE] = 104281.1;

    RuO_High_Table[155][RUO_TEMPERATURE] = 119.0; RuO_High_Table[155][RUO_RESISTANCE] = 104340.3;
    RuO_High_Table[156][RUO_TEMPERATURE] = 118.0; RuO_High_Table[156][RUO_RESISTANCE] = 104400.4;
    RuO_High_Table[157][RUO_TEMPERATURE] = 117.0; RuO_High_Table[157][RUO_RESISTANCE] = 104461.2;
    RuO_High_Table[158][RUO_TEMPERATURE] = 116.0; RuO_High_Table[158][RUO_RESISTANCE] = 104523.0;
    RuO_High_Table[159][RUO_TEMPERATURE] = 115.0; RuO_High_Table[159][RUO_RESISTANCE] = 104585.6;
    RuO_High_Table[160][RUO_TEMPERATURE] = 114.0; RuO_High_Table[160][RUO_RESISTANCE] = 104649.3;
    RuO_High_Table[161][RUO_TEMPERATURE] = 113.0; RuO_High_Table[161][RUO_RESISTANCE] = 104713.8;
    RuO_High_Table[162][RUO_TEMPERATURE] = 112.0; RuO_High_Table[162][RUO_RESISTANCE] = 104779.3;
    RuO_High_Table[163][RUO_TEMPERATURE] = 111.0; RuO_High_Table[163][RUO_RESISTANCE] = 104845.8;
    RuO_High_Table[164][RUO_TEMPERATURE] = 110.0; RuO_High_Table[164][RUO_RESISTANCE] = 104913.3;

    RuO_High_Table[165][RUO_TEMPERATURE] = 109.0; RuO_High_Table[165][RUO_RESISTANCE] = 104981.7;
    RuO_High_Table[166][RUO_TEMPERATURE] = 108.0; RuO_High_Table[166][RUO_RESISTANCE] = 105051.3;
    RuO_High_Table[167][RUO_TEMPERATURE] = 107.0; RuO_High_Table[167][RUO_RESISTANCE] = 105122.0;
    RuO_High_Table[168][RUO_TEMPERATURE] = 106.0; RuO_High_Table[168][RUO_RESISTANCE] = 105193.8;
    RuO_High_Table[169][RUO_TEMPERATURE] = 105.0; RuO_High_Table[169][RUO_RESISTANCE] = 105266.6;
    RuO_High_Table[170][RUO_TEMPERATURE] = 104.0; RuO_High_Table[170][RUO_RESISTANCE] = 105340.7;
    RuO_High_Table[171][RUO_TEMPERATURE] = 103.0; RuO_High_Table[171][RUO_RESISTANCE] = 105415.9;
    RuO_High_Table[172][RUO_TEMPERATURE] = 102.0; RuO_High_Table[172][RUO_RESISTANCE] = 105492.4;
    RuO_High_Table[173][RUO_TEMPERATURE] = 101.0; RuO_High_Table[173][RUO_RESISTANCE] = 105570.1;
    RuO_High_Table[174][RUO_TEMPERATURE] = 100.0; RuO_High_Table[174][RUO_RESISTANCE] = 105649.0;

    RuO_High_Table[175][RUO_TEMPERATURE] = 99.0; RuO_High_Table[175][RUO_RESISTANCE] = 105729.8;
    RuO_High_Table[176][RUO_TEMPERATURE] = 98.0; RuO_High_Table[176][RUO_RESISTANCE] = 105812.0;
    RuO_High_Table[177][RUO_TEMPERATURE] = 97.0; RuO_High_Table[177][RUO_RESISTANCE] = 105895.6;
    RuO_High_Table[178][RUO_TEMPERATURE] = 96.0; RuO_High_Table[178][RUO_RESISTANCE] = 105980.6;
    RuO_High_Table[179][RUO_TEMPERATURE] = 95.0; RuO_High_Table[179][RUO_RESISTANCE] = 106067.2;
    RuO_High_Table[180][RUO_TEMPERATURE] = 94.0; RuO_High_Table[180][RUO_RESISTANCE] = 106155.3;
    RuO_High_Table[181][RUO_TEMPERATURE] = 93.0; RuO_High_Table[181][RUO_RESISTANCE] = 106244.9;
    RuO_High_Table[182][RUO_TEMPERATURE] = 92.0; RuO_High_Table[182][RUO_RESISTANCE] = 106336.3;
    RuO_High_Table[183][RUO_TEMPERATURE] = 91.0; RuO_High_Table[183][RUO_RESISTANCE] = 106429.3;
    RuO_High_Table[184][RUO_TEMPERATURE] = 90.0; RuO_High_Table[184][RUO_RESISTANCE] = 106524.0;

    RuO_High_Table[185][RUO_TEMPERATURE] = 89.0; RuO_High_Table[185][RUO_RESISTANCE] = 106621.3;
    RuO_High_Table[186][RUO_TEMPERATURE] = 88.0; RuO_High_Table[186][RUO_RESISTANCE] = 106720.8;
    RuO_High_Table[187][RUO_TEMPERATURE] = 87.0; RuO_High_Table[187][RUO_RESISTANCE] = 106822.2;
    RuO_High_Table[188][RUO_TEMPERATURE] = 86.0; RuO_High_Table[188][RUO_RESISTANCE] = 106925.9;
    RuO_High_Table[189][RUO_TEMPERATURE] = 85.0; RuO_High_Table[189][RUO_RESISTANCE] = 107031.7;
    RuO_High_Table[190][RUO_TEMPERATURE] = 84.0; RuO_High_Table[190][RUO_RESISTANCE] = 107139.7;
    RuO_High_Table[191][RUO_TEMPERATURE] = 83.0; RuO_High_Table[191][RUO_RESISTANCE] = 107250.1;
    RuO_High_Table[192][RUO_TEMPERATURE] = 82.0; RuO_High_Table[192][RUO_RESISTANCE] = 107363.0;
    RuO_High_Table[193][RUO_TEMPERATURE] = 81.0; RuO_High_Table[193][RUO_RESISTANCE] = 107478.3;
    RuO_High_Table[194][RUO_TEMPERATURE] = 80.0; RuO_High_Table[194][RUO_RESISTANCE] = 107596.1;

    RuO_High_Table[195][RUO_TEMPERATURE] = 79.0; RuO_High_Table[195][RUO_RESISTANCE] = 107716.4;
    RuO_High_Table[196][RUO_TEMPERATURE] = 78.0; RuO_High_Table[196][RUO_RESISTANCE] = 107839.4;
    RuO_High_Table[197][RUO_TEMPERATURE] = 77.0; RuO_High_Table[197][RUO_RESISTANCE] = 107965.1;
    RuO_High_Table[198][RUO_TEMPERATURE] = 76.0; RuO_High_Table[198][RUO_RESISTANCE] = 108092.9;
    RuO_High_Table[199][RUO_TEMPERATURE] = 75.0; RuO_High_Table[199][RUO_RESISTANCE] = 108223.0;
    RuO_High_Table[200][RUO_TEMPERATURE] = 74.0; RuO_High_Table[200][RUO_RESISTANCE] = 108355.8;
    RuO_High_Table[201][RUO_TEMPERATURE] = 73.0; RuO_High_Table[201][RUO_RESISTANCE] = 108491.2;
    RuO_High_Table[202][RUO_TEMPERATURE] = 72.0; RuO_High_Table[202][RUO_RESISTANCE] = 108629.4;
    RuO_High_Table[203][RUO_TEMPERATURE] = 71.0; RuO_High_Table[203][RUO_RESISTANCE] = 108770.4;
    RuO_High_Table[204][RUO_TEMPERATURE] = 70.0; RuO_High_Table[204][RUO_RESISTANCE] = 108914.3;

    RuO_High_Table[205][RUO_TEMPERATURE] = 69.0; RuO_High_Table[205][RUO_RESISTANCE] = 109063.4;
    RuO_High_Table[206][RUO_TEMPERATURE] = 68.0; RuO_High_Table[206][RUO_RESISTANCE] = 109216.4;
    RuO_High_Table[207][RUO_TEMPERATURE] = 67.0; RuO_High_Table[207][RUO_RESISTANCE] = 109373.3;
    RuO_High_Table[208][RUO_TEMPERATURE] = 66.0; RuO_High_Table[208][RUO_RESISTANCE] = 109534.3;
    RuO_High_Table[209][RUO_TEMPERATURE] = 65.0; RuO_High_Table[209][RUO_RESISTANCE] = 109699.5;
    RuO_High_Table[210][RUO_TEMPERATURE] = 64.0; RuO_High_Table[210][RUO_RESISTANCE] = 109869.1;
    RuO_High_Table[211][RUO_TEMPERATURE] = 63.0; RuO_High_Table[211][RUO_RESISTANCE] = 110043.3;
    RuO_High_Table[212][RUO_TEMPERATURE] = 62.0; RuO_High_Table[212][RUO_RESISTANCE] = 110222.2;
    RuO_High_Table[213][RUO_TEMPERATURE] = 61.0; RuO_High_Table[213][RUO_RESISTANCE] = 110406.1;
    RuO_High_Table[214][RUO_TEMPERATURE] = 60.0; RuO_High_Table[214][RUO_RESISTANCE] = 110595.1;

    RuO_High_Table[215][RUO_TEMPERATURE] = 59.0; RuO_High_Table[215][RUO_RESISTANCE] = 110791.4;
    RuO_High_Table[216][RUO_TEMPERATURE] = 58.0; RuO_High_Table[216][RUO_RESISTANCE] = 110993.7;
    RuO_High_Table[217][RUO_TEMPERATURE] = 57.0; RuO_High_Table[217][RUO_RESISTANCE] = 111202.2;
    RuO_High_Table[218][RUO_TEMPERATURE] = 56.0; RuO_High_Table[218][RUO_RESISTANCE] = 111417.4;
    RuO_High_Table[219][RUO_TEMPERATURE] = 55.0; RuO_High_Table[229][RUO_RESISTANCE] = 111639.3;
    RuO_High_Table[220][RUO_TEMPERATURE] = 54.0; RuO_High_Table[220][RUO_RESISTANCE] = 111868.6;
    RuO_High_Table[221][RUO_TEMPERATURE] = 53.0; RuO_High_Table[221][RUO_RESISTANCE] = 112105.3;
    RuO_High_Table[222][RUO_TEMPERATURE] = 52.0; RuO_High_Table[222][RUO_RESISTANCE] = 112350.0;
    RuO_High_Table[223][RUO_TEMPERATURE] = 51.0; RuO_High_Table[223][RUO_RESISTANCE] = 112603.0;
    RuO_High_Table[224][RUO_TEMPERATURE] = 50.0; RuO_High_Table[224][RUO_RESISTANCE] = 112864.8;

    RuO_High_Table[225][RUO_TEMPERATURE] = 49.0; RuO_High_Table[225][RUO_RESISTANCE] = 113137.3;
    RuO_High_Table[226][RUO_TEMPERATURE] = 48.0; RuO_High_Table[226][RUO_RESISTANCE] = 113419.9;
    RuO_High_Table[227][RUO_TEMPERATURE] = 47.0; RuO_High_Table[227][RUO_RESISTANCE] = 113713.1;
    RuO_High_Table[228][RUO_TEMPERATURE] = 46.0; RuO_High_Table[228][RUO_RESISTANCE] = 114017.4;
    RuO_High_Table[229][RUO_TEMPERATURE] = 45.0; RuO_High_Table[229][RUO_RESISTANCE] = 114333.6;
    RuO_High_Table[230][RUO_TEMPERATURE] = 44.0; RuO_High_Table[230][RUO_RESISTANCE] = 114662.3;
    RuO_High_Table[231][RUO_TEMPERATURE] = 43.0; RuO_High_Table[231][RUO_RESISTANCE] = 115004.2;
    RuO_High_Table[232][RUO_TEMPERATURE] = 42.0; RuO_High_Table[232][RUO_RESISTANCE] = 115360.3;
    RuO_High_Table[233][RUO_TEMPERATURE] = 41.0; RuO_High_Table[233][RUO_RESISTANCE] = 115731.3;
    RuO_High_Table[234][RUO_TEMPERATURE] = 40.0; RuO_High_Table[234][RUO_RESISTANCE] = 116118.1;

    RuO_High_Table[235][RUO_TEMPERATURE] = 39.0; RuO_High_Table[235][RUO_RESISTANCE] = 116523.6;
    RuO_High_Table[236][RUO_TEMPERATURE] = 38.0; RuO_High_Table[236][RUO_RESISTANCE] = 116947.6;
    RuO_High_Table[237][RUO_TEMPERATURE] = 37.0; RuO_High_Table[237][RUO_RESISTANCE] = 117391.9;
    RuO_High_Table[238][RUO_TEMPERATURE] = 36.0; RuO_High_Table[238][RUO_RESISTANCE] = 117857.8;
    RuO_High_Table[239][RUO_TEMPERATURE] = 35.0; RuO_High_Table[239][RUO_RESISTANCE] = 118347.3;
    RuO_High_Table[240][RUO_TEMPERATURE] = 34.0; RuO_High_Table[240][RUO_RESISTANCE] = 118861.6;
    RuO_High_Table[241][RUO_TEMPERATURE] = 33.0; RuO_High_Table[241][RUO_RESISTANCE] = 119404.0;
    RuO_High_Table[242][RUO_TEMPERATURE] = 32.0; RuO_High_Table[242][RUO_RESISTANCE] = 119976.2;
    RuO_High_Table[243][RUO_TEMPERATURE] = 31.0; RuO_High_Table[243][RUO_RESISTANCE] = 120580.7;
    RuO_High_Table[244][RUO_TEMPERATURE] = 30.0; RuO_High_Table[244][RUO_RESISTANCE] = 121220.3;

    RuO_High_Table[245][RUO_TEMPERATURE] = 29.0; RuO_High_Table[245][RUO_RESISTANCE] = 121898.4;
    RuO_High_Table[246][RUO_TEMPERATURE] = 28.0; RuO_High_Table[246][RUO_RESISTANCE] = 122618.5;
    RuO_High_Table[247][RUO_TEMPERATURE] = 27.0; RuO_High_Table[247][RUO_RESISTANCE] = 123386.0;
    RuO_High_Table[248][RUO_TEMPERATURE] = 26.0; RuO_High_Table[248][RUO_RESISTANCE] = 124205.1;
    RuO_High_Table[249][RUO_TEMPERATURE] = 25.0; RuO_High_Table[249][RUO_RESISTANCE] = 125082.5;
    RuO_High_Table[250][RUO_TEMPERATURE] = 24.0; RuO_High_Table[250][RUO_RESISTANCE] = 126024.0;
    RuO_High_Table[251][RUO_TEMPERATURE] = 23.0; RuO_High_Table[251][RUO_RESISTANCE] = 127038.1;
    RuO_High_Table[252][RUO_TEMPERATURE] = 22.0; RuO_High_Table[252][RUO_RESISTANCE] = 128132.9;
    RuO_High_Table[253][RUO_TEMPERATURE] = 21.0; RuO_High_Table[253][RUO_RESISTANCE] = 129317.7;
    RuO_High_Table[254][RUO_TEMPERATURE] = 20.0; RuO_High_Table[254][RUO_RESISTANCE] = 130604.6;

    RuO_High_Table[255][RUO_TEMPERATURE] = 19.0; RuO_High_Table[255][RUO_RESISTANCE] = 132008.6;
    RuO_High_Table[256][RUO_TEMPERATURE] = 18.0; RuO_High_Table[256][RUO_RESISTANCE] = 133549.6;
    RuO_High_Table[257][RUO_TEMPERATURE] = 17.0; RuO_High_Table[257][RUO_RESISTANCE] = 135245.9;
    RuO_High_Table[258][RUO_TEMPERATURE] = 16.0; RuO_High_Table[258][RUO_RESISTANCE] = 137122.0;
    RuO_High_Table[259][RUO_TEMPERATURE] = 15.0; RuO_High_Table[259][RUO_RESISTANCE] = 139213.9;
    RuO_High_Table[260][RUO_TEMPERATURE] = 14.0; RuO_High_Table[260][RUO_RESISTANCE] = 141556.3;
    RuO_High_Table[261][RUO_TEMPERATURE] = 13.0; RuO_High_Table[261][RUO_RESISTANCE] = 144196.7;
    RuO_High_Table[262][RUO_TEMPERATURE] = 12.0; RuO_High_Table[262][RUO_RESISTANCE] = 147197.2;
    RuO_High_Table[263][RUO_TEMPERATURE] = 11.0; RuO_High_Table[263][RUO_RESISTANCE] = 150624.2;
    RuO_High_Table[264][RUO_TEMPERATURE] = 10.0; RuO_High_Table[264][RUO_RESISTANCE] = 154569.1;

    RuO_High_Table[265][RUO_TEMPERATURE] = 9.0; RuO_High_Table[265][RUO_RESISTANCE] = 159156.5;
    RuO_High_Table[266][RUO_TEMPERATURE] = 8.0; RuO_High_Table[266][RUO_RESISTANCE] = 164590.2;
    RuO_High_Table[267][RUO_TEMPERATURE] = 7.0; RuO_High_Table[267][RUO_RESISTANCE] = 171089.3;
    RuO_High_Table[268][RUO_TEMPERATURE] = 6.0; RuO_High_Table[268][RUO_RESISTANCE] = 178961.5;
    RuO_High_Table[269][RUO_TEMPERATURE] = 5.0; RuO_High_Table[269][RUO_RESISTANCE] = 188603.8;
    RuO_High_Table[270][RUO_TEMPERATURE] = 4.0; RuO_High_Table[270][RUO_RESISTANCE] = 200519.3;
    RuO_High_Table[271][RUO_TEMPERATURE] = 3.0; RuO_High_Table[271][RUO_RESISTANCE] = 214778.0;
    RuO_High_Table[272][RUO_TEMPERATURE] = 2.0; RuO_High_Table[272][RUO_RESISTANCE] = 230349.3;
}
