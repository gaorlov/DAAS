#include "Acquireu.h"
#include <analysis.h>
#include <formatio.h>
#include <userint.h>
#include <ansi_c.h>

#include "util.h"
#include "utilu.h"
#include "list.h"
#include "listu.h"
#include "channel.h"
#include "acqchan.h"
#include "curve.h"
#include "acqcrv.h"
#include "graph.h"
#include "acqcrvu.h"

#define FALSE 0
#define TRUE 1

void acqcurve_Init (acqcurvePtr acqcurve);
void acqcurve_PlotReading (void* graphP, int panel, int control, acqcurvePtr acqcurve);
void acqcurve_Hide (int panel, int control, acqcurvePtr acqcurve);
void acqcurve_Plot (void *graphP, int panel, int control, acqcurvePtr acqcurve);

void        acqcurve_InitPanel (int panel, int control, acqcurvePtr acqcurve);
static void acqcurve_NextColor (int *color);
curvePtr    acqcurve_MakeCurve (int n, char *title, acqcurvePtr acqcurve);



curvePtr acqcurve_MakeCurve (int n, char *title, acqcurvePtr acqcurve)
{
    acqchanPtr acqchan;
    curvePtr curve;
    int i;

    curve = curve_Create ();
    if (!curve) return NULL;

    curve->attr.ptfreq = acqcurve->ptfreq;
    curve->attr.color = acqcurve->color;
    Fmt (curve->attr.note, acqcurve->note);

    Fmt (curve->attr.label, "Curve#%i", n);
    if (acqcurve->autoattr.color) acqcurve_NextColor (&acqcurve->color);
    curve->curvepts = acqcurve->marker.pts;
    curve->pts = acqcurve->marker.pts;

    if (acqcurve->x0) {
        Fmt (curve->x0.label, acqcurve->x0->channel->label);
        curve->x0.reading = acqcurve->x0->channel->readings[utilG.acq.pt];
    }

    curve->x = channel_CopySubset (acqcurve->marker.offset, acqcurve->marker.pts, acqcurve->x->channel);
    Fmt (curve->x->label, "%s[a]<[%i]", n);
    Fmt (curve->x->note, "%s[a]<Coeff: %f[e2p5]\n", acqcurve->x->coeff);
    channel_AddCurve (curve->x, curve->attr.label, title);
    channellist_AddChannel (curve->x);

    curve->y = channel_CopySubset (acqcurve->marker.offset, acqcurve->marker.pts, acqcurve->y->channel);
    Fmt (curve->y->label, "%s[a]<[%i]", n);
    Fmt (curve->y->note, "%s[a]<Coeff: %f[e2p5]\n", acqcurve->y->coeff);
    channel_AddCurve (curve->y, curve->attr.label, title);
    channellist_AddChannel (curve->y);

    for (i = 0; i < acqchanG.notes.nItems; i++) {
        acqchan = acqchanNote_GetItem (i);
        Fmt (curve->attr.note, "%s[a]<%s:%f\n", acqchan->channel->label, acqchan->reading);
    }

    return curve;
}

static void acqcurve_NextColor (int *color)
{
    int c_blue, c_red, c_green;

    c_blue = *color&0x0000FFL;
    c_red = (*color&0xFF0000L)>>16;
    c_green = (*color&0x00FF00L)>>8;

    c_blue+=51;
    if (c_blue > 255)
    {
        c_blue = 0;
        c_green+=51;
        if (c_green > 255)
        {
            c_green = 0;
            c_red+=51;
            if (c_red > 255)
            {
                c_red = 0;
                c_blue = 0;
            }
        }
    }
    *color = MakeColor (c_red, c_green, c_blue);
}

