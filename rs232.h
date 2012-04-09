typedef void (*rsUpdateReadingsPtr) (int panel, void *ptr);
typedef struct {
	int id, menuitem_id;
	void *device, *devType;
	struct{ int port, baud, parity, data, stop, inQ, outQ; char *name;}COM;
	rsUpdateReadingsPtr UpdateReadings;
}rs_232Type;
typedef rs_232Type *rs_232Ptr;

typedef int  (*rsInitDevicePtr) (rs_232Ptr dev);
typedef void (*rsDeviceFuncPtr) (rs_232Ptr dev);
typedef void *(*rsCreateDevicePtr) (rs_232Ptr dev);
typedef void (*rsRemoveDevicePtr) (void *ptr);

typedef struct{
	rsInitDevicePtr InitDevice;
	MenuCallbackPtr OperateDevice;
	rsUpdateReadingsPtr UpdateReadings;
	rsDeviceFuncPtr SaveDevice, LoadDevice;
	rsCreateDevicePtr CreateDevice;
	rsRemoveDevicePtr RemoveDevice;
	char label[50], id[256];
} rsDevTypeItem;
typedef rsDevTypeItem *rsDevTypePtr;

extern void rs232_Init(void);
