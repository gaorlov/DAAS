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
#include "chanfncu.h"

#define TRUE 1
#define FALSE 0
 int NoErr;
static struct chanfuncStruct
{
    int p, NoErr;
    char note[256];
}   chanfunc;

void chanfunc_Init (void);

static void chanfunc_AddtoList (char *label, channelPtr chan);
static void chanfunc_CalcStatistics (channelPtr chan);

void InitLinEvalCallback(int menubar, int menuItem, void *callbackData, int panel);
void InitStatisticsCallback(int menubar, int menuItem, void *callbackData, int panel);
void NegativeCallback(int menubar, int menuItem, void *callbackData, int panel);
void InverseCallback(int menubar, int menuItem, void *callbackData, int panel);
void InitExponentCallback(int menubar, int menuItem, void *callbackData, int panel);
void AbsoluteCallback(int menubar, int menuItem, void *callbackData, int panel);
void LnCallback(int menubar, int menuItem, void *callbackData, int panel);
void LogCallback(int menubar, int menuItem, void *callbackData, int panel);
void NormalScaleCallback(int menubar, int menuItem, void *callbackData, int panel);
void QuickScaleCallback(int menubar, int menuItem, void *callbackData, int panel);
void InitManipulateCallback(int menubar, int menuItem, void *callbackData, int panel);
int  ManipulateGraphCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  ManipulateSelectionCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  ManipulateReadingCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CopyChannelCallback(int menubar, int menuItem, void *callbackData, int panel);
void InitSubsetCallback(int menubar, int menuItem, void *callbackData, int panel);
int  SubsetDoneCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void SortChannelCallback(int menubar, int menuItem, void *callbackData, int panel);
void ReverseChannelCallback(int menubar, int menuItem, void *callbackData, int panel);
void InitClipCallback(int menubar, int menuItem, void *callbackData, int panel);
void InitDecimateCallback(int menubar, int menuItem, void *callbackData, int panel);

/*
main()
{
    utilG_Init();
    listG_Init();
    fileG_Init();
    channelG_Init();
    changen_Init();

    chanfunc_Init();

    RunUserInterface();
}
*/

void chanfunc_Init (void)
{
    int menubar;

    util_ChangeInitMessage ("Channel Functions Menu...");

    menubar = GetPanelMenuBar (channelG.p.channels);

    InstallMenuCallback (menubar, CHANMENUS_FUNC_LINEVAL, InitLinEvalCallback, 0);
    InstallMenuCallback (menubar, CHANMENUS_FUNC_STATISTICS, InitStatisticsCallback, 0);
    InstallMenuCallback (menubar, CHANMENUS_FUNC_MATH_NEG, NegativeCallback, 0);
    InstallMenuCallback (menubar, CHANMENUS_FUNC_MATH_INVERSE, InverseCallback, 0);
    InstallMenuCallback (menubar, CHANMENUS_FUNC_MATH_EXPONENT, InitExponentCallback, 0);
    InstallMenuCallback (menubar, CHANMENUS_FUNC_MATH_ABSOLUTE, AbsoluteCallback, 0);
    InstallMenuCallback (menubar, CHANMENUS_FUNC_MATH_LN, LnCallback, 0);
    InstallMenuCallback (menubar, CHANMENUS_FUNC_MATH_LOG, LogCallback, 0);
    InstallMenuCallback (menubar, CHANMENUS_FUNC_SCALE_NORMAL, NormalScaleCallback, 0);
    InstallMenuCallback (menubar, CHANMENUS_FUNC_SCALE_QUICK, QuickScaleCallback, 0);
    InstallMenuCallback (menubar, CHANMENUS_FUNC_MANIP, InitManipulateCallback, 0);
    InstallMenuCallback (menubar, CHANMENUS_FUNC_MISC_COPY, CopyChannelCallback, 0);
    InstallMenuCallback (menubar, CHANMENUS_FUNC_MISC_SUBSET, InitSubsetCallback, 0);
    InstallMenuCallback (menubar, CHANMENUS_FUNC_MISC_SORT, SortChannelCallback, 0);
    InstallMenuCallback (menubar, CHANMENUS_FUNC_MISC_REVERSE, ReverseChannelCallback, 0);
    InstallMenuCallback (menubar, CHANMENUS_FUNC_MISC_CLIP, InitClipCallback, 0);
    InstallMenuCallback (menubar, CHANMENUS_FUNC_MISC_DECIMATE, InitDecimateCallback, 0);
}

void InitDecimateCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    int i;
    channelPtr chan;

    chanfunc.p = LoadPanel (0, "chanfncu.uir", DECIMATE);
    
    util_InitClose (chanfunc.p, DECIMATE_CLOSE, FALSE);
    SetPanelPos (chanfunc.p, 100, 100);

    channellist_Copy (chanfunc.p, DECIMATE_CHANNELS);

    InstallPopup (chanfunc.p);
}

int  DoDecimateCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int i, dfact, ave;
    channelPtr chan, newchan;

    if (event == EVENT_COMMIT)
    {
        GetCtrlVal (panel, DECIMATE_CHANNELS, &i);
        GetCtrlVal (panel, DECIMATE_DFACT, &dfact);
        GetCtrlVal (panel, DECIMATE_AVE, &ave);

        chan = channellist_GetItem (i);

        newchan = channel_Create();
        if (newchan && channel_AllocMem (newchan, (int)(chan->pts/dfact)) &&
            (Decimate (chan->readings, chan->pts, dfact, ave, newchan->readings) == NoErr))
        {
            Fmt (newchan->label, "%s (decimated)", chan->label);
            Fmt (newchan->note, "%s\n%s\ndfact = %i\nave = %i\n",
                 chan->note, newchan->label, dfact, ave);
            channellist_AddChannel (newchan);
            return 1;
        }

        MessagePopup ("Decimate Channel Message", "Error decimating channel--function voided");
        if (newchan)
        {
            if (newchan->readings) free (newchan->readings);
            free (newchan);
        }
    }
    return 0;
}

void InitClipCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    int i;
    channelPtr chan;

    chanfunc.p = LoadPanel (0, "chanfncu.uir", CLIP);
    
    util_InitClose (chanfunc.p, CLIP_CLOSE, FALSE);
    SetPanelPos (chanfunc.p, 100, 100);

    channellist_Copy (chanfunc.p, CLIP_CHANNELS);

    InstallPopup (chanfunc.p);
}

int  DoClipCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int i;
    double min, max;
    channelPtr chan, newchan;

    if (event == EVENT_COMMIT)
    {
        GetCtrlVal (panel, CLIP_CHANNELS, &i);
        GetCtrlVal (panel, CLIP_MIN, &min);
        GetCtrlVal (panel, CLIP_MAX, &max);

        chan = channellist_GetItem (i);

        newchan = channel_Create();
        if (newchan && channel_AllocMem (newchan, chan->pts) &&
            (Clip (chan->readings, chan->pts, max, min, newchan->readings) == NoErr))
        {
            Fmt (newchan->label, "%s (clipped)", chan->label);
            Fmt (newchan->note, "%s\n%s\nmin = %f[e2p3]\nmax = %f[e2p3]\n",
                 chan->note, newchan->label, min, max);
            channellist_AddChannel (newchan);
            return 1;
        }

        MessagePopup ("Clip Channel Message", "Error clipping channel--function voided");
        if (newchan)
        {
            if (newchan->readings) free (newchan->readings);
            free (newchan);
        }
    }
    return 0;
}

void ReverseChannelCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    channelPtr chan, newchan;

    chan = channellist_GetSelection();

    newchan = channel_Create();
    if (newchan && channel_AllocMem (newchan, chan->pts))
    {
        Fmt (newchan->label, "%s (reversed)", chan->label);
        Fmt (newchan->note, "%s\n%s\n", chan->note, newchan->label);
        Reverse (chan->readings, chan->pts, newchan->readings);
        channellist_AddChannel (newchan);
    }
}

void SortChannelCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    channelPtr chan, newchan;
    int button;

    chan = channellist_GetSelection ();

    button = GenericMessagePopup ("Sort Channel Readings",
                                  "Select direction to sort channel readings",
                                  "Ascending", "Descending", "Cancel", 0,
                                  0, 0, VAL_GENERIC_POPUP_BTN1,
                                  VAL_GENERIC_POPUP_BTN1, VAL_GENERIC_POPUP_BTN3);

    if (button == VAL_GENERIC_POPUP_BTN3) return;

    newchan = channel_Create();
    if (newchan && channel_AllocMem(newchan, chan->pts))
    {
        Fmt (newchan->label, "%s<%s (sorted)", chan->label);
        Fmt (newchan->note, "%s\n%s\n", chan->note, newchan->label);
        switch (button)
        {
            case VAL_GENERIC_POPUP_BTN1:
                Sort (chan->readings, chan->pts, 0, newchan->readings);
                break;
            case VAL_GENERIC_POPUP_BTN2:
                Sort (chan->readings, chan->pts, 1, newchan->readings);
                break;
        }
        channellist_AddChannel(newchan);
        return;
    }

    if (newchan)
    {
        if (newchan->readings) free (newchan->readings);
        free (newchan);
    }
}

void InitSubsetCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    channelPtr chan;
    channel_InitViewPanel();
    chan = channellist_GetSelection();
    channel_UpdateViewPanel(chan);
    SetCtrlAttribute (chanview.p1, CHANVIEW_CHANNELS, ATTR_CTRL_MODE,
                      VAL_INDICATOR);
    InstallCtrlCallback (chanview.p1, CHANVIEW_DONE, SubsetDoneCallback,
                         chan);
    InstallPopup (chanview.p1);
    MessagePopup ("Subset Channel Message",
                  "Place window over desired readings"
                  " and press DONE to create subset");
}

int  SubsetDoneCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    channelPtr chan, newchan;
    int offset, pts;

    if (event == EVENT_COMMIT)
    {
        chan = callbackData;
        GetCtrlVal (panel, CHANVIEW_OFFSET, &offset);
        GetCtrlVal (panel, CHANVIEW_POINTS_2, &pts);
        newchan = channel_Create();
        if (newchan && channel_AllocMem (newchan, pts))
        {
            Subset1D (chan->readings, chan->pts, offset, pts, newchan->readings);
            Fmt (newchan->label, "%s<%s (subset)", chan->label);
            Fmt (newchan->note, "%s\nSubset of: %s\noffset: %i\npts: %i\n",
                             chan->note, chan->label, offset, pts);
            channellist_AddChannel (newchan);
        }
        
        DiscardPanel (chanview.p1);
    }
    return 0;
}

void CopyChannelCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    channelPtr chan, newchan;

    chan = channellist_GetSelection();
    newchan = channel_Create();
    if (newchan && channel_AllocMem (newchan, chan->pts))
    {
        Copy1D (chan->readings, chan->pts, newchan->readings);
        Fmt (newchan->label, "%s<Copy(%s)", chan->label);
        Fmt (newchan->note, "%s<Copy of: %s\n%s\n", chan->label, chan->note);
        channellist_AddChannel (newchan);
    }
}

void InitManipulateCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    int i, height1, height2;
    channelPtr chan;

    channel_InitViewPanel();
    InstallCtrlCallback (chanview.p1, CHANVIEW_CHANNELS,
                         ManipulateSelectionCallback, 0);
    ClearListCtrl (chanview.p1, CHANVIEW_CHANNELS);
    for (i = 0; i < channelG.channels.nItems; i++)
    {
        chan = channellist_GetItem (i);
        if (!chan->curves.nItems)
            InsertListItem (chanview.p1, CHANVIEW_CHANNELS, -1, chan->label, i);
    }

    SetCtrlIndex (chanview.p1, CHANVIEW_CHANNELS, 0);
    GetCtrlVal (chanview.p1, CHANVIEW_CHANNELS, &i);
    chan = channellist_GetItem (i);
    InstallCtrlCallback (chanview.p1, CHANVIEW_GRAPH,
                         ManipulateGraphCallback, chan);

    channel_UpdateViewPanel(chan);

    SetCtrlAttribute (chanview.p1, CHANVIEW_GRAPH, ATTR_CTRL_MODE,
                      VAL_HOT);
    SetCtrlAttribute (chanview.p1, CHANVIEW_GRAPH, ATTR_NUM_CURSORS, 1);
    SetCursorAttribute (chanview.p1, CHANVIEW_GRAPH, 1, ATTR_CURSOR_MODE,
                        VAL_SNAP_TO_POINT);
    SetCursorAttribute (chanview.p1, CHANVIEW_GRAPH, 1,
                        ATTR_CROSS_HAIR_STYLE, VAL_SHORT_CROSS);
    SetCursorAttribute (chanview.p1, CHANVIEW_GRAPH, 1, ATTR_CURSOR_COLOR,
                        VAL_YELLOW);
    GetPanelAttribute (chanview.p1, ATTR_HEIGHT, &height1);

    chanview.p2 = LoadPanel (chanview.p1, "chanfncu.uir", MANIP);
    
    GetPanelAttribute (chanview.p2, ATTR_HEIGHT, &height2);
    SetPanelAttribute (chanview.p1, ATTR_HEIGHT, height1+height2+6);
    SetPanelPos (chanview.p2, height1, 6);

    InstallCtrlCallback (chanview.p2, MANIP_READING,
                         ManipulateReadingCallback, chan);

    DisplayPanel (chanview.p2);
    InstallPopup (chanview.p1);
    MessagePopup ("Manipulate Channel Message", "Only channels not connected to a curve may be edited");
}

int  ManipulateGraphCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int handle, index;
    channelPtr chan;
    double x, y, above, below;

    if (event == EVENT_VAL_CHANGED)
    {
        chan = callbackData;
        GetGraphCursor (chanview.p1, CHANVIEW_GRAPH, 1, &x, &y);
        SetCtrlVal (chanview.p2, MANIP_INDEX, (int)x);
        SetCtrlVal (chanview.p2, MANIP_READING, y);
    }

    if (event == EVENT_COMMIT)
    {
        chan = callbackData;
        GetGraphCursorIndex (chanview.p1, CHANVIEW_GRAPH, 1, &handle, &index);
        GetGraphCursor (chanview.p1, CHANVIEW_GRAPH, 1, &x, &y);
        SetCtrlVal (chanview.p2, MANIP_INDEX, index);
        SetCtrlVal (chanview.p2, MANIP_READING, y);
        if (x == 0) below = 0.0;
        else below = chan->readings[(int)x-1];
        if (x == (chan->pts-1)) above = 0.0;
        else above = chan->readings[(int)x+1];
        if (fabs(above - chan->readings[(int)x]) < fabs(below - chan->readings[(int)x]))
            SetCtrlAttribute (chanview.p2, MANIP_READING, ATTR_INCR_VALUE,
            fabs(above - chan->readings[(int)x])/2);
        else
            SetCtrlAttribute (chanview.p2, MANIP_READING, ATTR_INCR_VALUE,
            fabs(below - chan->readings[(int)x])/2);
    }

    return 0;
}

int  ManipulateSelectionCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int i;
    channelPtr chan;
    if (event == EVENT_COMMIT)
    {
        GetCtrlVal (panel, control, &i);
        chan = channellist_GetItem (i);
        channel_UpdateViewPanel (chan);
        SetCtrlAttribute (chanview.p1, CHANVIEW_GRAPH, ATTR_CALLBACK_DATA, chan);
        SetCtrlAttribute (chanview.p2, MANIP_READING, ATTR_CALLBACK_DATA, chan);
    }
    return 0;
}

