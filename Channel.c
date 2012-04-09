/* DAAS4a
- added capability to read data files (menu item)
*/

#include <analysis.h>
#include <formatio.h>
#include <utility.h>
#include <userint.h>
#include <ansi_c.h>

#include "util.h"
#include "utilu.h"
#include "list.h"
#include "listu.h"
#include "channel.h"
#include "channelu.h"

#define FALSE 0
#define TRUE 1

struct channelinfoType channelG;

struct chanviewStruct chanview;

/* CHANNEL FUNCTIONS */

channelPtr  channel_Create (void);
int         channel_AllocMem (channelPtr c, unsigned int pts);
channelPtr  channel_CopySubset (int offset, int pts, channelPtr c);
double      channel_GetReading (channelPtr c, int index);
void        channel_AddCurve (channelPtr chan, char *curve, char *graph);
void        channel_RemoveCurve (channelPtr chan, char *curve);
void        channel_Save (channelPtr c);
void        channel_SaveSubset (channelPtr chan);
channelPtr  channel_Load (void);
void        channel_UpdateReadingList (channelPtr chan);
void        channel_FillLocation (channelPtr chan);

void channel_InitEditPanel (channelPtr chan, int editlabel);
void channel_UpdateEditPanel (channelPtr chan);

void channel_InitViewPanel (void);
void channel_UpdateViewPanel (channelPtr chan);
void channel_UpdateViewGraph (channelPtr chan, int scatter);

/* CHANNEL LIST FUNCTIONS */

channelPtr  channellist_GetItem (int i);
channelPtr  channellist_GetSelection (void);
void        channellist_Copy (int panel, int control);
void        channellist_UpdatePanel (void);
void        channellist_AddChannel (channelPtr chan);
void        channellist_RemoveChannel (channelPtr chan);

void channelG_Init (void);
void channelG_Exit (void);

int  RemoveSelectedChannelsCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  SaveSelectedChannelsCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  ShowChannelsCallback(int panel, int control, int event,
                          void *callbackData, int eventData1, int eventData2);

channelPtr channel_Create (void)
{
    channelPtr c;

    c = malloc (sizeof(channelType));

    if (!c) {util_OutofMemory("Create Channel Error"); return NULL;}

    Fmt (c->label, "New Channel");
    Fmt (c->note, "");
    c->pts = 0;
    c->readings = NULL;
    list_Init (&c->graphs);
    list_Init (&c->curves);
    return c;
}

int channel_AllocMem (channelPtr c, unsigned int pts)
{
    char msg[260];
    c->pts = pts;
    c->readings = calloc (pts, sizeof (double));
    if (c->readings) return TRUE;
    else return FALSE;
}

channelPtr channel_CopySubset (int offset, int pts, channelPtr c)
{
    channelPtr dupchan;

    dupchan = channel_Create ();
    if (!dupchan) { util_OutofMemory ("Channel Copy Error"); return NULL;}

    if (!channel_AllocMem (dupchan, pts))
        {util_OutofMemory ("Channel Copy Error");free (dupchan); return NULL;}

    Fmt (dupchan->label, c->label);


    Subset1D (c->readings, c->pts, offset, pts, dupchan->readings);
    dupchan->pts = pts;
    return dupchan;
}

double channel_GetReading (channelPtr c, int index)
{
    return c->readings[index];
}

void channel_AddCurve (channelPtr chan, char *curve, char *graph)
{
    list_AddItem (&chan->graphs, graph);
    list_AddItem (&chan->curves, curve);
}

void channel_RemoveCurve (channelPtr chan, char *curve)
{
    int i;

    i = list_FindItem (chan->curves, curve);
    if (i != NOT_IN_LIST)
    {
        list_RemoveItem (&chan->curves, i, FALSE);
        list_RemoveItem (&chan->graphs, i, FALSE);
    }
}

void channel_Save (channelPtr c)
{
    FmtFile (fileHandle.analysis, "Channel   : %s;\n", c->label);
    FmtFile (fileHandle.analysis, "Attribute : %i [READINGS]\n", c->pts);
    util_SaveNote (c->note);
    FmtFile (fileHandle.analysis, "%s<%*f[e2p5w13]\n", c->pts, c->readings);
}

