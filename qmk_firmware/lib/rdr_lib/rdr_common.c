// SPDX-License-Identifier: GPL-2.0-or-later

#include "rdr_common.h"

#include "action_util.h"
#include "host.h"
#include "keycode_config.h"

#include <string.h>

/*
 * Source implementation of the FS026-side keyboard, SPI transport, power,
 * battery, and wake logic. The firmware links this file directly and does not
 * require the former vendor binary library.
 */

Keyboard_Info_t Keyboard_Info = {
    .Key_Mode            = INIT_WORK_MODE,
    .Ble_Channel         = INIT_BLE_CHANNEL,
    .Batt_Number         = INIT_BATT_NUMBER,
    .Nkro                = INIT_ALL_SIX_KEY,
    .Mac_Win_Mode        = INIT_WIN_MAC_MODE,
    .Win_Lock            = INIT_WIN_LOCK_NLOCK,
    .Key_Change_Func     = INIT_ON_OFF_CHANGE,
    .Led_On_Off          = INIT_LED_ON_OFF,
    .Debounce_Delay      = DEBOUNCE_DELAY_CLASS,
    .User_Sleep_Time     = SLEEP_TIME_CLASS,
    .User_DSleep_Time    = USER_DSLEEP_TIME,
#if LOGO_LED_ENABLE
    .Logo_On_Off         = INIT_LOGO_ON_OFF,
    .Logo_Mode           = INIT_LOGO_MODE,
    .Logo_Colour         = INIT_LOGO_COLOUR,
    .Logo_Saturation     = INIT_LOGO_SATURATION,
    .Logo_Brightness     = INIT_LOGO_BRIGHTNESS,
    .Logo_Speed          = INIT_LOGO_SPEED,
#endif
#if SIDE_LED_ENABLE
    .Side_On_Off         = INIT_SIDE_ON_OFF,
    .Side_Mode           = INIT_SIDE_MODE,
    .Side_Colour         = INIT_SIDE_COLOUR,
    .Side_Saturation     = INIT_SIDE_SATURATION,
    .Side_Brightness     = INIT_SIDE_BRIGHTNESS,
    .Side_Speed          = INIT_SIDE_SPEED,
#endif
#if LATTICE_LED_ENABLE
    .Lattice_On_Off      = INIT_LATTICE_ON_OFF,
    .Lattice_Mode        = INIT_LATTICE_MODE,
    .Lattice_Colour      = INIT_LATTICE_COLOUR,
    .Lattice_Colour_Blue = INIT_LATTICE_COLOUR_BLUE,
    .Lattice_Saturation  = INIT_LATTICE_SATURATION,
    .Lattice_Brightness  = INIT_LATTICE_BRIGHTNESS,
    .Lattice_Speed       = INIT_LATTICE_SPEED,
#endif
};

Keyboard_Status_t Keyboard_Status;

bool     Key_2p4g_Status;
bool     Key_Ble_1_Status;
bool     Key_Ble_2_Status;
bool     Key_Ble_3_Status;
bool     Key_Fn_Status;
bool     Key_Sys_Mode_Status;
uint16_t Win_Mac_Key_3s_Count;
bool     User_Change_Flag;
bool     User_Kana_Flag;

uint8_t  Systick_6ms_Count;
uint8_t  Systick_10ms_Count;
uint8_t  Systick_12ms_Count;
uint16_t Systick_Interval_Count;
uint16_t Time_3s_Count;

unsigned int Debounce_Delay;
uint8_t      Debounce_Point_Count;
uint16_t     User_Key_3s_Count;
bool         Debounce_Function_Count;
bool         Debounce_Function_Status;

uint8_t          app_2g4_data[APP_2G4_BUF_CNT][APP_2G4_BUF_SIZE];
volatile uint8_t app_2g4_data_send;
volatile uint8_t app_2g4_data_rev;
uint8_t          Spi_Main_Loop_Count;

bool Emi_Test_Start;

volatile uint8_t  Spi_Send_Recv_Flg;
volatile uint16_t Spi_Send_Recv_count;
uint16_t          Spi_Interval = SPI_DELAY_RF_TIME;
uint8_t           g_es_spi_rx_buf[USER_KEYBOARD_LENGTH];
uint8_t           g_es_spi_tx_buf[USER_KEYBOARD_LENGTH];
uint8_t           Repet_Send_Count;
uint8_t           Send_Key_Type;
bool              Init_Spi_Power_Up = true;
uint8_t           Init_Spi_100ms_Delay;
bool              Ble_Name_Spi_Send;
uint8_t           Ble_Name_Spi_Count = 1;

const uint32_t g_es_dma_ch2pri_cfg = 0xaa008006;
const uint32_t g_es_dma_ch2alt_cfg = 0xc0000007;
const md_spi_inittypedef SPI2_InitStruct = {
    .Mode              = MD_SPI_MODE_MASTER,
    .ClockPhase        = MD_SPI_PHASE_2EDGE,
    .ClockPolarity     = MD_SPI_POLARITY_HIGH,
    .BaudRate          = MD_SPI_BAUDRATEPRESCALER_DIV8,
    .BitOrder          = MD_SPI_MSB_FIRST,
    .TransferDirection = MD_SPI_FULL_DUPLEX,
    .DataWidth         = MD_SPI_FRAME_FORMAT_8BIT,
    .NSS               = MD_SPI_NSS_HARD,
    .CRCCalculation    = MD_SPI_CRCCALCULATION_DISABLE,
    .CRCPoly           = 0,
};

volatile host_driver_t *es_qmk_driver;
const host_driver_t es_user_driver = {
    .keyboard_leds = es_keyboard_leds,
    .send_keyboard = es_send_keyboard,
    .send_nkro     = es_send_nkro,
    .send_mouse    = es_send_mouse,
    .send_extra    = es_send_extra,
};

uint16_t Time_3s_EE_CLR_Count;
bool     Key_Sys_Mode_Flag;
bool     User_QMK_EE_CLR_Flag;
bool     User_EE_CLR_Start_Flag;

uint8_t User_Sleep_Timer_Count;
bool    User_Sleep_Time_Send;
bool    User_DSleep_Time_Send;

uint16_t User_State_Flag;
uint16_t User_State_Count;
uint16_t User_State_Fulfill_Flag;
bool     User_State_EE_CLR_LED_Flag;
bool     User_State_Fulfill_LED_Flag;
bool     User_State_DEL_INS_Flag;

uint8_t Key_Switch_Scan;
uint8_t Key_Switch_Check;
uint8_t Key_Switch_Old;
uint8_t Key_Switch_delay;
bool    Scan_Switch_Ok;

bool     Save_Flash;
bool     Reset_Save_Flash;
uint16_t Save_Flash_3S_Count;
bool     Led_Rf_Pair_Flg;
bool     Usb_Change_Mode_Wakeup;
uint8_t  Temp_System_Led_Status = 0xff;
bool     Mode_Synchronization_Signal;
uint16_t g_usb_sof_frame_id;
uint16_t g_usb_sof_frame_id_last;
bool     Usb_Dis_Connect;
uint16_t Usb_Suspend_Delay;
uint16_t Usb_Change_Mode_Delay;

uint16_t User_Adc_Batt[USER_BATT_SCAN_COUNT];
uint16_t User_Scan_Batt[USER_BATT_SCAN_COUNT];
uint8_t  User_Adc_Batt_Count;
uint8_t  User_Batt_BaiFen;
uint8_t  User_Batt_Old_BaiFen;
uint8_t  User_Batt_10ms_Count;
uint16_t User_Batt_Time_15S_Count;
bool     User_Batt_Power_Up;
bool     User_Batt_Send_Spi;
uint16_t User_Batt_Power_Up_Delay_100ms_Count;
bool     User_Batt_Power_Up_Delay;
bool     User_Power_Low;
uint8_t  User_Power_Low_Count;
uint8_t  es_stdby_pin_state;
bool     User_Key_Batt_Num_Show;
uint8_t  User_Key_Batt_Count;
uint8_t  Batt_Led_Count;

uint8_t Led_Colour_Tab[9][3] = {
    {0xff, 0x00, 0x00},
    {0xff, 0x80, 0x00},
    {0xff, 0xff, 0x00},
    {0x00, 0xff, 0x00},
    {0x00, 0xff, 0xff},
    {0x00, 0x00, 0xff},
    {0x80, 0x00, 0xff},
    {0xff, 0xff, 0xff},
    {0x00, 0x00, 0x00},
};

