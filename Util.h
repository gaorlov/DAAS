#include <stdarg.h>
#define FILE_READ 1
#define FILE_WRITE 2
#define FILE_ITEM_SEP "-----------------------------------------\n"

typedef enum acqstatusSet {
    ACQ_BUSY, ACQ_STOPPED, ACQ_DONE, ACQ_PAUSED,
    ACQ_NONE, ACQ_BEGIN, ACQ_TERMINATE} acqstatusType;

typedef enum expSet {EXP_FLOAT, EXP_SOURCE} expType;

extern struct utilStruct
{
    struct {unsigned int pt, nPts;
            acqstatusType status;} acq;
    expType exp;
    int p, beep, err;
    void (*DiscardPanels)(void);
}   utilG;

extern struct fileStruct {
    int data, analysis;
}   fileHandle;

extern void *util_formatParse(char *format, va_list *list, char *msg, int index);
extern void util_formatParseRead(char *format, va_list *list, char *msg, int index);

extern int  util_HidePanelCallback(int panel, int event, void *callbackData, int eventData1, int eventData2);
extern int util_HidePanelOnLoseFocus (int panel, int event, void *callbackData, int eventData1, int eventData2);
extern int  util_DiscardPanelCallback(int panel, int event, void *callbackData, int eventData1, int eventData2);
extern void util_OutofMemory(char *header);
extern void util_Delay (double sec);
extern int  util_TakingData (void);
extern void util_InitClose (int panel, int control, int visible);
extern void util_IncAcqPt (void);
extern int  util_NoteCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
extern int  util_DiscardCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
extern void util_RemoveInitMessage (void);
extern void util_ChangeInitMessage (char *msg);
extern void util_MessagePopup(char* Title, char* Message, ...);

extern int  util_OpenFile(char *path, int action, int ascii);
extern void util_CloseFile(void);

extern void util_SaveNote (char *note);
extern void util_LoadNote (char *note);

extern void utilG_Exit(void);
extern void utilG_Init (void (*DiscardPanels)(void));