channelPtr channel_Load (void)
{
    channelPtr c;
    char info[260];
    unsigned int pts;
    int i;

    c = channel_Create ();
    if (!c) return NULL;

    ScanFile (fileHandle.analysis, "%s>Channel   : %s[t59]%s[w1d]", c->label);
    ScanFile (fileHandle.analysis, "%s>Attribute : %i [READINGS]%s[w1d]", &pts);
    util_LoadNote (c->note);

    if (!channel_AllocMem (c, pts)) { free (c); return NULL;}

    ScanFile (fileHandle.analysis, "%s>%*f%s[w1d]", pts, c->readings);
    return c;
}

void channel_SaveSubset (channelPtr chan)
{
    char path[260], info[260];
    int filestatus;

    filestatus = FileSelectPopup ("", "*.chn", "*.chn",
                                  "Save Channel Data", VAL_SAVE_BUTTON, 0,
                                  1, 1, 1, path);
    if (filestatus == VAL_NEW_FILE_SELECTED)
    {
        fileHandle.analysis = util_OpenFile(path, FILE_WRITE, FALSE);
        FmtFile (fileHandle.analysis, "#CHANNELDATA\n");
        channel_Save (chan);
        FmtFile (fileHandle.analysis, "#ENDCHANNELDATA\n");
        util_CloseFile();
    }
}

void channel_FillLocation (channelPtr chan)
{
    int index;
    nodePtr node;
    char label[260], label1[260];

    SetInputMode (channelG.p.channels, CHANNELS_LOCATION, chan->curves.nItems);
    ClearListCtrl (channelG.p.channels, CHANNELS_LOCATION);
    if (chan->curves.nItems)
    {
        for (index = 0; index < chan->curves.nItems; index++)
        {
            node = list_GetNode (chan->graphs, index);
            Fmt (label, node->item);
            node = list_GetNode (chan->curves, index);
            Fmt (label1, node->item);
            Fmt (label, "%s[a]<:%s", label1);
            InsertListItem (channelG.p.channels, CHANNELS_LOCATION, index, label,
                            index);
        }
    }
}

void channel_InitViewPanel (void)
{
    int i;

    chanview.p1 = LoadPanel (0, "channelu.uir", CHANVIEW);
    

    util_InitClose (chanview.p1, CHANVIEW_DONE, TRUE);

    SetPanelPos (chanview.p1, VAL_AUTO_CENTER, VAL_AUTO_CENTER);

    chanview.p2 = 0;

    channellist_Copy (chanview.p1, CHANVIEW_CHANNELS);

    GetCtrlVal (channelG.p.channels, CHANNELS_LIST, &i);
    SetCtrlVal (chanview.p1, CHANVIEW_CHANNELS, i);

    SetCtrlVal (chanview.p1, CHANVIEW_LOG, FALSE);
    SetCtrlVal (chanview.p1, CHANVIEW_GRID, TRUE);
    SetCtrlVal (chanview.p1, CHANVIEW_SCATTER, FALSE);
}

void channel_UpdateViewPanel (channelPtr chan)
{
    SetInputMode (chanview.p1, CHANVIEW_OFFSET, FALSE);
    SetInputMode (chanview.p1, CHANVIEW_OFFSETVAL, FALSE);
    SetCtrlAttribute (chanview.p1, CHANVIEW_OFFSET, ATTR_CALLBACK_DATA, chan);

    SetCtrlVal (chanview.p1, CHANVIEW_POINTS, chan->pts);
    SetCtrlVal (chanview.p1, CHANVIEW_POINTS_2, chan->pts);
    SetCtrlAttribute (chanview.p1, CHANVIEW_POINTS_2, ATTR_CALLBACK_DATA, chan);

    chanview.offset = 0;
    chanview.pts = chan->pts;

    SetCtrlAttribute (chanview.p1, CHANVIEW_POINTS_2, ATTR_MAX_VALUE, chan->pts);

    SetCtrlVal (chanview.p1, CHANVIEW_SCATTER, FALSE);
    SetCtrlAttribute (chanview.p1, CHANVIEW_SCATTER, ATTR_CALLBACK_DATA, chan);

    channel_UpdateViewGraph (chan, FALSE);
    SetAxisRange (chanview.p1, CHANVIEW_GRAPH,
                  VAL_AUTOSCALE, 0.0, 1.0,
                  VAL_AUTOSCALE, 0.0, 1.0);
}