uint8_t Led_Wave_Pwm_Tab[128] = {
    0x00, 0x04, 0x08, 0x0c, 0x10, 0x14, 0x18, 0x1c, 0x20, 0x24, 0x28, 0x2c, 0x30, 0x34, 0x38, 0x3c,
    0x40, 0x44, 0x48, 0x4c, 0x50, 0x54, 0x58, 0x5c, 0x60, 0x64, 0x68, 0x6c, 0x70, 0x74, 0x78, 0x7c,
    0x80, 0x84, 0x88, 0x8c, 0x90, 0x94, 0x98, 0x9c, 0xa0, 0xa4, 0xa8, 0xac, 0xb0, 0xb4, 0xb8, 0xbc,
    0xc0, 0xc4, 0xc8, 0xcc, 0xd0, 0xd4, 0xd8, 0xdc, 0xe0, 0xe4, 0xe8, 0xec, 0xf0, 0xf4, 0xf8, 0xff,
    0xff, 0xf8, 0xf4, 0xf0, 0xec, 0xe8, 0xe4, 0xe0, 0xdc, 0xd8, 0xd4, 0xd0, 0xcc, 0xc8, 0xc4, 0xc0,
    0xbc, 0xb8, 0xb4, 0xb0, 0xac, 0xa8, 0xa4, 0xa0, 0x9c, 0x98, 0x94, 0x90, 0x8c, 0x88, 0x84, 0x80,
    0x7c, 0x78, 0x74, 0x70, 0x6c, 0x68, 0x64, 0x60, 0x5c, 0x58, 0x54, 0x50, 0x4c, 0x48, 0x44, 0x40,
    0x3c, 0x38, 0x34, 0x30, 0x2c, 0x28, 0x24, 0x20, 0x1c, 0x18, 0x14, 0x10, 0x0c, 0x08, 0x04, 0x00,
};

uint8_t Led_Batt_Index_Tab[10] = {14, 15, 16, 17, 18, 19, 20, 21, 22, 23};
uint8_t Led_Point_buf[10]      = {LED_CAP_INDEX, LED_WIN_L_INDEX};

uint8_t  Systick_Led_Count;
uint8_t  Led_Point_Count;
uint8_t  Mac_Win_Point_Count;
uint8_t  INIT_ALL_SIX_KEY_Count;
uint8_t  INIT_ALL_KEY_Count;
bool     Led_Flash_Busy;
bool     Led_Off_Start;
bool     Led_Power_Up;
uint16_t Led_Power_Up_Delay;
bool     Usb_If_Ok_Led;
bool     Usb_If_Ok;
bool     Led_Point_Sleep;
uint16_t Usb_If_Ok_Delay;

static void wait_for_radio_and_flash_idle(void) {
    uint16_t timeout = 0x8ca0;

    do {
        if (Spi_Send_Recv_Flg == 0 && (GPIOA->ID & BIT(4)) == 0 && !Reset_Save_Flash) {
            break;
        }
    } while (--timeout != 0);
}

static void system_reset(void) {
    __DSB();
    SCB->AIRCR = 0x05fa0004;
    __DSB();
    while (true) {
    }
}

void mcu_reset(void) {
    wait_for_radio_and_flash_idle();
    GPIOD->BSBR = BIT(16);
    system_reset();
}

void es_mcu_reset(void) {
    mcu_reset();
}

void bootloader_jump(void) {
    wait_for_radio_and_flash_idle();
    GPIOD->BSBR = BIT(16);

    md_fc_lock();
    SYSCFG->REMAP &= ~SYSCFG_REMAP_MEMMOD_MSK;
    SYSCFG->REMAP &= ~SYSCFG_REMAP_EFBASE_MSK;
    SYSCFG->REMAP |= SYSCFG_REMAP_REMAP;
    system_reset();
}

void User_Systime_Init(void) {
    RCU->APB1EN |= RCU_APB1EN_BS16T1EN;
    BS16T1->PRES = 47;
    BS16T1->AR   = 2000;
    BS16T1->IER  = TIMER_IER_UPD;
    BS16T1->CON1 = 1;
    NVIC_SetPriority(BS16T1_IRQn, 2);
    NVIC_EnableIRQ(BS16T1_IRQn);
}

void User_Systime_Deinit(void) {
    RCU->APB1RST |= RCU_APB1RST_BS16T1EN;
    RCU->APB1RST &= ~RCU_APB1RST_BS16T1EN;
    RCU->APB1EN &= ~RCU_APB1EN_BS16T1EN;
}

void Init_Gpio_Infomation(void) {
    GPIOD->BSBR = BIT(16);
    _pal_lld_setgroupmode(GPIOD, BIT(0), PAL_MODE_OUTPUT_PUSHPULL);
    GPIOD->BSBR = BIT(16);

    GPIOA->BSBR = BIT(3);
    _pal_lld_setgroupmode(GPIOA, BIT(3), PAL_MODE_OUTPUT_PUSHPULL);
    GPIOA->BSBR = BIT(3);

    GPIOD->BSBR = BIT(1);
    _pal_lld_setgroupmode(GPIOD, BIT(1), PAL_MODE_OUTPUT_PUSHPULL);
    GPIOD->BSBR = BIT(1);

    _pal_lld_setgroupmode(GPIOA, BIT(13), PAL_MODE_INPUT_PULLUP);
    _pal_lld_setgroupmode(GPIOC, BIT(5), PAL_MODE_INPUT);
    _pal_lld_setgroupmode(GPIOB, BIT(2), PAL_MODE_INPUT_PULLUP);
    _pal_lld_setgroupmode(GPIOB, BIT(10), PAL_MODE_INPUT_PULLUP);
    _pal_lld_setgroupmode(GPIOB, BIT(13), PAL_MODE_INPUT_PULLUP);
    _pal_lld_setgroupmode(GPIOB, BIT(12), PAL_MODE_INPUT_PULLUP);

    md_gpio_inittypedef gpio = {
        .Pin        = BIT(4),
        .Mode       = 0,
        .OutputType = 0,
        .Pull       = 2,
        .OutDrive   = 0,
        .Function   = 0,
    };
    md_gpio_init(GPIOA, &gpio);

    EXTI->ICFG1 = 0;
    EXTI->IER |= BIT(4);
    EXTI->RTS |= BIT(4);
    EXTI->FTS |= BIT(4);
    NVIC_SetPriority(EXTI_4to15_IRQn, 3);
    NVIC_EnableIRQ(EXTI_4to15_IRQn);
}

void User_Sleep(void) {
    _pal_lld_setgroupmode(GPIOD, BIT(0), PAL_MODE_OUTPUT_PUSHPULL);
    GPIOD->BSBR = BIT(16);
    _pal_lld_setgroupmode(GPIOA, BIT(3), PAL_MODE_OUTPUT_PUSHPULL);
    GPIOA->BSBR = BIT(19);
    _pal_lld_setgroupmode(GPIOD, BIT(1), PAL_MODE_OUTPUT_PUSHPULL);
    GPIOD->BSBR = BIT(17);
}

void User_Wakeup(void) {
    _pal_lld_setgroupmode(GPIOD, BIT(1), PAL_MODE_OUTPUT_PUSHPULL);
    GPIOD->BSBR = BIT(1);
}

void Board_Wakeup_Init(void) {
    RCU->AHBEN |= RCU_AHBEN_CSUEN;
    CSU->CON |= CSU_CON_AUTOEN;
    CSU->CON |= CSU_CON_CNTEN;

    es_ble_spi_init();
    User_Adc_Init();
    rgb_matrix_driver_init();
    Init_Gpio_Infomation();
    User_Systime_Init();
    if (Keyboard_Info.Key_Mode == QMK_USB_MODE) {
        User_Usb_Init();
    } else {
        Usb_Disconnect();
    }

    Init_Spi_Power_Up   = true;
    Init_Spi_100ms_Delay = 0;
    Spi_Interval         = SPI_DELAY_RF_TIME;
    NVIC_SetPriority(PendSV_IRQn, 3);
    NVIC_SetPriority(SysTick_IRQn, 3);
    Keyboard_Status.System_Work_Status = 0;
    Keyboard_Status.System_Sleep_Mode  = 0;
    Usb_Change_Mode_Wakeup = false;
    Init_Batt_Infomation();
    Led_Power_Up  = false;
    Emi_Test_Start = false;
    Logo_Init();
    Lattice_Init();
}

void User_Keyboard_Post_Init(void) {
    if (keymap_config.nkro != (Keyboard_Info.Nkro != 0)) {
        keymap_config.nkro = Keyboard_Info.Nkro != 0;
    }
    if (Keyboard_Info.Mac_Win_Mode != 0 && biton(layer_state) != 1) {
        layer_on(1);
    }
}

