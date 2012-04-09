typedef struct
{
    struct {double reading; char label[260];} x0;
    int plothandle;
    unsigned int pts, offset, curvepts;
    channelPtr x, y;
    struct
    {
        char label[100], note[1000];
        struct {int plot, point, line;} style;
        int ptfreq, color, hidden, fat;
    }   attr;
}   curveType;

typedef curveType *curvePtr;

typedef struct CT
{
    listType list;
    int panel;
}   curvelistType;

typedef curvelistType *curvelistPtr;

extern struct curveGStruct
{
    int panel, control, menuBar;
    char *title;
    curvelistPtr curves;
}   curveG;

extern struct curveviewStruct
{
    int p1, p2, offset, pts, h1, h2;
}   curveview;

extern int curveMenus;

extern void     curveG_Init (int graph, int control, char *title, curvelistPtr curves);
extern void     curve_Exit (void);
extern curvePtr curve_Create (void);
extern void     curve_Save (curvePtr c);
extern curvePtr curve_Load (char *graphtitle);
extern void     curve_Plot (int panel, int control, curvePtr c, void *graphP);
extern void     curve_Hide (int panel, int control, curvePtr c, int delayed);
extern char     *curve_CompleteListItem (curvePtr curve, int offset, int pts);
extern void     curve_CopyAttrs (curvePtr curve, curvePtr copy);

extern void     curve_InitViewPanel (int pts2);
extern void     curve_UpdateViewPanel (curvePtr curve, int pts2);
extern void     curve_UpdateViewGraph (curvePtr curve);

extern void     curvelist_Init (curvelistPtr curves);
extern curvePtr curvelist_GetItem (listType list, int i);
extern curvePtr curvelist_GetSelection (void);
extern void     curvelist_UpdatePanel (curvelistType curves);
extern void     curvelist_AddCurve (curvelistPtr curves, curvePtr c);
extern void     curvelist_RemoveCurve (curvelistPtr curves, curvePtr c);