int  ManipulateReadingCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    channelPtr chan;
    double reading, min_x, max_x, min_y, max_y;
    int handle, index, scatter, mode, i;

    if (event == EVENT_VAL_CHANGED)
    {
        chan = callbackData;
        GetGraphCursorIndex (chanview.p1, CHANVIEW_GRAPH, 1, &handle, &index);
        GetCtrlVal (panel, control, &reading);
        GetCtrlVal (panel, MANIP_INDEX, &index);
        chan->readings[index] = reading;
        GetCtrlVal (chanview.p1, CHANVIEW_SCATTER, &scatter);
        channel_UpdateViewGraph(chan, scatter);
        GetAxisRange (chanview.p1, CHANVIEW_GRAPH, &mode, &min_x, &max_x,
                      &mode, &min_y, &max_y);
        MaxMin1D (chan->readings + (int)min_x, (int)(max_x - min_x + 1), &max_y,
                  &i, &min_y, &i);
        SetAxisRange (chanview.p1, CHANVIEW_GRAPH, VAL_NO_CHANGE, 0.0, 1.0,
                      VAL_MANUAL, min_y, max_y);
        SetGraphCursor (chanview.p1, CHANVIEW_GRAPH, 1, index, reading);
    }
    return 0;
}

void QuickScaleCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    channelPtr chan, newchan;
    double scale;

    chan = channellist_GetSelection();
    newchan = channel_Create();
    if (newchan && channel_AllocMem (newchan, chan->pts))
    {
        QScale1D (chan->readings, chan->pts, newchan->readings, &scale);
        Fmt (newchan->label, "Quick Scale(%s)", chan->label);
        Fmt (newchan->note, "%s\n%s\nscale factor: %f[e2p3]\n",
             chan->note, newchan->label, scale);
        channellist_AddChannel (newchan);
    }
}

void NormalScaleCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    channelPtr chan, newchan;
    double scale, offset;

    chan = channellist_GetSelection();
    newchan = channel_Create();
    if (newchan && channel_AllocMem (newchan, chan->pts))
    {
        Scale1D (chan->readings, chan->pts, newchan->readings, &offset, &scale);
        Fmt (newchan->label, "Scale(%s)", chan->label);
        Fmt (newchan->note, "%s\n%s\noffset: %f[e2p3]\nscale factor: %f[e2p3]\n",
             chan->note, newchan->label, offset, scale);
        channellist_AddChannel (newchan);
    }
}

void LogCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    int pt, err;
    channelPtr chan, newchan;

    chan = channellist_GetSelection();
    newchan = channel_Create();
    err = FALSE;
    if (newchan && channel_AllocMem (newchan, chan->pts))
    {
        for (pt = 0; pt < chan->pts; pt++)
        {
            if (chan->readings[pt] > 0.0)
                newchan->readings[pt] = log10(chan->readings[pt]);
            else {err = TRUE; break;}
        }
        if (!err)
        {
            Fmt (newchan->label, "log(%s)", chan->label);
            Fmt (newchan->note, "%s\nlog(%s)\n", chan->note, chan->label);
            channellist_AddChannel (newchan);
        }
        else {free (newchan->readings); free (newchan); newchan = NULL;}
    }
    else err = TRUE;
    if (err)
    {
        MessagePopup ("Ln Function Message", "Channel readings include 0.0 or < 0.0 - function voided");
        if (newchan) free (newchan);
    }
}

void LnCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    int pt, err;
    channelPtr chan, newchan;

    chan = channellist_GetSelection();
    newchan = channel_Create();
    err = FALSE;
    if (newchan && channel_AllocMem (newchan, chan->pts))
    {
        for (pt = 0; pt < chan->pts; pt++)
        {
            if (chan->readings[pt] > 0.0)
                newchan->readings[pt] = log(chan->readings[pt]);
            else {err = TRUE; break;}
        }
        if (!err)
        {
            Fmt (newchan->label, "ln(%s)", chan->label);
            Fmt (newchan->note, "%s\nln(%s)\n", chan->note, chan->label);
            channellist_AddChannel (newchan);
        }
        else {free (newchan->readings); free (newchan); newchan = NULL;}
    }
    else err = TRUE;
    if (err)
    {
        MessagePopup ("Ln Function Message", "Channel readings include 0.0 or < 0.0 - function voided");
        if (newchan) free (newchan);
    }
}

void AbsoluteCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    channelPtr chan, newchan;

    chan = channellist_GetSelection();
    newchan = channel_Create();
    if (newchan && channel_AllocMem (newchan, chan->pts))
    {
        Abs1D (chan->readings, chan->pts, newchan->readings);
        Fmt (newchan->label, "abs(%s)", chan->label);
        Fmt (newchan->note, "%s\n%s\n", chan->note, newchan->label);
        channellist_AddChannel (newchan);
    }
}

void InitExponentCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    int i;
    channelPtr chan;

    chanfunc.p = LoadPanel (0, "chanfncu.uir", EXPONENT);
    
    util_InitClose (chanfunc.p, EXPONENT_CLOSE, FALSE);
    SetPanelPos (chanfunc.p, 100, 100);

    channellist_Copy (chanfunc.p, EXPONENT_CHANNELS);

    InstallPopup (chanfunc.p);
}

int  DoExponentCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int i, err, pt;
    double expon;
    channelPtr chan, newchan = NULL;

    if (event == EVENT_COMMIT)
    {
        GetCtrlVal (panel, EXPONENT_CHANNELS, &i);
        chan = channellist_GetItem (i);
        GetCtrlVal (panel, EXPONENT_EXPON, &expon);
        newchan = channel_Create();
        err = FALSE;
        if (newchan && channel_AllocMem (newchan, chan->pts))
        {
            for (pt = 0; pt < chan->pts; pt++) {
                if (chan->readings[pt] != 0.0)
                    newchan->readings[pt] = exp(expon/2*log(chan->readings[pt]*chan->readings[pt]));
                else {
                    err = TRUE;
                    break;
                }
            }
            if (!err) {
                Fmt (newchan->label, "Power[%s]", chan->label);
                Fmt (newchan->note, "%s\nPower [%s]\n"
                                "y = x^a\n"
                                "a: %f[e2p5]\n",
                                chan->note, chan->label, expon);
                channellist_AddChannel (newchan);
            }
        } else err = TRUE;
        if (err)
        {
            MessagePopup ("Exponent Function Message", "Channel readings include 0.0 or < 0.0 - function voided");
            if (newchan) {if (newchan->readings) free (newchan->readings); free (newchan);}
        }
    }
    return 0;
}

void InverseCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    channelPtr chan, newchan;
    int pt, err;

    chan = channellist_GetSelection();

    err = FALSE;
    newchan = channel_Create();
    if (newchan && channel_AllocMem (newchan, chan->pts))
    {
        for (pt = 0; pt < chan->pts; pt++)
        {
            if (chan->readings[pt] != 0.0)
                newchan->readings[pt] = 1/chan->readings[pt];
            else {err = TRUE; break;}
        }
        if (!err)
        {
            Fmt (newchan->label, "%s<1 / (%s)", chan->label);
            Fmt (newchan->note, "%s\n%s\n", chan->note, newchan->label);
            channellist_AddChannel (newchan);
        }
        else {free (newchan->readings); free (newchan); newchan = NULL;}
    }
    else err = TRUE;
    if (err)
    {
        MessagePopup ("Inverse Function Message", "Channel readings include 0.0 - function voided");
        if (newchan) free (newchan);
    }
}

void NegativeCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    channelPtr chan, newchan;
    chan = channellist_GetSelection();

    newchan = channel_Create();
    if (newchan && channel_AllocMem (newchan, chan->pts) &&
        (Neg1D (chan->readings, chan->pts, newchan->readings) == NoErr))
    {
        Fmt (newchan->label, "%s<- (%s)", chan->label);
        Fmt (newchan->note, "%s\n%s\n", chan->note, newchan->label);
        channellist_AddChannel (newchan);
    }
    else MessagePopup ("Negative Function Message", "Random error...try again");
}

void InitStatisticsCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    int i;
    channelPtr chan;

    chanfunc.p = LoadPanel (0, "chanfncu.uir", STATISTICS);
    
    util_InitClose (chanfunc.p, STATISTICS_CLOSE, FALSE);
    SetPanelPos (chanfunc.p, 100, 100);

    channellist_Copy (chanfunc.p, STATISTICS_CHANNELS);

    chan = channellist_GetSelection();
    chanfunc_CalcStatistics (chan);
    SetInputMode (chanfunc.p, STATISTICS_SAVE,
            ((StringLength (chan->note) + StringLength(chanfunc.note)) < 255));

    InstallPopup (chanfunc.p);
}

int  DoStatisticsCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int i;
    channelPtr chan;

    if (event == EVENT_COMMIT)
    {
        GetCtrlIndex (panel, STATISTICS_CHANNELS, &i);
        chan = channellist_GetItem (i);
        chanfunc_CalcStatistics (chan);
        SetInputMode (panel, STATISTICS_SAVE,
            ((StringLength (chan->note) + StringLength(chanfunc.note)) < 255));
    }
    return 0;
}

int  SaveStatisticsCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int i;
    channelPtr chan;
    if (event == EVENT_COMMIT)
    {
        GetCtrlIndex (panel, STATISTICS_CHANNELS, &i);
        chan = channellist_GetItem (i);
        if ((StringLength (chan->note) + StringLength(chanfunc.note)) < 255)
        {
            Fmt (chan->note, "%s[a]<\n%s", chanfunc.note);
            SetInputMode (panel, control, FALSE);
        }
    }
    return 0;
}

