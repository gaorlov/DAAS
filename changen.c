#include <analysis.h>
#include <formatio.h>
#include <ansi_c.h>
#include <userint.h>

#include "util.h"
#include "list.h"
#include "channel.h"
#include "channelu.h"
#include "changen.h"
#include "changenu.h"

#define TRUE 1
#define FALSE 0
 int NoErr;
static struct changenStruct {int p, NoErr;} changen;

static void InitPopupPanel (int panel, int control);
static void AddtoList (char *label, channelPtr chan);

void InitConstantCallback(int menubar, int menuItem, void *callbackData, int panel);
void InitImpulseCallback(int menubar, int menuItem, void *callbackData, int panel);
void InitPulseCallback(int menubar, int menuItem, void *callbackData, int panel);
void InitRampCallback(int menubar, int menuItem, void *callbackData, int panel);
void InitTriangleCallback(int menubar, int menuItem, void *callbackData, int panel);
void InitSinePatternCallback(int menubar, int menuItem, void *callbackData, int panel);

void InitChirpCallback(int menubar, int menuItem, void *callbackData, int panel);
void InitSawtoothCallback(int menubar, int menuItem, void *callbackData, int panel);
void InitSineCallback(int menubar, int menuItem, void *callbackData, int panel);
void InitSincCallback(int menubar, int menuItem, void *callbackData, int panel);
void InitSquareCallback(int menubar, int menuItem, void *callbackData, int panel);
void InitTriangleWvfmCallback(int menubar, int menuItem, void *callbackData, int panel);

void InitUniformCallback(int menubar, int menuItem, void *callbackData, int panel);
void InitWhiteCallback(int menubar, int menuItem, void *callbackData, int panel);
void InitGaussCallback(int menubar, int menuItem, void *callbackData, int panel);

/*
main()
{
    utilG_Init();
    listG_Init();
    fileG_Init();
    channelG_Init();

    changen_Init();

    RunUserInterface();
}
*/

void changen_Init (void)
{
    int menubar;

    util_ChangeInitMessage ("Generate Channel Menu...");

    menubar = GetPanelMenuBar (channelG.p.channels);

    InstallMenuCallback (menubar, CHANMENUS_GEN_CONSTANT, InitConstantCallback, 0);
    InstallMenuCallback (menubar, CHANMENUS_GEN_IMPULSE, InitImpulseCallback, 0);
    InstallMenuCallback (menubar, CHANMENUS_GEN_PULSE, InitPulseCallback, 0);
    InstallMenuCallback (menubar, CHANMENUS_GEN_RAMP, InitRampCallback, 0);
    InstallMenuCallback (menubar, CHANMENUS_GEN_TRIANGLE, InitTriangleCallback, 0);
    InstallMenuCallback (menubar, CHANMENUS_GEN_SINEPAT, InitSinePatternCallback, 0);

    InstallMenuCallback (menubar, CHANMENUS_GEN_WAVES_CHIRP, InitChirpCallback, 0);
    InstallMenuCallback (menubar, CHANMENUS_GEN_WAVES_SAWTOOTH, InitSawtoothCallback, 0);
    InstallMenuCallback (menubar, CHANMENUS_GEN_WAVES_SINE, InitSineCallback, 0);
    InstallMenuCallback (menubar, CHANMENUS_GEN_WAVES_SINC, InitSincCallback, 0);
    InstallMenuCallback (menubar, CHANMENUS_GEN_WAVES_SQUARE, InitSquareCallback, 0);
    InstallMenuCallback (menubar, CHANMENUS_GEN_WAVES_TRIANGLE, InitTriangleWvfmCallback, 0);

    InstallMenuCallback (menubar, CHANMENUS_GEN_NOISE_UNIFORM, InitUniformCallback, 0);
    InstallMenuCallback (menubar, CHANMENUS_GEN_NOISE_WHITE, InitWhiteCallback, 0);
    InstallMenuCallback (menubar, CHANMENUS_GEN_NOISE_GAUSS, InitGaussCallback, 0);
}

void InitGaussCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    InitPopupPanel (GAUSS, GAUSS_CLOSE);
}