int  AcqCurveControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    int filestatus, i, list_panel;
    acqcurvePtr c;
    acqchanPtr acqchan;


    switch (control)
    {
        case ACQCURVE_AUTONOTES:
            if (event == EVENT_COMMIT) {
                list_panel = LoadPanel (utilG.p, "acqcrvu.uir", AUTONOTES);
                SetPanelPos (list_panel, VAL_AUTO_CENTER, VAL_AUTO_CENTER);
                if (acqchanG.notes.nItems > 0) ClearListCtrl (list_panel, AUTONOTES_LIST);
                for (i = 0; i < acqchanG.notes.nItems; i++) {
                    acqchan = acqchanNote_GetItem (i);
                    InsertListItem (list_panel, AUTONOTES_LIST, -1,
                                    acqchan->channel->label, i);
                }
                DisplayPanel (list_panel);
            }
            break;
        case ACQCURVE_AUTOCOLOR:
            if (event == EVENT_COMMIT) {
                c = callbackData;
                GetCtrlVal (panel, control, &c->autoattr.color);
            }
            break;
        case ACQCURVE_X0:
            if (event == EVENT_COMMIT) {
                c = callbackData;
                if (c->x0) acqchan_RemoveGraph (c->x0, c);
                GetCtrlVal (panel, control, &i);
                if (i == NOT_IN_LIST) c->x0 = NULL;
                else {
                    c->x0 = acqchanlist_GetItem (i);
                    acqchan_AddGraph (c->x0, c);
                }
            }
            break;
        case ACQCURVE_AUTOSAVE:
            if (event == EVENT_COMMIT) {
                c = callbackData;
                GetCtrlVal (panel, control, &c->autoattr.save);
                SetInputMode (panel, ACQCURVE_GRAPHFILE, c->autoattr.save);
                if (c->autoattr.save) {
                    filestatus = FileSelectPopup ("", "*.grf", "*.grf",
                                              "Auto save graph selection",
                                              VAL_SAVE_BUTTON, 0, 1, 1, 1, c->path);
                    if (filestatus != VAL_NEW_FILE_SELECTED) {
                        c->autoattr.save = 0;
                        SetCtrlVal (panel, control, 0);
                        SetInputMode (panel, ACQCURVE_GRAPHFILE, FALSE);
                    }
                    else SetCtrlVal (panel, ACQCURVE_GRAPHFILE, c->path);
                }
            }
            break;
    }
    return 0;
}

void acqcurve_Plot (void *graphP, int panel, int control, acqcurvePtr acqcurve)
{
	graphPtr graph = graphP;
    int pts;

    if (panel) {
    	acqcurve_Hide(panel, control, acqcurve);
        if (acqcurve->x && acqcurve->y &&
            acqcurve->x->channel->readings &&
            acqcurve->y->channel->readings &&
            (acqcurve->marker.pts > 1))
		{
            pts = utilG.acq.pt - acqcurve->marker.offset;
            if (pts > 0) 
			{
				acqcurve->buffer[0] = PlotXY (panel, control, acqcurve->x->channel->readings + acqcurve->marker.offset, cqcurve->y->channel->readings + acqcurve->marker.offset,
                                              pts, VAL_DOUBLE, VAL_DOUBLE,
                                              VAL_THIN_LINE, VAL_NO_POINT, VAL_SOLID,
                                              acqcurve->ptfreq, acqcurve->color);
                acqcurve->bufferpts++;
            }
        }
    }
}

void acqcurve_Hide (int panel, int control, acqcurvePtr acqcurve)
{
    int i;
    for (i = 0; i < acqcurve->bufferpts; i++)
	{
		if(acqcurve->buffer[i] > 0)
			DeleteGraphPlot (panel, control, acqcurve->buffer[i], VAL_DELAYED_DRAW);
	}
    acqcurve->bufferpts = 0;
}

void acqcurve_PlotReading (void* graphP, int panel, int control, acqcurvePtr acqcurve)
{
	graphPtr graph = graphP;
    if (acqcurve->x && acqcurve->y && (utilG.acq.pt > 0)) {
        if (acqcurve->bufferpts < ACQCURVE_BUFFERSIZE) {
            if (utilG.acq.pt> 0)
            {acqcurve->buffer[acqcurve->bufferpts] =
               PlotLine (panel, control,
                      acqchan_Measurement(acqcurve->x->channel->readings[utilG.acq.pt-1], acqcurve->x->coeff, graph->x.conversion.val),
                      acqchan_Measurement(acqcurve->y->channel->readings[utilG.acq.pt-1], acqcurve->y->coeff, graph->y.conversion.val),
                      acqchan_Measurement(acqcurve->x->channel->readings[utilG.acq.pt], acqcurve->x->coeff, graph->x.conversion.val),
					  acqchan_Measurement(acqcurve->y->channel->readings[utilG.acq.pt], acqcurve->y->coeff, graph->y.conversion.val),
					  acqcurve->color);//*/
              }        
            acqcurve->bufferpts++;
        }
        else acqcurve_Plot (graph, panel, control, acqcurve);
    }
}

void acqcurve_Init (acqcurvePtr acqcurve)
{
    acqcurve->x = NULL;
    acqcurve->y = NULL;
    acqcurve->x0 = NULL;

    acqcurve->ptfreq = 1;
    acqcurve->color = 0x00FF00;

    acqcurve->bufferpts = 0;

    acqcurve->autoattr.color = FALSE;
    acqcurve->autoattr.save = FALSE;

    acqcurve->marker.offset = 0;
    acqcurve->marker.pts = utilG.acq.nPts;

    acqcurve->marker.on = TRUE;
    acqcurve->marker.source = NULL;
    acqcurve->marker.segment = TRUE;

    Fmt (acqcurve->path, "");
    Fmt (acqcurve->note, "");
}



//*/