void channel_UpdateViewGraph (channelPtr chan, int scatter)
{
    if (scatter) scatter = VAL_SCATTER; else scatter = VAL_THIN_LINE;
    DeleteGraphPlot (chanview.p1, CHANVIEW_GRAPH, -1, VAL_DELAYED_DRAW);
    PlotY (chanview.p1, CHANVIEW_GRAPH, chan->readings,
           chan->pts, VAL_DOUBLE, scatter, VAL_SMALL_X, VAL_SOLID,
           1, VAL_GREEN);
}

int  ChannelViewDoneCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    if (event == EVENT_COMMIT)
    {
        if (chanview.p2)
        {
            
            DiscardPanel (chanview.p2);
            chanview.p2 = 0;
        }
        
        DiscardPanel (chanview.p1);
    }
    return 0;
}

int  ChannelViewScatterCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int scatter;
    channelPtr chan;
    if (event == EVENT_COMMIT)
    {
        chan = callbackData;
        GetCtrlVal (panel, control, &scatter);
        channel_UpdateViewGraph (chan, scatter);
    }
    return 0;
}

int  ChannelViewGraphCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int doLog, doGrid;
    if (event == EVENT_COMMIT)
    {
        switch (control)
        {
            case CHANVIEW_LOG:
                GetCtrlVal (panel, control, &doLog);
                if (doLog)
                    SetCtrlAttribute (panel, CHANVIEW_GRAPH, ATTR_YMAP_MODE, VAL_LOG);
                else
                    SetCtrlAttribute (panel, CHANVIEW_GRAPH, ATTR_YMAP_MODE, VAL_LINEAR);
                break;
            case CHANVIEW_GRID:
                GetCtrlVal (panel, control, &doGrid);
                SetCtrlAttribute (panel, CHANVIEW_GRAPH, ATTR_YGRID_VISIBLE, doGrid);
                break;
        }

    }
    return 0;
}

int  ChannelViewSelectCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int i;
    channelPtr chan;
    if (event == EVENT_COMMIT)
    {
        GetCtrlVal (panel, control, &i);
        chan = channellist_GetItem (i);
        channel_UpdateViewPanel (chan);
        channel_UpdateEditPanel (chan);
    }
    return 0;
}

int  ChannelViewWindowCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int min_i, max_i, y_scaling;
    double min, max;
    channelPtr chan;

    if (event == EVENT_VAL_CHANGED)
    {
        chan = callbackData;
        switch (control)
        {
            case CHANVIEW_OFFSET:
                GetCtrlVal (panel, control, &chanview.offset);
                SetCtrlVal (panel, CHANVIEW_OFFSETVAL, chanview.offset);
                break;
            case CHANVIEW_POINTS_2:
                GetCtrlVal (panel, control, &chanview.pts);
                if ((chanview.offset + chanview.pts) > chan->pts)
                {
                    chanview.offset = chan->pts - chanview.pts;
                    SetCtrlVal (panel, CHANVIEW_OFFSET, chanview.offset);
                    SetCtrlVal (panel, CHANVIEW_OFFSETVAL, chanview.offset);
                }
                SetInputMode (panel, CHANVIEW_OFFSET, !(chan->pts == chanview.pts));
                SetInputMode (panel, CHANVIEW_OFFSETVAL, !(chan->pts == chanview.pts));
                SetCtrlAttribute (panel, CHANVIEW_OFFSET,
                      ATTR_MAX_VALUE, chan->pts - chanview.pts);
                break;
        }

        SetAxisRange (panel, CHANVIEW_GRAPH, VAL_MANUAL, chanview.offset,
                  chanview.offset + chanview.pts - 1, VAL_NO_CHANGE, 0.0, 1.0);
        MaxMin1D (chan->readings + chanview.offset, chanview.pts, &max, &max_i, &min,
              &min_i);

        if (min >= max) y_scaling = VAL_AUTO; else y_scaling = VAL_MANUAL;
        SetAxisRange (panel, CHANVIEW_GRAPH, VAL_NO_CHANGE,
                  0.0, 1.0, y_scaling, min, max);
    }
    return 0;
}

