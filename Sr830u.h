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

#define  SR830_ADC                       1
#define  SR830_ADC_LABEL_4               2       /* callback function: ADCControlCallback */
#define  SR830_ADC_COEFF_4               3       /* callback function: ADCControlCallback */
#define  SR830_ADC_MEAS_4                4
#define  SR830_ADC_NOTE_4                5       /* callback function: ADCControlCallback */
#define  SR830_ADC_ACQ_4                 6       /* callback function: ADCControlCallback */
#define  SR830_ADC_LABEL_3               7       /* callback function: ADCControlCallback */
#define  SR830_ADC_COEFF_3               8       /* callback function: ADCControlCallback */
#define  SR830_ADC_NOTE_3                9       /* callback function: ADCControlCallback */
#define  SR830_ADC_MEAS_3                10
#define  SR830_ADC_ACQ_3                 11      /* callback function: ADCControlCallback */
#define  SR830_ADC_LABEL_2               12      /* callback function: ADCControlCallback */
#define  SR830_ADC_NOTE_2                13      /* callback function: ADCControlCallback */
#define  SR830_ADC_COEFF_2               14      /* callback function: ADCControlCallback */
#define  SR830_ADC_MEAS_2                15
#define  SR830_ADC_ACQ_2                 16      /* callback function: ADCControlCallback */
#define  SR830_ADC_NOTE_1                17      /* callback function: ADCControlCallback */
#define  SR830_ADC_LABEL_1               18      /* callback function: ADCControlCallback */
#define  SR830_ADC_COEFF_1               19      /* callback function: ADCControlCallback */
#define  SR830_ADC_MEAS_1                20
#define  SR830_ADC_ACQ_1                 21      /* callback function: ADCControlCallback */
#define  SR830_ADC_CLOSE                 22      /* callback function: ADCControlCallback */
#define  SR830_ADC_CONVERSION_4          23      /* callback function: ADCControlCallback */
#define  SR830_ADC_CONVERSION_3          24      /* callback function: ADCControlCallback */
#define  SR830_ADC_CONVERSION_2          25      /* callback function: ADCControlCallback */
#define  SR830_ADC_CONVERSION_1          26      /* callback function: ADCControlCallback */
#define  SR830_ADC_TEXT_1                27
#define  SR830_ADC_TEXT_2                28
#define  SR830_ADC_TEXT_3                29
#define  SR830_ADC_TEXT_4                30

#define  SR830_CTRL                      2       /* callback function: SR830ControlPanelCallback */
#define  SR830_CTRL_REF                  2       /* callback function: OperateReferenceCallback */
#define  SR830_CTRL_TC                   3       /* callback function: SR830ControlCallback */
#define  SR830_CTRL_AUTOSENSE            4       /* callback function: SR830ControlCallback */
#define  SR830_CTRL_SENS                 5       /* callback function: SR830ControlCallback */
#define  SR830_CTRL_XDISP                6
#define  SR830_CTRL_YDISP                7
#define  SR830_CTRL_FILTSLOPE            8       /* callback function: SR830ControlCallback */
#define  SR830_CTRL_SYNC                 9       /* callback function: SR830ControlCallback */
#define  SR830_CTRL_DYNRES               10      /* callback function: SR830ControlCallback */
#define  SR830_CTRL_REJECT               11      /* callback function: SR830ControlCallback */
#define  SR830_CTRL_INPUTOVERLOAD        12
#define  SR830_CTRL_FILTEROVERLOAD       13
#define  SR830_CTRL_OUTPUTOVERLOAD       14
#define  SR830_CTRL_MAG                  15
#define  SR830_CTRL_PHASE                16
#define  SR830_CTRL_HARMON               17      /* callback function: SR830ControlCallback */
#define  SR830_CTRL_HARMVAL              18      /* callback function: SR830ControlCallback */

#define  SR830_DAC                       3
#define  SR830_DAC_LABEL_4               2       /* callback function: DACControlCallback */
#define  SR830_DAC_COEFF_4               3       /* callback function: DACControlCallback */
#define  SR830_DAC_MEAS_4                4
#define  SR830_DAC_NOTE_4                5       /* callback function: DACControlCallback */
#define  SR830_DAC_ACQ_4                 6       /* callback function: DACControlCallback */
#define  SR830_DAC_LABEL_3               7       /* callback function: DACControlCallback */
#define  SR830_DAC_COEFF_3               8       /* callback function: DACControlCallback */
#define  SR830_DAC_NOTE_3                9       /* callback function: DACControlCallback */
#define  SR830_DAC_MEAS_3                10
#define  SR830_DAC_ACQ_3                 11      /* callback function: DACControlCallback */
#define  SR830_DAC_LABEL_2               12      /* callback function: DACControlCallback */
#define  SR830_DAC_NOTE_2                13      /* callback function: DACControlCallback */
#define  SR830_DAC_COEFF_2               14      /* callback function: DACControlCallback */
#define  SR830_DAC_MEAS_2                15
#define  SR830_DAC_ACQ_2                 16      /* callback function: DACControlCallback */
#define  SR830_DAC_NOTE_1                17      /* callback function: DACControlCallback */
#define  SR830_DAC_LABEL_1               18      /* callback function: DACControlCallback */
#define  SR830_DAC_COEFF_1               19      /* callback function: DACControlCallback */
#define  SR830_DAC_MEAS_1                20
#define  SR830_DAC_ACQ_1                 21      /* callback function: DACControlCallback */
#define  SR830_DAC_CLOSE                 22      /* callback function: DACControlCallback */
#define  SR830_DAC_TEXT_1                23
#define  SR830_DAC_TEXT_2                24
#define  SR830_DAC_TEXT_3                25
#define  SR830_DAC_TEXT_4                26