uint8_t app_2g4_buffer_full(void) {
    uint8_t next = app_2g4_data_rev + 1;
    if (next >= APP_2G4_BUF_CNT) {
        next = 0;
    }
    return app_2g4_data_send == next;
}

uint8_t app_2g4_buffer_empty(void) {
    return app_2g4_data_rev == app_2g4_data_send;
}

void app_2g4_buffer_rev_add(void) {
    if (++app_2g4_data_rev >= APP_2G4_BUF_CNT) {
        app_2g4_data_rev = 0;
    }
}

void app_2g4_buffer_send_add(void) {
    if (++app_2g4_data_send >= APP_2G4_BUF_CNT) {
        app_2g4_data_send = 0;
    }
}

uint8_t es_keyboard_leds(void) {
    return es_qmk_driver ? es_qmk_driver->keyboard_leds() : 0;
}

void User_bluetooth_send_keyboard(uint8_t *report, uint32_t len) {
    uint8_t frame[USER_KEYBOARD_LENGTH] = {0};
    uint32_t copy_len                  = len > 61 ? 61 : len;

    if (app_2g4_buffer_full()) {
        return;
    }

    frame[0] = USER_KEYBOARD_COMMAND;
    frame[1] = USER_KEYBOARD_LENGTH;
    if (len == USER_KEY_BYTE_LENGTH) {
        memcpy(&frame[3], report, USER_KEY_BYTE_LENGTH);
        frame[2] = USER_KEY_BYTE_DATA;
    } else {
        switch (report[0]) {
            case MOUSE_REPORT_ID:
                memcpy(&frame[3], report, copy_len);
                frame[2] = USER_MOUSE_DATA;
                break;
            case SYS_REPORT_ID:
                memcpy(&frame[3], report, copy_len);
                frame[2] = USER_SYSTEM_DATA;
                break;
            case CON_REPORT_ID:
                memcpy(&frame[3], report, copy_len);
                frame[2] = USER_CONSUMER_DATA;
                break;
            case KB_REPORT_ID:
                memcpy(&frame[3], report, copy_len);
                frame[2] = USER_KEY_BIT_DATA;
                break;
            default:
                break;
        }
    }

    memcpy(app_2g4_data[app_2g4_data_rev], frame, APP_2G4_BUF_SIZE);
    app_2g4_buffer_rev_add();
}

void es_send_keyboard(report_keyboard_t *report) {
    if (Keyboard_Info.Key_Mode < QMK_USB_MODE) {
        User_bluetooth_send_keyboard(&report->mods, USER_KEY_BYTE_LENGTH);
    }
    if (es_qmk_driver) {
        es_qmk_driver->send_keyboard(report);
    }
}

void es_send_nkro(report_nkro_t *report) {
    if (Keyboard_Info.Key_Mode < QMK_USB_MODE) {
        User_bluetooth_send_keyboard(&report->report_id, sizeof(*report));
    }
    if (es_qmk_driver) {
        es_qmk_driver->send_nkro(report);
    }
}

void es_send_mouse(report_mouse_t *report) {
    if (Keyboard_Info.Key_Mode < QMK_USB_MODE) {
        User_bluetooth_send_keyboard(&report->report_id, 6);
    }
    if (es_qmk_driver) {
        es_qmk_driver->send_mouse(report);
    }
}

void es_send_extra(report_extra_t *report) {
    if (Keyboard_Info.Key_Mode < QMK_USB_MODE) {
        User_bluetooth_send_keyboard(&report->report_id, sizeof(*report));
    }
    if (es_qmk_driver) {
        es_qmk_driver->send_extra(report);
    }
}

void Mode_Synchronization(void) {
    uint8_t command;
    switch (Keyboard_Info.Key_Mode) {
        case QMK_2P4G_MODE:
            command = USER_SWITCH_2P4G_MODE;
            break;
        case QMK_USB_MODE:
            command = USER_SWITCH_USB_MODE;
            break;
        case QMK_BLE_MODE:
            if (Keyboard_Info.Ble_Channel < QMK_BLE_CHANNEL_1 || Keyboard_Info.Ble_Channel > QMK_BLE_CHANNEL_3) {
                return;
            }
            command = Keyboard_Info.Ble_Channel;
            break;
        default:
            return;
    }
    Spi_Send_Commad(command);
}

void Ble_Name_Synchronization(void) {
    if (Ble_Name_Spi_Count >= QMK_BLE_CHANNEL_1 && Ble_Name_Spi_Count <= QMK_BLE_CHANNEL_3) {
        Spi_Send_Commad(USER_BLE1_WRITE_NAME + Ble_Name_Spi_Count - 1);
        Ble_Name_Spi_Count++;
    }
    if (Ble_Name_Spi_Count > QMK_BLE_CHANNEL_3) {
        Ble_Name_Spi_Count = QMK_BLE_CHANNEL_1;
        Ble_Name_Spi_Send  = false;
    }
}

void Sleep_Time_Synchronization(void) {
    Spi_Send_Commad(USER_SLEEP_TIME_WRITE);
    User_Sleep_Time_Send = false;
}

void DSleep_Time_Synchronization(void) {
    Spi_Send_Commad(USER_DSLEEP_TIME_WRITE);
    User_DSleep_Time_Send = false;
}

void Save_Flash_Set(void) {
    Save_Flash          = true;
    Save_Flash_3S_Count = 0;
}

void U16_Buff_Clear(uint16_t *Buff, uint8_t Len) {
    memset(Buff, 0, (size_t)Len * sizeof(*Buff));
}

void Emi_Init(void) {
    Spi_Interval                 = SPI_DELAY_USB_TIME;
    Keyboard_Status.System_Work_Status = 0;
    Keyboard_Status.System_Sleep_Mode  = 0;
    Mode_Synchronization_Signal = false;
    Led_Rf_Pair_Flg             = false;
    Ble_Name_Spi_Send           = false;
    User_Sleep_Time_Send        = false;
    User_DSleep_Time_Send       = false;
}

void Emi_Write_Data(uint8_t *User_Data, uint8_t User_Length) {
    (void)User_Length;
    if (Emi_Test_Start && User_Data[0] != USER_KEYBOARD_COMMAND) {
        User_Data[0] = USER_EMI_COMMAND;
        raw_hid_send(User_Data, 32);
    }
}

void General_Key_Reorder(uint8_t Spot_Index) {
    if (Spot_Index >= 5) {
        return;
    }
    for (uint8_t i = Spot_Index; i < 5; ++i) {
        if (keyboard_report->keys[i + 1] != 0) {
            keyboard_report->keys[i]     = keyboard_report->keys[i + 1];
            keyboard_report->keys[i + 1] = 0;
        }
    }
}

void del_key_from_report(uint8_t key) {
    for (uint8_t i = 0; i < sizeof(keyboard_report->keys); ++i) {
        if (keyboard_report->keys[i] == key) {
            keyboard_report->keys[i] = 0;
            General_Key_Reorder(i);
            keymap_config.User_Send_Type = false;
            return;
        }
    }

    if (keyboard_protocol && keymap_config.nkro && key < 0xf0) {
        nkro_report->bits[key >> 3] &= ~(1U << (key & 7));
        keymap_config.User_Send_Type = true;
    }
}

void User_send_6kro_report(void) {
    keyboard_report->mods = real_mods | weak_mods;
    nkro_report->mods     = 0;
    host_keyboard_send(keyboard_report);
}

void User_send_nkro_report(void) {
    nkro_report->mods = 0;
    host_nkro_send(nkro_report);
}

void User_Clear_Board(void) {
    clear_mods();
    clear_weak_mods();
    memset(keyboard_report->keys, 0, sizeof(keyboard_report->keys));
    User_send_6kro_report();
    memset(nkro_report->bits, 0, sizeof(nkro_report->bits));
    User_send_nkro_report();
}

void User_Clear_Board_2ms(void) {
    clear_mods();
    clear_weak_mods();
    memset(keyboard_report->keys, 0, sizeof(keyboard_report->keys));
    User_send_6kro_report();
    chThdSleep(2);
    memset(nkro_report->bits, 0, sizeof(nkro_report->bits));
    User_send_nkro_report();
    chThdSleep(2);
}

void User_Keyboard_Init(void) {
    Board_Wakeup_Init();
}

void User_Keyboard_Reset(void) {
    es_chibios_user_idle_loop_hook();
    if (Spi_Send_Recv_Flg == 1 && Spi_Send_Recv_count > 49) {
        Spi_Send_Recv_Flg   = 0;
        Spi_Send_Recv_count = 0;
        Spi_Ack_Send_Commad(USER_GET_RF_STATUS);
    }
}