void channel_InitEditPanel (channelPtr chan, int editlabel)
{
    int width1, width2;

    chanview.p2 = LoadPanel (chanview.p1, "channelu.uir", CHANEDIT);
    

    GetPanelAttribute (chanview.p1, ATTR_WIDTH, &width1);
    SetPanelPos (chanview.p2, 6, width1);

    GetPanelAttribute (chanview.p2, ATTR_WIDTH, &width2);
    SetPanelAttribute (chanview.p1, ATTR_WIDTH, width1 + width2 + 6);

    if (editlabel) editlabel = VAL_HOT; else editlabel = VAL_INDICATOR;
    SetCtrlAttribute (chanview.p2, CHANEDIT_LABEL, ATTR_CTRL_MODE, editlabel);
    channel_UpdateEditPanel (chan);

    DisplayPanel (chanview.p2);
    InstallPopup (chanview.p1);
}

void channel_UpdateEditPanel (channelPtr chan)
{
    SetCtrlVal(chanview.p2, CHANEDIT_LABEL, chan->label);
    SetCtrlAttribute (chanview.p2, CHANEDIT_LABEL, ATTR_CALLBACK_DATA, chan);

    ResetTextBox (chanview.p2, CHANEDIT_NOTE, chan->note);
    SetCtrlAttribute (chanview.p2, CHANEDIT_NOTE, ATTR_CALLBACK_DATA, &chan->note);
}

int  ChannelLabelCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    channelPtr chan;
    int c;
    char label[260];

    if (event == EVENT_VAL_CHANGED)
    {
        chan = callbackData;
        GetCtrlVal (panel, control, chan->label);
        c = list_FindItem (channelG.channels, chan);
        ReplaceListItem (channelG.p.channels, CHANNELS_LIST, c, chan->label,
                         c);
        ReplaceListItem (chanview.p1, CHANVIEW_CHANNELS, c, chan->label,
                         c);
    }
    return 0;
}

/* LIST THINGS */

channelPtr channellist_GetItem (int i)
{
    nodePtr node;
    node = list_GetNode (channelG.channels, i);
    return node->item;
}

channelPtr channellist_GetSelection (void)
{
    int i;
    nodePtr node;
    GetCtrlIndex (channelG.p.channels, CHANNELS_LIST, &i);
    return channellist_GetItem (i);
}

void channellist_Copy (int panel, int control)
{
    int i;
    channelPtr chan;
    ClearListCtrl (panel, control);
    for (i = 0; i < channelG.channels.nItems; i++) {
        chan = channellist_GetItem (i);
        InsertListItem (panel, control, i, chan->label, i);
    }
    SetInputMode (panel, control, i);
}

void channellist_UpdatePanel (void)
{
    int index, menubar;
    channelPtr chan;

    SetInputMode (channelG.p.channels, CHANNELS_INDEX, channelG.channels.nItems);
    SetInputMode (channelG.p.channels, CHANNELS_TOTAL, channelG.channels.nItems);
    SetInputMode (channelG.p.channels, CHANNELS_LOCATION, channelG.channels.nItems);
    SetInputMode (channelG.p.channels, CHANNELS_SAVE, channelG.channels.nItems);
    SetInputMode (channelG.p.channels, CHANNELS_REMOVE,
        channelG.channels.nItems && (utilG.acq.status != ACQ_BUSY));
    SetInputMode (channelG.p.channels, CHANNELS_LIST, channelG.channels.nItems);

    menubar = GetPanelMenuBar (channelG.p.channels);
    SetMenuBarAttribute (menubar, CHANMENUS_FUNC, ATTR_DIMMED,
                         !channelG.channels.nItems);
    SetMenuBarAttribute (menubar, CHANMENUS_OP, ATTR_DIMMED,
                         (channelG.channels.nItems < 2));
    GetCtrlIndex (channelG.p.channels, CHANNELS_LIST, &index);
    SetCtrlVal (channelG.p.channels, CHANNELS_INDEX, index+1);
    SetCtrlAttribute (channelG.p.channels, CHANNELS_INDEX, ATTR_MAX_VALUE,
                      channelG.channels.nItems);
    SetCtrlVal (channelG.p.channels, CHANNELS_TOTAL, channelG.channels.nItems);

    if (channelG.channels.nItems)
    {
        chan = channellist_GetSelection();
        channel_FillLocation (chan);
    }
}

