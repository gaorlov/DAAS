/* ver 1.3
- added Keithley 2000 multimeter to program
*/

#include <ansi_c.h>
#include <formatio.h>
#include <utility.h>
#include <userint.h>


#include "util.h"
#include "utilu.h"

#define TRUE 1
#define FALSE 0

struct utilStruct utilG;
struct fileStruct fileHandle = {0, 0};
static int initP, fileP;

void *util_formatParse(char *format, va_list *list, char *msg, int index);
void util_formatParseRead(char *format, va_list *list, char *msg, int index);

void util_MessagePopup(char* Title, char* Message, ...);
void util_RemoveInitMessage (void);
void util_ChangeInitMessage (char *msg);
int  util_HidePanelOnLoseFocus (int panel, int event, void *callbackData, int eventData1, int eventData2);
int  util_HidePanelCallback(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  util_DiscardPanelCallback(int panel, int event, void *callbackData, int eventData1, int eventData2);
void util_OutofMemory(char *header);
void util_Delay (double sec);
int  util_TakingData (void);
void util_InitClose (int panel, int control, int visible);
void util_IncAcqPt (void);
int  util_NoteCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  util_DiscardCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);

void utilG_Init (void (*DiscardPanels)(void));

void utilG_Exit(void);

int util_OpenFile(char *path, int action, int ascii);
void util_CloseFile(void);

void util_SaveNote (char *note);
void util_LoadNote (char *note);


int util_OpenFile(char *path, int action, int ascii)
{
    int handle;
    unsigned int  nBytes, pos, width;
    long size;
    char info[256];

    if (action == FILE_WRITE) {
        handle = OpenFile (path, FILE_WRITE, 0, ascii);
        if (!ascii) FmtFile (handle, "BINARY\n");
    }
    else {
        handle = OpenFile (path, action, 0, 0);
        nBytes = ReadLine (handle, info, 255);
        if (CompareBytes (info, 0, "BINARY", 0, 6, 0) != 0) {
            CloseFile (handle);
            handle = OpenFile (path, action, 0, 1);
            ascii = TRUE;
        }
    }
	if(handle)
    {
		fileP = LoadPanel (0, "utilu.uir", FILESTAT);

    	

    	if (action == FILE_WRITE) {
        	Fmt (info, "Saving file: %s", path);
        	if (ascii) Fmt (info, "%s[a]< (ASCII file...go for coffee!)");
    	} else {
        	GetFileSize (path, &size);
        	Fmt (info, "Loading file: %s (%i kB)", path, size/1000);
        	if (ascii) Fmt (info, "%s[a]< (ASCII file...take a nap!)");
    	}

    	SetCtrlVal (fileP, FILESTAT_TEXT, info);
    	GetCtrlAttribute (fileP, FILESTAT_TEXT, ATTR_WIDTH, &width);
    	SetPanelAttribute (fileP, ATTR_WIDTH, width+12);
    	SetCtrlAttribute (fileP, FILESTAT_TEXT, ATTR_LEFT, 6);

    	SetPanelPos (fileP, VAL_AUTO_CENTER, VAL_AUTO_CENTER);
    	InstallPopup (fileP);
	}
	else
		handle = 0;
	return handle;
}

void util_CloseFile(void)
{
    if(fileHandle.analysis)
	{
		DiscardPanel (fileP);
    	
    	CloseFile(fileHandle.analysis);
	}
}

void util_SaveNote (char *note)
{
    FmtFile (fileHandle.analysis, "==========\n");
    FmtFile (fileHandle.analysis, "%s\n", note);
    FmtFile (fileHandle.analysis, "==========\n");
}

void util_LoadNote (char *note)
{
    char info[256];
    int i;

    i = 1;
    FillBytes (note, 0, 256, 0x00);
    ReadLine (fileHandle.analysis, info, 255); /* ========== */
    while (i)
    {
        ReadLine (fileHandle.analysis, info, 255);
        i = CompareBytes ("==========", 0, info, 0, 10, 0);
        if (i) Fmt (note, "%s[a]<%s\n", info);
    }
}

int  util_DiscardCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    if (event == EVENT_COMMIT)
    {
        DiscardPanel (panel);
    }
    return 0;
}

int util_HidePanelOnLoseFocus (int panel, int event, void *callbackData, int eventData1, int eventData2)
{
	if (event == EVENT_LOST_FOCUS)
		HidePanel(panel);
	return 0;
}

int  util_NoteCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
    char *note;
    if (event == EVENT_VAL_CHANGED)
    {
        note = callbackData;
        GetCtrlVal (panel, control, note);
    }
    return 0;
}

void util_IncAcqPt (void)
{
    utilG.acq.pt++;
	if(utilG.beep)
    	Beep();
}

void util_InitClose (int panel, int control, int visible)
{
    SetPanelAttribute (panel, ATTR_CLOSE_CTRL, control);
    SetPanelAttribute (panel, ATTR_SYSTEM_MENU_VISIBLE, TRUE);
    SetPanelAttribute (panel, ATTR_CLOSE_ITEM_VISIBLE, TRUE);
    SetCtrlAttribute (panel, control, ATTR_VISIBLE, visible);
}