static bool spi_ack_is_low(void) {
    return (GPIOA->ID & BIT(4)) == 0;
}

void es_ble_spi_init(void) {
    md_gpio_inittypedef gpio = {
        .Pin        = BIT(0),
        .Mode       = 2,
        .Pull       = 1,
        .OutputType = 0,
        .OutDrive   = 0,
        .Function   = 0,
    };

    RCU->APB1EN |= RCU_APB1EN_SPI2EN;
    md_gpio_init(GPIOC, &gpio);
    gpio.Pin = BIT(1);
    md_gpio_init(GPIOC, &gpio);
    gpio.Pin = BIT(2);
    md_gpio_init(GPIOC, &gpio);
    gpio.Pin = BIT(3);
    md_gpio_init(GPIOC, &gpio);
    md_spi_init(SPI2, (md_spi_inittypedef *)&SPI2_InitStruct);
}

void es_ble_spi_deinit(void) {
    RCU->APB1RST |= RCU_APB1RST_SPI2EN;
    RCU->APB1RST &= ~RCU_APB1RST_SPI2EN;
    RCU->APB1EN &= ~RCU_APB1EN_SPI2EN;

    GPIOC->MOD |= 0xff;
    GPIOC->PUD &= ~0xff;
    GPIOC->OT &= ~0xff;
    GPIOC->DS &= ~0xff;
    GPIOC->AFL &= ~0xff;
}

void es_spi_send_recv_by_dma(uint32_t num, uint8_t *rx_buf, uint8_t *tx_buf) {
    uint32_t tx_index = num & 1U;
    uint32_t rx_index = 0;
    bool zero_fill    = (uintptr_t)tx_buf < SRAM_BASE;

    __disable_irq();

    if (tx_index != 0) {
        SPI2->DATA = zero_fill ? 0 : tx_buf[0];
    }

    while (tx_index < num) {
        if (((SPI2->STAT & SPI_STAT_TXFLV_MSK) >> SPI_STAT_TXFLV_POSS) < 3) {
            SPI2->DATA = zero_fill ? 0 : tx_buf[tx_index];
            tx_index++;
            SPI2->DATA = zero_fill ? 0 : tx_buf[tx_index];
            tx_index++;
        }
        if (((SPI2->STAT & SPI_STAT_RXFLV_MSK) >> SPI_STAT_RXFLV_POSS) != 0) {
            rx_buf[rx_index++] = (uint8_t)SPI2->DATA;
        }
    }

    while (rx_index < num) {
        if (((SPI2->STAT & SPI_STAT_RXFLV_MSK) >> SPI_STAT_RXFLV_POSS) != 0) {
            rx_buf[rx_index++] = (uint8_t)SPI2->DATA;
        }
    }

    __enable_irq();
}

void Emi_Read_Data(uint8_t *User_Data, uint8_t User_Length) {
    if (!Emi_Test_Start || Init_Spi_Power_Up || Spi_Send_Recv_Flg || !spi_ack_is_low()) {
        return;
    }

    Spi_Send_Recv_Flg = 1;
    Send_Key_Type     = 1;
    Repet_Send_Count  = 0;
    for (uint8_t i = 0; i < User_Length; ++i) {
        g_es_spi_tx_buf[i] = User_Data[i];
    }
    es_spi_send_recv_by_dma(USER_KEYBOARD_LENGTH, g_es_spi_rx_buf, g_es_spi_tx_buf);
}

void Spi_Main_Loop(void) {
    if (Init_Spi_Power_Up || Keyboard_Status.System_Work_Status != 0 || app_2g4_buffer_empty() || Spi_Send_Recv_Flg || !spi_ack_is_low()) {
        return;
    }

    Spi_Send_Recv_Flg = 1;
    Send_Key_Type     = GPIOA->ID & BIT(4);
    es_spi_send_recv_by_dma(USER_KEYBOARD_LENGTH, g_es_spi_rx_buf, app_2g4_data[app_2g4_data_send]);
    app_2g4_buffer_send_add();
}

static void spi_write_be32(uint8_t *dst, uint32_t value) {
    dst[0] = (uint8_t)(value >> 24);
    dst[1] = (uint8_t)(value >> 16);
    dst[2] = (uint8_t)(value >> 8);
    dst[3] = (uint8_t)value;
}

void Spi_Send_Commad(uint8_t Commad) {
    if (Init_Spi_Power_Up || Keyboard_Status.System_Work_Status != 0) {
        return;
    }

    for (uint16_t delay = 2000; delay != 0; --delay) {
        if (Spi_Send_Recv_Flg || !spi_ack_is_low()) {
            continue;
        }

        Spi_Send_Recv_Flg = 1;
        Send_Key_Type     = GPIOA->ID & BIT(4);
        Spi_Interval      = SPI_DELAY_RF_TIME;
        g_es_spi_tx_buf[0] = USER_KEYBOARD_COMMAND;
        g_es_spi_tx_buf[1] = USER_KEYBOARD_LENGTH;
        g_es_spi_tx_buf[2] = Commad;

        if (Commad == USER_BATTERY_DATA) {
            g_es_spi_tx_buf[3] = Keyboard_Info.Batt_Number;
        } else if (Commad >= USER_BLE1_WRITE_NAME && Commad <= USER_BLE3_WRITE_NAME) {
            static const char name_prefix[] = "P75 JIS BT";
            g_es_spi_tx_buf[3] = (uint8_t)(USER_BlE_ID >> 8);
            g_es_spi_tx_buf[4] = (uint8_t)USER_BlE_ID;
            g_es_spi_tx_buf[5] = sizeof(name_prefix);
            memcpy(&g_es_spi_tx_buf[6], name_prefix, sizeof(name_prefix) - 1);
            g_es_spi_tx_buf[16] = (uint8_t)('1' + Commad - USER_BLE1_WRITE_NAME);
        } else if (Commad == USER_SLEEP_TIME_WRITE) {
            spi_write_be32(&g_es_spi_tx_buf[3], Keyboard_Info.User_Sleep_Time);
        } else if (Commad == USER_DSLEEP_TIME_WRITE) {
            spi_write_be32(&g_es_spi_tx_buf[3], Keyboard_Info.User_DSleep_Time);
        }

        es_spi_send_recv_by_dma(USER_KEYBOARD_LENGTH, g_es_spi_rx_buf, g_es_spi_tx_buf);
        return;
    }
}

uint8_t Spi_Ack_Send_Commad(uint8_t Commad) {
    if (Init_Spi_Power_Up || Spi_Send_Recv_Flg || !spi_ack_is_low()) {
        return 0;
    }

    Spi_Send_Recv_Flg = 1;
    Send_Key_Type     = 1;
    Repet_Send_Count  = 0;
    g_es_spi_tx_buf[0] = USER_KEYBOARD_COMMAND;
    g_es_spi_tx_buf[1] = USER_KEYBOARD_LENGTH;
    g_es_spi_tx_buf[2] = Commad;
    es_spi_send_recv_by_dma(USER_KEYBOARD_LENGTH, g_es_spi_rx_buf, g_es_spi_tx_buf);
    return 1;
}

static uint32_t read_be32(const uint8_t *src) {
    return ((uint32_t)src[0] << 24) | ((uint32_t)src[1] << 16) | ((uint32_t)src[2] << 8) | src[3];
}

