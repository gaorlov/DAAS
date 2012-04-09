typedef void (*UpdateReadingsPtr) (int panel, void *ptr);   
typedef enum {ANALOGUE, DIGITAL} DevType;
typedef enum {OUT_PORT, IN_PORT} PortDirection;
typedef struct port{
	struct{
		struct{
			int port, devtype, bits, returnport, measPanel;
			struct{int bitnum, val, panel; struct port *port;}*bitarr;
			union {sourcePtr source;
				acqchanPtr acqchan;
			}IO;
		}digitalIOport;
		struct{
			int channel, range;
			union {sourcePtr source;
				acqchanPtr acqchan;
			}IO;
		}analogueIOport;
	}port;
	int direction, control, menuitem_id, measPanel, type, averaging;
}portType;
typedef portType *portPtr;

typedef struct{
	int  BoardNum, menuitem_id, baseAddr, iPanel;
	void *device, *devType;
	char *name;
}MCCdevType;

typedef MCCdevType *MCCdevPtr;

typedef void (*board_DeviceFuncPtr) (MCCdevPtr dev);
typedef void (*board_CreateDevicePtr) (MCCdevPtr dev);
typedef void (*RemoveDevicePtr) (void *ptr);
typedef struct {
	MenuCallbackPtr 		OperateDevice;
	UpdateReadingsPtr 		UpdateReadings;
	RemoveDevicePtr 		RemoveDevice;
	board_DeviceFuncPtr 	SaveDevice, LoadDevice;
	board_CreateDevicePtr 	CreateDevice;
	char label[50], id[256];
}MCCdevTypeItem;
typedef MCCdevTypeItem *MCCdevTypePtr;

extern void source_GetRange(sourcePtr src, int rangeInt);
extern void ReadAnalogue (acqchanPtr acqchan);
extern void ReadAnalogueOut (acqchanPtr acqchan);
extern void SetAnalogue (sourcePtr src);
extern void ReadDigital (acqchanPtr acqchan);
extern void ReadDigitalOut (acqchanPtr acqchan);
extern void SetDigital (sourcePtr src);
extern void boards_Load(void *dev, void (*load) (MCCdevPtr));
extern void boards_Save(void *dev, void (*save) (MCCdevPtr));

extern portPtr create_Port(void *dev, char *name, int type, int direction, GetReadingPtr GetReading,  ...);
extern void port_Load (void *dev, portPtr port);
extern void port_Save (portPtr port);

extern void boards_DevTypeList_AddDev(MCCdevTypePtr devType);
extern void init_MCCdevices(void);
