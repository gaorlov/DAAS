typedef void (*UpdateReadingsPtr) (int panel, void *ptr);
typedef struct{
	int baud, parity, data, stop, inQ, outQ, port, inUse;
	char name[10];
	void *device;
}comType;
typedef comType *comPtr;

typedef struct {
	int id, menuitem_id, logio, iPanel;
	void *device, *devType;
	comPtr COM;
	UpdateReadingsPtr UpdateReadings;
	MenuCallbackPtr OperateDevice;
	char label[256];
}rs232Type;
typedef rs232Type *rs232Ptr;

typedef int  (*rsInitDevicePtr) (rs232Ptr dev);
typedef void (*rsDeviceFuncPtr) (rs232Ptr dev);
typedef void (*rsCreateDevicePtr) (rs232Ptr dev);
typedef void (*rsRemoveDevicePtr) (void *ptr);

typedef struct{
	rsInitDevicePtr InitDevice;
	MenuCallbackPtr OperateDevice;
	UpdateReadingsPtr UpdateReadings;
	rsDeviceFuncPtr SaveDevice, LoadDevice;
	rsCreateDevicePtr CreateDevice;
	rsRemoveDevicePtr RemoveDevice;
	char label[50], id[256];
} rsDevTypeItem;
typedef rsDevTypeItem *rsDevTypePtr;

extern int rs232Write(rs232Ptr dev, char *format, ...);
extern int rs232Read (rs232Ptr dev, char *format, ...);

extern void rsDevTypeItem_AddItem(rsDevTypePtr devType);

extern void rs232_Init(void);