void channellist_AddChannel (channelPtr chan)
{
    if (chan && list_AddItem (&channelG.channels, chan))
    {
        InsertListItem (channelG.p.channels, CHANNELS_LIST,
                        channelG.channels.nItems-1, chan->label,
                        channelG.channels.nItems-1);
        channellist_UpdatePanel();
    }
}

void channellist_RemoveChannel (channelPtr chan)
{
    int i;

    i = list_FindItem (channelG.channels, chan);
    if ((i != -1) && (!chan->curves.nItems))
    {
        if (chan->readings) free (chan->readings);
        list_RemoveAllItems (&chan->curves, FALSE);
        list_RemoveAllItems (&chan->graphs, FALSE);
        list_RemoveItem (&channelG.channels, i, TRUE);
        DeleteListItem (channelG.p.channels, CHANNELS_LIST, i, 1);
    }
}

int  ChannelSelectionCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int index;
    channelPtr chan;
    if (event == EVENT_VAL_CHANGED)
    {
        chan = channellist_GetSelection();
        GetCtrlIndex (channelG.p.channels, CHANNELS_LIST, &index);
        SetCtrlVal (channelG.p.channels, CHANNELS_INDEX, index+1);
        channel_FillLocation (chan);
    }

    if (event == EVENT_LEFT_DOUBLE_CLICK)
    {
        chan = channellist_GetSelection();
/*      channel_FillLocation(chan);*/
        channel_InitViewPanel ();
        channel_UpdateViewPanel (chan);
        channel_InitEditPanel (chan, TRUE);
    }
    return 0;
}

int  ChannelIndexCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int i, cnt;
    channelPtr chan;
    if (event == EVENT_VAL_CHANGED)
    {
        GetCtrlVal (panel, control, &i);
        GetCtrlVal (panel, CHANNELS_TOTAL, &cnt);
        if (i > cnt) i = cnt;
        if (i < 1) i = 1;
        SetCtrlIndex (panel, CHANNELS_LIST, i-1);
        SetCtrlVal (panel, control, i);
        chan = channellist_GetSelection();
        channel_FillLocation(chan);
        return 1;
    }
    return 0;
}

int  SaveChannelsCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    if (event == EVENT_COMMIT)
    {
        list_InitPanel("Save DAAS Channels", "Save");
        channellist_Copy (listG.p, LIST_ITEMS);
        InstallCtrlCallback (listG.p, LIST_CONT, SaveSelectedChannelsCallback, 0);
        InstallPopup (listG.p);
    }
    return 0;
}

