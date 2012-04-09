extern struct acquireStruct
{
    struct {int setup, datainfo;} p;
}   acqG;

extern struct expStruct {
    void (*InitExp) (void);
    void (*UpdatePanel) (void);
    void (*DoExp) (void);
    double delay;
    acqstatusType acqstatus;
}   expG;

extern void acquire_Init (void);
extern void acquire_Exit(void);
extern int  acquire_GetMenuBar (void);
extern void acquire_UpdatePanel(void);
extern void acquire_UpdatePanelExp (void);
extern void acquire_UpdateDataInfoPanel (void);
extern void acquire_IncDataFileExt(void);

extern int exp_Begin(void);