int  GenGaussCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int pts, seed;
    double stddev;
    channelPtr chan;

    if (event == EVENT_COMMIT)
    {
        GetCtrlVal (panel, GAUSS_PTS, &pts);
        GetCtrlVal (panel, GAUSS_STDDEV, &stddev);
        GetCtrlVal (panel, GAUSS_SEED, &seed);

        DiscardPanel (changen.p);
        
        chan = channel_Create();
        changen.NoErr = chan && channel_AllocMem (chan, pts) &&
            (GaussNoise(pts, stddev, seed, chan->readings) == NoErr);
        AddtoList ("Gauss Noise", chan);
    }
    return 0;
}

void InitWhiteCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    InitPopupPanel (WHITE, WHITE_CLOSE);
}

int  GenWhiteCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int pts, seed;
    double amp;
    channelPtr chan;

    if (event == EVENT_COMMIT)
    {
        GetCtrlVal (panel, WHITE_PTS, &pts);
        GetCtrlVal (panel, WHITE_AMP, &amp);
        GetCtrlVal (panel, WHITE_SEED, &seed);

        DiscardPanel (changen.p);
        
        chan = channel_Create();
        changen.NoErr = chan && channel_AllocMem (chan, pts) &&
            (WhiteNoise(pts, amp, seed, chan->readings) == NoErr);
        AddtoList ("White Noise", chan);
    }
    return 0;
}

void InitUniformCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    InitPopupPanel (UNIFORM, UNIFORM_CLOSE);
}

int  GenUniformCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int pts, seed;
    channelPtr chan;

    if (event == EVENT_COMMIT)
    {
        GetCtrlVal (panel, UNIFORM_PTS, &pts);
        GetCtrlVal (panel, UNIFORM_SEED, &seed);

        DiscardPanel (changen.p);
        
        chan = channel_Create();
        changen.NoErr = chan && channel_AllocMem (chan, pts) &&
            (Uniform(pts, seed, chan->readings) == NoErr);
        AddtoList ("Uniform Noise", chan);
    }
    return 0;
}

void InitTriangleWvfmCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    InitPopupPanel (TRIWAVE, TRIWAVE_CLOSE);
}

int  GenTriangleWvfmCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int pts;
    double amp, freq, phase;
    channelPtr chan;

    if (event == EVENT_COMMIT)
    {
        GetCtrlVal (panel, TRIWAVE_PTS, &pts);
        GetCtrlVal (panel, TRIWAVE_AMP, &amp);
        GetCtrlVal (panel, TRIWAVE_FREQ, &freq);
        GetCtrlVal (panel, TRIWAVE_PHASE, &phase);

        DiscardPanel (changen.p);
        
        chan = channel_Create();
        changen.NoErr = chan && channel_AllocMem (chan, pts) &&
            (TriangleWave(pts, amp, freq, &phase, chan->readings) == NoErr);
        AddtoList ("Triangle Waveform", chan);
    }
    return 0;
}

void InitSquareCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    InitPopupPanel (SQUARE, SQUARE_CLOSE);
}

int  GenSquareCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int pts;
    double amp, freq, phase, duty;
    channelPtr chan;

    if (event == EVENT_COMMIT)
    {
        GetCtrlVal (panel, SQUARE_PTS, &pts);
        GetCtrlVal (panel, SQUARE_AMP, &amp);
        GetCtrlVal (panel, SQUARE_FREQ, &freq);
        GetCtrlVal (panel, SQUARE_PHASE, &phase);
        GetCtrlVal (panel, SQUARE_DUTY, &duty);

        DiscardPanel (changen.p);
        
        chan = channel_Create();
        changen.NoErr = chan && channel_AllocMem (chan, pts) &&
            (SquareWave(pts, amp, freq, &phase, duty, chan->readings) == NoErr);
        AddtoList ("Square", chan);
    }
    return 0;
}

void InitSincCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    InitPopupPanel (SINC, SINC_CLOSE);
}

