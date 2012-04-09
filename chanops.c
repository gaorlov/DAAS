#include <analysis.h>
#include <formatio.h>
#include <ansi_c.h>
#include <userint.h>
#include "util.h"
#include "list.h"
#include "channel.h"
#include "channelu.h"
#include "changen.h"
#include "chanfnc.h"
#include "chanops.h"
#include "chanopsu.h"

#define TRUE 1
#define FALSE 0
 int NoErr;
static struct chanopsStruct
{
    int p;
}   chanops;

void chanops_Init (void);

void InitAddChannelsCallback(int menubar, int menuItem, void *callbackData, int panel);
int DoChannelAdditionCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void InitMultiplyChannelsCallback(int menubar, int menuItem, void *callbackData, int panel);
int DoChannelMultiplicationCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void InitSubtractChannelsCallback(int menubar, int menuItem, void *callbackData, int panel);
int DoChannelSubtractionCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void InitDivideChannelsCallback(int menubar, int menuItem, void *callbackData, int panel);
int DoChannelDivisionCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);

/*
main()
{
    utilG_Init();
    listG_Init();
    fileG_Init();
    channelG_Init();

    changen_Init();
    chanfunc_Init();

    chanops_Init();

    RunUserInterface();
}
*/

void chanops_Init (void)
{
    int menubar;

    util_ChangeInitMessage ("Channel Operations Menu...");

    menubar = GetPanelMenuBar (channelG.p.channels);

    InstallMenuCallback (menubar, CHANMENUS_OP_ADD, InitAddChannelsCallback, 0);
    InstallMenuCallback (menubar, CHANMENUS_OP_MULTIPLY, InitMultiplyChannelsCallback, 0);
    InstallMenuCallback (menubar, CHANMENUS_OP_SUBTRACT, InitSubtractChannelsCallback, 0);
    InstallMenuCallback (menubar, CHANMENUS_OP_DIVISION, InitDivideChannelsCallback, 0);
}

void InitDivideChannelsCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    int i;
    channelPtr chan;

    chanops.p = LoadPanel (0, "chanopsu.uir", CHANOPS_2);
    
    util_InitClose (chanops.p, CHANOPS_2_CANCEL, TRUE);
    SetPanelPos (chanops.p, 100, 100);

    InstallCtrlCallback (chanops.p, CHANOPS_2_GO,
                         DoChannelDivisionCallback, 0);
    SetPanelAttribute (chanops.p, ATTR_TITLE, "Channel Division");
    channellist_Copy (chanops.p, CHANOPS_2_CHAN_1);
    channellist_Copy (chanops.p, CHANOPS_2_CHAN_2);

    InstallPopup (chanops.p);
}

int DoChannelDivisionCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    channelPtr chan1, chan2, newchan;
    int i, err, pts;

    if (event == EVENT_COMMIT)
    {
        GetCtrlVal (panel, CHANOPS_2_CHAN_1, &i);
        chan1 = channellist_GetItem (i);
        GetCtrlVal (panel, CHANOPS_2_CHAN_2, &i);
        chan2 = channellist_GetItem (i);
        if (chan1->pts < chan2->pts) pts = chan1->pts;
        else pts = chan2->pts;

        newchan = channel_Create();
        if (newchan && channel_AllocMem (newchan, pts) &&
            (Div1D (chan1->readings, chan2->readings, pts, newchan->readings) == NoErr))
        {
            Fmt (newchan->label, "Division Result");
            Fmt (newchan->note, "%s /\n%s\n", chan1->label, chan2->label);
            channellist_AddChannel (newchan);
        }
        else
        {
            MessagePopup ("Channel Division Message",
                          "Error during Division calculation");
            if (newchan)
            {
                if (newchan->readings) free (newchan->readings);
                free (newchan);
            }
        }

        
        DiscardPanel (chanops.p);
    }
    return 0;
}

void InitSubtractChannelsCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    int i;
    channelPtr chan;

    chanops.p = LoadPanel (0, "chanopsu.uir", CHANOPS_2);
    
    util_InitClose (chanops.p, CHANOPS_2_CANCEL, TRUE);
    SetPanelPos (chanops.p, 100, 100);

    InstallCtrlCallback (chanops.p, CHANOPS_2_GO,
                         DoChannelSubtractionCallback, 0);
    SetPanelAttribute (chanops.p, ATTR_TITLE, "Channel Subtraction");
    channellist_Copy (chanops.p, CHANOPS_2_CHAN_1);
    channellist_Copy (chanops.p, CHANOPS_2_CHAN_2);

    InstallPopup (chanops.p);
}

int DoChannelSubtractionCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    channelPtr chan1, chan2, newchan;
    int i, err, pts;

    if (event == EVENT_COMMIT)
    {
        GetCtrlVal (panel, CHANOPS_2_CHAN_1, &i);
        chan1 = channellist_GetItem (i);
        GetCtrlVal (panel, CHANOPS_2_CHAN_2, &i);
        chan2 = channellist_GetItem (i);
        if (chan1->pts < chan2->pts) pts = chan1->pts;
        else pts = chan2->pts;

        newchan = channel_Create();
        if (newchan && channel_AllocMem (newchan, pts) &&
            (Sub1D (chan1->readings, chan2->readings, pts, newchan->readings) == NoErr))
        {
            Fmt (newchan->label, "Subtraction Result");
            Fmt (newchan->note, "%s -\n%s\n", chan1->label, chan2->label);
            channellist_AddChannel (newchan);
        }
        else
        {
            MessagePopup ("Channel Subtraction Message",
                          "Error during substraction calculation");
            if (newchan)
            {
                if (newchan->readings) free (newchan->readings);
                free (newchan);
            }
        }

        
        DiscardPanel (chanops.p);
    }
    return 0;
}

void InitMultiplyChannelsCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    int i;
    channelPtr chan;

    chanops.p = LoadPanel (0, "chanopsu.uir", CHANOPS_1);
    
    util_InitClose (chanops.p, CHANOPS_1_CANCEL, TRUE);
    SetPanelPos (chanops.p, 100, 100);

    InstallCtrlCallback (chanops.p, CHANOPS_1_GO,
                         DoChannelMultiplicationCallback, 0);
    SetPanelAttribute (chanops.p, ATTR_TITLE, "Channel Multiplication");
    channellist_Copy (chanops.p, CHANOPS_1_CHANNELS);

    InstallPopup (chanops.p);
}

int DoChannelMultiplicationCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    channelPtr chan, newchan;
    int i, n, checked, err, pts;

    if (event == EVENT_COMMIT)
    {
        GetNumCheckedItems (chanops.p, CHANOPS_1_CHANNELS, &n);
        if (n < 2)
        {
            MessagePopup ("Channel Multiplication Message",
                          "Cannot perform multiplication w/ less than 2 channels");
            return 0;
        }

        err = FALSE;
        newchan = NULL;
        for (i = 0; i < channelG.channels.nItems; i++)
        {
            IsListItemChecked (panel, CHANOPS_1_CHANNELS, 0, &checked);
            DeleteListItem (panel, CHANOPS_1_CHANNELS, 0, 1);
            if (checked)
            {
                chan = channellist_GetItem (i);
                if (newchan)
                {
                    if (newchan->pts < chan->pts) pts = newchan->pts;
                    else pts = chan->pts;
                    err = Mul1D (newchan->readings, chan->readings, pts,
                                 newchan->readings);
                    if (!err)
                        Fmt (newchan->note, "%s[a]<*\n%s", chan->label);
                    else
                    {
                        MessagePopup ("Channel Multiplication Message",
                                      "Error during calculation -- operation cancelled");
                        break;
                    }
                }
                else
                {
                    newchan = channel_Create();
                    if (newchan && channel_AllocMem (newchan, chan->pts))
                    {
                        Copy1D (chan->readings, chan->pts, newchan->readings);
                        Fmt (newchan->label, "Multiplication Result");
                        Fmt (newchan->note, "%s", chan->label);
                    }
                    else
                    {
                        util_OutofMemory ("Channel Multiplication Message");
                        err = TRUE;
                        break;
                    }
                }
            }
            GetNumCheckedItems (panel, CHANOPS_1_CHANNELS, &n);
            if (n == 0) break;
        }
        if (!err)
        {
            Fmt (newchan->note, "%s[a]<\n");
            channellist_AddChannel (newchan);
        }
        else if (newchan)
        {
            if (newchan->readings) free (newchan->readings);
            free (newchan);
        }

        
        DiscardPanel (chanops.p);
    }
    return 0;
}

void InitAddChannelsCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    int i;
    channelPtr chan;

    chanops.p = LoadPanel (0, "chanopsu.uir", CHANOPS_1);
    
    util_InitClose (chanops.p, CHANOPS_1_CANCEL, TRUE);
    SetPanelPos (chanops.p, 100, 100);

    InstallCtrlCallback (chanops.p, CHANOPS_1_GO,
                         DoChannelAdditionCallback, 0);
    SetPanelAttribute (chanops.p, ATTR_TITLE, "Channel Addition");
    channellist_Copy (chanops.p, CHANOPS_1_CHANNELS);

    InstallPopup (chanops.p);
}

int DoChannelAdditionCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    channelPtr chan, newchan;
    int i, n, checked, err, pts;

    if (event == EVENT_COMMIT)
    {
        GetNumCheckedItems (chanops.p, CHANOPS_1_CHANNELS, &n);
        if (n < 2)
        {
            MessagePopup ("Channel Addition Message",
                          "Cannot perform addition w/ less than 2 channels");
            return 0;
        }

        err = FALSE;
        newchan = NULL;
        for (i = 0; i < channelG.channels.nItems; i++)
        {
            IsListItemChecked (panel, CHANOPS_1_CHANNELS, 0, &checked);
            DeleteListItem (panel, CHANOPS_1_CHANNELS, 0, 1);
            if (checked)
            {
                chan = channellist_GetItem (i);
                if (newchan)
                {
                    if (newchan->pts < chan->pts) pts = newchan->pts;
                    else pts = chan->pts;
                    err = Add1D (newchan->readings, chan->readings, pts,
                                 newchan->readings);
                    if (!err)
                        Fmt (newchan->note, "%s[a]<+\n%s", chan->label);
                    else
                    {
                        MessagePopup ("Channel Addition Message",
                                      "Error during calculation -- operation cancelled");
                        break;
                    }
                }
                else
                {
                    newchan = channel_Create();
                    if (newchan && channel_AllocMem (newchan, chan->pts))
                    {
                        Copy1D (chan->readings, chan->pts, newchan->readings);
                        Fmt (newchan->label, "Addition Result");
                        Fmt (newchan->note, "%s", chan->label);
                    }
                    else
                    {
                        util_OutofMemory ("Channel Addition Message");
                        err = TRUE;
                        break;
                    }
                }
            }
            GetNumCheckedItems (panel, CHANOPS_1_CHANNELS, &n);
            if (n == 0) break;
        }
        if (!err)
        {
            Fmt (newchan->note, "%s[a]<\n");
            channellist_AddChannel (newchan);
        }
        else if (newchan)
        {
            if (newchan->readings) free (newchan->readings);
            free (newchan);
        }

        
        DiscardPanel (chanops.p);
    }
    return 0;
}

