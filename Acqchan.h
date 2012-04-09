#define RUO_LOW_TABLEITEMS 65
#define RUO_HIGH_TABLEITEMS 273
int	RUO_USR_TABLEITEMS;

static double   RuO_Low_Table [RUO_LOW_TABLEITEMS][2],
                RuO_High_Table [RUO_HIGH_TABLEITEMS][2], 
				User_Table [RUO_HIGH_TABLEITEMS][2];

typedef struct acqchanStruct
{
    int             acquire, conversion, newreading, p, note, menuitem_id;
    double          reading, coeff;
    channelPtr      channel;
    listType        graphs;
    void            *dev, *upLvl;
    void            (*GetReading) (struct acqchanStruct *acqchan);
}   acqchanType;

typedef acqchanType *acqchanPtr;
typedef void (*GetReadingPtr)(acqchanPtr acqchan);


extern struct acqchanGStruct {
    listType channels, notes;
    double time;
}   acqchanG;

extern void acqchan_Init (void);
extern void acqchan_Exit(void);
extern void acqchan_Remove (acqchanPtr chan);
extern acqchanPtr acqchanlist_GetItem (int i);
extern acqchanPtr acqchanlist_GetItemByTitle(char *label);
extern void acqchan_AddGraph (acqchanPtr chan, void *item);
extern void acqchan_RemoveGraph (acqchanPtr chan, void *item);
extern void acqchan_UpdateReadingPanel(acqchanPtr acqchan);
extern void acqchan_Save (acqchanPtr acqchan);
extern void acqchan_Load (void *dev, acqchanPtr acqchan);

extern acqchanPtr acqchan_Create (char *label, void *dev, GetReadingPtr GetReading);
extern double acqchan_Measurement (double reading, double coeff, int conversion);
extern double **acqchan_MeasurementArray (double *readings, double coeff, int conversion, int pts);
extern void acqchanlist_Dimmed (int dimmed);
extern int  acqchanlist_FindItem (acqchanPtr chan);
extern void acqchanlist_Display(void);
extern void acqchanlist_Copy (int panel, int control);
extern void acqchanlist_AddChannel (acqchanPtr chan);
extern void acqchanlist_RemoveChannel (acqchanPtr chan);
extern void acqchanlist_ReplaceChannel (acqchanPtr chan);
extern int  acqchanlist_AllocMemory (void);
extern void acqchanlist_InitDataFile (char *path);
extern void acqchanlist_GetandSaveReadings (void);
extern void acqchanlist_CloseFile (void);
extern void acqchanlist_CopytoChannelList (void);
extern acqchanPtr acqchanNote_GetItem (int i);
