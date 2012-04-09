typedef struct
{
    char    label[100], note[1000];
    unsigned int pts;
    double  *readings;
    listType curves, graphs;
}   channelType;

typedef channelType *channelPtr;

/************** Global Variable Declarations **************/
extern struct channelinfoType
{
    listType channels;
    struct {int channels;} p;
}   channelG;

extern struct chanviewStruct
{
    int p1, p2, offset, pts;
}   chanview;


/************** Global Function Declarations **************/
extern channelPtr   channel_Create(void);
extern int          channel_AllocMem(channelPtr c, unsigned int pts);
extern channelPtr   channel_CopySubset (int offset, int pts, channelPtr c);
extern double       channel_GetReading(channelPtr c, int index);
extern void         channel_AddCurve (channelPtr chan, char *curve, char *graph);
extern void         channel_RemoveCurve (channelPtr chan, char *curve);
extern void         channel_Save(channelPtr c);
extern channelPtr   channel_Load(void);
extern void         channel_SaveSubset(channelPtr chan);
extern void         channel_UpdateReadingList(channelPtr chan);

extern channelPtr  channellist_GetItem (int i);
extern channelPtr  channellist_GetSelection (void);
extern void channellist_Copy (int panel, int control);

extern void channel_InitViewPanel (void);
extern void channel_UpdateViewPanel (channelPtr chan);
extern void channel_UpdateViewGraph (channelPtr chan, int scatter);
extern void channel_InitEditPanel (channelPtr chan, int editlabel);

extern void channellist_UpdatePanel(void);
extern void channellist_AddChannel(channelPtr chan);
extern void channellist_RemoveChannel(channelPtr chan);
extern void channelG_Init(void);
extern void channelG_Exit(void);