int  SaveSelectedChannelsCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int n, n1, i, checked, filestatus;
    channelPtr chan;
    char path[260];

    if (event == EVENT_COMMIT)
    {
        GetNumCheckedItems (panel, LIST_ITEMS, &n);
        if (n == 0)
        {
            MessagePopup ("Save Channels Message", "No channels selected to save");
            
            DiscardPanel (listG.p);
            return 0;
        }

        filestatus = FileSelectPopup ("", "*.chn", "*.chn", "Save Channel Data",
                                      VAL_SAVE_BUTTON, 0, 0, 1, 1, path);

        if ((filestatus == VAL_NEW_FILE_SELECTED) ||
            (filestatus == VAL_EXISTING_FILE_SELECTED)) {
            fileHandle.analysis = util_OpenFile(path, FILE_WRITE, FALSE);
            FmtFile (fileHandle.analysis, "#CHANNELDATA\n");
            FmtFile (fileHandle.analysis, "Channels %i\n", n);

            GetNumListItems (listG.p, LIST_ITEMS, &n);
            n1 = n;
            for (i = 0; i < n; i++)
            {
                IsListItemChecked (panel, LIST_ITEMS, 0, &checked);
                DeleteListItem (panel, LIST_ITEMS, 0, 1);
                if (checked)
                {
                    channel_Save (channellist_GetItem(i));
                    GetNumCheckedItems (panel, LIST_ITEMS, &n1);
                    if (n1 > 0)
                        FmtFile (fileHandle.analysis, FILE_ITEM_SEP);
                }
                if (n1 == 0) break;
            }
            FmtFile (fileHandle.analysis, "#ENDCHANNELDATA\n");
            util_CloseFile();
            
            DiscardPanel (listG.p);
        }
    }
    return 0;
}

int  LoadChannelsCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int filestatus, i, n;
    char path[260], info[260];
    channelPtr chan;

    char label[260];

    if (event == EVENT_COMMIT)
    {
        filestatus = FileSelectPopup ("", "*.chn", "*.chn",
                                      "Load Channel Data", VAL_LOAD_BUTTON, 0,
                                      1, 1, 0, path);
        if (filestatus == VAL_EXISTING_FILE_SELECTED)
        {
            fileHandle.analysis = util_OpenFile (path, FILE_READ, FALSE);
            ScanFile (fileHandle.analysis, "%s", info);
            if (CompareBytes (info, 0, "#CHANNELDATA", 0, StringLength ("#CHANNELDATA"), 1))
            {
                util_CloseFile();
                MessagePopup ("Load Channel Error", "Wrong file type");
                return 0;
            }

            ScanFile (fileHandle.analysis, "%s>Channels %i%s[w1d]", &n);
            for (i = 0; i < n; i++)
            {
                chan = channel_Load ();
                if (chan)
                {
                    ReadLine (fileHandle.analysis, info, 255);
                    channellist_AddChannel (chan);
                }
                else break;
            }
            util_CloseFile();
            channellist_UpdatePanel();
        }
    }
    return 0;
}

void LoadDataFileCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    int filestatus, i, n, numfmtItems, nChannels, strt_chan, pts = 0;
    char path[260], info[260], labels[260], datastr[260];
    channelPtr chan;
    int labels_length, label_length;
    double *dataline;
    static long posfromStart;

    filestatus = FileSelectPopup ("", "*.*", "*.*", "Load Data File",
                                  VAL_LOAD_BUTTON, 0, 0, 1, 0, path);
    if (filestatus == VAL_EXISTING_FILE_SELECTED) {
        fileHandle.analysis = util_OpenFile (path, FILE_READ, FALSE);
        ScanFile (fileHandle.analysis, "%s", info);
        if (CompareBytes (info, 0, "#ACQDATA", 0, StringLength ("#ACQDATA"), 1))
            MessagePopup ("Load Data File Error", "Wrong file type");
        else {
            numfmtItems = ScanFile (fileHandle.analysis, "%s", labels);
            labels_length = StringLength (labels);

            chan = channel_Create ();
            if (chan) {
                numfmtItems = Scan (labels, "%s[xt44]", chan->label);
                label_length = StringLength (chan->label);
                channellist_AddChannel (chan);
                strt_chan = channelG.channels.nItems-1;

                CopyString (labels, 0, labels, label_length, labels_length-label_length);
                labels_length = StringLength (labels);
                nChannels = 1;

                while (labels_length > 0) {
                    chan = channel_Create ();
                    numfmtItems = Scan (labels, "%s[xdt44]%s[xt44]", chan->label);
                    label_length = StringLength (chan->label);
                    CopyString (labels, 0, labels, label_length+1, labels_length-(label_length+1));
                    labels_length = StringLength (labels);
                    channellist_AddChannel (chan);
                    nChannels++;
                }

                dataline = calloc (nChannels, sizeof(double));
                while (ScanFile (fileHandle.analysis, "%s", datastr)) {
                    numfmtItems = Scan (datastr, "%s>%*f[x]", nChannels, dataline);
                    if (numfmtItems == 1) pts++;
                }

                for (i = 0; i < nChannels; i++) {
                    chan = channellist_GetItem (strt_chan+i);
                    if (!channel_AllocMem (chan, pts)) free (chan);
                }

                posfromStart = SetFilePtr (fileHandle.analysis, 0, 0);
                ScanFile (fileHandle.analysis, "%s", info);
                ScanFile (fileHandle.analysis, "%s", labels);

                pts = 0;
                while (ScanFile (fileHandle.analysis, "%s", datastr)) {
                    numfmtItems = Scan (datastr, "%s>%*f[x]", nChannels, dataline);
                    if (numfmtItems == 1) {
                        for (i = 0; i < nChannels; i++) {
                            chan = channellist_GetItem (strt_chan+i);
                            chan->readings[pts] = dataline[i];
                        }
                    pts++;
                    }
                }
                free (dataline);
            }
        }

        util_CloseFile();
        channellist_UpdatePanel();
    }
}

