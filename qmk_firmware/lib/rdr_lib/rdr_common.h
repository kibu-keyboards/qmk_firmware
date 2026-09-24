/*
 * Modified by KIBU (kibu-keyboards) <M@kibu.jp> on 2026-09-24.
 * Changes: Translated and clarified existing comments in English.
 * No executable code was changed by this comment revision.
 * Original authorship, copyright, and license notices are retained below.
 */
#pragma once

#include "quantum.h"
#include "debounce.h"
#include "usb_descriptor.h"
#include "usb_main.h"
#include "raw_hid.h"

/************************I/O pins**************************/
/************************I/O pins**************************/
/************************I/O pins**************************/
#define WHEEL_ZA_IO         (B2)
#define WHEEL_ZB_IO         (B10)

#define MODE_2P4G_IO        (B13)
#define MODE_BLE_IO         (B12)

#define ES_BATT_STDBY_IO    (A13)
#define ES_USB_POWER_IO     (C5)
#define ES_SPI_ACK_IO       (A4)
#define ES_PWM_DMA_IO       (A2)

#define ES_WUKEUP_IO        (D1)
#define ES_SDB_POWER_IO     (A3)
#define ES_LED_POWER_IO     (D0)

/************************SPI commands**************************/
/************************SPI commands**************************/
/************************SPI commands**************************/
#define USER_EMI_COMMAND	    0XBB
#define USER_KEYBOARD_COMMAND	0X0A
#define USER_KEYBOARD_LENGTH    (64)

#define USER_SWITCH_2P4G_MODE	0X00
#define USER_SWITCH_BLE_1_MODE	0X01
#define USER_SWITCH_BLE_2_MODE	0X02
#define USER_SWITCH_BLE_3_MODE	0X03
#define USER_SWITCH_2P4G_PAIR	0X04
#define USER_SWITCH_BLE_1_PAIR	0X05
#define USER_SWITCH_BLE_2_PAIR	0X06
#define USER_SWITCH_BLE_3_PAIR	0X07
#define USER_SWITCH_USB_MODE	0X08

#define USER_KEYBOARD_SLEEP		0X09
#define USER_KEYBOARD_WAKEUP	0X0A

#define USER_KEY_BYTE_DATA		0X0B
#define USER_KEY_BIT_DATA		0X0C
#define USER_MOUSE_DATA			0X0D
#define USER_CONSUMER_DATA		0X0E
#define USER_SYSTEM_DATA		0X0F

#define USER_BATTERY_DATA		0X10

#define USER_GET_RF_STATUS	    0X11

#define USER_BLE1_WRITE_NAME	0X12
#define USER_BLE2_WRITE_NAME    0X13
#define USER_BLE3_WRITE_NAME    0X14

#define USER_SLEEP_TIME_WRITE   0X15       // Sleep timeout
#define USER_DSLEEP_TIME_WRITE  0X16       // Deep-sleep timeout

#define USER_KEY_BYTE_LENGTH	0X08
#define USER_KEY_BIT_LENGTH		0X0F
#define USER_MOUSE_LENGTH		0X08
#define USER_CONSUMER_LENGTH	0X03
#define USER_SYSTEM_LENGTH		0X03
#define USER_BATTERY_LENGTH		0X02

#define KB_REPORT_ID            0x06    // Extend keyboard report ID.
#define SYS_REPORT_ID     	    0x03    // Extend System   report ID.
#define CON_REPORT_ID     	    0x04    // Extend Consumer report ID.
#define MOUSE_REPORT_ID  	    0x02    // Extend mouse	   report ID.

#define LOGO_LED_ENABLE         (1)
#define SIDE_LED_ENABLE         (0)
#define LATTICE_LED_ENABLE      (1)

/************************Core definitions**************************/
/************************Core definitions**************************/
/************************Core definitions**************************/
enum Custom_KeyModes {
    QMK_BLE_MODE = 0,
    QMK_2P4G_MODE,
    QMK_USB_MODE
};

enum Custom_BleChannels {
    QMK_BLE_CHANNEL_1 = 1,
    QMK_BLE_CHANNEL_2,
    QMK_BLE_CHANNEL_3
};

enum Custom_Spi_Ack_S {
    SPI_NACK,
    SPI_ACK
};

enum Custom_Spi_Busy_S {
    SPI_BUSY,
    SPI_IDLE
};

enum Custom_Ble_24G_Status_S {
    BLE_24G_NONE,
    BLE_24G_PIAR,
    BLE_24G_RETURN
};

typedef enum {
    KB_MODE_CONNECT_OK,  	        //Connected
    KB_MODE_CONNECT_PAIR,	        //Pairing
    KB_MODE_CONNECT_RETURN,	        //Reconnecting
} keyboard_System_state_e;

typedef enum {
    USER_SLEEP_PASS,	            //Sleep entry succeeded
    USER_SLEEP_FIAL,	            //Sleep entry failed
} keyboard_System_Sleep_Status_s;

/************************Default operating mode**************************/
/************************Default operating mode**************************/
/************************Default operating mode**************************/
#define INIT_WORK_MODE              (QMK_USB_MODE)                    // Default operating mode
#define INIT_BLE_CHANNEL            (QMK_BLE_CHANNEL_1)               // Default Bluetooth channel
#define INIT_BATT_NUMBER            (50)                              // Initial battery level at power-on

