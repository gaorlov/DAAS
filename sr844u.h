/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/* Copyright (c) National Instruments 2007. All Rights Reserved.          */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  SR844_ADC                       1       /* callback function: util_HidePanelCallback */
#define  SR844_ADC_LABEL_2               2       /* callback function: ADCControlCallback844 */
#define  SR844_ADC_NOTE_2                3       /* callback function: ADCControlCallback844 */
#define  SR844_ADC_COEFF_2               4       /* callback function: ADCControlCallback844 */
#define  SR844_ADC_MEAS_2                5
#define  SR844_ADC_ACQ_2                 6       /* callback function: ADCControlCallback844 */
#define  SR844_ADC_NOTE_1                7       /* callback function: ADCControlCallback844 */
#define  SR844_ADC_LABEL_1               8       /* callback function: ADCControlCallback844 */
#define  SR844_ADC_COEFF_1               9       /* callback function: ADCControlCallback844 */
#define  SR844_ADC_MEAS_1                10
#define  SR844_ADC_ACQ_1                 11      /* callback function: ADCControlCallback844 */
#define  SR844_ADC_CLOSE                 12      /* callback function: ADCControlCallback844 */
#define  SR844_ADC_CONVERSION_2          13      /* callback function: ADCControlCallback844 */
#define  SR844_ADC_CONVERSION_1          14      /* callback function: ADCControlCallback844 */
#define  SR844_ADC_TEXT_1                15
#define  SR844_ADC_TEXT_2                16

#define  SR844_CTRL                      2       /* callback function: SR844ControlPanelCallback */
#define  SR844_CTRL_TC                   2       /* callback function: SR844ControlCallback */
#define  SR844_CTRL_AUTOSENSE            3       /* callback function: SR844ControlCallback */
#define  SR844_CTRL_SENS                 4       /* callback function: SR844ControlCallback */
#define  SR844_CTRL_XDISP                5
#define  SR844_CTRL_YDISP                6
#define  SR844_CTRL_FILTSLOPE            7       /* callback function: SR844ControlCallback */
#define  SR844_CTRL_RV                   8
#define  SR844_CTRL_PHASE                9
#define  SR844_CTRL_THETA                10
#define  SR844_CTRL_REF                  11      /* callback function: OperateReferenceCallback844 */
#define  SR844_CTRL_GPIBADDR             12
#define  SR844_CTRL_1MO50O               13      /* callback function: SR844ControlCallback */

#define  SR844_REF                       3       /* callback function: util_HidePanelCallback */
#define  SR844_REF_PHASE                 2       /* callback function: SR844RefCallback */
#define  SR844_REF_FREQ                  3       /* callback function: SR844RefCallback */
#define  SR844_REF_CLOSE                 4       /* callback function: util_DiscardCallback */

#define  SR844_XYMP                      4       /* callback function: util_HidePanelCallback */
#define  SR844_XYMP_PLABEL               2       /* callback function: XYMPControlCallback844 */
#define  SR844_XYMP_PCOEFF               3       /* callback function: XYMPControlCallback844 */
#define  SR844_XYMP_PMEAS                4
#define  SR844_XYMP_NOTE_4               5       /* callback function: XYMPControlCallback844 */
#define  SR844_XYMP_PACQ                 6       /* callback function: XYMPControlCallback844 */
#define  SR844_XYMP_RLABEL               7       /* callback function: XYMPControlCallback844 */
#define  SR844_XYMP_RCOEFF               8       /* callback function: XYMPControlCallback844 */
#define  SR844_XYMP_NOTE_3               9       /* callback function: XYMPControlCallback844 */
#define  SR844_XYMP_RMEAS                10
#define  SR844_XYMP_RACQ                 11      /* callback function: XYMPControlCallback844 */
#define  SR844_XYMP_YLABEL               12      /* callback function: XYMPControlCallback844 */
#define  SR844_XYMP_NOTE_2               13      /* callback function: XYMPControlCallback844 */
#define  SR844_XYMP_YCOEFF               14      /* callback function: XYMPControlCallback844 */
#define  SR844_XYMP_YMEAS                15
#define  SR844_XYMP_YACQ                 16      /* callback function: XYMPControlCallback844 */
#define  SR844_XYMP_NOTE_1               17      /* callback function: XYMPControlCallback844 */
#define  SR844_XYMP_XLABEL               18      /* callback function: XYMPControlCallback844 */
#define  SR844_XYMP_XCOEFF               19      /* callback function: XYMPControlCallback844 */
#define  SR844_XYMP_XMEAS                20
#define  SR844_XYMP_XACQ                 21      /* callback function: XYMPControlCallback844 */
#define  SR844_XYMP_CLOSE                22      /* callback function: XYMPControlCallback844 */
#define  SR844_XYMP_XTEXT                23
#define  SR844_XYMP_YTEXT                24
#define  SR844_XYMP_RTEXT                25
#define  SR844_XYMP_PTEXT                26


     /* Menu Bars, Menus, and Menu Items: */

#define  SR844MENU                       1
#define  SR844MENU_SOURCES               2
#define  SR844MENU_SOURCES_DAC1          3       /* callback function: sr844_MenuCallback */
#define  SR844MENU_SOURCES_DAC2          4       /* callback function: sr844_MenuCallback */
#define  SR844MENU_SOURCES_FREQ          5       /* callback function: sr844_MenuCallback */
#define  SR844MENU_MEASURE               6
#define  SR844MENU_MEASURE_LIA           7       /* callback function: sr844_MenuCallback */
#define  SR844MENU_MEASURE_ADCS          8       /* callback function: sr844_MenuCallback */


     /* Callback Prototypes: */ 

int  CVICALLBACK ADCControlCallback844(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OperateReferenceCallback844(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK sr844_MenuCallback(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK SR844ControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK SR844ControlPanelCallback(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK SR844RefCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK util_DiscardCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK util_HidePanelCallback(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK XYMPControlCallback844(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