int  GenSincCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int pts;
    double amp, dly, dt;
    channelPtr chan;

    if (event == EVENT_COMMIT)
    {
        GetCtrlVal (panel, SINC_PTS, &pts);
        GetCtrlVal (panel, SINC_AMP, &amp);
        GetCtrlVal (panel, SINC_DELAY, &dly);
        GetCtrlVal (panel, SINC_DT, &dt);

        DiscardPanel (changen.p);
        
        chan = channel_Create();
        changen.NoErr = chan && channel_AllocMem (chan, pts) &&
            (Sinc(pts, amp, dly, dt, chan->readings) == NoErr);
        AddtoList ("Sinc", chan);
    }
    return 0;
}

void InitSineCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    InitPopupPanel (SINE, SINE_CLOSE);
}

int  GenSineCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int pts;
    double amp, freq, phase;
    channelPtr chan;

    if (event == EVENT_COMMIT)
    {
        GetCtrlVal (panel, SINE_PTS, &pts);
        GetCtrlVal (panel, SINE_AMP, &amp);
        GetCtrlVal (panel, SINE_FREQ, &freq);
        GetCtrlVal (panel, SINE_PHASE, &phase);

        DiscardPanel (changen.p);
        
        chan = channel_Create();
        changen.NoErr = chan && channel_AllocMem (chan, pts) &&
            (SineWave(pts, amp, freq, &phase, chan->readings) == NoErr);
        AddtoList ("Sine", chan);
    }
    return 0;
}

void InitSawtoothCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    InitPopupPanel (SAWTOOTH, SAWTOOTH_CLOSE);
}

int  GenSawtoothCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int pts;
    double amp, freq, phase;
    channelPtr chan;

    if (event == EVENT_COMMIT)
    {
        GetCtrlVal (panel, SAWTOOTH_PTS, &pts);
        GetCtrlVal (panel, SAWTOOTH_AMP, &amp);
        GetCtrlVal (panel, SAWTOOTH_FREQ, &freq);
        GetCtrlVal (panel, SAWTOOTH_PHASE, &phase);

        DiscardPanel (changen.p);
        
        chan = channel_Create();
        changen.NoErr = chan && channel_AllocMem (chan, pts) &&
            (SawtoothWave(pts, amp, freq, &phase, chan->readings) == NoErr);
        AddtoList ("Sawtooth", chan);
    }
    return 0;
}

void InitChirpCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    InitPopupPanel (CHIRP, CHIRP_CLOSE);
}

int  GenChirpCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int pts;
    double amp, fl, fh;
    channelPtr chan;

    if (event == EVENT_COMMIT)
    {
        GetCtrlVal (panel, CHIRP_PTS, &pts);
        GetCtrlVal (panel, CHIRP_AMP, &amp);
        GetCtrlVal (panel, CHIRP_START, &fl);
        GetCtrlVal (panel, CHIRP_STOP, &fh);

        DiscardPanel (changen.p);
        
        chan = channel_Create();
        changen.NoErr = chan && channel_AllocMem (chan, pts) &&
            (Chirp(pts, amp, fl, fh, chan->readings) == NoErr);
        AddtoList ("Chirp", chan);
    }
    return 0;
}

void InitSinePatternCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    InitPopupPanel (SINEPAT, SINEPAT_CLOSE);
}

int  GenSinePatternCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int pts;
    double amp, phase, cycles;
    channelPtr chan;

    if (event == EVENT_COMMIT)
    {
        GetCtrlVal (panel, SINEPAT_PTS, &pts);
        GetCtrlVal (panel, SINEPAT_AMP, &amp);
        GetCtrlVal (panel, SINEPAT_PHASE, &phase);
        GetCtrlVal (panel, SINEPAT_CYCLES, &cycles);

        DiscardPanel (changen.p);
        
        chan = channel_Create();
        changen.NoErr = chan && channel_AllocMem (chan, pts) &&
            (SinePattern(pts, amp, phase, cycles, chan->readings) == NoErr);
        AddtoList ("Sine Pattern", chan);
    }
    return 0;
}

void InitTriangleCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    InitPopupPanel (TRIANGLE, TRIANGLE_CLOSE);
}

