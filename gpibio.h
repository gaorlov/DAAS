typedef enum {DEV_LOCAL, DEV_REMOTE, DEV_OFFLINE} devstatusType;

typedef void (*UpdateReadingsPtr) (int panel, void *ptr);
typedef struct gpibiostruct
{
    int             id, menuitem_id;
    char            label[50];
    void            *device, *devType;
    devstatusType   status;
    int             panel, logio, iPanel;
    unsigned int    nIO;
    short  paddr, saddr;
    UpdateReadingsPtr UpdateReadings;
}   gpibioType;

typedef gpibioType *gpibioPtr;

typedef int (*InitDevicePtr) (gpibioPtr dev);
typedef void (*DeviceFuncPtr) (gpibioPtr dev);
typedef void *(*CreateDevicePtr) (gpibioPtr dev);
typedef void (*RemoveDevicePtr) (void *ptr);

typedef struct devTypeStruct {
    InitDevicePtr InitDevice;
    MenuCallbackPtr OperateDevice;
    UpdateReadingsPtr UpdateReadings;
    DeviceFuncPtr SaveDevice, LoadDevice;
    CreateDevicePtr CreateDevice;
    RemoveDevicePtr RemoveDevice;
    char label[50], id[256];
}   devTypeItem;

typedef devTypeItem *devTypePtr;

typedef struct UpdateReadingsStruct {
    int panel;
    void *ptr;
    UpdateReadingsPtr UpdateReadings;
}   UpdateReadingsItem;

typedef UpdateReadingsItem *UpdateReadingsItemPtr;

extern void gpibPrint(gpibioPtr dev, const char *format, ...);
extern double gpib_GetDoubleVal(gpibioPtr dev, char *msg);
extern void gpib_GetCharVal(gpibioPtr dev, char *msg, char *ret);
extern int  gpib_GetIntVal(gpibioPtr dev, char *msg);

extern int  gpibio_DeviceMatch (gpibioPtr dev, char *cmd, char *id);
extern void gpibio_Init (void);
extern void gpibio_Exit (void);
extern int  gpibio_Addr (gpibioPtr dev);
extern int  gpibio_Clear (gpibioPtr dev);
extern int  gpibio_Local (int done, gpibioPtr dev);
extern int  gpibio_Remote (gpibioPtr dev);
extern int  gpibio_Out (gpibioPtr dev, char *cmd);
extern int  gpibio_In (gpibioPtr dev, char *msg_in );
extern int  gpibio_SRQ (gpibioPtr dev);
extern int  gpibio_GetStatusByte (gpibioPtr dev, short *status);

extern void devPanel_UpdateReadings(void);
extern void devPanel_Add (int panel, void *ptr, UpdateReadingsPtr UpdateReadings);
extern void devPanel_Remove (int panel);

extern void dev_StoreCommand (gpibioPtr dev, char *cmd);
extern void dev_StoreResponse (gpibioPtr dev, char *rsp);
extern void devTypeList_AddItem (devTypePtr devType);
extern void OperateDevCallback (int menubar, int menuItem, void *callbackData, int panel);