#define INIT_SIX_KEY                (0)                               // 6-key rollover
#define INIT_ALL_KEY                (1)                               // N-key rollover
#define INIT_ALL_SIX_KEY            (INIT_ALL_KEY)                    // N-key rollover

#define INIT_WIN_MODE               (0)                               // Windows
#define INIT_MAC_MODE               (1)                               // Mac
#define INIT_WIN_MAC_MODE           (INIT_WIN_MODE)                   // Windows

#define INIT_WIN_NLOCK              (0)                               // Windows key unlocked
#define INIT_WIN_LOCK               (1)                               // Windows key lock
#define INIT_WIN_LOCK_NLOCK         (INIT_WIN_NLOCK)                  // Windows key unlocked

#define INIT_OFF_CHANGE             (0)                               // Function/number-row swap disabled
#define INIT_ON_CHANGE              (1)                               // Function/number-row swap enabled
#define INIT_ON_OFF_CHANGE          (INIT_OFF_CHANGE)                 // Function/number-row swap disabled

#define INIT_LED_ON                 (0)                               // Backlighting enabled
#define INIT_LED_OFF                (1)                               // Backlighting disabled
#define INIT_LED_ON_OFF             (INIT_LED_ON)                     // Backlighting enabled

#define DEBOUNCE_DELAY_ONE          (2)                               // Debounce level 1
#define DEBOUNCE_DELAY_TWO          (5)                               // Debounce level 2
#define DEBOUNCE_DELAY_CLASS        (DEBOUNCE_DELAY_TWO)  

#define SLEEP_TIME_ONE              (60)                              // Sleep timeout: 1 minute
#define SLEEP_TIME_TWO              (180)                             // Sleep timeout: 3 minutes
#define SLEEP_TIME_THREE            (600)                             // Sleep timeout: 10 minutes
#define SLEEP_TIME_FOUR             (1800)                            // Sleep timeout: 30 minutes
#define SLEEP_TIME_CLASS            (SLEEP_TIME_TWO)                  // Default sleep timeout (SLEEP_TIME_TWO)

#define USER_DSLEEP_TIME            0x147AE0                          //Deep-sleep timeout, in seconds

#define USER_DEFINE_KEY         (QK_KB)
enum Custom_Keycodes {
    QMK_KB_MODE_2P4G = USER_DEFINE_KEY,
    QMK_KB_MODE_BLE1,
    QMK_KB_MODE_BLE2,
    QMK_KB_MODE_BLE3,
    QMK_KB_MODE_USB,
    QMK_BATT_NUM,
    QMK_WIN_LOCK,
    QMK_KB_SIX_N_CH,
    QMK_MAC_WIN_CH,
    QMK_FN_LCTRL_MODE,
    RGB_RTOG,
    U_EE_CLR,
    QMK_DEBOUNCE,
    QMK_TIME_SET,
    QMK_DTIME_SET,
    QMK_TEST_COLOUR,
#if LOGO_LED_ENABLE
    LOGO_TOG,
    LOGO_MOD,
    LOGO_RMOD,
    LOGO_HUI,
    LOGO_HUD,
    LOGO_SAI,
    LOGO_SAD,
    LOGO_VAI,
    LOGO_VAD,
    LOGO_SPI,
    LOGO_SPD,
#endif
#if SIDE_LED_ENABLE
    SIDE_TOG,
    SIDE_MOD,
    SIDE_RMOD,
    SIDE_HUI,
    SIDE_HUD,
    SIDE_SAI,
    SIDE_SAD,
    SIDE_VAI,
    SIDE_VAD,
    SIDE_SPI,
    SIDE_SPD,
#endif
#if LATTICE_LED_ENABLE
    LATTICE_TOG,
    LATTICE_MOD,
    LATTICE_RMOD,
    LATTICE_HUI,
    LATTICE_HUD,
    LATTICE_SAI,
    LATTICE_SAD,
    LATTICE_VAI,
    LATTICE_VAD,
    LATTICE_SPI,
    LATTICE_SPD,
#endif
    QMK_KB_2P4G_PAIR,
    QMK_KB_BLE1_PAIR,
    QMK_KB_BLE2_PAIR,
    QMK_KB_BLE3_PAIR
};

#define WIN_COL          (1)
#define WIN_ROL          (3)

#define MAC_COL          (2)
#define MAC_ROL          (3)

#define CHANGE_COL       (9)
#define CHANGE_ROL       (5)

#define KANA_COL         (10)
#define KANA_ROL         (5)

#define KC_K29 	         KC_BACKSLASH
#define KC_K42 	         KC_NONUS_HASH
#define KC_K45 	         KC_NONUS_BACKSLASH
#define KC_K56 	         KC_INTERNATIONAL_1
#define KC_K14           KC_INTERNATIONAL_3
#define KC_K132	         KC_INTERNATIONAL_4
#define KC_K131	         KC_INTERNATIONAL_5
#define KC_K133	         KC_INTERNATIONAL_2
#define KC_K151	         KC_LANGUAGE_1
#define KC_K150	         KC_LANGUAGE_2