int  GenTriangleCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int pts;
    double amp;
    channelPtr chan;

    if (event == EVENT_COMMIT)
    {
        GetCtrlVal (panel, TRIANGLE_PTS, &pts);
        GetCtrlVal (panel, TRIANGLE_AMP, &amp);
        DiscardPanel (changen.p);
        
        chan = channel_Create();
        changen.NoErr = chan && channel_AllocMem (chan, pts) &&
            (Triangle(pts, amp, chan->readings) == NoErr);
        AddtoList ("Triangle", chan);
    }
    return 0;
}

void InitRampCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    InitPopupPanel (RAMP, RAMP_CLOSE);
}

int  GenRampCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int pts;
    double first, last;
    channelPtr chan;

    if (event == EVENT_COMMIT)
    {
        GetCtrlVal (panel, RAMP_PTS, &pts);
        GetCtrlVal (panel, RAMP_FIRST, &first);
        GetCtrlVal (panel, RAMP_LAST, &last);
        DiscardPanel (changen.p);
        
        chan = channel_Create();
        changen.NoErr = chan && channel_AllocMem (chan, pts) &&
            (Ramp(pts, first, last, chan->readings) == NoErr);
        AddtoList ("Ramp", chan);
    }
    return 0;
}

void InitPulseCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    InitPopupPanel (PULSE, PULSE_CLOSE);
}

int  GenPulseCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int pts, width, dly;
    double amp;
    channelPtr chan;

    if (event == EVENT_COMMIT)
    {
        GetCtrlVal (panel, PULSE_PTS, &pts);
        GetCtrlVal (panel, PULSE_AMP, &amp);
        GetCtrlVal (panel, PULSE_DELAY, &dly);
        GetCtrlVal (panel, PULSE_WIDTH, &width);
        DiscardPanel (changen.p);
        
        chan = channel_Create();
        changen.NoErr = chan && channel_AllocMem (chan, pts) &&
            (Pulse(pts, amp, dly, width, chan->readings) == NoErr);
        AddtoList ("Pulse", chan);
    }
    return 0;
}

void InitImpulseCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    InitPopupPanel (IMPULSE, IMPULSE_CLOSE);
}

int  GenImpulseCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int pts, index;
    double amp;
    channelPtr chan;

    if (event == EVENT_COMMIT)
    {
        GetCtrlVal (panel, IMPULSE_PTS, &pts);
        GetCtrlVal (panel, IMPULSE_AMP, &amp);
        GetCtrlVal (panel, IMPULSE_INDEX, &index);
        DiscardPanel (changen.p);
        
        chan = channel_Create();
        changen.NoErr = chan && channel_AllocMem (chan, pts) &&
            (Impulse(pts, amp, index, chan->readings) == NoErr);
        AddtoList ("Impulse", chan);
    }
    return 0;
}

void InitConstantCallback(int menubar, int menuItem, void *callbackData, int panel)
{
    InitPopupPanel (CONSTANT, CONSTANT_CLOSE);
}

int  GenConstantCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int pts;
    double amp;
    channelPtr chan;

    if (event == EVENT_COMMIT)
    {
        GetCtrlVal (panel, CONSTANT_PTS, &pts);
        GetCtrlVal (panel, CONSTANT_AMP, &amp);
        DiscardPanel (changen.p);
        
        chan = channel_Create();
        changen.NoErr = chan && channel_AllocMem (chan, pts) &&
            (Set1D (chan->readings, pts, amp) == NoErr);
        AddtoList ("Constant", chan);
    }
    return 0;
}

static void InitPopupPanel (int panel, int control)
{
    changen.p = LoadPanel (0, "changenu.uir", panel);
    
    util_InitClose (changen.p, control, FALSE);
    SetPanelPos (changen.p, 100, 100);
    InstallPopup (changen.p);
}

static void AddtoList (char *label, channelPtr chan)
{
    char info[256];

    if (changen.NoErr)
    {
        Fmt (chan->label, label);
        channellist_AddChannel (chan);
    }
    else
    {
        Fmt (info, "Error generating %s", label);
        MessagePopup ("Generate Channel Message", info);
        if (chan)
        {
            if (chan->readings) free(chan->readings);
            free (chan);
        }
    }
}