#define  SR830_REF                       4
#define  SR830_REF_PHASE                 2       /* callback function: SR830RefCallback */
#define  SR830_REF_FREQ                  3       /* callback function: SR830RefCallback */
#define  SR830_REF_AMPLITUDE             4       /* callback function: SR830RefCallback */
#define  SR830_REF_CLOSE                 5       /* callback function: util_DiscardCallback */

#define  SR830_XYMP                      5
#define  SR830_XYMP_PLABEL               2       /* callback function: XYMPControlCallback */
#define  SR830_XYMP_PCOEFF               3       /* callback function: XYMPControlCallback */
#define  SR830_XYMP_PMEAS                4
#define  SR830_XYMP_NOTE_4               5       /* callback function: XYMPControlCallback */
#define  SR830_XYMP_PACQ                 6       /* callback function: XYMPControlCallback */
#define  SR830_XYMP_MLABEL               7       /* callback function: XYMPControlCallback */
#define  SR830_XYMP_MCOEFF               8       /* callback function: XYMPControlCallback */
#define  SR830_XYMP_NOTE_3               9       /* callback function: XYMPControlCallback */
#define  SR830_XYMP_MMEAS                10
#define  SR830_XYMP_MACQ                 11      /* callback function: XYMPControlCallback */
#define  SR830_XYMP_YLABEL               12      /* callback function: XYMPControlCallback */
#define  SR830_XYMP_NOTE_2               13      /* callback function: XYMPControlCallback */
#define  SR830_XYMP_YCOEFF               14      /* callback function: XYMPControlCallback */
#define  SR830_XYMP_YMEAS                15
#define  SR830_XYMP_YACQ                 16      /* callback function: XYMPControlCallback */
#define  SR830_XYMP_NOTE_1               17      /* callback function: XYMPControlCallback */
#define  SR830_XYMP_XLABEL               18      /* callback function: XYMPControlCallback */
#define  SR830_XYMP_XCOEFF               19      /* callback function: XYMPControlCallback */
#define  SR830_XYMP_XMEAS                20
#define  SR830_XYMP_XACQ                 21      /* callback function: XYMPControlCallback */
#define  SR830_XYMP_CLOSE                22      /* callback function: XYMPControlCallback */
#define  SR830_XYMP_XTEXT                23
#define  SR830_XYMP_YTEXT                24
#define  SR830_XYMP_MTEXT                25
#define  SR830_XYMP_PTEXT                26


     /* Menu Bars, Menus, and Menu Items: */

#define  SR830MENUS                      1
#define  SR830MENUS_FILE                 2
#define  SR830MENUS_FILE_SAVE            3       /* callback function: SaveSR830SetupCallback */
#define  SR830MENUS_FILE_LOAD            4       /* callback function: LoadSR830SetupCallback */
#define  SR830MENUS_FILE_SEP_1           5
#define  SR830MENUS_FILE_GPIB            6       /* callback function: OperateDevCallback */
#define  SR830MENUS_SOURCES              7
#define  SR830MENUS_SOURCES_DAC1         8       /* callback function: DACCallback */
#define  SR830MENUS_SOURCES_DAC2         9       /* callback function: DACCallback */
#define  SR830MENUS_SOURCES_DAC3         10      /* callback function: DACCallback */
#define  SR830MENUS_SOURCES_DAC4         11      /* callback function: DACCallback */
#define  SR830MENUS_SOURCES_FREQ         12      /* callback function: FreqCallback */
#define  SR830MENUS_MEASURE              13
#define  SR830MENUS_MEASURE_LIA          14      /* callback function: MeasXYMPCallback */
#define  SR830MENUS_MEASURE_DACS         15      /* callback function: MeasDACCallback */
#define  SR830MENUS_MEASURE_ADCS         16      /* callback function: MeasADCCallback */


     /* Callback Prototypes: */ 

int  CVICALLBACK ADCControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK DACCallback(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK DACControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK FreqCallback(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK LoadSR830SetupCallback(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK MeasADCCallback(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK MeasDACCallback(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK MeasXYMPCallback(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK OperateDevCallback(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK OperateReferenceCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK SaveSR830SetupCallback(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK SR830ControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK SR830ControlPanelCallback(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK SR830RefCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK util_DiscardCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK XYMPControlCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