#define MD_24G	         QMK_KB_MODE_2P4G
#define MD_BLE1	         QMK_KB_MODE_BLE1
#define MD_BLE2	         QMK_KB_MODE_BLE2
#define MD_BLE3	         QMK_KB_MODE_BLE3
#define MD_USB	         QMK_KB_MODE_USB
#define QK_BAT           QMK_BATT_NUM
#define QK_WLO	         QMK_WIN_LOCK
#define SIX_N	         QMK_KB_SIX_N_CH
#define MW_CH	         QMK_MAC_WIN_CH
#define CAP_CTRL         QMK_CAPS_TR_CTRL
#define FN_LCTRL         QMK_FN_LCTRL_MODE
#define KEY_DEB	         QMK_DEBOUNCE
#define TIME_ST	         QMK_TIME_SET
#define TIME_DT	         QMK_DTIME_SET
#define TEST_CL          QMK_TEST_COLOUR

/************************Core state variables**************************/
/************************Core state variables**************************/
/************************Core state variables**************************/
typedef struct {
    uint8_t Key_Mode;               // Keyboard operating mode
    uint8_t Ble_Channel;            // Bluetooth channel
    uint8_t Batt_Number;            // Battery level
    uint8_t Nkro;                   // 6-key / N-key rollover
    uint8_t Mac_Win_Mode;           // Mac / Windows mode
    uint8_t Win_Lock;               // Windows key lock
	uint8_t Key_Change_Func;        // Swap the function and number rows.
    uint8_t Led_On_Off;             // Backlight on/off state
    uint8_t Debounce_Delay;         // Key debounce
    uint32_t User_Sleep_Time;        // Sleep
    uint32_t User_DSleep_Time;       // Deep sleep
#if LOGO_LED_ENABLE
    uint8_t Logo_On_Off;            // Logo lighting on/off state
    uint8_t Logo_Mode;              // Logo lighting effect
    uint8_t Logo_Colour;            // Logo lighting hue
    uint8_t Logo_Saturation;        // Logo lighting saturation
    uint8_t Logo_Brightness;        // Logo lighting brightness
    uint8_t Logo_Speed;             // Logo animation speed
#endif
#if SIDE_LED_ENABLE
    uint8_t Side_On_Off;            // Side lighting on/off state
    uint8_t Side_Mode;              // Side lighting effect
    uint8_t Side_Colour;            // Side lighting hue
    uint8_t Side_Saturation;        // Side lighting saturation
    uint8_t Side_Brightness;        // Side lighting brightness
    uint8_t Side_Speed;             // Side animation speed
#endif
#if LATTICE_LED_ENABLE
    uint8_t Lattice_On_Off;         // Dot-matrix lighting on/off state
    uint8_t Lattice_Mode;           // Dot-matrix lighting effect
    uint8_t Lattice_Colour;         // Dot-matrix lighting hue
    uint8_t Lattice_Colour_Blue;    // Ice-blue lighting hue
    uint8_t Lattice_Saturation;     // Dot-matrix lighting saturation
    uint8_t Lattice_Brightness;     // Dot-matrix lighting brightness
    uint8_t Lattice_Speed;          // Dot-matrix animation speed
#endif
} Keyboard_Info_t;
extern Keyboard_Info_t Keyboard_Info;

typedef struct {
    uint8_t System_Work_Status;     // System state
    uint8_t System_Work_Mode;       // Operating mode
    uint8_t System_Work_Channel;    // Active channel
    uint8_t System_Connect_Status;  // Connection state
    uint8_t System_Led_Status;      // System status indicator
    uint8_t System_Sleep_Mode;      // System sleep state
} Keyboard_Status_t;
extern Keyboard_Status_t Keyboard_Status;

extern bool     Key_2p4g_Status;
extern bool     Key_Ble_1_Status;
extern bool     Key_Ble_2_Status;
extern bool     Key_Ble_3_Status;
extern bool     Key_Fn_Status;
extern bool     Key_Sys_Mode_Status;
extern uint16_t Win_Mac_Key_3s_Count;

extern bool User_Change_Flag;
extern bool User_Kana_Flag;

extern uint8_t  Systick_6ms_Count;
extern uint8_t  Systick_10ms_Count;
extern uint8_t  Systick_12ms_Count;
extern uint16_t Systick_Interval_Count;
extern uint16_t Time_3s_Count;

/************************Key debounce**************************/
/************************Key debounce**************************/
/************************Key debounce**************************/
extern unsigned int Debounce_Delay;        //Key debounce count; maximum 127
extern uint8_t Debounce_Point_Count;
extern uint16_t User_Key_3s_Count;
extern bool Debounce_Function_Count;
extern bool Debounce_Function_Status;

/************************Data queue**************************/
/************************Data queue**************************/
/************************Data queue**************************/
#define APP_2G4_BUF_SIZE            (24)
#define APP_2G4_BUF_CNT             (40)

extern uint8_t app_2g4_data[APP_2G4_BUF_CNT][APP_2G4_BUF_SIZE];
extern volatile uint8_t app_2g4_data_send;
extern volatile uint8_t app_2g4_data_rev;
extern uint8_t Spi_Main_Loop_Count;

