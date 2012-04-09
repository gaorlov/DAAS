#define OUTPUT_DC 0
#define OUTPUT_STEP 1
#define OUTPUT_EXTERN 2

#define MAX_SEGMENTS 200

typedef struct{
	double maxVal, minVal,  resolution;
}rangeType;
typedef rangeType *rangePtr;

typedef struct {
    double       des_start, start, stop, des_stop, step, rate, time;
    unsigned int points, pt, log;
	int output, optimize;
    double delay; /* sec */
    char label[30];
	struct{int on; double val;}error;
	double *valArray;
	void *source;
}   segmentType;

typedef segmentType *segmentPtr;

typedef struct sourceStruct
{
    acqchanPtr acqchan;
    segmentPtr segments[MAX_SEGMENTS];
    int nSegments, points, inlist, freq, sourceOn, panel, menu, messagePanel, menuitem_id, precAssist, seg;
    double biaslevel, time, min, max;
    void (*SetLevel)(struct sourceStruct *src);
	struct {rangePtr *range, *temprange, *nullrange; int autoscale;}ranges;
}   sourceType;

typedef sourceType *sourcePtr;
typedef void (*SetLevelPtr) (sourcePtr src);

extern void exp_InitSourcePanel (void);
extern void source_Init (void);
extern void gensrc_InitPanel (sourcePtr src);
extern void source_InitPanel (sourcePtr src);
extern rangePtr		range_Create  	(double maxVal, double minVal, double resolution);
extern sourcePtr	source_Create	(char *label, void *dev, SetLevelPtr SetLevel,
                                  GetReadingPtr GetReading);
extern void source_UpdatePanel (int panel, sourcePtr src);
extern void source_Save (sourcePtr src);
extern void source_Load (void *dev, sourcePtr src);
extern void source_Remove (sourcePtr src);
extern void exp_UpdateSourcePanel(void);

extern void sourcelist_InitAcqCurveSourceList (int panel, void *src_marker);      
extern int  AcqCurveSourceMarkerCallback(int panel, int control, int event,
						void *callbackData, int eventData1, int eventData2);
extern int 	AcqCurvePanelCallback (int panel, int event, void *callbackData,
						int eventData1, int eventData2);
extern void updateGraphSource(void);
extern void sourcelist_RemoveSource(sourcePtr src);