int  RemoveChannelsCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    if (event == EVENT_COMMIT)
    {
        list_InitPanel("Remove DAAS Channels", "Remove");
        channellist_Copy (listG.p, LIST_ITEMS);
        InstallCtrlCallback (listG.p, LIST_CONT, RemoveSelectedChannelsCallback, 0);
        InstallPopup (listG.p);
    }
    return 0;
}

int  RemoveSelectedChannelsCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int n, checked, nChecked, i;
    channelPtr chan;
    nodePtr node;

    if ((event == EVENT_COMMIT) &&
        ConfirmPopup ("Channel Remove Message",
                      "This function cannot be undone.\n"
                      "Do you want to proceed?"))
    {
        GetNumCheckedItems (panel, LIST_ITEMS, &nChecked);
        if (!nChecked)
        {
            MessagePopup ("Remove Channels Message", "No channels selected to remove");
            
            DiscardPanel (listG.p);
            return 0;
        }

        node = channelG.channels.first;
        GetNumListItems (listG.p, LIST_ITEMS, &n);
        for (i = 0; i < n; i++) {
            IsListItemChecked (listG.p, LIST_ITEMS, 0, &checked);
            DeleteListItem (listG.p, LIST_ITEMS, 0, 1);
            chan = node->item; node = node->next;
            if (checked && !chan->curves.nItems) channellist_RemoveChannel (chan);
            GetNumCheckedItems (listG.p, LIST_ITEMS, &nChecked);
            if (!nChecked) break;
        }
        channellist_UpdatePanel();
        
        DiscardPanel (listG.p);
    }
    return 0;
}

void channelG_Init (void)
{
    util_ChangeInitMessage ("Channel Utilities...");

    channelG.p.channels = LoadPanel (utilG.p, "channelu.uir", CHANNELS);

    

    SetPanelPos (channelG.p.channels, VAL_AUTO_CENTER, VAL_AUTO_CENTER);

    list_Init (&channelG.channels);

    InstallCtrlCallback (utilG.p, BG_CHANNELS,
                         ShowChannelsCallback, 0);
}

void channelG_Exit (void)
{
    int i;
    channelPtr chan;
    if (channelG.channels.nItems) {
        for (i = 0; i < channelG.channels.nItems; i++) {
            chan = channellist_GetItem (i);
            list_RemoveAllItems (&chan->curves, FALSE);
            list_RemoveAllItems (&chan->graphs, FALSE);
            if (chan->readings) free (chan->readings);
        }
        list_RemoveAllItems (&channelG.channels, TRUE);
    }
    
    DiscardPanel (channelG.p.channels);

    utilG_Exit();
}

int  ShowChannelsCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    if (event == EVENT_COMMIT)
    {
        channellist_UpdatePanel();
        DisplayPanel (channelG.p.channels);
    }
    return 0;
}