uint8_t app_2g4_buffer_full(void);
uint8_t app_2g4_buffer_empty(void);
void app_2g4_buffer_rev_add(void);
void app_2g4_buffer_send_add(void);

/**************************EMI****************************/
/**************************EMI****************************/
/**************************EMI****************************/
extern bool Emi_Test_Start;
void Emi_Init(void);
void Emi_Read_Data(uint8_t *User_Data, uint8_t User_Length);
void Emi_Write_Data(uint8_t *User_Data, uint8_t User_Length);

/**************************SPI****************************/
/**************************SPI****************************/
/**************************SPI****************************/
#define SPI_DELAY_RF_TIME           (60)
#define SPI_DELAY_USB_TIME          (500 * 3)

#define MAX_NAME_LEN                (18)
#define USER_BlE_ID                 (0x112C)
#define USER_BlE1_NAME              "P75 JIS BT1"
#define USER_BlE2_NAME              "P75 JIS BT2"
#define USER_BlE3_NAME              "P75 JIS BT3"

extern volatile uint8_t Spi_Send_Recv_Flg;
extern volatile uint16_t Spi_Send_Recv_count;
extern uint16_t Spi_Interval;
extern uint8_t  g_es_spi_rx_buf[64];
extern uint8_t  g_es_spi_tx_buf[64];
extern uint8_t  Repet_Send_Count;
extern uint8_t  Send_Key_Type;
extern bool     Init_Spi_Power_Up;
extern uint8_t  Init_Spi_100ms_Delay;
extern bool     Ble_Name_Spi_Send;
extern uint8_t  Ble_Name_Spi_Count;
// uint16_t Init_Spi_Power_Count;           // +
// bool     Init_Spi_Power_Flag;            // +

extern const uint32_t g_es_dma_ch2pri_cfg;
extern const uint32_t g_es_dma_ch2alt_cfg;
extern const md_spi_inittypedef SPI2_InitStruct;

void es_ble_spi_init(void);
void es_ble_spi_deinit(void);
void es_spi_send_recv_by_dma(uint32_t num, uint8_t *rx_buf, uint8_t *tx_buf);
void Spi_Main_Loop(void);
void Spi_Send_Commad(uint8_t Commad);
uint8_t Spi_Ack_Send_Commad(uint8_t Commad);
void Get_Spi_Return_Data(uint8_t *Data);

extern int usbd_ep_start_write(const uint8_t ep, const uint8_t *data, uint32_t data_len);

/**************************Mode switching****************************/
/**************************Mode switching****************************/
/**************************Mode switching****************************/
extern volatile host_driver_t *es_qmk_driver;
extern const    host_driver_t es_user_driver;

uint8_t es_keyboard_leds(void);
void es_send_keyboard(report_keyboard_t *report);
void es_send_nkro(report_nkro_t *report);
void es_send_mouse(report_mouse_t *report);
void es_send_extra(report_extra_t *report);
void Mode_Synchronization(void);
void Ble_Name_Synchronization(void);
void User_bluetooth_send_keyboard(uint8_t *report, uint32_t len);

/************************Keyboard reset initialization variables********************/
/************************Keyboard reset initialization variables********************/
/************************Keyboard reset initialization variables********************/
extern uint16_t Time_3s_EE_CLR_Count;      // +
extern bool     Key_Sys_Mode_Flag;         // +
extern bool     User_QMK_EE_CLR_Flag;      // +
extern bool     User_EE_CLR_Start_Flag;    // +

/******************Custom sleep state variables********************/
/******************Custom sleep state variables********************/
/******************Custom sleep state variables********************/
extern uint8_t User_Sleep_Timer_Count;
extern bool    User_Sleep_Time_Send;
extern bool    User_DSleep_Time_Send;

void Sleep_Time_Synchronization(void);
void DSleep_Time_Synchronization(void);

/************************Keyboard state control variables********************/
/************************Keyboard state control variables********************/
/************************Keyboard state control variables********************/
extern uint16_t User_State_Flag;
extern uint16_t User_State_Count;
extern uint16_t User_State_Fulfill_Flag;
extern bool     User_State_EE_CLR_LED_Flag;
extern bool     User_State_Fulfill_LED_Flag;
extern bool     User_State_DEL_INS_Flag;

/************************Shift the 6-key report buffer after key release********************/
/************************Shift the 6-key report buffer after key release********************/
/************************Shift the 6-key report buffer after key release********************/
// void General_Key_Reorder(uint8_t Spot_Index);
// void del_key_from_report(uint8_t key);

/************************Key matrix variables********************/
/************************Key matrix variables********************/
/************************Key matrix variables********************/
// bool Key_Win_Status;

/************************Slide switch**************************/
/************************Slide switch**************************/
/************************Slide switch**************************/
extern uint8_t Key_Switch_Scan;
extern uint8_t Key_Switch_Check;
extern uint8_t Key_Switch_Old;
extern uint8_t Key_Switch_delay;
extern bool    Scan_Switch_Ok;

