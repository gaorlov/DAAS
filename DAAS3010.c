
// **************GPIB+USB+PCI+RS232 version************* 11-06-07
#include <cvintwrk.h>
#include <userint.h>
//password for instacal univ lib .exe is: mcc5_72
#include "util.h"
#include "list.h"
#include "channel.h"
#include "changen.h"
#include "chanfnc.h"
#include "chanops.h"
#include "acqchan.h"
#include "curve.h"
#include "acqcrv.h"
#include "graph.h"
#include "curveop.h"
#include "acquire.h"
#include "acquireu.h"
#include "gpibio.h"
#include "source.h"
#include "rs232util.h"
#include "MCCdevices.h"

#include "sr830.h"
#include "sr844.h"
#include "k236.h"
#include "k2000.h"
#include "k213.h"
#include "HP33120A.h"
#include "LS340.h"
#include "itc4.h"
#include "HP4156.h"
#include "DAS-6036.h"
#include "das-1602.h"
#include "dda08.h"
#include "USB-PMD1208LS.h"


int AcquireData (int panel, int control, int event, void *callbackData, int event1, int event2);

main (void)
{
    utilG_Init (gpibio_Exit);
    listG_Init ();
    channelG_Init();
    changen_Init();
    chanfunc_Init();
    chanops_Init();
    acqchan_Init();
    graphG_Init();
    curveop_Init();
    acquire_Init();
    gpibio_Init();
	rs232_Init();
    source_Init();
	
/*Don't change the order of these because the load function will not work properly*/
    k236_Init();
    k2000_Init();
    k213_Init();
	sr844_Init();
	sr830_Init();
	hp33120a_Init();
	LS340_Init();
	hp4156_Init();
/*rs232 instruments init*/

	itc4_Init();

/*PCI slot and USB Instacal compatible devices*/
	das6036_Init();
	das1602_Init();
	dda08_Init();
	usb1208ls_Init();
	init_MCCdevices();
/***********************************************************************************/    
    SetIdleEventRate (0);
    InstallMainCallback (AcquireData, 0, 1);
	
	utilG.err = 0;
    util_RemoveInitMessage();
    RunUserInterface();
}

int AcquireData (int panel, int control, int event, void *callbackData, int event1, int event2)
{
    if ((event == EVENT_IDLE) && (utilG.acq.status != ACQ_NONE)) {

        devPanel_UpdateReadings();

        if (utilG.acq.pt == utilG.acq.nPts)
            utilG.acq.status = ACQ_TERMINATE;

        switch (utilG.acq.status) {
            case ACQ_BEGIN:
                if (!exp_Begin()) utilG.acq.status = ACQ_STOPPED;
                acquire_UpdatePanel();
                break;
            case ACQ_BUSY:
                expG.DoExp();
                acquire_UpdateDataInfoPanel();
                util_IncAcqPt();
                break;
            case ACQ_TERMINATE:
                acqchanlist_CloseFile();
                graphlist_RemoveReadings();
                graphlist_PlotCurves();
                graphlist_AutoSave();
                acqchanlist_CopytoChannelList();
                acquire_IncDataFileExt();

                if (utilG.acq.pt == utilG.acq.nPts) utilG.acq.status = ACQ_DONE;
                else utilG.acq.status = ACQ_STOPPED;
                acquire_UpdatePanel();
                utilG.acq.pt = 0;
                break;
        }
    }
	if(event == EVENT_KEYPRESS && event1 == VAL_F1_VKEY)
	{
		InetLaunchDefaultWebBrowser ("help.html");
	}
    return 0;
	
}