void Get_Spi_Return_Data(uint8_t *Data) {
    if (Emi_Test_Start) {
        Emi_Write_Data(Data, USER_KEYBOARD_LENGTH);
        return;
    }

    if (Data[2] == USER_GET_RF_STATUS) {
        Keyboard_Status.System_Work_Status = Data[3];
        if (Keyboard_Status.System_Work_Status != 0) {
            if (Keyboard_Info.Key_Mode == QMK_USB_MODE) {
                Keyboard_Status.System_Work_Status = 0;
                Spi_Interval                       = SPI_DELAY_USB_TIME;
            } else if (!Usb_Change_Mode_Wakeup) {
                Keyboard_Status.System_Work_Status = 0;
            } else {
                User_Sleep();
            }
        }

        Keyboard_Status.System_Work_Mode = Data[4];
        if (Keyboard_Status.System_Work_Mode == Keyboard_Info.Key_Mode) {
            if (Keyboard_Info.Key_Mode == QMK_USB_MODE) {
                Spi_Interval = SPI_DELAY_USB_TIME;
            }
        } else {
            Mode_Synchronization_Signal = true;
            if (Keyboard_Status.System_Work_Status != 0) {
                Keyboard_Status.System_Work_Status = 0;
            }
        }

        Keyboard_Status.System_Work_Channel = Data[5];
        if (Keyboard_Info.Key_Mode == QMK_BLE_MODE && Keyboard_Info.Ble_Channel != Keyboard_Status.System_Work_Channel) {
            Mode_Synchronization_Signal = true;
            if (Keyboard_Status.System_Work_Status != 0) {
                Keyboard_Status.System_Work_Status = 0;
            }
        }

        Keyboard_Status.System_Connect_Status = Data[6];
        if (Temp_System_Led_Status != Keyboard_Status.System_Connect_Status) {
            Temp_System_Led_Status = Keyboard_Status.System_Connect_Status;
            Led_Rf_Pair_Flg        = Keyboard_Info.Key_Mode != QMK_USB_MODE;
        }
        Keyboard_Status.System_Led_Status = Data[7];

        if ((((uint16_t)Data[8] << 8) | Data[9]) != USER_BlE_ID && !Ble_Name_Spi_Send) {
            Ble_Name_Spi_Send  = true;
            Ble_Name_Spi_Count = QMK_BLE_CHANNEL_1;
        }
        if (read_be32(&Data[11]) != Keyboard_Info.User_Sleep_Time && !User_Sleep_Time_Send) {
            User_Sleep_Time_Send = true;
        }
        if (read_be32(&Data[15]) != Keyboard_Info.User_DSleep_Time && !User_DSleep_Time_Send) {
            User_DSleep_Time_Send = true;
        }
    } else if (Data[2] == USER_KEYBOARD_SLEEP) {
        if (Keyboard_Status.System_Work_Status == 0 || Data[3] != 0) {
            if (Data[3] == 1) {
                Keyboard_Status.System_Work_Status = 0;
                Keyboard_Status.System_Sleep_Mode  = 0;
            }
        } else {
            Keyboard_Status.System_Sleep_Mode = Keyboard_Info.Key_Mode == QMK_USB_MODE ? 0 : 1;
        }
    }

    if (Data[10] == USER_EMI_COMMAND) {
        Emi_Test_Start = true;
        Emi_Init();
    }
}

static uint16_t battery_filtered_sum(void) {
    uint8_t min_index = 0;
    uint8_t max_index = 0;
    for (uint8_t i = 0; i < USER_BATT_SCAN_COUNT; ++i) {
        if (User_Scan_Batt[max_index] < User_Scan_Batt[i]) {
            max_index = i;
        }
        if (User_Scan_Batt[i] < User_Scan_Batt[min_index]) {
            min_index = i;
        }
    }

    uint16_t sum = 0;
    if (min_index == max_index) {
        for (uint8_t i = 0; i < USER_BATT_SCAN_COUNT - 2; ++i) {
            sum = (uint16_t)(sum + User_Scan_Batt[i]);
        }
    } else {
        for (uint8_t i = 0; i < USER_BATT_SCAN_COUNT; ++i) {
            if (i != min_index && i != max_index) {
                sum = (uint16_t)(sum + User_Scan_Batt[i]);
            }
        }
    }
    return sum;
}

static uint8_t battery_percentage(uint16_t sum) {
    uint16_t average = sum >> 3;
    if (average >= USER_BATT_HIGH_POWER) {
        return 100;
    }
    if (average > USER_BATT_STDOWN_POWER) {
        return (uint8_t)(((uint32_t)(average - USER_BATT_STDOWN_POWER) * 100U) /
                         (USER_BATT_HIGH_POWER - USER_BATT_STDOWN_POWER));
    }
    return 0;
}

void Init_Batt_Infomation(void) {
    User_Batt_Power_Up = (GPIOC->ID & BIT(5)) == 0;
    if (!User_Batt_Power_Up) {
        User_Batt_BaiFen     = Keyboard_Info.Batt_Number;
        User_Batt_Old_BaiFen = Keyboard_Info.Batt_Number;
    }

    User_Batt_10ms_Count                  = 0;
    User_Adc_Batt_Count                   = 0;
    User_Batt_Time_15S_Count              = 0;
    User_Power_Low                        = false;
    User_Power_Low_Count                  = 0;
    U16_Buff_Clear(User_Adc_Batt, USER_BATT_SCAN_COUNT);
    U16_Buff_Clear(User_Scan_Batt, USER_BATT_SCAN_COUNT);
    User_Batt_Power_Up_Delay_100ms_Count  = 0;
    User_Batt_Power_Up_Delay              = true;
}

void Get_User_Adc_Batt_Power_Up_Init(void) {
    uint16_t sum      = battery_filtered_sum();
    uint16_t average  = sum >> 3;
    uint8_t percentage = battery_percentage(sum);

    if (average >= USER_BATT_HIGH_POWER) {
        User_Power_Low_Count = 0;
    } else if (average > USER_BATT_STDOWN_POWER) {
        if (average > USER_BATT_LOW_POWER) {
            User_Power_Low_Count = 0;
        } else if (++User_Power_Low_Count > 9) {
            User_Power_Low_Count = 0;
            User_Power_Low       = true;
        }
    } else if (++User_Power_Low_Count > 3) {
        User_Power_Low_Count = 0;
        User_Power_Low       = true;
    }

    User_Batt_BaiFen     = percentage;
    User_Batt_Old_BaiFen = percentage;
    if (Keyboard_Info.Batt_Number != percentage) {
        Keyboard_Info.Batt_Number = percentage;
        Save_Flash_Set();
    }
    User_Adc_Batt_Count      = 0;
    User_Batt_10ms_Count     = 0;
    User_Batt_Time_15S_Count = 0;
    User_Batt_Power_Up       = false;
}

void Get_User_Adc_Batt_Number(void) {
    uint8_t stored_percentage = Keyboard_Info.Batt_Number;

    if (es_stdby_pin_state == 2) {
        User_Batt_BaiFen     = 100;
        User_Batt_Old_BaiFen = 100;
        if (stored_percentage != 100) {
            Keyboard_Info.Batt_Number = 100;
            User_Batt_Send_Spi        = true;
            Save_Flash_Set();
        }
        User_Power_Low       = false;
        User_Power_Low_Count = 0;
        return;
    }

    uint16_t sum       = battery_filtered_sum();
    uint16_t average   = sum >> 3;
    uint8_t target     = battery_percentage(sum);

    if (es_stdby_pin_state == 1) {
        if (target == 0) {
            target = 1;
        } else if (target > 99) {
            target = 99;
        }

        if (target > User_Batt_Old_BaiFen) {
            if (User_Batt_Time_15S_Count > USER_BATT_DELAY_TIME - 1) {
                User_Batt_Time_15S_Count = 0;
                if (User_Batt_BaiFen < 99) {
                    User_Batt_BaiFen++;
                }
                User_Batt_Old_BaiFen = User_Batt_BaiFen;
            }
        } else {
            User_Batt_Time_15S_Count = 0;
        }
        User_Power_Low_Count = 0;
        User_Power_Low       = false;
    } else {
        if (average > USER_BATT_LOW_POWER) {
            User_Power_Low_Count = 0;
        } else if (average > USER_BATT_STDOWN_POWER) {
            if (++User_Power_Low_Count > 9) {
                User_Power_Low_Count = 0;
                User_Power_Low       = true;
            }
        } else if (++User_Power_Low_Count > 4) {
            User_Power_Low_Count = 0;
            User_Power_Low       = true;
        }

        if (target < User_Batt_Old_BaiFen) {
            if (User_Batt_Time_15S_Count > USER_BATT_DELAY_TIME - 1) {
                User_Batt_Time_15S_Count = 0;
                if (User_Batt_BaiFen != 0) {
                    User_Batt_BaiFen--;
                }
                User_Batt_Old_BaiFen = User_Batt_BaiFen;
            }
        } else {
            User_Batt_Time_15S_Count = 0;
        }
    }

    if (User_Batt_BaiFen != stored_percentage) {
        Keyboard_Info.Batt_Number = User_Batt_BaiFen;
        User_Batt_Send_Spi        = true;
        Save_Flash_Set();
    }
}

static void increment_led_counter(uint8_t *counter) {
    uint8_t next = (uint8_t)(*counter + 1);
    *counter     = next == UINT8_MAX ? 0 : next;
}