// uint8_t Key_Mac_Win_Scan;
// uint8_t Key_Mac_Win_Check;
// uint8_t Key_Mac_Win_Old;
// uint8_t Key_Mac_Win_delay;

void Key_Switch_Mode_Scan(void);
void Key_Switch_Mode_Power(void);

/**************************System functions****************************/
/**************************System functions****************************/
/**************************System functions****************************/
#define KEYBAORD_COL                (16)
#define KEYBAORD_ROL                (6)

#define MATRIX_USER_COL_PINS        { D15, D14, C15, C14, C13, D3, D2, C12, C11, C10, A14, C9, C8, C7, C6, B15 }
#define MATRIX_USER_ROW_PINS        { B0, B3, B4, B5, B6, B7}

extern bool     Save_Flash;
extern bool     Reset_Save_Flash;
extern uint16_t Save_Flash_3S_Count;
extern bool     Led_Rf_Pair_Flg;
extern bool     Usb_Change_Mode_Wakeup;
extern uint8_t  Temp_System_Led_Status;
extern bool     Mode_Synchronization_Signal;
extern uint16_t g_usb_sof_frame_id;
extern uint16_t g_usb_sof_frame_id_last;
extern bool     Usb_Dis_Connect;
extern uint16_t Usb_Suspend_Delay;
extern uint16_t Usb_Change_Mode_Delay;

void es_mcu_reset(void);
void bootloader_jump(void);
void mcu_reset(void);
// void User_Mac_Win_Change(void);
void User_Keyboard_Reset(void);
void Save_Flash_Set(void);
void User_Systime_Init(void);
void User_Systime_Deinit(void);
void Init_Gpio_Infomation(void);
void User_Sleep(void);
void User_Wakeup(void);
void Board_Wakeup_Init(void);
void es_chibios_user_idle_loop_hook(void);
void Init_Keyboard_Infomation(void);
void es_change_qmk_nkro_mode_enable(void);
void es_change_qmk_nkro_mode_disable(void);
void User_Keyboard_Init(void);
void User_Keyboard_Post_Init(void);

/************************USB attachment handling**************************/
/************************USB attachment handling**************************/
/************************USB attachment handling**************************/
void User_Usb_Init(void);
void es_restart_usb_driver(void);
void Usb_Disconnect(void);
void User_Usb_Deinit(void);

/**************************FLASH****************************/
/**************************FLASH****************************/
/**************************FLASH****************************/
void eeprom_driver_init(void);
void eeprom_write_block_user(const void *buf, void *addr, size_t len);
void eeprom_read_block_user(void *buf, const void *addr, size_t len);

/*************************Battery******************************/
/*************************Battery******************************/
/*************************Battery******************************/
#define USER_BATT_POWER_SCAN_COUNT  (10)
#define USER_BATT_SCAN_COUNT        (10)

#define USER_BATT_HIGH_POWER        (2555)      //Full charge: 2565 * 3.3 /4096 = 2.066, 4.13 V; the circuit has a voltage drop.
#define USER_BATT_LOW_POWER         (1970)      //Low battery: 2065 * 3.3 /4096 = 1.663, 3.32 V; a full battery is 4.2 V even with lighting off.
#define USER_BATT_STDOWN_POWER      (1865)      //Shutdown: 1865 * 3.3 /4096 = 1.502, 3.04 V; the board sees only about 4.1 V at full charge.

#define USER_BATT_DELAY_TIME        (100 * 25)  //25S
#define USER_TIME_3S_TIME           (300)       //3S
#define USER_TIME_2S_TIME           (200)       //2S
#define USER_KEY_SHORT_LONG         (50)        //500mS

extern uint16_t User_Adc_Batt[USER_BATT_SCAN_COUNT];
extern uint16_t User_Scan_Batt[USER_BATT_SCAN_COUNT];
extern uint8_t  User_Adc_Batt_Count;
extern uint8_t  User_Batt_BaiFen;
extern uint8_t  User_Batt_Old_BaiFen;
extern uint8_t  User_Batt_10ms_Count;
extern uint16_t User_Batt_Time_15S_Count;
extern bool     User_Batt_Power_Up;
extern bool     User_Batt_Send_Spi;
extern uint16_t User_Batt_Power_Up_Delay_100ms_Count;
extern bool     User_Batt_Power_Up_Delay;
extern bool     User_Power_Low;
extern uint8_t  User_Power_Low_Count;
extern uint8_t  es_stdby_pin_state;
extern bool     User_Key_Batt_Num_Show;
extern uint8_t  User_Key_Batt_Count;
extern uint8_t  Batt_Led_Count;

extern const md_adc_initial adc_initStruct;

void Init_Batt_Infomation(void);
void User_Adc_Init(void);
void User_Adc_Deinit(void);
void U16_Buff_Clear(uint16_t *Buff, uint8_t Len);
void Get_User_Adc_Batt_Power_Up_Init(void);
void Get_User_Adc_Batt_Number(void);

/************************Main lighting**************************/
/************************Main lighting**************************/
/************************Main lighting**************************/
#define ES_PWM_LED_SIZE         (42)
#define ES_PWM_LED_BYTE         (24)
#define ES_PWM_DMA_SIZE         (ES_PWM_LED_SIZE * ES_PWM_LED_BYTE)