static void chanfunc_CalcStatistics (channelPtr chan)
{
    double mean, std_dev, variance, rms, moment, median, mode, min, max;
    int err, order, min_i, max_i, intervals;
    char newnote[256];

    Fmt (chanfunc.note, "");
    err = MaxMin1D (chan->readings, chan->pts, &max, &max_i, &min, &min_i);
    SetInputMode (chanfunc.p, STATISTICS_MIN, !err);
    SetCtrlVal (chanfunc.p, STATISTICS_MIN, min);
    SetInputMode (chanfunc.p, STATISTICS_MAX, !err);
    SetCtrlVal (chanfunc.p, STATISTICS_MAX, max);
    if (err == NoErr)
    {
        Fmt (chanfunc.note, "%s<Min: %f[e2p5]\n", min);
        Fmt (chanfunc.note, "%s[a]<Max: %f[e2p5]\n", max);
    }

    err = Mean (chan->readings, chan->pts, &mean);
    SetInputMode (chanfunc.p, STATISTICS_MEAN, !err);
    SetCtrlVal (chanfunc.p, STATISTICS_MEAN, mean);
    if (err == NoErr) Fmt (chanfunc.note, "%s[a]<Mean: %f[e2p5]\n", mean);

    err = StdDev (chan->readings, chan->pts, &mean, &std_dev);
    SetInputMode (chanfunc.p, STATISTICS_STDDEV, !err);
    SetCtrlVal (chanfunc.p, STATISTICS_STDDEV, std_dev);
    if (err == NoErr) Fmt (chanfunc.note, "%s[a]<StdDev: %f[e2p5]\n", std_dev);

    err = Variance (chan->readings, chan->pts, &mean, &variance);
    SetInputMode (chanfunc.p, STATISTICS_VAR, !err);
    SetCtrlVal (chanfunc.p, STATISTICS_VAR, variance);
    if (err == NoErr) Fmt (chanfunc.note, "%s[a]<Variance: %f[e2p5]\n", variance);

    err = RMS (chan->readings, chan->pts, &rms);
    SetInputMode (chanfunc.p, STATISTICS_RMS, !err);
    SetCtrlVal (chanfunc.p, STATISTICS_RMS, rms);
    if (err == NoErr) Fmt (chanfunc.note, "%s[a]<RMS: %f[e2p5]\n", rms);

    GetCtrlVal (chanfunc.p, STATISTICS_ORDER, &order);
    err = Moment (chan->readings, chan->pts, order, &moment);
    SetInputMode (chanfunc.p, STATISTICS_MOMENT, !err);
    SetInputMode (chanfunc.p, STATISTICS_ORDER, !err);
    SetCtrlVal (chanfunc.p, STATISTICS_MOMENT, moment);
    if (err == NoErr) Fmt (chanfunc.note, "%s[a]<Moment: %f[e2p5] (order: %i)\n", moment, order);

    err = Median (chan->readings, chan->pts, &median);
    SetInputMode (chanfunc.p, STATISTICS_MEDIAN, !err);
    SetCtrlVal (chanfunc.p, STATISTICS_MEDIAN, median);
    if (err == NoErr) Fmt (chanfunc.note, "%s[a]<Median: %f[e2p5]\n", median);

    GetCtrlVal (chanfunc.p, STATISTICS_INTERVAL, &intervals);
    err = Mode (chan->readings, chan->pts, min, max, intervals, &mode);
    SetInputMode (chanfunc.p, STATISTICS_INTERVAL, !err);
    SetInputMode (chanfunc.p, STATISTICS_MODE, !err);
    SetCtrlVal (chanfunc.p, STATISTICS_INTERVAL, intervals);
    SetCtrlVal (chanfunc.p, STATISTICS_MODE, mode);
    if (err == NoErr) Fmt (chanfunc.note, "%s[a]<Mode: %f[e2p5] (intervals: %i)\n", mode, intervals);
}

void InitLinEvalCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    int i;
    channelPtr chan;

    chanfunc.p = LoadPanel (0, "chanfncu.uir", LINEVAL);
    
    util_InitClose (chanfunc.p, LINEVAL_CLOSE, FALSE);
    SetPanelPos (chanfunc.p, 100, 100);

    channellist_Copy (chanfunc.p, LINEVAL_CHANNELS);

    InstallPopup (chanfunc.p);
}

int  DoLinEvalCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int i;
    double a, b;
    channelPtr chan, newchan;

    if (event == EVENT_COMMIT)
    {
        GetCtrlVal (panel, LINEVAL_CHANNELS, &i);
        chan = channellist_GetItem (i);
        GetCtrlVal (panel, LINEVAL_MULT, &a);
        GetCtrlVal (panel, LINEVAL_ADD, &b);
        newchan = channel_Create();
        Fmt (newchan->note, "%s\nLinear Evaluation [%s]\n"
                             "y = ax + b\n"
                             "a: %f[e2p5]  b: %f[e2p5]\n",
                              chan->note, chan->label, a, b);
        chanfunc.NoErr = newchan && channel_AllocMem (newchan, chan->pts) &&
                    (LinEv1D (chan->readings, chan->pts, a, b, newchan->readings) == NoErr);
        chanfunc_AddtoList ("Lin Eval", newchan);
    }
    return 0;
}

static void chanfunc_AddtoList (char *label, channelPtr chan)
{
    char info[256];

    if (chanfunc.NoErr)
    {
        Fmt (chan->label, label);
        channellist_AddChannel (chan);
    }
    else
    {
        Fmt (info, "Error during %s function", label);
        MessagePopup ("Generate Channel Message", info);
        if (chan)
        {
            if (chan->readings) free(chan->readings);
            free (chan);
        }
    }
}