static void systime_sample_battery(void) {
    bool power_up = User_Batt_Power_Up;
    uint8_t next  = (uint8_t)(User_Batt_10ms_Count + 1);
    uint8_t limit = power_up ? 4 : 20;

    if (next < limit) {
        User_Batt_10ms_Count = next;
        return;
    }

    User_Batt_10ms_Count = 0;
    if ((ADC->SR & ADC_SR_NDRE) == 0) {
        User_Adc_Batt[User_Adc_Batt_Count] = (uint16_t)ADC->NCHDR;
        ADC->CON = (ADC->CON & ~ADC_CON_NSTART_MSK) | ADC_CON_NSTART;
        if (++User_Adc_Batt_Count >= USER_BATT_SCAN_COUNT) {
            User_Adc_Batt_Count = 0;
            memcpy(User_Scan_Batt, User_Adc_Batt, sizeof(User_Scan_Batt));
            if (power_up) {
                Get_User_Adc_Batt_Power_Up_Init();
            } else {
                Get_User_Adc_Batt_Number();
            }
        }
    }

    if (!power_up) {
        User_Batt_Time_15S_Count++;
    }
}

static void systime_handle_pairing_hold(void) {
    if (++Time_3s_Count < 300) {
        return;
    }
    Time_3s_Count = 0;

    uint8_t command = UINT8_MAX;
    if (Keyboard_Info.Key_Mode == QMK_2P4G_MODE && Key_2p4g_Status) {
        Key_2p4g_Status = false;
        command         = USER_SWITCH_2P4G_PAIR;
    } else if (Keyboard_Info.Key_Mode == QMK_BLE_MODE) {
        if (Keyboard_Info.Ble_Channel == QMK_BLE_CHANNEL_1 && Key_Ble_1_Status) {
            Key_Ble_1_Status = false;
            command          = USER_SWITCH_BLE_1_PAIR;
        } else if (Keyboard_Info.Ble_Channel == QMK_BLE_CHANNEL_2 && Key_Ble_2_Status) {
            Key_Ble_2_Status = false;
            command          = USER_SWITCH_BLE_2_PAIR;
        } else if (Keyboard_Info.Ble_Channel == QMK_BLE_CHANNEL_3 && Key_Ble_3_Status) {
            Key_Ble_3_Status = false;
            command          = USER_SWITCH_BLE_3_PAIR;
        }
    }

    if (command != UINT8_MAX) {
        Spi_Send_Commad(command);
        Led_Rf_Pair_Flg = true;
    }
}

static void systime_unregister_mode_keys(void) {
    unregister_code(0xe2);
    unregister_code(0xe3);
    unregister_code(0xe6);
    unregister_code(0xe7);
    unregister_code(0x65);
}

static void systime_handle_long_press_settings(void) {
    if (User_QMK_EE_CLR_Flag && ++Time_3s_EE_CLR_Count >= 300) {
        Time_3s_EE_CLR_Count = 0;
        User_QMK_EE_CLR_Flag = false;
        User_EE_CLR_Start_Flag = true;
    }

    if (Key_Sys_Mode_Status && ++Win_Mac_Key_3s_Count >= 300) {
        Win_Mac_Key_3s_Count = 0;
        Key_Sys_Mode_Status  = false;

        bool target_layer;
        if (Keyboard_Info.Mac_Win_Mode == INIT_MAC_MODE) {
            Keyboard_Info.Mac_Win_Mode = INIT_WIN_MODE;
            Mac_Win_Point_Count         = 1;
            target_layer                = false;
        } else {
            Keyboard_Info.Mac_Win_Mode = INIT_MAC_MODE;
            Keyboard_Info.Win_Lock     = INIT_WIN_NLOCK;
            Mac_Win_Point_Count         = 3;
            target_layer                = true;
        }
        systime_unregister_mode_keys();
        Save_Flash_Set();
        if ((biton(layer_state) != 0) != target_layer) {
            layer_move(target_layer);
        }
    }

    if (Debounce_Function_Status && ++User_Key_3s_Count >= 300) {
        User_Key_3s_Count          = 0;
        Debounce_Function_Status   = false;
        Debounce_Function_Count    = !Debounce_Function_Count;
        Keyboard_Info.Debounce_Delay = Debounce_Function_Count ? DEBOUNCE_DELAY_TWO : DEBOUNCE_DELAY_ONE;
        Debounce_Delay             = Keyboard_Info.Debounce_Delay;
        Debounce_Point_Count       = 3;
        Save_Flash_Set();
    }
}

static void systime_handle_deferred_flash(void) {
    if (!Save_Flash) {
        Save_Flash_3S_Count = 0;
        return;
    }

    if (++Save_Flash_3S_Count < 200) {
        return;
    }
    if (Spi_Send_Recv_Flg != 0 || (GPIOA->ID & BIT(4)) != 0 || !Led_Flash_Busy) {
        Save_Flash_3S_Count = 290;
        return;
    }

    Reset_Save_Flash = true;
    eeprom_write_block_user(&Keyboard_Info, (void *)0, sizeof(Keyboard_Info));
    Save_Flash          = false;
    Reset_Save_Flash    = false;
    Save_Flash_3S_Count = 0;
}

static void systime_handle_usb_and_battery(void) {
    if (Keyboard_Info.Key_Mode == QMK_USB_MODE) {
        if (Usb_If_Ok && Led_Point_Count == 0 && ++Usb_If_Ok_Delay >= 200) {
            Usb_If_Ok_Delay = 0;
            Usb_If_Ok       = false;
        }
        g_usb_sof_frame_id = (uint16_t)USB->FRAME1 | ((uint16_t)USB->FRAME2 << 8);
        Usb_Dis_Connect    = true;
    } else {
        if (User_Batt_Send_Spi) {
            User_Batt_Send_Spi = false;
            Spi_Send_Commad(USER_BATTERY_DATA);
        }
        Usb_If_Ok     = false;
        Usb_If_Ok_Led = false;
    }

    if ((GPIOC->ID & BIT(5)) == 0) {
        es_stdby_pin_state = 0;
    } else {
        es_stdby_pin_state = (GPIOA->ID & BIT(13)) != 0 ? 1 : 2;
    }
}

static void systime_10ms_tasks(void) {
    if (Mode_Synchronization_Signal) {
        Mode_Synchronization_Signal = false;
        Mode_Synchronization();
    }
    if (Ble_Name_Spi_Send) {
        Ble_Name_Synchronization();
    }
    if (User_Sleep_Time_Send) {
        Sleep_Time_Synchronization();
    }
    if (User_DSleep_Time_Send) {
        DSleep_Time_Synchronization();
    }

    increment_led_counter(&Systick_Led_Count);
    increment_led_counter(&Batt_Led_Count);
    if (!Led_Power_Up && ++Led_Power_Up_Delay >= 50) {
        Led_Power_Up_Delay = 0;
        Led_Power_Up       = true;
        if (Keyboard_Info.Key_Mode == QMK_BLE_MODE) {
            User_Batt_Send_Spi = true;
        }
    }
    increment_led_counter(&Logo_Flash_Count);
    increment_led_counter(&Logo_Led_Count);
    if (!Lattice_Power_Flag) {
        increment_led_counter(&Lattice_Led_Count);
        increment_led_counter(&Lattice_Heart_Count);
    } else {
        increment_led_counter(&Lattice_Power_Up_Count);
    }

    if (++Usb_Change_Mode_Delay >= 300) {
        Usb_Change_Mode_Delay  = 0;
        Usb_Change_Mode_Wakeup = true;
    }

    systime_handle_deferred_flash();
    systime_handle_usb_and_battery();
    systime_handle_pairing_hold();
    systime_handle_long_press_settings();

    if (Spi_Send_Recv_Flg == 0) {
        Spi_Send_Recv_count = 0;
    } else {
        Spi_Send_Recv_count++;
    }
}