#define ES_PWM_WS2812_H_VALUE   (43)
#define ES_PWM_WS2812_L_VALUE   (17)

#define U_PWM                   (RGB_MATRIX_MAXIMUM_BRIGHTNESS)

extern uint8_t Led_Colour_Tab[9][3];
extern uint8_t Led_Wave_Pwm_Tab[128];
extern uint8_t Led_Batt_Index_Tab[10];
extern uint8_t Led_Point_buf[10];

extern uint8_t  Systick_Led_Count;
extern uint8_t  Led_Point_Count;
extern uint8_t  Mac_Win_Point_Count;
extern uint8_t  INIT_ALL_SIX_KEY_Count;
extern uint8_t  INIT_ALL_KEY_Count;
extern bool     Led_Flash_Busy;
extern bool     Led_Off_Start;
extern bool     Led_Power_Up;
extern uint16_t Led_Power_Up_Delay;
extern bool     Usb_If_Ok_Led;
extern bool     Usb_If_Ok;         // +
extern bool     Led_Point_Sleep;   // +
extern uint16_t Usb_If_Ok_Delay;   // +
extern bool     Test_Led;          // +
extern uint8_t  Test_Colour;       // +

extern rgb_led_t rgb_matrix_ws2812_array[RGB_MATRIX_LED_COUNT];
extern uint8_t g_es_pwm_rgb_matrix_array_dma_buf[(RGB_MATRIX_LED_COUNT * ES_PWM_LED_BYTE) + 2];
extern md_dma_channel_config_typedef DMA_list[5];
extern const rgb_matrix_driver_t rgb_matrix_driver;

void rgb_matrix_driver_init(void);
void User_Pwm_Deinit(void);
void rgb_matrix_driver_flush_pwm_dma_start(void);
void rgb_matrix_driver_flush(void);
void rgb_matrix_driver_set_color(int index, uint8_t r, uint8_t g, uint8_t b);
void rgb_matrix_driver_set_color_all(uint8_t r, uint8_t g, uint8_t b);

#define LED_LOW_P_INDEX     (81)
#define LED_CAP_INDEX       (43)
#define LED_WIN_L_INDEX     (74)
#define LED_DEB_INDEX       (49)
#define LED_SLEEP_ST_INDEX  (34)

#define LED_BLE_1_INDEX     (14)
#define LED_BLE_2_INDEX     (15)
#define LED_BLE_3_INDEX     (16)
#define LED_2P4G_INDEX      (17)
#define LED_USB_INDEX       (18)

void Led_Power_Low_Show(void);
void Led_Rf_Mode_Show(void);
void Led_Batt_Number_Show(void);
void Led_Point_Flash_Show(void);
void User_Led_Show(void);
void User_Get_Led_Power_Status(void);
void Led_Charge_Show(void);
void User_Clear_Board_2ms(void); 

/************************Side lighting**************************/
/************************Side lighting**************************/
/************************Side lighting**************************/
#if LOGO_LED_ENABLE
#define LOGO_LED_PLAY_SPEED	        (0)                                                     // Lighting refresh rate
#define LOGO_LED_SIZE	            (33)                                                    // LED count

#define LOGO_LED_ON                 (0)                                                     // Lighting enabled
#define LOGO_LED_OFF                (1)                                                     // Lighting disabled

#define LOGO_WAVE_RGB_MODE          (1)                                                     // Rainbow wave
#define LOGO_WAVE_DS_MODE           (2)                                                     // Single-color wave
#define LOGO_SPECTRUM_MODE          (3)                                                     // Spectrum
#define LOGO_BREATH_MODE            (4)                                                     // Single-color breathing
#define LOGO_LIGHT_MODE             (5)                                                     // Static single color
#define LOGO_OFF_MODE               (6)                                                     // Off

#define LOGO_MAX_COLOUR             (255)                                                   // Maximum hue
#define LOGO_MIN_COLOUR             (0)                                                     // Minimum hue
#define COLOUR_LEVEL                (15)                                                    // Hue step

#define LOGO_MAX_SATURATION         (0)                                                     // Maximum saturation
#define LOGO_MIN_SATURATION         (255)                                                   // Minimum saturation
#define SATURATION_LEVEL            (15)                                                    // Saturation step

#define LOGO_MAX_BRIGHTNESS         (RGB_MATRIX_MAXIMUM_BRIGHTNESS)                         // Maximum brightness
#define LOGO_MIN_BRIGHTNESS         (0)                                                     // Minimum brightness
#define BRIGHTNESS_LEVEL            (25)                                                    // Brightness step

#define LOGO_MAX_SPEED              (4)                                                     // Maximum speed
#define LOGO_MIN_SPEED              (0)                                                     // Minimum speed
#define SPEED_LEVEL                 (1)                                                     // Speed step