void util_OutofMemory(char *header)
{
    MessagePopup (header, "Sorry, out of memory, aborting process");
}

void util_Delay (double sec)
{
    double time;
    time = Timer();
    while ((Timer()-time) < sec) ProcessSystemEvents();
}

int util_TakingData (void)
{
    return ((utilG.acq.status == ACQ_BUSY) ||
            (utilG.acq.status == ACQ_PAUSED) ||
            (utilG.acq.status == ACQ_TERMINATE));
}

void utilG_Init (void (*DiscardPanels)(void))
{
    int p, control, top, height, width;
	int grw, chw, asw, acw;
    

    utilG.acq.pt = 0;
    utilG.acq.nPts = 0;
    utilG.acq.status = ACQ_NONE;

    utilG.p = LoadPanel (0, "utilu.uir", BG);
    
    utilG.DiscardPanels = DiscardPanels;

	SetPanelAttribute (utilG.p, ATTR_WINDOW_ZOOM, VAL_MAXIMIZE);
    DisplayPanel (utilG.p);
	
	GetCtrlAttribute(utilG.p, BG_GRAPHS, ATTR_WIDTH, &grw);
	GetCtrlAttribute(utilG.p, BG_CHANNELS, ATTR_WIDTH, &chw);
	GetCtrlAttribute(utilG.p, BG_ACQSETUP, ATTR_WIDTH, &asw);
	GetCtrlAttribute(utilG.p, BG_ACQCHANNELS, ATTR_WIDTH, &acw);
	GetPanelAttribute(utilG.p, ATTR_WIDTH, &width);
	
	SetCtrlAttribute(utilG.p, BG_GRAPHS, ATTR_LEFT,			(3*width/20) - grw);
	SetCtrlAttribute(utilG.p, BG_CHANNELS, ATTR_LEFT, 		(7*width/20) - chw);
	SetCtrlAttribute(utilG.p, BG_ACQSETUP, ATTR_LEFT, 		(9*width/15) - asw);
	SetCtrlAttribute(utilG.p, BG_ACQCHANNELS, ATTR_LEFT, 	(9*width/10) - acw);
	
	initP = LoadPanel (utilG.p, "utilu.uir", INIT);
    
    SetPanelPos (initP, VAL_AUTO_CENTER, VAL_AUTO_CENTER);

    SetCtrlAttribute (initP, INIT_TEXT, ATTR_VISIBLE, FALSE);
    DisplayPanel (initP);

    GetUserEvent (1, &p, &control);
    switch (control) {
        case INIT_CONTROL: utilG.acq.status = ACQ_DONE; break;
        case INIT_ANALYSIS: utilG.acq.status = ACQ_NONE; break;
        case INIT_EXIT:
            utilG_Exit();
            QuitUserInterface(0);
            exit (EXIT_SUCCESS);
            break;
    }
    SetMouseCursor (VAL_HOUR_GLASS_CURSOR);

    SetCtrlAttribute (initP, INIT_TEXT, ATTR_VISIBLE, TRUE);
    SetCtrlAttribute (initP, INIT_TEXT_10, ATTR_VISIBLE, FALSE);
    SetCtrlAttribute (initP, INIT_CONTROL, ATTR_VISIBLE, FALSE);
    SetCtrlAttribute (initP, INIT_ANALYSIS, ATTR_VISIBLE, FALSE);
    SetCtrlAttribute (initP, INIT_EXIT, ATTR_VISIBLE, FALSE);

    GetCtrlAttribute (initP, INIT_TEXT, ATTR_TOP, &top);
    GetCtrlAttribute (initP, INIT_TEXT, ATTR_HEIGHT, &height);
    SetPanelAttribute (initP, ATTR_HEIGHT, top+height+6);
    util_ChangeInitMessage ("DAAS Utilities...");
}

void utilG_Exit(void)
{
    
    DiscardPanel (utilG.p);
}

int  util_HidePanelCallback(int panel, int event, void *callbackData, int eventData1, int eventData2)
{
    if (((event == EVENT_KEYPRESS) && (eventData1 == VAL_ESC_VKEY)) || (event == EVENT_RIGHT_DOUBLE_CLICK))
		HidePanel (panel);
	return 0;
}
int  util_DiscardPanelCallback(int panel, int event, void *callbackData, int eventData1, int eventData2)
{
    if (((event == EVENT_KEYPRESS) && (eventData1 == VAL_ESC_VKEY)) || (event == EVENT_RIGHT_DOUBLE_CLICK))
		HidePanel (panel);
	return 0;
}

void util_ChangeInitMessage (char *msg)
{
    int width;

    SetCtrlVal (initP, INIT_TEXT, msg);
}

void util_RemoveInitMessage (void)
{
    SetMouseCursor (VAL_DEFAULT_CURSOR);
    
    DiscardPanel (initP);
}