OSAL_IRQ_HANDLER(Vector78) {
    OSAL_IRQ_PROLOGUE();
    BS16T1->ICR = BS16T1->IFM;

    if (Init_Spi_Power_Up) {
        uint8_t next = (uint8_t)(Init_Spi_100ms_Delay + 1);
        if (next > 9) {
            Init_Spi_100ms_Delay = 0;
            if ((GPIOA->ID & BIT(4)) == 0) {
                Init_Spi_Power_Up = false;
            } else {
                Init_Spi_100ms_Delay = 5;
            }
        } else {
            Init_Spi_100ms_Delay = next;
        }
    } else if (++Systick_12ms_Count > 5) {
        Systick_12ms_Count = 0;
        Scan_Switch_Ok     = true;
    }

    if (Keyboard_Info.Key_Mode != QMK_USB_MODE) {
        if (Keyboard_Status.System_Work_Status == 0 || Keyboard_Status.System_Sleep_Mode != 0) {
            if (Keyboard_Info.Key_Mode == QMK_BLE_MODE) {
                if (++Spi_Main_Loop_Count > 3) {
                    Spi_Main_Loop_Count = 0;
                    Spi_Main_Loop();
                }
            } else {
                Spi_Main_Loop();
                Spi_Main_Loop_Count = 0;
            }
        } else {
            Spi_Ack_Send_Commad(USER_KEYBOARD_SLEEP);
        }
    }

    if (User_Batt_Power_Up_Delay) {
        if (++User_Batt_Power_Up_Delay_100ms_Count >= 50) {
            User_Batt_Power_Up_Delay_100ms_Count = 0;
            User_Batt_Power_Up_Delay = false;
        }
    } else {
        systime_sample_battery();
    }

    if (++Systick_6ms_Count >= 3) {
        Systick_6ms_Count = 0;
        host_driver_t *driver = host_get_driver();
        if (driver != &es_user_driver && driver != NULL) {
            es_qmk_driver = host_get_driver();
            host_set_driver((host_driver_t *)&es_user_driver);
        }
    }

    if (++Systick_10ms_Count >= 5) {
        Systick_10ms_Count = 0;
        systime_10ms_tasks();
    }

    if (++Systick_Interval_Count >= Spi_Interval) {
        Systick_Interval_Count = 0;
        if (Keyboard_Status.System_Work_Status == 0 && !Spi_Ack_Send_Commad(USER_GET_RF_STATUS)) {
            Systick_Interval_Count = (uint16_t)(Spi_Interval - 10);
        }
    }

    OSAL_IRQ_EPILOGUE();
}

static const ioline_t sleep_matrix_col_pins[MATRIX_COLS] = MATRIX_COL_PINS;
static const ioline_t sleep_matrix_row_pins[MATRIX_ROWS] = MATRIX_ROW_PINS;

static ioportid_t sleep_line_port(ioline_t line) {
    return (ioportid_t)((uint32_t)line & 0xfffffff0U);
}

static ioportmask_t sleep_line_mask(ioline_t line) {
    return BIT((uint32_t)line & 0x0fU);
}

static bool sleep_line_is_low(ioline_t line) {
    return (sleep_line_port(line)->ID & sleep_line_mask(line)) == 0;
}

static void sleep_set_line_level(ioline_t line, bool high) {
    ioportmask_t mask = sleep_line_mask(line);
    sleep_line_port(line)->BSBR = high ? mask : (mask << 16);
}

static void sleep_set_all_columns(bool high) {
    for (uint8_t col = 0; col < MATRIX_COLS; ++col) {
        sleep_set_line_level(sleep_matrix_col_pins[col], high);
    }
}

typedef enum {
    RADIO_SLEEP_RETRY_EXHAUSTED,
    RADIO_SLEEP_READY,
    RADIO_SLEEP_SPI_TIMEOUT,
} radio_sleep_result_t;

static radio_sleep_result_t request_radio_sleep(void) {
    User_Sleep();

    for (uint16_t attempt = 0x0e10; attempt != 0; --attempt) {
        if (Spi_Ack_Send_Commad(USER_KEYBOARD_SLEEP) != 1) {
            continue;
        }

        uint16_t timeout = 0x8ca0;
        while (Spi_Send_Recv_Flg != 0) {
            if (--timeout == 0) {
                User_Wakeup();
                return RADIO_SLEEP_SPI_TIMEOUT;
            }
        }
        return RADIO_SLEEP_READY;
    }

    User_Wakeup();
    return RADIO_SLEEP_RETRY_EXHAUSTED;
}

static bool usb_suspend_requests_mcu_sleep(void) {
    if (!Usb_Dis_Connect) {
        return false;
    }
    Usb_Dis_Connect = false;

    if (g_usb_sof_frame_id_last != g_usb_sof_frame_id || !Usb_Change_Mode_Wakeup) {
        g_usb_sof_frame_id_last = g_usb_sof_frame_id;
        Usb_Suspend_Delay       = 0;
        return false;
    }

    Usb_If_Ok_Led = false;
    if (++Usb_Suspend_Delay <= 799) {
        return false;
    }
    Usb_Suspend_Delay = 0;

    while ((DMA1->CHENSET & BIT(2)) != 0) {
    }

    radio_sleep_result_t result = request_radio_sleep();
    if (result == RADIO_SLEEP_RETRY_EXHAUSTED) {
        Usb_Suspend_Delay = 600;
    }
    return result == RADIO_SLEEP_READY;
}

static void prepare_matrix_wakeup_pins(void) {
    for (uint8_t col = 0; col < MATRIX_COLS; ++col) {
        ioline_t line = sleep_matrix_col_pins[col];
        _pal_lld_setgroupmode(sleep_line_port(line), sleep_line_mask(line), PAL_MODE_OUTPUT_PUSHPULL);
        sleep_set_line_level(line, false);
    }
    for (uint8_t row = 0; row < MATRIX_ROWS; ++row) {
        ioline_t line = sleep_matrix_row_pins[row];
        _pal_lld_setgroupmode(sleep_line_port(line), sleep_line_mask(line), PAL_MODE_INPUT_PULLUP);
    }

    _pal_lld_setgroupmode(GPIOC, BIT(5), PAL_MODE_INPUT);
    _pal_lld_setgroupmode(GPIOB, BIT(2), PAL_MODE_INPUT);
    _pal_lld_setgroupmode(GPIOB, BIT(10), PAL_MODE_INPUT);
    _pal_lld_setgroupmode(GPIOB, BIT(13), PAL_MODE_INPUT);
    _pal_lld_setgroupmode(GPIOB, BIT(12), PAL_MODE_INPUT);
}

static void configure_sleep_wakeup_sources(void) {
    uint32_t rising_edges  = (GPIOC->ID & BIT(5)) == 0 ? BIT(5) : 0;
    uint32_t falling_edges = 0x00fd;

    if ((GPIOB->ID & BIT(2)) == 0) {
        rising_edges |= BIT(2);
        falling_edges = 0x00f9;
    }
    if ((GPIOB->ID & BIT(10)) == 0) {
        rising_edges |= BIT(10);
    } else {
        falling_edges |= BIT(10);
    }
    if ((GPIOB->ID & BIT(13)) == 0) {
        rising_edges |= BIT(13);
    } else {
        falling_edges |= BIT(13);
    }
    if ((GPIOB->ID & BIT(12)) == 0) {
        rising_edges |= BIT(12);
    } else {
        falling_edges |= BIT(12);
    }

    GPIOB->MOD = (GPIOB->MOD & ~(3U << 10)) | (2U << 10);
    GPIOB->AFL = (GPIOB->AFL & ~(0x0fU << 20)) | (3U << 20);

    EXTI->ICFG1 = 0x11211101;
    EXTI->ICFG2 = 0x00110100;
    EXTI->IER |= 0x34fd;
    EXTI->RTS |= rising_edges;
    EXTI->FTS |= falling_edges;
    NVIC_EnableIRQ(EXTI_0to1_IRQn);
    NVIC_EnableIRQ(EXTI_2to3_IRQn);
    NVIC_EnableIRQ(EXTI_4to15_IRQn);

    RCU->APB1EN |= RCU_APB1EN_GP16C4T2EN;
    GP16C4T2->PRES  = 1;
    GP16C4T2->AR    = UINT16_MAX;
    GP16C4T2->CHMR1 = (GP16C4T2->CHMR1 & ~TIMER_CHMR1_INPUT_CC2SSEL_MSK) |
                      (1U << TIMER_CHMR1_INPUT_CC2SSEL_POSS);
    GP16C4T2->CCEP = (GP16C4T2->CCEP & ~TIMER_CCEP_CC2POL_MSK) | TIMER_CCEP_CC2POL;
    GP16C4T2->CCEP |= TIMER_CCEP_CC2EN;
    GP16C4T2->CON1 |= TIMER_CON1_CNTEN;
    GP16C4T2->IER |= TIMER_IER_CH2;
    NVIC_EnableIRQ(GP16C4T2_IRQn);
    RCU->APB1SL |= RCU_APB1SL_GP16C4T2EN;
}