#define INIT_LOGO_ON_OFF            (LOGO_LED_ON)                                           // Lighting enabled
#define INIT_LOGO_MODE              (LOGO_WAVE_RGB_MODE)                                    // Rainbow wave
#define INIT_LOGO_COLOUR            (LOGO_MIN_COLOUR)                                       // Minimum hue
#define INIT_LOGO_SATURATION        (LOGO_MAX_SATURATION)                                   // Maximum saturation
#define INIT_LOGO_BRIGHTNESS        (LOGO_MAX_BRIGHTNESS - (4 * BRIGHTNESS_LEVEL))          // Default logo brightness
#define INIT_LOGO_SPEED             (2)                                                     // Medium speed

extern uint8_t Logo_Flash_Count;
extern uint8_t Logo_Led_Count;
void Logo_Init(void);
void Logo_Mode_Show(void);
void User_Via_Qmk_Logo_Get_Value(uint8_t *data);
void User_Via_Qmk_Logo_Set_Value(uint8_t *data);
void User_Via_Qmk_Logo_Command(uint8_t *data, uint8_t length);

#endif
//--------------------------------------------------------------------------------------------------------
#if SIDE_LED_ENABLE
#define SIDE_LED_PLAY_SPEED	        (0)                                                     // Lighting refresh rate
#define SIDE_LED_GROUP              (7)                                                     // LED group count
#define SIDE_LED_SIZE	            (7)                                                     // LED count

#define SIDE_LED_ON                 (0)                                                     // Lighting enabled
#define SIDE_LED_OFF                (1)                                                     // Lighting disabled

#define SIDE_HEART_MODE             (1)                                                     // Static heart
#define SIDE_SCAN_MODE              (2)                                                     // Scan effect
#define SIDE_TYPE_MODE              (3)                                                     // Key-triggered effect
#define SIDE_RAIN_MODE              (4)                                                     // Falling rain
#define SIDE_TER_MODE               (5)                                                     // Terminal effect
#define SIDE_USER_MODE              (6)                                                     // Custom effect
#define SIDE_CIRCLE_MODE            (7)                                                     // Sharingan effect
#define SIDE_RANDOM_MODE            (8)                                                     // Random raindrop effect
#define SIDE_OFF_MODE               (9)                                                     // Off
#define SIDE_LIGHT_MODE_COUNT       (SIDE_CIRCLE_MODE)                                      // Lighting effect count

#define SIDE_MAX_COLOUR             (255)                                                   // Maximum hue
#define SIDE_BLUE_COLOUR            (150)                                                   // Ice blue
#define SIDE_MIN_COLOUR             (0)                                                     // Minimum hue
#define SIDE_COLOUR_LEVEL           (15)                                                    // Hue step

#define SIDE_MAX_SATURATION         (0)                                                     // Maximum saturation
#define SIDE_MIN_SATURATION         (255)                                                   // Minimum saturation
#define SIDE_SATURATION_LEVEL       (15)                                                    // Saturation step

#define SIDE_MAX_BRIGHTNESS         (RGB_MATRIX_MAXIMUM_BRIGHTNESS)                         // Maximum brightness
#define SIDE_MIN_BRIGHTNESS         (0)                                                     // Minimum brightness
#define SIDE_BRIGHTNESS_LEVEL       (14)                                                    // Brightness step

#define SIDE_MAX_SPEED              (4)                                                     // Maximum speed
#define SIDE_MIN_SPEED              (0)                                                     // Minimum speed
#define SIDE_SPEED_LEVEL            (1)                                                     // Speed step

#define INIT_SIDE_ON_OFF            (SIDE_LED_ON)                                           // Lighting enabled
#define INIT_SIDE_MODE              (SIDE_HEART_MODE)                                       // Heart effect
#define INIT_SIDE_COLOUR            (SIDE_MIN_COLOUR)                                       // Minimum hue
#define INIT_SIDE_COLOUR_BLUE       (SIDE_BLUE_COLOUR)                                      // Ice blue
#define INIT_SIDE_SATURATION        (SIDE_MAX_SATURATION)                                   // Maximum saturation
#define INIT_SIDE_BRIGHTNESS        (SIDE_BRIGHTNESS_LEVEL)                                 // Default side-lighting brightness
#define INIT_SIDE_SPEED             (2)                                                     // Medium speed

void User_Via_Qmk_Side_Get_Value(uint8_t *data);
void User_Via_Qmk_Side_Set_Value(uint8_t *data);
void User_Via_Qmk_Side_Command(uint8_t *data, uint8_t length);
void Side_Init(void);
void Side_Mode_Show(void);

#endif

/******************************Dot-matrix lighting**************************************/
#if LATTICE_LED_ENABLE
#define LATTICE_LED_GROUP           (7)                                                     // LED group count
#define LATTICE_LED_SIZE	        (7)                                                     // LED count

#define LATTICE_LED_ON              (0)                                                     // Lighting enabled
#define LATTICE_LED_OFF             (1)                                                     // Lighting disabled

#define LATTICE_HEART_MODE          (0)                                                     // Static heart
#define LATTICE_SCAN_MODE           (1)                                                     // Scan effect
#define LATTICE_TYPE_MODE           (2)                                                     // Key-triggered effect
#define LATTICE_RAIN_MODE           (3)                                                     // Raindrop effect
#define LATTICE_TER_MODE            (4)                                                     // Terminal effect
#define LATTICE_USER_MODE           (5)                                                     // Custom effect
#define LATTICE_CIRCLE_MODE         (6)                                                     // Radiating rays
#define LATTICE_OFF_MODE            (7)                                                     // Off
#define LATTICE_LIGHT_MODE_COUNT    (LATTICE_OFF_MODE + 1)                                  // Lighting effect count

