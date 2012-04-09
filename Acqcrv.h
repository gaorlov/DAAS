#define ACQCURVE_BUFFERSIZE 100

typedef struct
{
    acqchanPtr x, y, x0;
    int color, ptfreq;
    int bufferpts, buffer[ACQCURVE_BUFFERSIZE];
    struct {int color, save;} autoattr;
    struct {int segment, on, offset, pts; void *source;} marker;
    char path[260], note[260];
}   acqcurveType;

typedef acqcurveType *acqcurvePtr;

typedef struct {
    void *graph;
	void (*InitSourceMarker) (int, void *);
    CtrlCallbackPtr ControlCallback;
	PanelCallbackPtr PanelCallback;
	int Apanel;
} acqcurveGs;

typedef acqcurveGs *acqcurveGptr;

struct acqcurveLStruct{
	acqcurveGptr aPtr;
	listType acqcurves;	
}	acqcurveL;

extern void     acqcurve_Init (acqcurvePtr acqcurve);
extern void     acqcurve_InitPanel (int panel, int control, acqcurvePtr acqcurve);
extern curvePtr acqcurve_MakeCurve (int n, char *title, acqcurvePtr acqcurve);
extern void     acqcurve_PlotReading (void* graphP, int panel, int control, acqcurvePtr acqcurve);
extern void     acqcurve_Plot (void *graphP, int panel, int control, acqcurvePtr acqcurve);
extern void     acqcurve_Hide (int panel, int control, acqcurvePtr acqcurve);