int  BGDoneCallback(int panel, int event, void *callbackData, int eventData1, int eventData2)
{
    if (event == EVENT_CLOSE)
        if (ConfirmPopup ("Exit DAAS?", "Are you so sure you want to exit DAAS?"))
        {
            //utilG.DiscardPanels();
            QuitUserInterface(0);   /* user selected "Exit" from the menu */
            exit(0);
        }
    return 0;
}
/***************************************************/
/*
formatParse() recogninzes %i, %f, %s, ¬ and [] if statements, where the [] requires a conditional variable
and if it's true everything in the [] will be printed. nesting is allowed. everything is passed by
reference, except the conditionals, which have to be (int).
example: 

rs232Write(rs232Ptr dev, char *format, ...);

rs232Write(dev, "i think i'll [write [this %s][ and ][that %s]]\n",
				cond1, cond2, msg1, (cond2 && cond3), cond3, msg2);

				
is the same as writing out:

Fmt(msg, "i think i'll ");
if(cond1)
{
	Fmt(msg, "%swrite ", msg)
	if(cond2 && !cond3)
		Fmt(msg, "%s this %s", msg, msg1);
	if(cond2 && cond3)
		Fmt(msg, "%s this %s and that %s", msg, msg1, msg2);
	if(!cond2 && cond3)
		Fmt(msg, "%s that %s", msg, msg2);
}
ComWrt(dev->COM->port, msg, StringLength(msg));
*/

void *util_formatParse(char *format, va_list *list, char *msg, int index)
{
	void *arg;
	char *temp = calloc(1024, sizeof(char));
	int i, cond;
	if(index < StringLength(format))
	{
		if (format[index] == '%')
		{
			index++;
			arg = va_arg(*list, void*);
			switch(format[index])
			{
				case 'i':Fmt(msg, "%s%i", msg, *((int *)   arg));break;
				case 'f':Fmt(msg, "%s%f", msg, *((double*) arg));break;
				case 's':Fmt(msg, "%s%s", msg, (char*)  arg);break;
			}
		}//*
		else if(format[index] == '[')
		{
			cond = va_arg(*list, int);
			index++;
			temp = util_formatParse(format, list, temp, index);
			Scan(temp, "%i,%s[t59]", &index, temp);
			if(cond)
			{
				Fmt(msg, "%s%s", msg, temp);
			}
		}//*/
		else if(format[index] == ']')
		{
			Fmt(temp, "%s<%i,%s;", index, msg);
			Fmt(msg, "%s", temp);
			return msg;
		}
		else
			Fmt(msg, "%s%c", msg, format[index]);
		index++;
		util_formatParse(format, list, msg, index);
	}	
	va_end(list);
	free(temp);
	return msg;
}
/*
formatParseRead() recognizes %i, %f, and %s. % cannot have whitespace, so for something like "two words"
2 char* would have to be used and appended later.
*/

void util_formatParseRead(char *format, va_list *list, char *msg, int index)
{
	void *arg;
	int i, cond;
	char temp;
	while(index < StringLength(format))
	{
		if (format[index] == '%')
		{
			index++;
			arg = va_arg(*list, void*);
			switch(format[index])
			{
				case 'i':Scan(msg, "%i%s[t59y]", (int*)		arg, msg);break;
				case 'f':Scan(msg, "%f%s[t59y]", (double*)	arg, msg);break;
				case 's':Scan(msg, "%s%s[t59y]", (char*)	arg, msg);break;
			}
			index++;
		}
		else
			Scan(msg, "%c%s[t59y]", &temp, msg);
		Fmt(msg, "%s;", msg);
		index++;
	}	
	va_end(list);
}

void util_MessagePopup(char* Title, char* Message, ...)
{
	va_list list;
	char msg[1024] = "";
	int width, pwidth, textX, okwidth, okx;
	va_start(list, Message);
	util_formatParse(Message, &list, msg, 0);
	utilG.err = LoadPanel(utilG.p, "utilu.uir", ERROR);
	SetPanelAttribute(utilG.err, ATTR_TITLE, Title);
	SetCtrlVal(utilG.err, ERROR_TEXT, msg);
	
	GetCtrlAttribute(utilG.err, ERROR_TEXT, ATTR_WIDTH, &width);
	GetCtrlAttribute(utilG.err, ERROR_TEXT, ATTR_LEFT, &textX);
	GetCtrlAttribute(utilG.err, ERROR_OK, ATTR_WIDTH, &okwidth);
	GetCtrlAttribute(utilG.err, ERROR_OK, ATTR_LEFT, &okx);

	pwidth = (width + textX * 2);
	SetPanelAttribute(utilG.err, ATTR_WIDTH, pwidth);
	SetCtrlAttribute(utilG.err, ERROR_OK, ATTR_LEFT, (pwidth/2 - okwidth/2));
	
	SetPanelPos (utilG.err, VAL_AUTO_CENTER, VAL_AUTO_CENTER);
	DisplayPanel(utilG.err);
	
}

int util_ErrorCloseCallback (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	if (event == EVENT_COMMIT)
		DiscardPanel(panel);
	return 0;
}