#define LATTICE_MAX_COLOUR          (255)                                                   // Maximum hue
#define LATTICE_BLUE_COLOUR         (150)                                                   // Ice blue
#define LATTICE_MIN_COLOUR          (0)                                                     // Minimum hue
#define LATTICE_COLOUR_LEVEL        (15)                                                    // Hue step

#define LATTICE_MAX_SATURATION      (0)                                                     // Maximum saturation
#define LATTICE_MIN_SATURATION      (255)                                                   // Minimum saturation
#define LATTICE_SATURATION_LEVEL    (15)                                                    // Saturation step

#define LATTICE_MAX_BRIGHTNESS      (60)                                                    // Maximum brightness
#define LATTICE_MIN_BRIGHTNESS      (0)                                                     // Minimum brightness
#define LATTICE_BRIGHTNESS_LEVEL    (15)                                                    // Brightness step

#define LATTICE_MAX_SPEED           (4)                                                     // Maximum speed
#define LATTICE_MIN_SPEED           (0)                                                     // Minimum speed
#define LATTICE_SPEED_LEVEL         (1)                                                     // Speed step

#define INIT_LATTICE_ON_OFF         (LATTICE_LED_ON)                                        // Lighting enabled
#define INIT_LATTICE_MODE           (LATTICE_HEART_MODE)                                    // Static heart
#define INIT_LATTICE_COLOUR         (LATTICE_MIN_COLOUR)                                    // Minimum hue
#define INIT_LATTICE_COLOUR_BLUE    (LATTICE_BLUE_COLOUR)                                   // Ice blue
#define INIT_LATTICE_SATURATION     (LATTICE_MAX_SATURATION)                                // Maximum saturation
#define INIT_LATTICE_BRIGHTNESS     (LATTICE_BRIGHTNESS_LEVEL)                              // Default dot-matrix brightness
#define INIT_LATTICE_SPEED          (2)                                                     // Medium speed

extern uint8_t Lattice_Power_Flag;
extern uint8_t Lattice_Led_Count;
extern uint8_t Lattice_Power_Up_Count;
extern uint8_t Lattice_Heart_Count;
void Lattice_Init(void);
void Lattice_Mode_Show(void);
void User_Via_Qmk_Lattice_Get_Value(uint8_t *data);
void User_Via_Qmk_Lattice_Set_Value(uint8_t *data);
void User_Via_Qmk_Lattice_Command(uint8_t *data, uint8_t length);

// Dot-matrix lighting
// ***************************** R1
#define  LT_R1_1_INDEX  118
#define  LT_R1_2_INDEX  119
#define  LT_R1_3_INDEX  120
#define  LT_R1_4_INDEX  121
#define  LT_R1_5_INDEX  122
#define  LT_R1_6_INDEX  123
#define  LT_R1_7_INDEX  124
// ***************************** R2
#define  LT_R2_1_INDEX  125
#define  LT_R2_2_INDEX  126
#define  LT_R2_3_INDEX  127
#define  LT_R2_4_INDEX  128
#define  LT_R2_5_INDEX  129
#define  LT_R2_6_INDEX  130
#define  LT_R2_7_INDEX  131
// ***************************** R3
#define  LT_R3_1_INDEX  132
#define  LT_R3_2_INDEX  133
#define  LT_R3_3_INDEX  134
#define  LT_R3_4_INDEX  135
#define  LT_R3_5_INDEX  136
#define  LT_R3_6_INDEX  137
#define  LT_R3_7_INDEX  138
// ***************************** R4
#define  LT_R4_1_INDEX  139
#define  LT_R4_2_INDEX  140
#define  LT_R4_3_INDEX  141
#define  LT_R4_4_INDEX  142
#define  LT_R4_5_INDEX  143
#define  LT_R4_6_INDEX  144
#define  LT_R4_7_INDEX  145
// ***************************** R5
#define  LT_R5_1_INDEX  146
#define  LT_R5_2_INDEX  147
#define  LT_R5_3_INDEX  148
#define  LT_R5_4_INDEX  149
#define  LT_R5_5_INDEX  150
#define  LT_R5_6_INDEX  151
#define  LT_R5_7_INDEX  152
// ***************************** R6
#define  LT_R6_1_INDEX  153
#define  LT_R6_2_INDEX  154
#define  LT_R6_3_INDEX  155
#define  LT_R6_4_INDEX  156
#define  LT_R6_5_INDEX  157
#define  LT_R6_6_INDEX  158
#define  LT_R6_7_INDEX  159
// ***************************** R7
#define  LT_R7_1_INDEX  160
#define  LT_R7_2_INDEX  161
#define  LT_R7_3_INDEX  162
#define  LT_R7_4_INDEX  163
#define  LT_R7_5_INDEX  164
#define  LT_R7_6_INDEX  165
#define  LT_R7_7_INDEX  166

#endif