static bool enter_low_power_wait(void) {
    bool usb_irq_disabled = false;
    if ((GPIOC->ID & BIT(5)) == 0 && Keyboard_Info.Key_Mode == QMK_USB_MODE) {
        NVIC_DisableIRQ(USB_IRQn);
        usb_irq_disabled = true;
    }

    SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;
    RCU->CFG &= ~RCU_CFG_SW_MSK;
    RCU->CON &= ~RCU_CON_PLL0ON;

    if (Keyboard_Info.Key_Mode == QMK_USB_MODE) {
        RCU->AHBSL |= RCU_AHBSL_USBEN;
        __WFI();
    } else {
        RCU->AHBEN &= ~RCU_AHBEN_USBEN;
        RCU->CON &= ~RCU_CON_HRC48ON;
        __WFI();

        RCU->CON |= RCU_CON_HRC48ON;
        uint32_t timeout = 0x1869f;
        do {
            if ((RCU->CON & RCU_CON_HRC48RDY) != 0) {
                break;
            }
        } while (--timeout != 0);
        RCU->AHBEN |= RCU_AHBEN_USBEN;
    }

    RCU->CFG = (RCU->CFG & ~RCU_CFG_SW_MSK) | 3U;
    if (usb_irq_disabled) {
        NVIC_EnableIRQ(USB_IRQn);
    }
    return usb_irq_disabled;
}

static void disable_sleep_wakeup_irqs(void) {
    NVIC_DisableIRQ(EXTI_0to1_IRQn);
    NVIC_DisableIRQ(EXTI_2to3_IRQn);
    NVIC_DisableIRQ(EXTI_4to15_IRQn);
    NVIC_DisableIRQ(GP16C4T2_IRQn);
}

static void find_wakeup_key(uint8_t active_rows, uint8_t *wake_row, uint8_t *wake_col) {
    *wake_row = UINT8_MAX;
    *wake_col = UINT8_MAX;

    sleep_set_all_columns(true);
    for (uint8_t row = 0; row < MATRIX_ROWS; ++row) {
        if ((active_rows & BIT(row)) == 0) {
            continue;
        }
        for (uint8_t col = 0; col < MATRIX_COLS; ++col) {
            sleep_set_line_level(sleep_matrix_col_pins[col], false);
            if (sleep_line_is_low(sleep_matrix_row_pins[row])) {
                *wake_row = row;
                *wake_col = col;
                break;
            }
            sleep_set_all_columns(true);
        }
    }
    sleep_set_all_columns(true);
}

static void resume_usb_and_replay_wakeup_key(uint8_t wake_row, uint8_t wake_col) {
    if ((USBD1.status & BIT(1)) != 0) {
        if ((USB->POWER & BIT(0)) == 0) {
            USB->POWER |= BIT(0);
        }
        if ((USB->POWER & BIT(1)) != 0) {
            USB->POWER |= BIT(2);
            chThdSleep(11);
            USB->POWER &= (uint8_t)~BIT(2);
        }

        int16_t wait = 201;
        while (--wait != 0) {
            if (USBD1.state != USB_ACTIVE) {
                chThdSleep(10);
            }
        }
    }

    if (wake_row != UINT8_MAX && wake_col != UINT8_MAX) {
        uint8_t keycode = (uint8_t)dynamic_keymap_get_keycode(0, wake_row, wake_col);
        register_code(keycode);
        chThdSleep(2);
        keycode = (uint8_t)dynamic_keymap_get_keycode(0, wake_row, wake_col);
        unregister_code(keycode);
        chThdSleep(2);
    }
}

void es_chibios_user_idle_loop_hook(void) {
    if (Keyboard_Info.Key_Mode == QMK_USB_MODE) {
        if (!usb_suspend_requests_mcu_sleep()) {
            return;
        }
    } else if (Keyboard_Status.System_Sleep_Mode == 0) {
        return;
    }

    if (Spi_Send_Recv_Flg != 0) {
        return;
    }
    uint8_t ack_wait = 100;
    while ((GPIOA->ID & BIT(4)) == 0) {
        ack_wait--;
        chThdSleep(1);
        if (ack_wait == 0) {
            return;
        }
    }

    _pal_lld_setgroupmode(GPIOA, BIT(4), PAL_MODE_INPUT_PULLUP);
    SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;
    User_Systime_Deinit();
    es_ble_spi_deinit();
    User_Pwm_Deinit();
    User_Usb_Deinit();
    User_Adc_Deinit();

    Save_Flash            = false;
    Save_Flash_3S_Count   = 0;
    Usb_Change_Mode_Wakeup = false;
    Usb_Change_Mode_Delay = 0;
    Led_Power_Up          = false;

    prepare_matrix_wakeup_pins();
    configure_sleep_wakeup_sources();
    enter_low_power_wait();

    uint8_t active_rows = 0;
    for (uint8_t row = 0; row < MATRIX_ROWS; ++row) {
        if (sleep_line_is_low(sleep_matrix_row_pins[row])) {
            active_rows |= BIT(row);
        }
    }

    disable_sleep_wakeup_irqs();
    uint8_t wake_row;
    uint8_t wake_col;
    find_wakeup_key(active_rows, &wake_row, &wake_col);

    _pal_lld_setgroupmode(GPIOA, BIT(4), PAL_MODE_INPUT_PULLDOWN);
    SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;
    if (Keyboard_Info.Key_Mode == QMK_USB_MODE) {
        resume_usb_and_replay_wakeup_key(wake_row, wake_col);
    }
    Board_Wakeup_Init();
}

void Vector4C(void) {
    SYSCFG->WKSR |= BIT(31);
    EXTI->ICR |= EXTI_ICR_WAKEUP;
}

OSAL_IRQ_HANDLER(Vector54) {
    OSAL_IRQ_PROLOGUE();
    EXTI->ICR = EXTI->IFM;
    OSAL_IRQ_EPILOGUE();
}

void Vector58(void) {
    Vector54();
}

OSAL_IRQ_HANDLER(Vector5C) {
    OSAL_IRQ_PROLOGUE();

    uint32_t pending = EXTI->IFM;
    EXTI->ICR        = pending;

    if (!Init_Spi_Power_Up && (pending & BIT(4)) != 0 && Spi_Send_Recv_Flg != 0) {
        uint32_t ack = GPIOA->ID & BIT(4);

        if (Send_Key_Type == 0) {
            if (ack == 0) {
                Spi_Send_Recv_Flg = 0;
                Spi_Send_Recv_count = 0;
            }
        } else if (ack == 0) {
            if (Spi_Send_Recv_Flg == 2) {
                if (Emi_Test_Start && (g_es_spi_rx_buf[0] & 0x7f) == 0x3b) {
                    Get_Spi_Return_Data(g_es_spi_rx_buf);
                    Spi_Send_Recv_Flg   = 0;
                    Spi_Send_Recv_count = 0;
                } else {
                    if (g_es_spi_rx_buf[0] == USER_KEYBOARD_COMMAND) {
                        Get_Spi_Return_Data(g_es_spi_rx_buf);
                    } else if (++Repet_Send_Count < 3) {
                        Spi_Send_Recv_Flg   = 1;
                        Spi_Send_Recv_count = 0;
                        es_spi_send_recv_by_dma(USER_KEYBOARD_LENGTH, g_es_spi_rx_buf, g_es_spi_tx_buf);
                        goto irq_done;
                    } else {
                        Repet_Send_Count = 0;
                    }
                    Spi_Send_Recv_Flg   = 0;
                    Spi_Send_Recv_count = 0;
                }
            }
        } else if (Spi_Send_Recv_Flg == 1) {
            Spi_Send_Recv_Flg   = 2;
            Spi_Send_Recv_count = 0;
            memset(g_es_spi_rx_buf, 0, sizeof(g_es_spi_rx_buf));
            es_spi_send_recv_by_dma(USER_KEYBOARD_LENGTH, g_es_spi_rx_buf, (uint8_t *)0x1000);
        }
    }

irq_done:
    OSAL_IRQ_EPILOGUE();
}

OSAL_IRQ_HANDLER(Vector68) {
    OSAL_IRQ_PROLOGUE();
    DMA1->ICR = DMA1->IFM;
    OSAL_IRQ_EPILOGUE();
}

OSAL_IRQ_HANDLER(Vector7C) {
    OSAL_IRQ_PROLOGUE();
    GP32C4T1->ICR = GP32C4T1->IFM;
    OSAL_IRQ_EPILOGUE();
}

OSAL_IRQ_HANDLER(Vector80) {
    OSAL_IRQ_PROLOGUE();
    GP16C4T1->ICR = GP16C4T1->IFM;
    OSAL_IRQ_EPILOGUE();
}

OSAL_IRQ_HANDLER(Vector84) {
    OSAL_IRQ_PROLOGUE();
    GP16C4T2->ICR = GP16C4T2->IFM;
    OSAL_IRQ_EPILOGUE();
}
