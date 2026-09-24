/* Copyright 2023 Finalkey
* Copyright 2023 LiWenLiu <https://github.com/LiuLiuQMK>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../../../lib/rdr_lib/rdr_common.h"

/************************灯光测试变量********************/
bool Test_Led = false;
uint8_t Test_Colour = 0U;
/*********************************************************/

/************************eeprom**************************/
#if defined(EEPROM_CUSTOM)
/* Exported Constants --------------------------------------------------------*/
/* Define the size of the sectors to be used */
#define MCU_PAGE_SIZE           (uint32_t)0x200   /* MCU page size = 0.5kB */
#define PAGE_SIZE               (uint32_t)0x2000  /* Page size = 8kB */
#define MCU_PAGE_NUM            PAGE_SIZE / MCU_PAGE_SIZE

/* EEPROM start address in Flash */
#define EEPROM_START_ADDRESS  ((uint32_t)0x1C000)

/* Pages 0 and 1 base and end addresses */
#define PAGE0_BASE_ADDRESS    ((uint32_t)(EEPROM_START_ADDRESS))
#define PAGE0_END_ADDRESS     ((uint32_t)(PAGE0_BASE_ADDRESS + (PAGE_SIZE - 1)))

#define PAGE1_BASE_ADDRESS    ((uint32_t)(EEPROM_START_ADDRESS + PAGE_SIZE))
#define PAGE1_END_ADDRESS     ((uint32_t)(PAGE1_BASE_ADDRESS + (PAGE_SIZE - 1)))

/* Used Flash pages for EEPROM emulation */
#define PAGE0                 ((uint32_t)0x00000000)
#define PAGE1                 ((uint32_t)0x00000001)

/* No valid page define */
#define NO_VALID_PAGE         ((uint32_t)0x000000AB)

/* Page status definitions */
#define ERASED                ((uint32_t)0xFFFFFFFF)     /* Page is empty */
#define RECEIVE_DATA          ((uint32_t)0xEEEEEEEE)     /* Page is marked to receive data */
#define VALID_PAGE            ((uint32_t)0x00000000)     /* Page containing valid data */

/* Valid pages in read and write defines */
#define READ_FROM_VALID_PAGE  ((uint8_t)0x00)
#define WRITE_IN_VALID_PAGE   ((uint8_t)0x01)

/* Page full define */
#define PAGE_FULL             ((uint8_t)0x80)

/* Variables' number */
#define NB_OF_VAR             (EEPROM_SIZE)
#define NB_OF_PRIVATE_VAR     (64)

#define ES_MCU_MEM_REMAP_OFFSET  ((((SYSCFG->REMAP)&SYSCFG_REMAP_REALBASE_MSK) >> SYSCFG_REMAP_REALBASE_POSS) << 12)

//__attribute__((aligned(4))) static uint8_t g_es_flash_eeprom_table[EEPROM_SIZE + 2];
__attribute__((aligned(4))) static uint8_t g_es_flash_eeprom_table[EEPROM_SIZE + NB_OF_PRIVATE_VAR + 2];

volatile uint32_t g_tst_remap_offset;

static uint32_t ee_format(void);
static uint32_t ee_find_valid_page(uint8_t operation);
static uint32_t ee_verify_pagefull_write_variable(uint32_t virt_address, uint32_t data);
static uint32_t ee_page_transfer(uint32_t virt_address, uint32_t data);

static uint32_t IAPROM_PAGE_ERASE(uint32_t addr)
{
    md_fc_ControlTypeDef SErasePara;

    if ((addr & 0x1ff) != 0)
    {
        return !SET;
    }

    __disable_irq();

        md_fc_unlock();

        SErasePara.SAddr = addr;
        SErasePara.SAddrC = ~(addr);

        md_fc_page_erase(&SErasePara);

        md_fc_lock();
    __enable_irq();

    return SET;
}

static uint32_t IAPROM_WORD_PROGRAM(uint32_t addr,uint32_t data)
{
    md_fc_ControlTypeDef ProgramPara;

    if ((addr & 0x3) != 0)
    {
        return !SET;
    }

    if ((((uint32_t)(&data)) & 0x3) != 0)
    {
        return !SET;
    }

    __disable_irq();

        md_fc_unlock();

        ProgramPara.BCnt = 4;
        ProgramPara.pU32Buf = &data;
        ProgramPara.SAddr = addr;
        ProgramPara.SAddrC = ~(addr);

        md_fc_program(&ProgramPara);

        md_fc_lock();
    __enable_irq();

    return SET;
}

static uint32_t ee_init(void)
{
    uint32_t page_status0 = 6U, page_status1 = 6U;
    uint32_t var_idx = 0U;
    uint32_t eeprom_status = 0U;
    uint32_t flash_status;
    uint8_t addr_index;
    uint32_t rom_read_end;

    /* Get Page0 status */
    page_status0 = (*(__IO uint32_t *)(PAGE0_BASE_ADDRESS - ES_MCU_MEM_REMAP_OFFSET));
    /* Get Page1 status */
    page_status1 = (*(__IO uint32_t *)(PAGE1_BASE_ADDRESS - ES_MCU_MEM_REMAP_OFFSET));

    if(page_status0 == VALID_PAGE)
    {
        rom_read_end = PAGE0_END_ADDRESS - ES_MCU_MEM_REMAP_OFFSET;
        for(var_idx = PAGE0_BASE_ADDRESS - ES_MCU_MEM_REMAP_OFFSET + 4;var_idx < rom_read_end;var_idx += 4)
        {
            if(((*(__IO uint32_t *)var_idx) >> 16) < NB_OF_VAR)
            {
                *(((uint16_t*)((uint32_t)g_es_flash_eeprom_table)) + ((*(__IO uint32_t *)var_idx) >> 16)) = (*(__IO uint32_t *)var_idx) & 0xFFFF;
            }
            else
            {
                break;
            }
        }
    }

    if(page_status1 == VALID_PAGE)
    {
        rom_read_end = PAGE1_END_ADDRESS - ES_MCU_MEM_REMAP_OFFSET;
        for(var_idx = PAGE1_BASE_ADDRESS - ES_MCU_MEM_REMAP_OFFSET + 4;var_idx < rom_read_end;var_idx += 4)
        {
            if(((*(__IO uint32_t *)var_idx) >> 16) < NB_OF_VAR)
            {
                *(((uint16_t*)((uint32_t)g_es_flash_eeprom_table)) + ((*(__IO uint32_t *)var_idx) >> 16)) = (*(__IO uint32_t *)var_idx) & 0xFFFF;
            }
            else
            {
                break;
            }
        }
    }

    /* Check for invalid header states and repair if necessary */
    switch (page_status0)
    {
        case ERASED:
            if (page_status1 == VALID_PAGE) /* Page0 erased, Page1 valid */
            {
                /* Erase Page0 */
                for (addr_index = 0; addr_index < MCU_PAGE_NUM; addr_index++)
                {
                    IAPROM_PAGE_ERASE(PAGE0_BASE_ADDRESS + addr_index * MCU_PAGE_SIZE);
                }
            }
            else if (page_status1 == RECEIVE_DATA) /* Page0 erased, Page1 receive */
            {
                /* Erase Page0 */
                for (addr_index = 0; addr_index < MCU_PAGE_NUM; addr_index++)
                {
                    IAPROM_PAGE_ERASE(PAGE0_BASE_ADDRESS + addr_index * MCU_PAGE_SIZE);
                }

                IAPROM_WORD_PROGRAM(PAGE1_BASE_ADDRESS, VALID_PAGE);
            }
            else /* First EEPROM access (Page0&1 are erased) or invalid state -> format EEPROM */
            {
                /* Erase both Page0 and Page1 and set Page0 as valid page */
                flash_status = ee_format();

                if (flash_status != SET)
                    return flash_status;
            }

            break;

        case RECEIVE_DATA:
            if (page_status1 == VALID_PAGE) /* Page0 receive, Page1 valid */
            {
                /* Transfer data from Page1 to Page0 */
                for (var_idx = 0; var_idx < (NB_OF_VAR / 2 + 1); var_idx++)
                {
                    if((*(((__IO uint16_t*)((uint32_t)g_es_flash_eeprom_table)) + var_idx)) != 0)
                    {
                        /* Transfer the variable to the Page0 */
                            eeprom_status = ee_verify_pagefull_write_variable(var_idx, (*(((__IO uint16_t*)((uint32_t)g_es_flash_eeprom_table)) + var_idx)));

                            /* If program operation was failed, a Flash error code is returned */
                            if (eeprom_status != SET)
                                return eeprom_status;
                    }
                }

                IAPROM_WORD_PROGRAM(PAGE0_BASE_ADDRESS, VALID_PAGE);

                /* Erase Page1 */
                for (addr_index = 0; addr_index < MCU_PAGE_NUM; addr_index++)
                {
                    IAPROM_PAGE_ERASE(PAGE1_BASE_ADDRESS + addr_index * MCU_PAGE_SIZE);
                }
            }
            else if (page_status1 == ERASED) /* Page0 receive, Page1 erased */
            {
                /* Erase Page1 */
                for (addr_index = 0; addr_index < MCU_PAGE_NUM; addr_index++)
                {
                    IAPROM_PAGE_ERASE(PAGE1_BASE_ADDRESS + addr_index * MCU_PAGE_SIZE);
                }

                IAPROM_WORD_PROGRAM(PAGE0_BASE_ADDRESS, VALID_PAGE);
            }
            else /* Invalid state -> format eeprom */
            {
                flash_status = ee_format();
                if (flash_status != SET)
                    return flash_status;
            }

            break;

        case VALID_PAGE:
            if (page_status1 == VALID_PAGE) /* Invalid state -> format eeprom */
            {
                flash_status = ee_format();
                if (flash_status != SET)
                    return flash_status;
            }
            else if (page_status1 == ERASED) /* Page0 valid, Page1 erased */
            {
                for (addr_index = 0; addr_index < MCU_PAGE_NUM; addr_index++)
                {
                    IAPROM_PAGE_ERASE(PAGE1_BASE_ADDRESS + addr_index * MCU_PAGE_SIZE);
                }
            }
            else /* Page0 valid, Page1 receive */
            {
                /* Transfer data from Page0 to Page1 */
                for (var_idx = 0; var_idx < (NB_OF_VAR / 2 + 1); var_idx++)
                {
                    if((*(((__IO uint16_t*)((uint32_t)g_es_flash_eeprom_table)) + var_idx)) != 0)
                    {
                        /* Transfer the variable to the Page0 */
                            eeprom_status = ee_verify_pagefull_write_variable(var_idx, (*(((__IO uint16_t*)((uint32_t)g_es_flash_eeprom_table)) + var_idx)));

                            /* If program operation was failed, a Flash error code is returned */
                            if (eeprom_status != SET)
                                return eeprom_status;
                    }
                }

                IAPROM_WORD_PROGRAM(PAGE1_BASE_ADDRESS, VALID_PAGE);

                for (addr_index = 0; addr_index < MCU_PAGE_NUM; addr_index++)
                {
                    IAPROM_PAGE_ERASE(PAGE0_BASE_ADDRESS + addr_index * MCU_PAGE_SIZE);
                }
            }

            break;

        default:  /* Any other state -> format eeprom */
            flash_status = ee_format();
            if (flash_status != SET)
                return flash_status;

            break;
    }

    return SET;
}

static uint32_t ee_write_variable(uint32_t virt_address)
{
    uint32_t status = 0U;
    uint16_t data;

    if(virt_address >= NB_OF_VAR)
    {
        return !SET;
    }

    virt_address = virt_address / 2;

    data = *(((uint16_t*)((uint32_t)g_es_flash_eeprom_table)) + virt_address);

    /* Write the variable virtual address and value in the EEPROM */
    status = ee_verify_pagefull_write_variable(virt_address, data);

    /* In case the EEPROM active page is full */
    if (status == PAGE_FULL)
        status = ee_page_transfer(virt_address, data);  /* Perform Page transfer */

    /* Return last operation status */
    return status;
}

static uint32_t ee_format(void)
{
    uint32_t flash_status = SET;
    uint8_t addr_index;

    /* Erase Page0 */
    for (addr_index = 0; addr_index < MCU_PAGE_NUM; addr_index++)
    {
        IAPROM_PAGE_ERASE(PAGE0_BASE_ADDRESS + addr_index * MCU_PAGE_SIZE);
    }

    /* Set Page0 as valid page: Write VALID_PAGE at Page0 base address */
    IAPROM_WORD_PROGRAM(PAGE0_BASE_ADDRESS, VALID_PAGE);

    /* Erase Page1 */
    for (addr_index = 0; addr_index < MCU_PAGE_NUM; addr_index++)
    {
        IAPROM_PAGE_ERASE(PAGE1_BASE_ADDRESS + addr_index * MCU_PAGE_SIZE);
    }

    /* Return Page1 erase operation status */
    return flash_status;
}

static uint32_t ee_find_valid_page(uint8_t operation)
{
    uint32_t page_status0 = 6U, page_status1 = 6U;

    /* Get Page0 status */
    page_status0 = (*(__IO uint32_t *)(PAGE0_BASE_ADDRESS - ES_MCU_MEM_REMAP_OFFSET));
    /* Get Page1 status */
    page_status1 = (*(__IO uint32_t *)(PAGE1_BASE_ADDRESS - ES_MCU_MEM_REMAP_OFFSET));

    /* Write or read operation */
    switch (operation)
    {
        case WRITE_IN_VALID_PAGE:   /* ---- Write operation ---- */
            if (page_status1 == VALID_PAGE)
            {
                /* Page0 receiving data */
                if (page_status0 == RECEIVE_DATA)
                    return PAGE0;         /* Page0 valid */
                else
                    return PAGE1;         /* Page1 valid */
            }
            else if (page_status0 == VALID_PAGE)
            {
                /* Page1 receiving data */
                if (page_status1 == RECEIVE_DATA)
                    return PAGE1;         /* Page1 valid */
                else
                    return PAGE0;         /* Page0 valid */
            }
            else
            {
                return NO_VALID_PAGE;   /* No valid Page */
            }

        case READ_FROM_VALID_PAGE:  /* ---- Read operation ---- */
            if (page_status0 == VALID_PAGE)
                return PAGE0;           /* Page0 valid */
            else if (page_status1 == VALID_PAGE)
                return PAGE1;           /* Page1 valid */
            else
                return NO_VALID_PAGE ;  /* No valid Page */

        default:
            return PAGE0;             /* Page0 valid */
    }
}

static uint32_t ee_verify_pagefull_write_variable(uint32_t virt_address, uint32_t data)
{
    uint32_t flash_status = SET;
    uint32_t valid_page = PAGE0;
    uint32_t address = EEPROM_START_ADDRESS;
    uint32_t page_endaddress = EEPROM_START_ADDRESS + PAGE_SIZE;

    /* Get valid Page for write operation */
    valid_page = ee_find_valid_page(WRITE_IN_VALID_PAGE);

    /* Check if there is no valid page */
    if (valid_page == NO_VALID_PAGE)
        return NO_VALID_PAGE;

    /* Get the valid Page start address */
    address = (uint32_t)(EEPROM_START_ADDRESS + (uint32_t)(valid_page * PAGE_SIZE)) - ES_MCU_MEM_REMAP_OFFSET;

    /* Get the valid Page end address */
    page_endaddress = (uint32_t)((EEPROM_START_ADDRESS - 4) + (uint32_t)((1 + valid_page) * PAGE_SIZE)) - ES_MCU_MEM_REMAP_OFFSET;

    /* Check each active page address starting from begining */
    while (address <= page_endaddress)
    {
        /* Verify if address and address+4 contents are 0xFFFFFFFF */
        if ((*(__IO uint32_t *)address) == 0xFFFFFFFF)
        {
            /* Set variable virtual address and data */
            flash_status = IAPROM_WORD_PROGRAM(address + ES_MCU_MEM_REMAP_OFFSET, (virt_address << 16) | data);
            /* Return program operation status */
            return flash_status;
        }
        else
        {
            /* Next address location */
            address = address + 4;
        }
    }

    /* Return PAGE_FULL in case the valid page is full */
    return PAGE_FULL;
}

static uint32_t ee_page_transfer(uint32_t virt_address, uint32_t data)
{
    uint32_t flash_status = SET;
    uint32_t new_pageaddress = EEPROM_START_ADDRESS;
    uint32_t old_pageaddress;
    uint32_t valid_page = PAGE0, var_idx = 0U;
    uint32_t eeprom_status = 0U;
    uint8_t addr_index;

    /* Get active Page for read operation */
    valid_page = ee_find_valid_page(READ_FROM_VALID_PAGE);

    if (valid_page == PAGE1)       /* Page1 valid */
    {
        /* New page address where variable will be moved to */
        new_pageaddress = PAGE0_BASE_ADDRESS;

        /* Old page ID where variable will be taken from */
        old_pageaddress = PAGE1_BASE_ADDRESS;
    }
    else if (valid_page == PAGE0)  /* Page0 valid */
    {
        /* New page address  where variable will be moved to */
        new_pageaddress = PAGE1_BASE_ADDRESS;

        /* Old page ID where variable will be taken from */
        old_pageaddress = PAGE0_BASE_ADDRESS;
    }
    else
    {
        return NO_VALID_PAGE;       /* No valid Page */
    }

    /* Erase the new Page*/
    for (addr_index = 0; addr_index < MCU_PAGE_NUM; addr_index++)
    {
        IAPROM_PAGE_ERASE(new_pageaddress + addr_index * MCU_PAGE_SIZE);
    }

    /* Set the new Page status to RECEIVE_DATA status */
    IAPROM_WORD_PROGRAM(new_pageaddress, RECEIVE_DATA);

    /* Write the variable passed as parameter in the new active page */
    eeprom_status = ee_verify_pagefull_write_variable(virt_address, data);

    /* If program operation was failed, a Flash error code is returned */
    if (eeprom_status != SET)
        return eeprom_status;

    /* Transfer process: transfer variables from old to the new active page */
    //for (var_idx = 0; var_idx < NB_OF_VAR; var_idx++)
    {
                /* Transfer data from Page1 to Page0 */
                for (var_idx = 0; var_idx < (NB_OF_VAR / 2 + 1); var_idx++)
                {
                    if((*(((__IO uint16_t*)((uint32_t)g_es_flash_eeprom_table)) + var_idx)) != 0)
                    {
                        /* Transfer the variable to the Page0 */
                            eeprom_status = ee_verify_pagefull_write_variable(var_idx, (*(((__IO uint16_t*)((uint32_t)g_es_flash_eeprom_table)) + var_idx)));

                            /* If program operation was failed, a Flash error code is returned */
                            if (eeprom_status != SET)
                                return eeprom_status;
                    }
                }
    }

    /* Erase the old Page: Set old Page status to ERASED status */
    for (addr_index = 0; addr_index < MCU_PAGE_NUM; addr_index++)
    {
        IAPROM_PAGE_ERASE(old_pageaddress + addr_index * MCU_PAGE_SIZE);
    }

    /* Set new Page status to VALID_PAGE status */
    IAPROM_WORD_PROGRAM(new_pageaddress, VALID_PAGE);

    /* Return last operation flash status */
    return flash_status;
}

size_t clamp_length(intptr_t offset, size_t len) {
    if (offset + len > EEPROM_SIZE) {
        len = EEPROM_SIZE - offset;
    }

    return len;
}

size_t clamp_length_user(intptr_t offset, size_t len) {
    if (offset + len > NB_OF_PRIVATE_VAR) {
        len = NB_OF_PRIVATE_VAR - offset;
    }

    return len;
}

void eeprom_driver_erase(void) {
    ee_format();
    memset(g_es_flash_eeprom_table, 0x00, sizeof(g_es_flash_eeprom_table));
}

#define USER_EEPROM_START_ADDRESS1  (EEPROM_START_ADDRESS - (MCU_PAGE_SIZE))
volatile uint8_t es_eeprom_init_flag = 0U;

void eeprom_driver_init(void) {
    if(es_eeprom_init_flag)
        return;

    g_tst_remap_offset = ES_MCU_MEM_REMAP_OFFSET;
    (void)g_tst_remap_offset;
    memset(g_es_flash_eeprom_table, 0x00, sizeof(g_es_flash_eeprom_table));
    ee_init();
    memcpy(g_es_flash_eeprom_table, (uint8_t*)(USER_EEPROM_START_ADDRESS1 - ES_MCU_MEM_REMAP_OFFSET), NB_OF_PRIVATE_VAR);
    es_eeprom_init_flag = 1;
}
/*****************eeprom块读取********************/
void eeprom_read_block(void *buf, const void *addr, size_t len) {

    intptr_t offset = (intptr_t)addr;
    memset(buf, 0x00, len);//将buf地址里面的len长度字节全部初始化为0x00
    len = clamp_length(offset, len);
    if (len > 0) {
        memcpy(buf, &g_es_flash_eeprom_table[NB_OF_PRIVATE_VAR + offset], len);
    }
}
/*****************eeprom块写入********************/
void eeprom_write_block(const void *buf, void *addr, size_t len) {
    uint16_t i;
    intptr_t offset = (intptr_t)addr;
                len = clamp_length(offset, len);
    if (len > 0) {
        for(i = 0;i < len;i++)
        {
            if(g_es_flash_eeprom_table[i + offset + NB_OF_PRIVATE_VAR] != (*((uint8_t*)((uint32_t)buf) + i)))
            {
                g_es_flash_eeprom_table[i + offset + NB_OF_PRIVATE_VAR] = (*((uint8_t*)((uint32_t)buf) + i));
                ee_write_variable(i + offset + NB_OF_PRIVATE_VAR);

            }
        }

        memcpy(&g_es_flash_eeprom_table[offset + NB_OF_PRIVATE_VAR], buf, len);
    }
}

void eeprom_read_block_user(void *buf, const void *addr, size_t len) {
    intptr_t offset = (intptr_t)addr;
    memset(buf, 0x00, len);
    len = clamp_length_user(offset, len);
    if (len > 0) {
        memcpy(buf, (uint8_t*)(USER_EEPROM_START_ADDRESS1 - ES_MCU_MEM_REMAP_OFFSET) + offset, len);
    }
}

void eeprom_write_block_user(const void *buf, void *addr, size_t len) {
    md_fc_ControlTypeDef ProgramPara;
    intptr_t offset = (intptr_t)addr;
    len             = clamp_length_user(offset, len);
    if (len > 0) {
        __disable_irq();
            if(memcmp(buf,(uint8_t*)(USER_EEPROM_START_ADDRESS1 - ES_MCU_MEM_REMAP_OFFSET) + offset,len))
            {
                    memcpy(&g_es_flash_eeprom_table[offset], buf, len);

                    md_fc_unlock();
                    ProgramPara.SAddr = USER_EEPROM_START_ADDRESS1;
                    ProgramPara.SAddrC = ~(USER_EEPROM_START_ADDRESS1);
                    md_fc_page_erase(&ProgramPara);
                    md_fc_lock();

                    md_fc_unlock();
                    ProgramPara.BCnt = NB_OF_PRIVATE_VAR;
                    ProgramPara.pU32Buf = (uint32_t*)(&g_es_flash_eeprom_table);
                    ProgramPara.SAddr = USER_EEPROM_START_ADDRESS1;
                    ProgramPara.SAddrC = ~(USER_EEPROM_START_ADDRESS1);
                    md_fc_program(&ProgramPara);
                    md_fc_lock();
            }
        __enable_irq();
    }
}
#endif
/*********************************************************/

/************************ENCODER**************************/
/* Quadrature encoder portions derived from QMK drivers/encoder/encoder_quadrature.c.
 * Copyright 2018 Jack Humbert <jack.humb@gmail.com>
 * Copyright 2018-2023 Nick Brassel (@tzarc)
 * SPDX-License-Identifier: GPL-2.0-or-later
 * Attribution added by KIBU on 2026-09-23; no behavior change.
 */
#ifdef ENCODER_ENABLE

#if !defined(ENCODER_RESOLUTIONS) && !defined(ENCODER_RESOLUTION)
#    define ENCODER_RESOLUTION 2
#endif

#undef ENCODER_DEFAULT_PIN_API_IMPL
#if defined(ENCODERS_PAD_A) && defined(ENCODERS_PAD_B)
// Inform the quadrature driver that it needs to implement pin init/read functions
#    define ENCODER_DEFAULT_PIN_API_IMPL
#endif

extern volatile bool isLeftHand;

__attribute__((weak)) void    encoder_quadrature_init_pin(uint8_t index, bool pad_b);
__attribute__((weak)) uint8_t encoder_quadrature_read_pin(uint8_t index, bool pad_b);

#ifdef ENCODER_DEFAULT_PIN_API_IMPL

static pin_t encoders_pad_a[NUM_ENCODERS_MAX_PER_SIDE] = ENCODERS_PAD_A;
static pin_t encoders_pad_b[NUM_ENCODERS_MAX_PER_SIDE] = ENCODERS_PAD_B;

__attribute__((weak)) void encoder_wait_pullup_charge(void) {
    wait_us(100);
}

__attribute__((weak)) void encoder_quadrature_init_pin(uint8_t index, bool pad_b) {
    pin_t pin = pad_b ? encoders_pad_b[index] : encoders_pad_a[index];
    if (pin != NO_PIN) {
        gpio_set_pin_input_high(pin);
    }
}

__attribute__((weak)) uint8_t encoder_quadrature_read_pin(uint8_t index, bool pad_b) {
    pin_t pin = pad_b ? encoders_pad_b[index] : encoders_pad_a[index];
    if (pin != NO_PIN) {
        return gpio_read_pin(pin) ? 1 : 0;
    }
    return 0;
}

#endif // ENCODER_DEFAULT_PIN_API_IMPL

#ifdef ENCODER_RESOLUTIONS
static uint8_t encoder_resolutions[NUM_ENCODERS] = ENCODER_RESOLUTIONS;
#endif

#ifndef ENCODER_DIRECTION_FLIP
#    define ENCODER_CLOCKWISE true
#    define ENCODER_COUNTER_CLOCKWISE false
#else
#    define ENCODER_CLOCKWISE false
#    define ENCODER_COUNTER_CLOCKWISE true
#endif
static int8_t encoder_LUT[] = {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};

static uint8_t encoder_state[NUM_ENCODERS]  = {0};
static int8_t  encoder_pulses[NUM_ENCODERS] = {0};

// encoder counts
static uint8_t thisCount;

__attribute__((weak)) void encoder_quadrature_post_init_kb(void) {
    extern void encoder_quadrature_handle_read(uint8_t index, uint8_t pin_a_state, uint8_t pin_b_state);
    // Unused normally, but can be used for things like setting up pin-change interrupts in keyboard code.
    // During the interrupt, read the pins then call `encoder_handle_read()` with the pin states and it'll queue up an encoder event if needed.
}

void encoder_quadrature_post_init(void) {
#ifdef ENCODER_DEFAULT_PIN_API_IMPL
    for (uint8_t i = 0; i < thisCount; i++) {
        encoder_quadrature_init_pin(i, false);
        encoder_quadrature_init_pin(i, true);
    }
    encoder_wait_pullup_charge();
    for (uint8_t i = 0; i < thisCount; i++) {
        encoder_state[i] = (encoder_quadrature_read_pin(i, false) << 0) | (encoder_quadrature_read_pin(i, true) << 1);
    }
#else
    memset(encoder_state, 0, sizeof(encoder_state));
#endif

    encoder_quadrature_post_init_kb();
}

void encoder_driver_init(void) {
    thisCount                = NUM_ENCODERS;

#ifdef ENCODER_TESTS
    // Annoying that we have to clear out values during initialisation here, but
    // because all the arrays are static locals, rerunning tests in the same
    // executable doesn't reset any of these. Kinda crappy having test-only code
    // here, but it's the simplest solution.
    memset(encoder_state, 0, sizeof(encoder_state));
    memset(encoder_pulses, 0, sizeof(encoder_pulses));
    const pin_t encoders_pad_a_left[] = ENCODERS_PAD_A;
    const pin_t encoders_pad_b_left[] = ENCODERS_PAD_B;
    for (uint8_t i = 0; i < thisCount; i++) {
        encoders_pad_a[i] = encoders_pad_a_left[i];
        encoders_pad_b[i] = encoders_pad_b_left[i];
    }
#endif

    encoder_quadrature_post_init();
}

static void encoder_handle_state_change(uint8_t index, uint8_t state) {
    uint8_t i = index;

#ifdef ENCODER_RESOLUTIONS
    const uint8_t resolution = encoder_resolutions[index];
#else
    const uint8_t resolution = ENCODER_RESOLUTION;
#endif

    encoder_pulses[i] += encoder_LUT[state & 0xF];

#ifdef ENCODER_DEFAULT_POS
    if ((encoder_pulses[i] >= resolution) || (encoder_pulses[i] <= -resolution) || ((state & 0x3) == ENCODER_DEFAULT_POS)) {
        if (encoder_pulses[i] >= 1) {
#else
    if (encoder_pulses[i] >= resolution) {
#endif

            encoder_queue_event(index, ENCODER_COUNTER_CLOCKWISE);
        }

#ifdef ENCODER_DEFAULT_POS
        if (encoder_pulses[i] <= -1) {
#else
    if (encoder_pulses[i] <= -resolution) { // direction is arbitrary here, but this clockwise
#endif
            encoder_queue_event(index, ENCODER_CLOCKWISE);
        }
        encoder_pulses[i] %= resolution;
#ifdef ENCODER_DEFAULT_POS
        encoder_pulses[i] = 0;
    }
#endif
}

void encoder_quadrature_handle_read(uint8_t index, uint8_t pin_a_state, uint8_t pin_b_state) {
    uint8_t state = pin_a_state | (pin_b_state << 1);
    if ((encoder_state[index] & 0x3) != state) {
        encoder_state[index] <<= 2;
        encoder_state[index] |= state;
        encoder_handle_state_change(index, encoder_state[index]);
    }
}

__attribute__((weak)) void encoder_driver_task(void) {
    for (uint8_t i = 0; i < thisCount; i++) {
        encoder_quadrature_handle_read(i, encoder_quadrature_read_pin(i, false), encoder_quadrature_read_pin(i, true));
    }
}
#endif
/*********************************************************/

/************************灯光*****************************/
#define ES_PWM_LED_SIZE         (42)
#define ES_PWM_LED_BYTE         (24)
#define ES_PWM_DMA_SIZE         (ES_PWM_LED_SIZE * ES_PWM_LED_BYTE)

#define ES_PWM_WS2812_H_VALUE   (43)
#define ES_PWM_WS2812_L_VALUE   (17)

rgb_led_t rgb_matrix_ws2812_array[RGB_MATRIX_LED_COUNT];
uint8_t g_es_pwm_rgb_matrix_array_dma_buf[(RGB_MATRIX_LED_COUNT * ES_PWM_LED_BYTE) + 2] = {0};
md_dma_channel_config_typedef DMA_list[5] = {0};
/*****************rgb矩阵驱动初始化********************/
void rgb_matrix_driver_init(void)
{
    md_rcu_enable_dma1(RCU);//使能同步机制
    md_dma_set_configuration(DMA1, ENABLE);

    md_rcu_enable_gp16c2t1(RCU);
    md_timer_set_auto_reload_value_arrv(GP16C2T1, 60);//配置定时器C型GP16C2T1自动重装载
    md_timer_set_output_compare1_mode_ch1mod(GP16C2T1, MD_TIMER_OUTPUTMODE_PWMMODE1);//配置输出比较模式
    md_timer_set_capture_compare1_value_ccrv1(GP16C2T1, 0);//设置输入捕获寄存器值
    md_timer_enable_cc1_output_cc1en(GP16C2T1);//使能捕获计数器 ,md_timer_enable_main_output_goen(GP16C2T1);//使能死区和刹车寄存器
    md_timer_enable_main_output_goen(GP16C2T1);

    md_timer_enable_output_compare1_preload_ch1pen(GP16C2T1);//使能定时器预装载

    md_timer_enable_dma_upd(GP16C2T1);
    md_timer_enable_counter_cnten(GP16C2T1);

    gpio_set_pin_output(ES_PWM_DMA_IO);

    /*复用为PWM DMA方式*/
    GPIOA->AFL &= 0xFFFFF0FF;
    GPIOA->AFL |= 0x00000500;

    GPIOA->MOD &= 0xFFFFFFCF;
    GPIOA->MOD |= 0x00000020;

    md_dma_set_request_peripherals(DMA1, MD_DMA_CHANNEL2, MD_DMA_PRS_GP16C2T1_UP);

    if (rgb_matrix_get_val() <= 0) {                //优化背光速度调节到最低的时候，休眠唤醒灯光不亮问题
        memset((void *)g_es_pwm_rgb_matrix_array_dma_buf, ES_PWM_WS2812_L_VALUE, (RGB_MATRIX_LED_COUNT * ES_PWM_LED_BYTE) + 2);
    }
}

void User_Pwm_Deinit(void) {
    md_rcu_enable_gp16c2t1_reset(RCU);
    md_rcu_disable_gp16c2t1_reset(RCU);
    md_rcu_disable_gp16c2t1(RCU);
    md_rcu_enable_dma1_reset(RCU);
    md_rcu_disable_dma1_reset(RCU);
    md_rcu_disable_dma1(RCU);

    gpio_set_pin_output(ES_PWM_DMA_IO);
    gpio_write_pin_low(ES_PWM_DMA_IO);
}

void rgb_matrix_driver_flush_pwm_dma_start(void)
{
    while ((DMA1->CHENSET) & 0x4);

    if (Keyboard_Status.System_Sleep_Mode || ((Keyboard_Info.Key_Mode != QMK_USB_MODE) && Usb_Change_Mode_Wakeup && Keyboard_Status.System_Work_Status) || (!Led_Power_Up)) {
        Led_Off_Start = true;
        gpio_write_pin_low(ES_LED_POWER_IO);
        return;
    }

    if (rgblight_is_enabled() && (Led_Point_Sleep == false)) {
        gpio_write_pin_high(ES_LED_POWER_IO);
        if (Led_Off_Start) {
            Led_Off_Start = false;
            wait_ms(3);
        }
    } else {                            //LED灯光休眠，停止向LED灯供电
        Led_Off_Start = true;
        gpio_write_pin_low(ES_LED_POWER_IO);
    }

    md_timer_disable_dma_upd(GP16C2T1);

    /*将buff的最后两个字节赋值为0*/
    g_es_pwm_rgb_matrix_array_dma_buf[(RGB_MATRIX_LED_COUNT * ES_PWM_LED_BYTE)] = 0;
    g_es_pwm_rgb_matrix_array_dma_buf[(RGB_MATRIX_LED_COUNT * ES_PWM_LED_BYTE) + 1] = 0;

    #if (RGB_MATRIX_LED_COUNT <= 42)
        uint16_t Data_Size = ((RGB_MATRIX_LED_COUNT * ES_PWM_LED_BYTE) + 2);
        DMA_list[0].control.word = ((Data_Size - 1) << 4) | g_es_dma_ch2alt_cfg;
        DMA_list[0].source_data_end_address = (uint32_t)(g_es_pwm_rgb_matrix_array_dma_buf  + (Data_Size - 1));
        DMA_list[0].destination_data_end_address = (uint32_t)(&(GP16C2T1->CCVAL1));

        DMA1->PRI_CH02_SRC_DATA_END_PTR = ((uint32_t)(DMA_list)) + ((16 - 4));
        DMA1->PRI_CH02_CHANNEL_CFG = ((4 - 1) << 4) | g_es_dma_ch2pri_cfg;
    #elif (RGB_MATRIX_LED_COUNT <= 84)
        DMA_list[0].control.word = ((ES_PWM_DMA_SIZE - 1) << 4) | g_es_dma_ch2alt_cfg;
        DMA_list[0].source_data_end_address = (uint32_t)(g_es_pwm_rgb_matrix_array_dma_buf  + (ES_PWM_DMA_SIZE - 1));
        DMA_list[0].destination_data_end_address = (uint32_t)(&(GP16C2T1->CCVAL1));

        uint16_t Data_Size = (((RGB_MATRIX_LED_COUNT - ES_PWM_LED_SIZE) * ES_PWM_LED_BYTE) + 2);
        DMA_list[1].control.word = ((Data_Size - 1) << 4) | g_es_dma_ch2alt_cfg;
        DMA_list[1].source_data_end_address = (uint32_t)(g_es_pwm_rgb_matrix_array_dma_buf  + (ES_PWM_DMA_SIZE + Data_Size - 1));
        DMA_list[1].destination_data_end_address = (uint32_t)(&(GP16C2T1->CCVAL1));

        DMA1->PRI_CH02_SRC_DATA_END_PTR = ((uint32_t)(DMA_list)) + ((32 - 4));
        DMA1->PRI_CH02_CHANNEL_CFG = ((8 - 1) << 4) | g_es_dma_ch2pri_cfg;
    #elif (RGB_MATRIX_LED_COUNT <= 126)
        DMA_list[0].control.word = ((ES_PWM_DMA_SIZE - 1) << 4) | g_es_dma_ch2alt_cfg;
        DMA_list[0].source_data_end_address = (uint32_t)(g_es_pwm_rgb_matrix_array_dma_buf  + (ES_PWM_DMA_SIZE - 1));
        DMA_list[0].destination_data_end_address = (uint32_t)(&(GP16C2T1->CCVAL1));

        DMA_list[1].control.word = ((ES_PWM_DMA_SIZE - 1) << 4) | g_es_dma_ch2alt_cfg;
        DMA_list[1].source_data_end_address = (uint32_t)(g_es_pwm_rgb_matrix_array_dma_buf  + ((ES_PWM_DMA_SIZE * 2) - 1));
        DMA_list[1].destination_data_end_address = (uint32_t)(&(GP16C2T1->CCVAL1));

        uint16_t Data_Size = (((RGB_MATRIX_LED_COUNT - (ES_PWM_LED_SIZE * 2)) * ES_PWM_LED_BYTE) + 2);
        DMA_list[2].control.word = ((Data_Size - 1) << 4) | g_es_dma_ch2alt_cfg;
        DMA_list[2].source_data_end_address = (uint32_t)(g_es_pwm_rgb_matrix_array_dma_buf + ((ES_PWM_DMA_SIZE * 2) + Data_Size - 1));
        DMA_list[2].destination_data_end_address = (uint32_t)(&(GP16C2T1->CCVAL1));

        DMA1->PRI_CH02_SRC_DATA_END_PTR = ((uint32_t)(DMA_list)) + ((48 - 4));
        DMA1->PRI_CH02_CHANNEL_CFG = ((12 - 1) << 4) | g_es_dma_ch2pri_cfg;
    #elif (RGB_MATRIX_LED_COUNT <= 168)
        DMA_list[0].control.word = ((ES_PWM_DMA_SIZE - 1) << 4) | g_es_dma_ch2alt_cfg;
        DMA_list[0].source_data_end_address = (uint32_t)(g_es_pwm_rgb_matrix_array_dma_buf  + (ES_PWM_DMA_SIZE - 1));
        DMA_list[0].destination_data_end_address = (uint32_t)(&(GP16C2T1->CCVAL1));

        DMA_list[1].control.word = ((ES_PWM_DMA_SIZE - 1) << 4) | g_es_dma_ch2alt_cfg;
        DMA_list[1].source_data_end_address = (uint32_t)(g_es_pwm_rgb_matrix_array_dma_buf  + ((ES_PWM_DMA_SIZE * 2) - 1));
        DMA_list[1].destination_data_end_address = (uint32_t)(&(GP16C2T1->CCVAL1));

        DMA_list[2].control.word = ((ES_PWM_DMA_SIZE - 1) << 4) | g_es_dma_ch2alt_cfg;
        DMA_list[2].source_data_end_address = (uint32_t)(g_es_pwm_rgb_matrix_array_dma_buf  + ((ES_PWM_DMA_SIZE * 3) - 1));
        DMA_list[2].destination_data_end_address = (uint32_t)(&(GP16C2T1->CCVAL1));

        uint16_t Data_Size = (((RGB_MATRIX_LED_COUNT - (ES_PWM_LED_SIZE * 3)) * ES_PWM_LED_BYTE) + 2);
        DMA_list[3].control.word = ((Data_Size - 1) << 4) | g_es_dma_ch2alt_cfg;
        DMA_list[3].source_data_end_address = (uint32_t)(g_es_pwm_rgb_matrix_array_dma_buf  + ((ES_PWM_DMA_SIZE * 3) + Data_Size - 1));
        DMA_list[3].destination_data_end_address = (uint32_t)(&(GP16C2T1->CCVAL1));

        DMA1->PRI_CH02_SRC_DATA_END_PTR = ((uint32_t)(DMA_list)) + ((64 - 4));
        DMA1->PRI_CH02_CHANNEL_CFG = ((16 - 1) << 4) | g_es_dma_ch2pri_cfg;
    #endif

    DMA1->PRI_CH02_DST_DATA_END_PTR = ((uint32_t)(&(DMA1->ALT_CH02_SRC_DATA_END_PTR))) + 16 - 4;
    DMA1->CHENSET = 0x4;
    md_timer_enable_dma_upd(GP16C2T1);
}

void rgb_matrix_driver_flush(void)
{
    Led_Flash_Busy = false;
    if((GP16C2T1->AR) != 60)
    {
        rgb_matrix_driver_init();
    }

    rgb_matrix_driver_flush_pwm_dma_start();
}

void rgb_matrix_driver_set_color(int index, uint8_t r, uint8_t g, uint8_t b)
{
    uint8_t * buf;

    if (index == (RGB_MATRIX_LED_COUNT - 1)) {
        Led_Flash_Busy = false;
    } else if(index == 0){
        Led_Flash_Busy = true;
    }

    if (rgb_matrix_ws2812_array[index].r == r && rgb_matrix_ws2812_array[index].g == g && rgb_matrix_ws2812_array[index].b == b) {
        return;
    }

    rgb_matrix_ws2812_array[index].r = r;
    rgb_matrix_ws2812_array[index].g = g;
    rgb_matrix_ws2812_array[index].b = b;

    buf = g_es_pwm_rgb_matrix_array_dma_buf + (index * 24);

    for (unsigned char bit = 0; bit < 8; bit++) {
        bool is_one = g & (1 << (7 - bit));
        // using something like wait_ns(is_one ? T1L : T0L) here throws off timings

        *buf = (is_one)? ES_PWM_WS2812_H_VALUE : ES_PWM_WS2812_L_VALUE;
        buf++;
    }

    for (unsigned char bit = 0; bit < 8; bit++) {
        bool is_one = r & (1 << (7 - bit));
        // using something like wait_ns(is_one ? T1L : T0L) here throws off timings

        *buf = (is_one)? ES_PWM_WS2812_H_VALUE : ES_PWM_WS2812_L_VALUE;
        buf++;
    }

    for (unsigned char bit = 0; bit < 8; bit++) {
        bool is_one = b & (1 << (7 - bit));
        // using something like wait_ns(is_one ? T1L : T0L) here throws off timings

        *buf = (is_one)? ES_PWM_WS2812_H_VALUE : ES_PWM_WS2812_L_VALUE;
        buf++;
    }
}

void rgb_matrix_driver_set_color_all(uint8_t r, uint8_t g, uint8_t b)
{
    for(uint32_t i = 0;i < RGB_MATRIX_LED_COUNT;i++) {
        rgb_matrix_driver_set_color(i,r,g,b);
    }
}

// clang-format off
const rgb_matrix_driver_t rgb_matrix_driver = {
    .init          = rgb_matrix_driver_init,
    .flush         = rgb_matrix_driver_flush,
    .set_color     = rgb_matrix_driver_set_color,
    .set_color_all = rgb_matrix_driver_set_color_all,
};

#if LOGO_LED_ENABLE
static uint8_t LED_Mix_Colour_Tab[256][3] =  {
    {255, 0,   0  },//红 0
    {255, 1,   0  },//   1
    {255, 3,   0  },//   2
    {255, 4,   0  },//   3
    {255, 7,   0  },//   4
    {255, 8,   0  },//   5
    {255, 10,  0  },//   6
    {255, 12,  0  },//   7
    {255, 14,  0  },//   8
    {255, 15,  0  },//   9
    {255, 17,  0  },//   10
    {255, 21,  0  },//   11
    {255, 24,  0  },//   12
    {255, 27,  0  },//   13
    {255, 30,  0  },//   14
    {255, 35,  0  },//   15
    {255, 38,  0  },//   16
    {255, 41,  0  },//   17
    {255, 44,  0  },//   18
    {255, 45,  0  },//   19
    {255, 48,  0  },//   20
    {255, 51,  0  },//   21
    {255, 55,  0  },//   22
    {255, 58,  0  },//   23
    {255, 62,  0  },//橙 24
    {255, 64,  0  },//   25
    {255, 68,  0  },//   26
    {255, 71,  0  },//   27
    {255, 74,  0  },//   28
    {255, 78,  0  },//   29
    {255, 80,  0  },//   30
    {255, 82,  0  },//   31
    {255, 84,  0  },//   32
    {255, 88,  0  },//   33
    {255, 90,  0  },//   34
    {255, 95,  0  },//   35
    {255, 100, 0  },//   36
    {255, 103, 0  },//   37
    {255, 107, 0  },//   38
    {255, 110, 0  },//   39
    {255, 113, 0  },//   40
    {255, 117, 0  },//   41
    {255, 120, 0  },//   42
    {255, 124, 0  },//   43
    {255, 128, 0  },//   44
    {255, 130, 0  },//   45
    {255, 132, 0  },//   46
    {255, 136, 0  },//   47
    {255, 140, 0  },//   48
    {255, 142, 0  },//   49
    {255, 147, 0  },//   50
    {255, 149, 0  },//   51
    {255, 151, 0  },//   52
    {255, 153, 0  },//   53
    {255, 158, 0  },//   54
    {255, 160, 0  },//   55
    {255, 162, 0  },//   56
    {255, 168, 0  },//   57
    {255, 171, 0  },//   58
    {255, 174, 0  },//   59
    {255, 177, 0  },//   60
    {255, 180, 0  },//   61
    {255, 183, 0  },//   62
    {255, 187, 0  },//   63
    {255, 190, 0  },//   64
    {255, 193, 0  },//   65
    {255, 196, 0  },//   66
    {255, 199, 0  },//   67
    {255, 201, 0  },//   68
    {255, 205, 0  },//   69
    {255, 208, 0  },//   70
    {255, 212, 0  },//   71
    {255, 220, 0  },//   72
    {255, 225, 0  },//   73
    {255, 230, 0  },//   74
    {255, 235, 0  },//   75
    {255, 238, 0  },//   76
    {255, 240, 0  },//   77
    {255, 245, 0  },//   78
    {255, 250, 0  },//   79
    {255, 254, 0  },//   80

    {255, 255, 0  },//黄 0
    {245, 255, 0  },//   1
    {231, 255, 0  },//   2
    {224, 255, 0  },//   3
    {217, 255, 0  },//   4
    {203, 255, 0  },//   5
    {196, 255, 0  },//   6
    {189, 255, 0  },//   7
    {175, 255, 0  },//   8
    {168, 255, 0  },//   9
    {161, 255, 0  },//   10
    {147, 255, 0  },//   11
    {140, 255, 0  },//   12
    {133, 255, 0  },//   13
    {126, 255, 0  },//   14
    {119, 255, 0  },//   15
    {105, 255, 0  },//   16
    {98,  255, 0  },//   17
    {91,  255, 0  },//   18
    {84,  255, 0  },//   19
    {77,  255, 0  },//   20
    {70,  255, 0  },//   21
    {63,  255, 0  },//   22
    {56,  255, 0  },//   23
    {49,  255, 0  },//   24
    {42,  255, 0  },//   25
    {35,  255, 0  },//   26
    {28,  255, 0  },//   27
    {21,  255, 0  },//   28
    {14,  255, 0  },//   29
    {7,   255, 0  },//   30

    {0,   255, 0  },//绿 0
    {0,   255, 7  },//   1
    {0,   255, 14 },//   2
    {0,   255, 21 },//   3
    {0,   255, 28 },//   4
    {0,   255, 35 },//   5
    {0,   255, 42 },//   6
    {0,   255, 49 },//   7
    {0,   255, 56 },//   8
    {0,   255, 63 },//   9
    {0,   255, 70 },//   10
    {0,   255, 77 },//   11
    {0,   255, 84 },//   12
    {0,   255, 91 },//   13
    {0,   255, 98 },//   14
    {0,   255, 105},//   15
    {0,   255, 119},//   16
    {0,   255, 126},//   17
    {0,   255, 133},//   18
    {0,   255, 140},//   19
    {0,   255, 154},//   20
    {0,   255, 161},//   21
    {0,   255, 168},//   22
    {0,   255, 182},//   23
    {0,   255, 189},//   24
    {0,   255, 196},//   25
    {0,   255, 210},//   26
    {0,   255, 217},//   27
    {0,   255, 224},//   28
    {0,   255, 238},//   29
    {0,   255, 245},//   30

    {0,   255, 255},//青 0
    {0,   245, 255},//   1
    {0,   231, 255},//   2
    {0,   224, 255},//   3
    {0,   217, 255},//   4
    {0,   203, 255},//   5
    {0,   196, 255},//   6
    {0,   189, 255},//   7
    {0,   175, 255},//   8
    {0,   168, 255},//   9
    {0,   161, 255},//   10
    {0,   147, 255},//   11
    {0,   140, 255},//   12
    {0,   133, 255},//   13
    {0,   126, 255},//   14
    {0,   119, 255},//   15
    {0,   105, 255},//   16
    {0,   98,  255},//   17
    {0,   91,  255},//   18
    {0,   84,  255},//   19
    {0,   77,  255},//   20
    {0,   70,  255},//   21
    {0,   63,  255},//   22
    {0,   56,  255},//   23
    {0,   49,  255},//   24
    {0,   42,  255},//   25
    {0,   35,  255},//   26
    {0,   28,  255},//   27
    {0,   21,  255},//   28
    {0,   14,  255},//   29
    {0,   7,   255},//   30

    {0,   0,   255},//蓝 0
    {5,   0,   255},//   1
    {10,  0,   255},//   2
    {15,  0,   255},//   3
    {20,  0,   255},//   4
    {25,  0,   255},//   5
    {30,  0,   255},//   6
    {35,  0,   255},//   7
    {40,  0,   255},//   8
    {45,  0,   255},//   9
    {50,  0,   255},//   10
    {55,  0,   255},//   11
    {60,  0,   255},//   12
    {65,  0,   255},//   13
    {70,  0,   255},//   14
    {75,  0,   255},//   15
    {85,  0,   255},//   16
    {90,  0,   255},//   17
    {95,  0,   255},//紫 18
    {100, 0,   255},//   19
    {105, 0,   255},//   20
    {110, 0,   255},//   21
    {115, 0,   255},//   22
    {120, 0,   255},//   23
    {125, 0,   255},//   24
    {130, 0,   255},//   25
    {135, 0,   255},//   26
    {140, 0,   255},//   27
    {145, 0,   255},//   28
    {150, 0,   255},//   29
    {155, 0,   255},//   30
    {160, 0,   255},//   31
    {165, 0,   255},//   32
    {170, 0,   255},//   33
    {175, 0,   255},//   34
    {185, 0,   255},//   35
    {180, 0,   255},//   36
    {195, 0,   255},//   37
    {190, 0,   255},//   38
    {200, 0,   255},//   39
    {205, 0,   255},//   40
    {210, 0,   255},//   41
    {215, 0,   255},//   42
    {220, 0,   255},//   43
    {225, 0,   255},//   44
    {230, 0,   255},//   45
    {235, 0,   255},//   46
    {240, 0,   255},//   47
    {245, 0,   255},//   48
    {250, 0,   255},//   49
    {255, 0,   255},//   50

    {255, 0,   255},//粉 0
    {255, 0,   245},//   1
    {255, 0,   231},//   2
    {255, 0,   224},//   3
    {255, 0,   210},//   4
    {255, 0,   203},//   5
    {255, 0,   196},//   6
    {255, 0,   182},//   7
    {255, 0,   175},//   8
    {255, 0,   168},//   9
    {255, 0,   161},//   10
    {255, 0,   147},//   11
    {255, 0,   140},//   12
    {255, 0,   133},//   13
    {255, 0,   126},//   14
    {255, 0,   119},//   15
    {255, 0,   105},//   16
    {255, 0,   98 },//   17
    {255, 0,   91 },//   18
    {255, 0,   84 },//   19
    {255, 0,   77 },//   20
    {255, 0,   70 },//   21
    {255, 0,   63 },//   22
    {255, 0,   56 },//   23
    {255, 0,   49 },//   24
    {255, 0,   42 },//   25
    {255, 0,   35 },//   26
    {255, 0,   28 },//   27
    {255, 0,   21 },//   28
    {255, 0,   14 },//   29
    {255, 0,   7  },//   30
};

uint8_t Logo_Index_Tab[LOGO_LED_SIZE] = {
    115    , 116    , 117    , 114    , 113    , 112    , 111    ,
    110    , 109    , 108    , 107    , 106    , 105    , 104    ,
    103    , 102    , 101    , 100    , 99     , 98     , 97     ,
    96     , 95     , 94     , 93     , 92     , 91     , 90     , 
    89     , 88     , 87     , 86     , 85
};

uint8_t Logo_Flash_Count = 0x00;
uint8_t Logo_Led_Count = 0x00;
uint8_t Logo_Play_Point = 0;
uint8_t Logo_Pwm_R = 0;
uint8_t Logo_Pwm_G = 0;
uint8_t Logo_Pwm_B = 0;
uint8_t Logo_Pwm_Colour = 0;
/*********************************
         初始化函数
*********************************/
void Logo_Init(void) {
    for (uint8_t i = 0; i < LOGO_LED_SIZE; i++) {
        rgb_matrix_set_color(Logo_Index_Tab[i], 0x00, 0x00, 0x00);
    }
    Logo_Play_Point = 64;
    Logo_Pwm_R = 0;
    Logo_Pwm_G = 0;
    Logo_Pwm_B = 0;
    Logo_Pwm_Colour = 0;
}

void Logo_Pwm_Rgb_Updata(uint8_t Pwm) {
    Logo_Pwm_R |= Keyboard_Info.Logo_Saturation;
    Logo_Pwm_G |= Keyboard_Info.Logo_Saturation;
    Logo_Pwm_B |= Keyboard_Info.Logo_Saturation;

    uint16_t Temp_Pwm = 0;
    Temp_Pwm = Logo_Pwm_R * Pwm;
    Logo_Pwm_R = (Temp_Pwm >> 8);

    Temp_Pwm = Logo_Pwm_G * Pwm;
    Logo_Pwm_G = (Temp_Pwm >> 8);

    Temp_Pwm = Logo_Pwm_B * Pwm;
    Logo_Pwm_B = (Temp_Pwm >> 8);
}

void Logo_Pwm_Ds_Updata(uint8_t Pwm) {
    uint16_t Temp_Pwm = 0;
    Temp_Pwm = Logo_Pwm_R * Pwm;
    Logo_Pwm_R = (Temp_Pwm >> 8);
    Logo_Pwm_R |= Keyboard_Info.Logo_Saturation;
    Temp_Pwm = Logo_Pwm_R * Keyboard_Info.Logo_Brightness;
    Logo_Pwm_R = (Temp_Pwm >> 8);

    Temp_Pwm = Logo_Pwm_G * Pwm;
    Logo_Pwm_G = (Temp_Pwm >> 8);
    Logo_Pwm_G |= Keyboard_Info.Logo_Saturation;
    Temp_Pwm = Logo_Pwm_G * Keyboard_Info.Logo_Brightness;
    Logo_Pwm_G = (Temp_Pwm >> 8);

    Temp_Pwm = Logo_Pwm_B * Pwm;
    Logo_Pwm_B = (Temp_Pwm >> 8);
    Logo_Pwm_B |= Keyboard_Info.Logo_Saturation;
    Temp_Pwm = Logo_Pwm_B * Keyboard_Info.Logo_Brightness;
    Logo_Pwm_B = (Temp_Pwm >> 8);
}

/*********************************
         彩色波浪
*********************************/
void Logo_Wave_Rgb_mode_Show(void) {
    if (Logo_Led_Count > LOGO_LED_PLAY_SPEED) {
        Logo_Led_Count = 0;
        if (Keyboard_Info.Logo_Speed != LOGO_MIN_SPEED) {
            if(Logo_Play_Point >= Keyboard_Info.Logo_Speed) {
                Logo_Play_Point -= Keyboard_Info.Logo_Speed;
            } else {
                Logo_Play_Point = 255 - (Keyboard_Info.Logo_Speed - Logo_Play_Point);
            }
        }
    }

    uint8_t Temp_Point = Logo_Play_Point;
    for (uint8_t i = 0; i < LOGO_LED_SIZE; i++) {
        Logo_Pwm_R = LED_Mix_Colour_Tab[Temp_Point][0];
        Logo_Pwm_G = LED_Mix_Colour_Tab[Temp_Point][1];
        Logo_Pwm_B = LED_Mix_Colour_Tab[Temp_Point][2];

        Logo_Pwm_Rgb_Updata(Keyboard_Info.Logo_Brightness);

        rgb_matrix_set_color(Logo_Index_Tab[i], Logo_Pwm_R, Logo_Pwm_G, Logo_Pwm_B);

        Temp_Point += 12;
        if(Temp_Point >= 255) {
            Temp_Point = 0;
        }
    }
}
/*********************************
         单色波浪
*********************************/
void Logo_Wave_Ds_mode_Show(void) {
    if (Logo_Led_Count > LOGO_LED_PLAY_SPEED) {
        Logo_Led_Count = 0;
        if (Keyboard_Info.Logo_Speed != LOGO_MIN_SPEED) {
            if(Logo_Play_Point >= Keyboard_Info.Logo_Speed) {
                Logo_Play_Point -= Keyboard_Info.Logo_Speed;
            } else {
                Logo_Play_Point = 127 - (Keyboard_Info.Logo_Speed - Logo_Play_Point);
            }
        }
    }

    uint8_t Temp_Point = Logo_Play_Point;
    for (uint8_t i = 0; i < LOGO_LED_SIZE; i++) {
        Logo_Pwm_R = LED_Mix_Colour_Tab[Keyboard_Info.Logo_Colour][0];
        Logo_Pwm_G = LED_Mix_Colour_Tab[Keyboard_Info.Logo_Colour][1];
        Logo_Pwm_B = LED_Mix_Colour_Tab[Keyboard_Info.Logo_Colour][2];

        Logo_Pwm_Ds_Updata(Led_Wave_Pwm_Tab[Temp_Point]);

        rgb_matrix_set_color(Logo_Index_Tab[i], Logo_Pwm_R, Logo_Pwm_G, Logo_Pwm_B);

        Temp_Point += 8;
        if(Temp_Point >= 127) {
            Temp_Point = 0;
        }
    }
}
/*********************************
         光谱
*********************************/
void Logo_Spectrum_mode_Show(void) {
    if (Logo_Led_Count > LOGO_LED_PLAY_SPEED) {
        Logo_Led_Count = 0;
        if (Keyboard_Info.Logo_Speed != LOGO_MIN_SPEED) {
            if(Logo_Play_Point >= Keyboard_Info.Logo_Speed) {
                Logo_Play_Point -= Keyboard_Info.Logo_Speed;
            } else {
                Logo_Play_Point = 255 - (Keyboard_Info.Logo_Speed - Logo_Play_Point);
            }
        }
    }

    Logo_Pwm_R = LED_Mix_Colour_Tab[Logo_Play_Point][0];
    Logo_Pwm_G = LED_Mix_Colour_Tab[Logo_Play_Point][1];
    Logo_Pwm_B = LED_Mix_Colour_Tab[Logo_Play_Point][2];

    Logo_Pwm_Rgb_Updata(Keyboard_Info.Logo_Brightness);

    for (uint8_t i = 0; i < LOGO_LED_SIZE; i++) {
        rgb_matrix_set_color(Logo_Index_Tab[i], Logo_Pwm_R, Logo_Pwm_G, Logo_Pwm_B);
    }
}
/*********************************
         呼吸
*********************************/
void Logo_Breath_mode_Show(void) {
    if (Logo_Led_Count > LOGO_LED_PLAY_SPEED) {
        Logo_Led_Count = 0;
        if (Keyboard_Info.Logo_Speed != LOGO_MIN_SPEED) {
            if(Logo_Play_Point >= Keyboard_Info.Logo_Speed) {
                Logo_Play_Point -= Keyboard_Info.Logo_Speed;
            } else {
                Logo_Play_Point = 127 - (Keyboard_Info.Logo_Speed - Logo_Play_Point);
            }
        }
    }

    Logo_Pwm_R = LED_Mix_Colour_Tab[Keyboard_Info.Logo_Colour][0];
    Logo_Pwm_G = LED_Mix_Colour_Tab[Keyboard_Info.Logo_Colour][1];
    Logo_Pwm_B = LED_Mix_Colour_Tab[Keyboard_Info.Logo_Colour][2];

    Logo_Pwm_Ds_Updata(Led_Wave_Pwm_Tab[Logo_Play_Point]);

    for (uint8_t i = 0; i < LOGO_LED_SIZE; i++) {
        rgb_matrix_set_color(Logo_Index_Tab[i], Logo_Pwm_R, Logo_Pwm_G, Logo_Pwm_B);
    }
}
/*********************************
         常量
*********************************/
void Logo_Light_mode_Show(void) {
    Logo_Pwm_R = LED_Mix_Colour_Tab[Keyboard_Info.Logo_Colour][0];
    Logo_Pwm_G = LED_Mix_Colour_Tab[Keyboard_Info.Logo_Colour][1];
    Logo_Pwm_B = LED_Mix_Colour_Tab[Keyboard_Info.Logo_Colour][2];

    Logo_Pwm_Ds_Updata(255);

    for (uint8_t i = 0; i < LOGO_LED_SIZE; i++) {
        rgb_matrix_set_color(Logo_Index_Tab[i], Logo_Pwm_R, Logo_Pwm_G, Logo_Pwm_B);
    }
}
/*********************************
         关闭
*********************************/
void Logo_Off_mode_Show(void) {
    for (uint8_t i = 0; i < LOGO_LED_SIZE; i++) {
        rgb_matrix_set_color(Logo_Index_Tab[i], 0X00, 0X00, 0X00);
    }
}

void Logo_Mode_Show(void) {
    if (Keyboard_Info.Logo_On_Off) {
        Logo_Off_mode_Show();
    } else {
        switch (Keyboard_Info.Logo_Mode) {
            case LOGO_WAVE_RGB_MODE:    Logo_Wave_Rgb_mode_Show();  break;
            case LOGO_WAVE_DS_MODE:     Logo_Wave_Ds_mode_Show();   break;
            case LOGO_SPECTRUM_MODE:    Logo_Spectrum_mode_Show();  break;
            case LOGO_BREATH_MODE:      Logo_Breath_mode_Show();    break;
            case LOGO_LIGHT_MODE:       Logo_Light_mode_Show();     break;
            case LOGO_OFF_MODE:         Logo_Off_mode_Show();       break;
            default:                    Logo_Off_mode_Show();       break;
        }
    }
}

void User_Via_Qmk_Logo_Get_Value(uint8_t *data) {
    // data = [ value_id, value_data ]
    uint8_t *value_id   = &(data[0]);
    uint8_t *value_data = &(data[1]);

    switch (*value_id) {
        case id_qmk_rgb_matrix_brightness: {
            value_data[0] = Keyboard_Info.Logo_Brightness;
            break;
        }
        case id_qmk_rgb_matrix_effect: {
            value_data[0] = Keyboard_Info.Logo_Mode;
            break;
        }
        case id_qmk_rgb_matrix_effect_speed: {
            value_data[0] = Keyboard_Info.Logo_Speed;
            break;
        }
        case id_qmk_rgb_matrix_color: {
            value_data[0] = Keyboard_Info.Logo_Colour;
            value_data[1] = (255 - Keyboard_Info.Logo_Saturation);
            break;
        }
    }
}

void User_Via_Qmk_Logo_Set_Value(uint8_t *data) {
    // data = [ value_id, value_data ]
    uint8_t *value_id   = &(data[0]);
    uint8_t *value_data = &(data[1]);
    switch (*value_id) {
        case id_qmk_rgb_matrix_brightness: {    //设置亮度 0 ~ 255
            if (value_data[0] > RGB_MATRIX_MAXIMUM_BRIGHTNESS) {
                Keyboard_Info.Logo_Brightness = RGB_MATRIX_MAXIMUM_BRIGHTNESS;
            } else {
                Keyboard_Info.Logo_Brightness = value_data[0];
            }
            break;
        }
        case id_qmk_rgb_matrix_effect: {        //设置灯光模式
            if (value_data[0] == 0) {
                Keyboard_Info.Logo_On_Off = LOGO_LED_OFF;
            } else {
                Keyboard_Info.Logo_On_Off = LOGO_LED_ON;
                if (value_data[0] <= LOGO_OFF_MODE) {
                    Keyboard_Info.Logo_Mode = value_data[0];
                } else {
                    Keyboard_Info.Logo_Mode = INIT_LOGO_MODE;
                }
            }
            break;
        }
        case id_qmk_rgb_matrix_effect_speed: {  //设置灯光速度
            if (value_data[0] > LOGO_MAX_SPEED) {
                Keyboard_Info.Logo_Speed = LOGO_MAX_SPEED;
            } else {
                Keyboard_Info.Logo_Speed = value_data[0];
            }
            break;
        }
        case id_qmk_rgb_matrix_color: {         //设置颜色和饱和度
            Keyboard_Info.Logo_Colour = value_data[0];
            Keyboard_Info.Logo_Saturation = (255 - value_data[1]);
            break;
        }
    }
}

void User_Via_Qmk_Logo_Command(uint8_t *data, uint8_t length) {
    // data = [ command_id, channel_id, value_id, value_data ]
    uint8_t *command_id        = &(data[0]);
    uint8_t *value_id_and_data = &(data[2]);

    switch (*command_id) {
        case id_custom_set_value: {
            User_Via_Qmk_Logo_Set_Value(value_id_and_data);
            break;
        }
        case id_custom_get_value: {
            User_Via_Qmk_Logo_Get_Value(value_id_and_data);
            break;
        }
        case id_custom_save: {
            Save_Flash_Set();
            break;
        }
        default: {
            *command_id = id_unhandled;
            break;
        }
    }
}

#endif

#if SIDE_LED_ENABLE
uint8_t Side_Index_Tab[SIDE_LED_GROUP][SIDE_LED_SIZE] = {
    {100, 99},
    {101, 98},
    {102, 97},
    {103, 96}
};

uint8_t Side_Led_Count = 0x00;
uint8_t Side_Play_Point = 0;
uint8_t Side_Pwm_R = 0;
uint8_t Side_Pwm_G = 0;
uint8_t Side_Pwm_B = 0;
/*********************************
            初始化函数
*********************************/
void Side_Init(void) {
    for (uint8_t i = 0; i < SIDE_LED_GROUP; i++) {
        for (uint8_t j = 0; j < SIDE_LED_SIZE; j++) {
            rgb_matrix_set_color(Side_Index_Tab[i][j], 0x00, 0x00, 0x00);
        }
    }
    Side_Play_Point = 64;
    Side_Pwm_R = 0;
    Side_Pwm_G = 0;
    Side_Pwm_B = 0;
}

void Side_Pwm_Rgb_Updata(uint8_t Pwm) {
    Side_Pwm_R |= Keyboard_Info.Side_Saturation;
    Side_Pwm_G |= Keyboard_Info.Side_Saturation;
    Side_Pwm_B |= Keyboard_Info.Side_Saturation;

    uint16_t Temp_Pwm = 0;
    Temp_Pwm = Side_Pwm_R * Pwm;
    Side_Pwm_R = (Temp_Pwm >> 8);

    Temp_Pwm = Side_Pwm_G * Pwm;
    Side_Pwm_G = (Temp_Pwm >> 8);

    Temp_Pwm = Side_Pwm_B * Pwm;
    Side_Pwm_B = (Temp_Pwm >> 8);
}

void Side_Pwm_Ds_Updata(uint8_t Pwm) {
    uint16_t Temp_Pwm = 0;
    Temp_Pwm = Side_Pwm_R * Pwm;
    Side_Pwm_R = (Temp_Pwm >> 8);
    Side_Pwm_R |= Keyboard_Info.Side_Saturation;
    Temp_Pwm = Side_Pwm_R * Keyboard_Info.Side_Brightness;
    Side_Pwm_R = (Temp_Pwm >> 8);

    Temp_Pwm = Side_Pwm_G * Pwm;
    Side_Pwm_G = (Temp_Pwm >> 8);
    Side_Pwm_G |= Keyboard_Info.Side_Saturation;
    Temp_Pwm = Side_Pwm_G * Keyboard_Info.Side_Brightness;
    Side_Pwm_G = (Temp_Pwm >> 8);

    Temp_Pwm = Side_Pwm_B * Pwm;
    Side_Pwm_B = (Temp_Pwm >> 8);
    Side_Pwm_B |= Keyboard_Info.Side_Saturation;
    Temp_Pwm = Side_Pwm_B * Keyboard_Info.Side_Brightness;
    Side_Pwm_B = (Temp_Pwm >> 8);
}
/*********************************
            彩色波浪
*********************************/
void Side_Wave_Rgb_mode_Show(void) {
    if (Side_Led_Count > SIDE_LED_PLAY_SPEED) {
        Side_Led_Count = 0;
        if (Keyboard_Info.Side_Speed != SIDE_MIN_SPEED) {
	        if(Side_Play_Point >= Keyboard_Info.Side_Speed) {
		        Side_Play_Point -= Keyboard_Info.Side_Speed;
	        } else {
		        Side_Play_Point = 255 - (Keyboard_Info.Side_Speed - Side_Play_Point);
	        }
        }
    }

	uint8_t Temp_Point = Side_Play_Point;
    for (uint8_t i = 0; i < SIDE_LED_GROUP; i++) {
        for (uint8_t j = 0; j < SIDE_LED_SIZE; j++) {
            Side_Pwm_R = LED_Mix_Colour_Tab[Temp_Point][0];
            Side_Pwm_G = LED_Mix_Colour_Tab[Temp_Point][1];
            Side_Pwm_B = LED_Mix_Colour_Tab[Temp_Point][2];

            Side_Pwm_Rgb_Updata(Keyboard_Info.Side_Brightness);

            rgb_matrix_set_color(Side_Index_Tab[i][j], Side_Pwm_R, Side_Pwm_G, Side_Pwm_B);

            Temp_Point += 8;
            if (Temp_Point >= 255) {
                Temp_Point = 0;
            }
        }
    }
}
/*********************************
            单色波浪
*********************************/
void Side_Wave_Ds_mode_Show(void) {
    if (Side_Led_Count > SIDE_LED_PLAY_SPEED) {
        Side_Led_Count = 0;
        if (Keyboard_Info.Side_Speed != SIDE_MIN_SPEED) {
	        if(Side_Play_Point >= Keyboard_Info.Side_Speed) {
		        Side_Play_Point -= Keyboard_Info.Side_Speed;
	        } else {
		        Side_Play_Point = 127 - (Keyboard_Info.Side_Speed - Side_Play_Point);
	        }
        }
    }

	uint8_t Temp_Point = Side_Play_Point;
    for (uint8_t i = 0; i < SIDE_LED_GROUP; i++) {
        for (uint8_t j = 0; j < SIDE_LED_SIZE; j++) {
            Side_Pwm_R = LED_Mix_Colour_Tab[Keyboard_Info.Side_Colour][0];
            Side_Pwm_G = LED_Mix_Colour_Tab[Keyboard_Info.Side_Colour][1];
            Side_Pwm_B = LED_Mix_Colour_Tab[Keyboard_Info.Side_Colour][2];

            Side_Pwm_Ds_Updata(Led_Wave_Pwm_Tab[Temp_Point]);

            rgb_matrix_set_color(Side_Index_Tab[i][j], Side_Pwm_R, Side_Pwm_G, Side_Pwm_B);

            Temp_Point += 8;
            if (Temp_Point >= 127) {
                Temp_Point = 0;
            }
        }
    }
}
/*********************************
            光谱
*********************************/
void Side_Spectrum_mode_Show(void) {
    if (Side_Led_Count > SIDE_LED_PLAY_SPEED) {
        Side_Led_Count = 0;
        if (Keyboard_Info.Side_Speed != SIDE_MIN_SPEED) {
	        if(Side_Play_Point >= Keyboard_Info.Side_Speed) {
		        Side_Play_Point -= Keyboard_Info.Side_Speed;
	        } else {
		        Side_Play_Point = 255 - (Keyboard_Info.Side_Speed - Side_Play_Point);
	        }
        }
    }

    Side_Pwm_R = LED_Mix_Colour_Tab[Side_Play_Point][0];
    Side_Pwm_G = LED_Mix_Colour_Tab[Side_Play_Point][1];
    Side_Pwm_B = LED_Mix_Colour_Tab[Side_Play_Point][2];

    Side_Pwm_Rgb_Updata(Keyboard_Info.Side_Brightness);

    for (uint8_t i = 0; i < SIDE_LED_GROUP; i++) {
        for (uint8_t j = 0; j < SIDE_LED_SIZE; j++) {
            rgb_matrix_set_color(Side_Index_Tab[i][j], Side_Pwm_R, Side_Pwm_G, Side_Pwm_B);
        }
    }
}
/*********************************
            呼吸
*********************************/
void Side_Breath_mode_Show(void) {
    if (Side_Led_Count > SIDE_LED_PLAY_SPEED) {
        Side_Led_Count = 0;
        if (Keyboard_Info.Side_Speed != SIDE_MIN_SPEED) {
	        if(Side_Play_Point >= Keyboard_Info.Side_Speed) {
		        Side_Play_Point -= Keyboard_Info.Side_Speed;
	        } else {
		        Side_Play_Point = 127 - (Keyboard_Info.Side_Speed - Side_Play_Point);
	        }
        }
    }

    Side_Pwm_R = LED_Mix_Colour_Tab[Keyboard_Info.Side_Colour][0];
    Side_Pwm_G = LED_Mix_Colour_Tab[Keyboard_Info.Side_Colour][1];
    Side_Pwm_B = LED_Mix_Colour_Tab[Keyboard_Info.Side_Colour][2];

    Side_Pwm_Ds_Updata(Led_Wave_Pwm_Tab[Side_Play_Point]);

    for (uint8_t i = 0; i < SIDE_LED_GROUP; i++) {
        for (uint8_t j = 0; j < SIDE_LED_SIZE; j++) {
            rgb_matrix_set_color(Side_Index_Tab[i][j], Side_Pwm_R, Side_Pwm_G, Side_Pwm_B);
        }
    }
}
/*********************************
            常量
*********************************/
void Side_Light_mode_Show(void) {
    Side_Pwm_R = LED_Mix_Colour_Tab[Keyboard_Info.Side_Colour][0];
    Side_Pwm_G = LED_Mix_Colour_Tab[Keyboard_Info.Side_Colour][1];
    Side_Pwm_B = LED_Mix_Colour_Tab[Keyboard_Info.Side_Colour][2];

    Side_Pwm_Ds_Updata(255);

    for (uint8_t i = 0; i < SIDE_LED_GROUP; i++) {
        for (uint8_t j = 0; j < SIDE_LED_SIZE; j++) {
            rgb_matrix_set_color(Side_Index_Tab[i][j], Side_Pwm_R, Side_Pwm_G, Side_Pwm_B);
        }
    }
}
/*********************************
            关闭
*********************************/
void Side_Off_mode_Show(void) {
    for (uint8_t i = 0; i < SIDE_LED_GROUP; i++) {
        for (uint8_t j = 0; j < SIDE_LED_SIZE; j++) {
            rgb_matrix_set_color(Side_Index_Tab[i][j], 0X00, 0X00, 0X00);
        }
    }
}

void Side_Mode_Show(void) {
    if (Keyboard_Info.Side_On_Off) {
        Side_Off_mode_Show();
    } else {
        switch (Keyboard_Info.Side_Mode) {
            case SIDE_WAVE_RGB_MODE:    Side_Wave_Rgb_mode_Show();  break;
            case SIDE_WAVE_DS_MODE:     Side_Wave_Ds_mode_Show();   break;
            case SIDE_SPECTRUM_MODE:    Side_Spectrum_mode_Show();  break;
            case SIDE_BREATH_MODE:      Side_Breath_mode_Show();    break;
            case SIDE_LIGHT_MODE:       Side_Light_mode_Show();     break;
            case SIDE_OFF_MODE:         Side_Off_mode_Show();       break;
            default:                    Side_Off_mode_Show();       break;
        }
    }
}

void User_Via_Qmk_Side_Get_Value(uint8_t *data) {
    // data = [ value_id, value_data ]
    uint8_t *value_id   = &(data[0]);
    uint8_t *value_data = &(data[1]);

    switch (*value_id) {
        case id_qmk_rgb_matrix_brightness: {
            value_data[0] = Keyboard_Info.Side_Brightness;
            break;
        }
        case id_qmk_rgb_matrix_effect: {
            value_data[0] = Keyboard_Info.Side_Mode;
            break;
        }
        case id_qmk_rgb_matrix_effect_speed: {
            value_data[0] = Keyboard_Info.Side_Speed;
            break;
        }
        case id_qmk_rgb_matrix_color: {
            value_data[0] = Keyboard_Info.Side_Colour;
            value_data[1] = (255 - Keyboard_Info.Side_Saturation);
            break;
        }
    }
}

void User_Via_Qmk_Side_Set_Value(uint8_t *data) {
    // data = [ value_id, value_data ]
    uint8_t *value_id   = &(data[0]);
    uint8_t *value_data = &(data[1]);
    switch (*value_id) {
        case id_qmk_rgb_matrix_brightness: {    //设置亮度 0 ~ 255
            if (value_data[0] > RGB_MATRIX_MAXIMUM_BRIGHTNESS) {
                Keyboard_Info.Side_Brightness = RGB_MATRIX_MAXIMUM_BRIGHTNESS;
            } else {
                Keyboard_Info.Side_Brightness = value_data[0];
            }
            break;
        }
        case id_qmk_rgb_matrix_effect: {        //设置灯光模式
            if (value_data[0] == 0) {
                Keyboard_Info.Side_On_Off = SIDE_LED_OFF;
            } else {
                Keyboard_Info.Side_On_Off = SIDE_LED_ON;
                if (value_data[0] <= SIDE_OFF_MODE) {
                    Keyboard_Info.Side_Mode = value_data[0];
                } else {
                    Keyboard_Info.Side_Mode = INIT_SIDE_MODE;
                }
            }
            break;
        }
        case id_qmk_rgb_matrix_effect_speed: {  //设置灯光速度
            if (value_data[0] > SIDE_MAX_SPEED) {
                Keyboard_Info.Side_Speed = SIDE_MAX_SPEED;
            } else {
                Keyboard_Info.Side_Speed = value_data[0];
            }
            break;
        }
        case id_qmk_rgb_matrix_color: {         //设置颜色和饱和度
            Keyboard_Info.Side_Colour = value_data[0];
            Keyboard_Info.Side_Saturation = (255 - value_data[1]);
            break;
        }
    }
}

void User_Via_Qmk_Side_Command(uint8_t *data, uint8_t length) {
    // data = [ command_id, channel_id, value_id, value_data ]
    uint8_t *command_id        = &(data[0]);
    uint8_t *value_id_and_data = &(data[2]);

    switch (*command_id) {
        case id_custom_set_value: {
            User_Via_Qmk_Side_Set_Value(value_id_and_data);
            break;
        }
        case id_custom_get_value: {
            User_Via_Qmk_Side_Get_Value(value_id_and_data);
            break;
        }
        case id_custom_save: {
            Save_Flash_Set();
            break;
        }
        default: {
            *command_id = id_unhandled;
            break;
        }
    }
}

#endif

#if LATTICE_LED_ENABLE
const uint8_t Lattice_Index_Tab[LATTICE_LED_GROUP][LATTICE_LED_SIZE] = {
    {118, 119, 120, 121, 122, 123, 124},
    {125, 126, 127, 128, 129, 130, 131},
    {132, 133, 134, 135, 136, 137, 138},
    {139, 140, 141, 142, 143, 144, 145},
    {146, 147, 148, 149, 150, 151, 152},
    {153, 154, 155, 156, 157, 158, 159},
    {160, 161, 162, 163, 164, 165, 166}
};

// 自定义模式矩阵
const uint8_t Lattice_User_Index_Tab[LATTICE_LED_GROUP * LATTICE_LED_SIZE] = {
    118, 119, 120, 121, 122, 123, 124,
    125, 126, 127, 128, 129, 130, 131,
    132, 133, 134, 135, 136, 137, 138,
    139, 140, 141, 142, 143, 144, 145,
    146, 147, 148, 149, 150, 151, 152,
    153, 154, 155, 156, 157, 158, 159,
    160, 161, 162, 163, 164, 165, 166
};

// 自定义模式模式灯光位置存储
uint8_t Lattice_User_Mode_Show_Tab[LATTICE_LED_GROUP * LATTICE_LED_SIZE * 3] = {
    0
};

// 扫描模式矩阵
#define LED_MAP_SIZE    20
const uint8_t Lattice_Scan_Mode_Tab[LED_MAP_SIZE][8] = {
    {122, 129, 123, 124, 130, 131, 137, 138},
    {123, 124, 130, 131, 137, 138, 144, 145},
    {130, 131, 137, 138, 144, 145, 151, 152},
    {137, 138, 144, 145, 151, 152, 158, 159},
    {144, 145, 151, 152, 158, 159, 165, 166},
    
    {151, 152, 159, 166, 158, 165, 157, 164},
    {159, 166, 158, 165, 157, 164, 156, 163},
    {158, 165, 157, 164, 156, 163, 155, 162},
    {157, 164, 156, 163, 155, 162, 154, 161},
    {156, 163, 155, 162, 154, 161, 153, 160},

    {155, 162, 160, 161, 153, 154, 146, 147},
    {160, 161, 153, 154, 146, 147, 139, 140},
    {153, 154, 146, 147, 139, 140, 132, 133},
    {146, 147, 139, 140, 132, 133, 125, 126},
    {139, 140, 132, 133, 125, 126, 118, 119},

    {132, 133, 118, 125, 119, 126, 120, 127},
    {118, 125, 119, 126, 120, 127, 121, 128},
    {119, 126, 120, 127, 121, 128, 122, 129},
    {120, 127, 121, 128, 122, 129, 123, 129},
    {121, 128, 122, 129, 123, 129, 124, 131}
};

// 光芒四射
#define LED_FIRST_CIRCLE_SIZE   42
#define LED_SECOND_CIRCLE_SIZE  23
#define LED_SPILIT_FLAG         9
const uint8_t Lattice_First_Circle_Mode_Tab[LED_FIRST_CIRCLE_SIZE][14] = {
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 118,  125},
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 118,  125,  119,  126},
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 118,  125,  119,  126,  120,  127},
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 118,  125,  119,  126,  120,  127,  121,  128},
    {0xFF, 0xFF, 0xFF, 0xFF, 118,  125,  119,  126,  120,  127,  121,  128,  122,  129},
    {0xFF, 0xFF, 118,  125,  119,  126,  120,  127,  121,  128,  122,  129,  123,  130},
    {118,  125,  119,  126,  120,  127,  121,  128,  122,  129,  123,  130,  124,  131},        // 7
    
    {119,  126,  120,  127,  121,  128,  122,  129,  123,  130,  124,  131,  137,  138},
    {120,  127,  121,  128,  122,  129,  123,  130,  124,  131,  137,  138,  144,  145},        // 9

    {121,  128,  122,  129,  123,  130,  124,  131,  137,  138,  144,  145,  151,  152},
    {122,  129,  123,  130,  124,  131,  137,  138,  144,  145,  151,  152,  158,  159},
    {123,  130,  124,  131,  137,  138,  144,  145,  151,  152,  158,  159,  165,  166},

    {130,  131,  137,  138,  144,  145,  151,  152,  158,  159,  165,  166,  157,  164},
    {137,  138,  144,  145,  151,  152,  158,  159,  165,  166,  157,  164,  156,  163},
    {144,  145,  151,  152,  158,  159,  165,  166,  157,  164,  156,  163,  155,  162},
    {151,  152,  158,  159,  165,  166,  157,  164,  156,  163,  155,  162,  154,  161},
    {158,  159,  165,  166,  157,  164,  156,  163,  155,  162,  154,  161,  153,  160},
    
    {158,  165,  157,  164,  156,  163,  155,  162,  154,  161,  153,  160,  146,  147},
    {157,  164,  156,  163,  155,  162,  154,  161,  153,  160,  146,  147,  139,  140},
    {156,  163,  155,  162,  154,  161,  153,  160,  146,  147,  139,  140,  132,  133},
    {155,  162,  154,  161,  153,  160,  146,  147,  139,  140,  132,  133,  125,  126},
    {154,  161,  153,  160,  146,  147,  139,  140,  132,  133,  125,  126,  118,  119},

    {153,  154,  146,  147,  138,  139,  132,  133,  125,  126,  118,  119,  120,  127},
    {146,  147,  138,  139,  132,  133,  125,  126,  118,  119,  120,  127,  121,  128},
    {138,  139,  132,  133,  125,  126,  118,  119,  120,  127,  121,  128,  122,  129},
    {132,  133,  125,  126,  118,  119,  120,  127,  121,  128,  122,  129,  123,  130},
    {125,  126,  118,  119,  120,  127,  121,  128,  122,  129,  123,  130,  124,  131},

    {119,  126,  120,  127,  121,  128,  122,  129,  123,  129,  124,  131,  137,  138},
    {120,  127,  121,  128,  122,  129,  123,  129,  124,  131,  137,  138,  144,  145},        // 29

    // 第一条灯光
    {121,  128,  122,  129,  123,  130,  124,  131,  137,  138,  144,  145,  136,  143},        // 30
    {122,  129,  123,  130,  124,  131,  137,  138,  144,  145,  136,  143,  135,  142},
    {0xFF, 0xFF, 0xFF, 127,  128,  129,  130,  135,  136,  137,  142,  143,  144,  151},
    {0xFF, 0xFF, 0xFF, 127,  128,  129,  130,  134,  135,  136,  137,  142,  143,  144},

    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 134,  135,  136,  141,  142,  143,  148,  149,  150},        // 34
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 135,  141,  142,  143,  149},
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 142},
    
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 135,  141,  143,  149},
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 128,  135,  140,  141,  142,  143,  144,  149,  156},
    {0xFF, 0xFF, 121,  128,  135,  139,  140,  141,  143,  144,  145,  149,  156,  163},
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 121,  128,  139,  140,  144,  145,  156,  163},
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 121,  138,  145,  163},
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
};

const uint8_t Lattice_Second_Circle_Mode_Tab[LED_SECOND_CIRCLE_SIZE][14] = {
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 118,  125},
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 118,  125,  119,  126},
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 118,  125,  119,  126,  120,  127},
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 118,  125,  119,  126,  120,  127,  121,  128},
    {0xFF, 0xFF, 0xFF, 0xFF, 118,  125,  119,  126,  120,  127,  121,  128,  122,  129},
    {0xFF, 0xFF, 118,  125,  119,  126,  120,  127,  121,  128,  122,  129,  123,  130},
    {118,  125,  119,  126,  120,  127,  121,  128,  122,  129,  123,  130,  124,  131},        // 7
    
    {119,  126,  120,  127,  121,  128,  122,  129,  123,  130,  124,  131,  137,  138},
    {120,  127,  121,  128,  122,  129,  123,  130,  124,  131,  137,  138,  144,  145},        // 9

    {121,  128,  122,  129,  123,  130,  124,  131,  137,  138,  144,  145,  151,  152},
    {122,  129,  123,  130,  124,  131,  137,  138,  144,  145,  151,  152,  158,  159},
    {123,  130,  124,  131,  137,  138,  144,  145,  151,  152,  158,  159,  165,  166},

    {130,  131,  137,  138,  144,  145,  151,  152,  158,  159,  165,  166,  157,  164},
    {137,  138,  144,  145,  151,  152,  158,  159,  165,  166,  157,  164,  156,  163},
    {144,  145,  151,  152,  158,  159,  165,  166,  157,  164,  156,  163,  155,  162},
    {151,  152,  158,  159,  165,  166,  157,  164,  156,  163,  155,  162,  154,  161},
    {158,  159,  165,  166,  157,  164,  156,  163,  155,  162,  154,  161,  153,  160},
    
    {158,  165,  157,  164,  156,  163,  155,  162,  154,  161,  153,  160,  146,  147},
    {157,  164,  156,  163,  155,  162,  154,  161,  153,  160,  146,  147,  139,  140},
    
    // 第二条灯光
    {156,  163,  155,  162,  154,  161,  153,  160,  146,  147,  139,  140,  141,  148},        // 20
    {0xFF, 155,  162,  154,  151,  153,  160,  146,  147,  139,  140,  141,  148,  149},
    {0xFF, 0xFF, 133,  134,  140,  141,  147,  148,  149,  150,  154,  155,  156,  157},
    {0xFF, 0xFF, 0xFF, 0xFF, 140,  141,  147,  148,  149,  150,  154,  155,  156,  157},
};

/*
    打字机模式字模
*/
const uint8_t LED_Button_Tab[109][7] = {
    {0X00,0X00,0X00,0X00,0X00,0X00,0X00},	 // 0  四个空白键位
    {0X00,0X00,0X00,0X00,0X00,0X00,0X00},	 // 1
    {0X00,0X00,0X00,0X00,0X00,0X00,0X00},	 // 2
    {0X00,0X00,0X00,0X00,0X00,0X00,0X00},	 // 3

	{0x00,0x1C,0x22,0x3E,0x22,0x22,0x00},    // A
	{0x00,0x1E,0x22,0x3E,0x22,0x1E,0x00},    // B
	{0x00,0x3E,0x02,0x02,0x02,0x3E,0x00},    // C
	{0x00,0x1E,0x22,0x22,0x22,0x1E,0x00},    // D
    {0x00,0x3E,0x02,0x3E,0x02,0x3E,0x00},    // E
	{0x00,0x3E,0x02,0x3E,0x02,0x02,0x00},    // F
	{0x00,0x3E,0x02,0x3A,0x22,0x3E,0x00},    // G
	{0x00,0x22,0x22,0x3E,0x22,0x22,0x00},    // H
    {0x00,0x1C,0x08,0x08,0x08,0x1C,0x00},    // I
	{0x00,0x38,0x20,0x20,0x22,0x1C,0x00},    // J
	{0x00,0x22,0x12,0x0E,0x12,0x22,0x00},    // K
	{0x00,0x02,0x02,0x02,0x02,0x3E,0x00},    // L
	{0x00,0x22,0x36,0x2A,0x22,0x22,0x00},    // M
	{0x00,0x22,0x26,0x2A,0x32,0x22,0x00},    // N
    {0x00,0x3E,0x22,0x22,0x22,0x3E,0x00},    // O
    {0x00,0x3E,0x22,0x3E,0x02,0x02,0x00},    // P            20
    {0x00,0x1E,0x12,0x12,0x12,0x3E,0x00},    // Q
    {0x00,0x3E,0x22,0x3E,0x12,0x32,0x00},    // R
	{0x00,0x3E,0x02,0x3E,0x20,0x3E,0x00},    // S
    {0x00,0x3E,0x08,0x08,0x08,0x08,0x00},    // T
    {0x00,0x22,0x22,0x22,0x22,0x1C,0x00},    // U
	{0x00,0x22,0x22,0x22,0x14,0x08,0x00},    // V
    {0x00,0x2A,0x2A,0x2A,0x2A,0x14,0x00},    // W
	{0x00,0x22,0x14,0x08,0x14,0x22,0x00},    // X
    {0x00,0x22,0x22,0x14,0x08,0x08,0x00},    // Y
	{0x00,0x3E,0x10,0x08,0x04,0x3E,0x00},    // Z
    {0x00,0x08,0x0C,0x08,0x08,0x1C,0x00},    // 1
    {0x00,0x3E,0x20,0x3E,0x02,0x3E,0x00},    // 2
    {0x00,0x3E,0x20,0x3E,0x20,0x3E,0x00},    // 3
    {0x00,0x18,0x14,0x12,0x3E,0x10,0x00},    // 4
    {0x00,0x3E,0x02,0x3E,0x20,0x3E,0x00},    // 5
    {0x00,0x3E,0x02,0x3E,0x22,0x3E,0x00},    // 6
    {0x00,0x3E,0x20,0x10,0x08,0x04,0x00},    // 7
    {0x00,0x3E,0x22,0x3E,0x22,0x3E,0x00},    // 8
    {0x00,0x3E,0x22,0x3E,0x20,0x3E,0x00},    // 9
    {0x00,0x1C,0x22,0x22,0x22,0x1C,0x00},    // 0            40
	{0x00,0x22,0x22,0x00,0x22,0x1C,0x00},    // ENTER
	{0x00,0x0E,0x26,0x2A,0x20,0x1C,0x00},    // ESC
    {0x00,0x04,0x02,0x7F,0x02,0x04,0x00},    // BACKSPACE
    {0x40,0x48,0x50,0x7F,0x50,0x48,0x40},    // TAB
	{0x00,0x22,0x55,0x00,0x00,0x22,0x1C},    // SPACE
    {0x00,0x00,0x00,0x3E,0x00,0x00,0x00},    // -
    {0x00,0x08,0x14,0x22,0x41,0x00,0x00},    // =
    {0x3E,0x41,0x5D,0x55,0x7D,0x01,0x3E},    // [
    {0x3C,0x04,0x04,0x04,0x04,0x04,0x3C},    // ]
    {0x3C,0x20,0x20,0x20,0x20,0x20,0x3C},    // \|

    {0X00,0X00,0X00,0X00,0X00,0X00,0X00},    // 空白

	{0x00,0x00,0x08,0x00,0x08,0x08,0x00},    // ;
	{0x00,0x00,0x00,0x00,0x08,0x08,0x00},    // '
	{0x77,0x41,0x41,0x47,0x51,0x51,0x77},    // `
	{0x00,0x00,0x00,0x18,0x18,0x1C,0x00},    // ,
	{0x00,0x00,0x00,0x1C,0x14,0x1C,0x00},    // .
	{0x00,0x20,0x10,0x08,0x04,0x02,0x00},    // /
	{0x00,0x08,0x1C,0x3E,0x3E,0x1C,0x00},    // CAPS_LOCK
	{0x00,0x2E,0x22,0x2E,0x22,0x22,0x00},    // F1
	{0x00,0x77,0x41,0x77,0x11,0x71,0x00},    // F2            60
	{0x00,0x77,0x41,0x77,0x41,0x71,0x00},    // F3
	{0x00,0x57,0x51,0x77,0x41,0x41,0x00},    // F4
	{0x00,0x77,0x11,0x77,0x41,0x71,0x00},    // F5
	{0x00,0x77,0x11,0x77,0x51,0x71,0x00},    // F6
	{0x00,0x77,0x41,0x27,0x21,0x21,0x00},    // F7
	{0x00,0x77,0x51,0x77,0x51,0x71,0x00},    // F8
	{0x00,0x77,0x51,0x77,0x41,0x71,0x00},    // F9
	{0x00,0x7B,0x59,0x5B,0x59,0x79,0x00},    // F10
	{0x00,0x57,0x51,0x57,0x51,0x51,0x00},    // F11
	{0x00,0x6B,0x49,0x6B,0x29,0x69,0x00},    // F12           70

    {0X00,0X00,0X00,0X00,0X00,0X00,0X00},    // 空白
    {0X00,0X00,0X00,0X00,0X00,0X00,0X00},    // 空白
    {0X00,0X00,0X00,0X00,0X00,0X00,0X00},    // 空白
    {0X00,0X00,0X00,0X00,0X00,0X00,0X00},    // 空白
    {0X00,0X00,0X00,0X00,0X00,0X00,0X00},    // 空白

    {0x00,0x08,0x1C,0x3E,0x08,0x3E,0x00},    // PGUP
    {0x00,0x0E,0x12,0x22,0x12,0x0E,0x00},    // DELETE
	{0x00,0x3E,0x1C,0x08,0x00,0x3E,0x00},    // END
	{0x00,0x3E,0x08,0x3E,0x1C,0x08,0x00},    // PGDN
	{0x00,0x08,0x10,0x3E,0x10,0x08,0x00},    // RIGHT
	{0x00,0x08,0x04,0x3E,0x04,0x08,0x00},    // LEFT
	{0x00,0x08,0x08,0x2A,0x1C,0x08,0x00},    // DOWN
	{0x00,0x08,0x1C,0x2A,0x08,0x08,0x00},    // UP             83
    
	{0x00,0x00,0x08,0x14,0x22,0x00,0x00},    // CTRL_L
	{0x00,0x08,0x14,0x22,0x36,0x1C,0x00},    // SHIFT_L
	{0x00,0x00,0x36,0x08,0x30,0x00,0x00},    // ALT_L
	{0x00,0x08,0x14,0x22,0x14,0x08,0x00},    // WIN_L
	{0x00,0x08,0x14,0x22,0x36,0x1C,0x00},    // SHIFT_R
	{0x00,0x00,0x36,0x08,0x30,0x00,0x00},    // ALT_R
	{0x00,0x07,0x01,0x77,0x51,0x51,0x00},    // FN              
    {0x00,0x22,0x55,0x00,0x22,0x1C,0x00},    // 笑脸           91

    {0x00,0x45,0x29,0x7D,0x11,0x7D,0x11},    // K14            92
    {0x00,0x3E,0x00,0x02,0x04,0x08,0x10},    // K56            93
    {0x40,0x24,0x77,0x49,0x77,0x12,0x01},    // K131           94
    {0x00,0x04,0x77,0x41,0x77,0x10,0x00},    // K132           95
    {0x00,0x00,0x75,0x53,0x55,0x00,0x00},    // K133           96

    {0x49,0x2A,0x1C,0x77,0x1C,0x2A,0x49},    // 屏幕亮度+       97
    {0x00,0x2A,0x1C,0x36,0x1C,0x2A,0x00},    // 屏幕亮度-       98
    {0x08,0x1C,0x3E,0x7F,0x36,0x36,0x36},    // 主页            99
    {0x7F,0x63,0x55,0x49,0x41,0x7F,0x00},    // 邮箱            100
    {0x02,0x77,0x02,0x00,0x25,0x72,0x25},    // 计算器          101
    {0x7F,0x45,0x4D,0x5D,0x4D,0x45,0x7F},    // 播放器          102
    {0x00,0x11,0x19,0x1D,0x19,0x11,0x00},    // 上一曲          103
    {0x00,0x51,0x53,0x57,0x53,0x51,0x00},    // 播放/暂停       104
    {0x00,0x44,0x4C,0x5C,0x4C,0x44,0x00},    // 下一曲          105
    {0x10,0x18,0x1E,0x1E,0x1E,0x18,0x10},    // 静音            106
    {0x08,0x2C,0x5F,0x6F,0x5F,0x2C,0x08},    // 音量+           107
    {0x08,0x0C,0x2F,0x4F,0x2F,0x0C,0x08},    // 音量-           108
    {0x00,0x3E,0x22,0x3E,0x00,0x2A,0x7F},    // 任务栏          109
};

// 爱心模式矩阵
const uint8_t Lattice_Heart_Index_Tab[5][10] = {
    {0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF         },
    {LT_R2_4_INDEX, LT_R3_4_INDEX, LT_R4_4_INDEX, LT_R5_4_INDEX, LT_R6_4_INDEX, 0xFF,          0xFF,          0xFF,          0xFF,          0xFF         },
    {LT_R1_3_INDEX, LT_R1_5_INDEX, LT_R2_3_INDEX, LT_R2_5_INDEX, LT_R3_3_INDEX, LT_R3_5_INDEX, LT_R4_3_INDEX, LT_R4_5_INDEX, LT_R5_3_INDEX, LT_R5_5_INDEX},
    {LT_R1_2_INDEX, LT_R1_6_INDEX, LT_R2_2_INDEX, LT_R2_6_INDEX, LT_R3_2_INDEX, LT_R3_6_INDEX, LT_R4_2_INDEX, LT_R4_6_INDEX, 0xFF,          0xFF         },
    {LT_R2_1_INDEX, LT_R2_7_INDEX, LT_R3_1_INDEX, LT_R3_7_INDEX, 0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF         }
};

// 雨滴模式矩阵    
#define LED_RAIN_SIZE   12
const uint8_t Lattice_Rain_Mode_Tab[LATTICE_LED_SIZE][LED_RAIN_SIZE] = {
    {1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0},
    {0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1},
    {0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0}
};

// 雨滴模式灯光位置存储
uint8_t Lattice_Rain_Mode_Show_Tab[LATTICE_LED_SIZE][LATTICE_LED_SIZE] = {
    0
};

// 终端模式灯光位置存储
uint8_t Lattice_Terminal_Mode_Show_Tab[LATTICE_LED_GROUP][LATTICE_LED_SIZE] = {
    0
};

// 电池形状矩阵
const uint8_t Battery_Index_Tab[LATTICE_LED_GROUP][LATTICE_LED_SIZE] = {
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
    {125,  126,  127,  128,  129,  130,  0xFF},
    {132,  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 138 },
    {139,  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 145 },
    {146,  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 152 },
    {153,  154,  155,  156,  157,  158,  0xFF},
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}
};

// 充电电量矩阵
const uint8_t Charging_Index_Tab[LATTICE_LED_GROUP][LATTICE_LED_SIZE] = {
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
    {0xFF, 133,  134,  135,  136,  137,  0xFF},
    {0xFF, 140,  141,  142,  143,  144,  0xFF},
    {0xFF, 147,  148,  149,  150,  151,  0xFF},
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}
};

#define LT_POWER_UP_HEART_GROUP  9
#define LT_POWER_UP_HEART_SIZE   31
// 开机动画_1
const uint8_t Lattice_Power_Up_Heart_Tab[LT_POWER_UP_HEART_GROUP][LT_POWER_UP_HEART_SIZE] = {
    {0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,
	 0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,
	 0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,
	 0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          LT_R4_4_INDEX},

    {0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,
	 0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,
	 0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,
	 0xFF,          LT_R3_3_INDEX, LT_R3_5_INDEX, LT_R4_3_INDEX, LT_R4_4_INDEX, LT_R4_5_INDEX, LT_R5_4_INDEX},

    {0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,
	 0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          LT_R2_2_INDEX, LT_R2_3_INDEX, LT_R2_5_INDEX,
	 LT_R2_6_INDEX, LT_R3_2_INDEX, LT_R3_3_INDEX, LT_R3_4_INDEX, LT_R3_5_INDEX, LT_R3_6_INDEX, LT_R4_2_INDEX, LT_R4_3_INDEX,
	 LT_R4_4_INDEX, LT_R4_5_INDEX, LT_R4_6_INDEX, LT_R5_3_INDEX, LT_R5_4_INDEX, LT_R5_5_INDEX, LT_R6_4_INDEX},

    {LT_R1_2_INDEX, LT_R1_6_INDEX, LT_R2_1_INDEX, LT_R2_2_INDEX, LT_R2_3_INDEX, LT_R2_5_INDEX, LT_R2_6_INDEX, LT_R2_7_INDEX,
	 LT_R3_1_INDEX, LT_R3_2_INDEX, LT_R3_3_INDEX, LT_R3_4_INDEX, LT_R3_5_INDEX, LT_R3_6_INDEX, LT_R3_7_INDEX, LT_R4_1_INDEX,
	 LT_R4_2_INDEX, LT_R4_3_INDEX, LT_R4_4_INDEX, LT_R4_5_INDEX, LT_R4_6_INDEX, LT_R4_7_INDEX, LT_R5_2_INDEX, LT_R5_3_INDEX,
	 LT_R5_4_INDEX, LT_R5_5_INDEX, LT_R5_6_INDEX, LT_R6_3_INDEX, LT_R6_4_INDEX, LT_R6_5_INDEX, LT_R7_4_INDEX},
	
    {0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,
	 0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          LT_R2_2_INDEX, LT_R2_3_INDEX, LT_R2_5_INDEX,
	 LT_R2_6_INDEX, LT_R3_2_INDEX, LT_R3_3_INDEX, LT_R3_4_INDEX, LT_R3_5_INDEX, LT_R3_6_INDEX, LT_R4_2_INDEX, LT_R4_3_INDEX,
	 LT_R4_4_INDEX, LT_R4_5_INDEX, LT_R4_6_INDEX, LT_R5_3_INDEX, LT_R5_4_INDEX, LT_R5_5_INDEX, LT_R6_4_INDEX},
	
    {0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,
	 0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,
	 0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,
	 0xFF,          LT_R3_3_INDEX, LT_R3_5_INDEX, LT_R4_3_INDEX, LT_R4_4_INDEX, LT_R4_5_INDEX, LT_R5_4_INDEX},
	
    {0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,
	 0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,
	 0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,
	 0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          LT_R4_4_INDEX},
	
    {0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,
	 0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,
	 0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,
	 0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF},
	
    {0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,
	 0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,
	 0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,
	 0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF},
};

#define LT_POWER_UP_WAVE_GROUP  27
#define LT_POWER_UP_WAVE_SIZE   14
// 开机动画_2
const uint8_t Lattice_Power_Up_Wave_Tab[LT_POWER_UP_WAVE_GROUP][LT_POWER_UP_WAVE_SIZE] = {
    {0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,
	 0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          LT_R4_1_INDEX},

    {0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,
	 0xFF,          0xFF,          0xFF,          LT_R3_1_INDEX, LT_R4_1_INDEX, LT_R4_2_INDEX, LT_R5_1_INDEX},

    {0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          LT_R2_1_INDEX,
	 LT_R3_1_INDEX, LT_R3_2_INDEX, LT_R4_2_INDEX, LT_R4_3_INDEX, LT_R5_1_INDEX, LT_R5_2_INDEX, LT_R6_1_INDEX},

    {0xFF,          0xFF,          LT_R1_1_INDEX, LT_R2_1_INDEX, LT_R2_2_INDEX, LT_R3_2_INDEX, LT_R3_3_INDEX,
	 LT_R4_3_INDEX, LT_R4_4_INDEX, LT_R5_2_INDEX, LT_R5_3_INDEX, LT_R6_1_INDEX, LT_R6_2_INDEX, LT_R7_1_INDEX},
	
    {LT_R1_1_INDEX, LT_R1_2_INDEX, LT_R2_2_INDEX, LT_R2_3_INDEX, LT_R3_3_INDEX, LT_R3_4_INDEX, LT_R4_4_INDEX,
	 LT_R4_5_INDEX, LT_R5_3_INDEX, LT_R5_4_INDEX, LT_R6_2_INDEX, LT_R6_3_INDEX, LT_R7_1_INDEX, LT_R7_2_INDEX},
	
    {LT_R1_2_INDEX, LT_R1_3_INDEX, LT_R2_3_INDEX, LT_R2_4_INDEX, LT_R3_4_INDEX, LT_R3_5_INDEX, LT_R4_5_INDEX,
	 LT_R4_6_INDEX, LT_R5_4_INDEX, LT_R5_5_INDEX, LT_R6_3_INDEX, LT_R6_4_INDEX, LT_R7_2_INDEX, LT_R7_3_INDEX},
	
    {LT_R1_3_INDEX, LT_R1_4_INDEX, LT_R2_4_INDEX, LT_R2_5_INDEX, LT_R3_5_INDEX, LT_R3_6_INDEX, LT_R4_6_INDEX,
	 LT_R4_7_INDEX, LT_R5_5_INDEX, LT_R5_6_INDEX, LT_R6_4_INDEX, LT_R6_5_INDEX, LT_R7_3_INDEX, LT_R7_4_INDEX},
	
	 
    {LT_R1_4_INDEX, LT_R1_5_INDEX, LT_R2_5_INDEX, LT_R2_6_INDEX, LT_R3_6_INDEX, LT_R3_7_INDEX, LT_R4_7_INDEX,
	 LT_R5_6_INDEX, LT_R5_7_INDEX, LT_R6_5_INDEX, LT_R6_6_INDEX, LT_R7_4_INDEX, LT_R7_5_INDEX, LT_R4_1_INDEX},   // 第二条
	
    {LT_R1_5_INDEX, LT_R1_6_INDEX, LT_R2_6_INDEX, LT_R2_7_INDEX, LT_R3_7_INDEX, LT_R5_7_INDEX, LT_R6_6_INDEX, 
	 LT_R6_7_INDEX, LT_R7_5_INDEX, LT_R7_6_INDEX, LT_R3_1_INDEX, LT_R4_1_INDEX, LT_R4_2_INDEX, LT_R5_1_INDEX},
	
    {LT_R1_6_INDEX, LT_R1_7_INDEX, LT_R2_7_INDEX, LT_R6_7_INDEX, LT_R7_6_INDEX, LT_R7_7_INDEX, LT_R2_1_INDEX, 
	 LT_R3_1_INDEX, LT_R3_2_INDEX, LT_R4_2_INDEX, LT_R4_3_INDEX, LT_R5_1_INDEX, LT_R5_2_INDEX, LT_R6_1_INDEX},
	
    {LT_R1_7_INDEX, LT_R7_7_INDEX, LT_R1_1_INDEX, LT_R2_1_INDEX, LT_R2_2_INDEX, LT_R3_2_INDEX, LT_R3_3_INDEX, 
	 LT_R4_3_INDEX, LT_R4_4_INDEX, LT_R5_2_INDEX, LT_R5_3_INDEX, LT_R6_1_INDEX, LT_R6_2_INDEX, LT_R7_1_INDEX},
	
    {LT_R1_1_INDEX, LT_R1_2_INDEX, LT_R2_2_INDEX, LT_R2_3_INDEX, LT_R3_3_INDEX, LT_R3_4_INDEX, LT_R4_4_INDEX,
	 LT_R4_5_INDEX, LT_R5_3_INDEX, LT_R5_4_INDEX, LT_R6_2_INDEX, LT_R6_3_INDEX, LT_R7_1_INDEX, LT_R7_2_INDEX},
	
    {LT_R1_2_INDEX, LT_R1_3_INDEX, LT_R2_3_INDEX, LT_R2_4_INDEX, LT_R3_4_INDEX, LT_R3_5_INDEX, LT_R4_5_INDEX,
	 LT_R4_6_INDEX, LT_R5_4_INDEX, LT_R5_5_INDEX, LT_R6_3_INDEX, LT_R6_4_INDEX, LT_R7_2_INDEX, LT_R7_3_INDEX},
	
    {LT_R1_3_INDEX, LT_R1_4_INDEX, LT_R2_4_INDEX, LT_R2_5_INDEX, LT_R3_5_INDEX, LT_R3_6_INDEX, LT_R4_6_INDEX,
	 LT_R4_7_INDEX, LT_R5_5_INDEX, LT_R5_6_INDEX, LT_R6_4_INDEX, LT_R6_5_INDEX, LT_R7_3_INDEX, LT_R7_4_INDEX},
	
	 
    {LT_R1_4_INDEX, LT_R1_5_INDEX, LT_R2_5_INDEX, LT_R2_6_INDEX, LT_R3_6_INDEX, LT_R3_7_INDEX, LT_R4_7_INDEX,
	 LT_R5_6_INDEX, LT_R5_7_INDEX, LT_R6_5_INDEX, LT_R6_6_INDEX, LT_R7_4_INDEX, LT_R7_5_INDEX, LT_R4_1_INDEX},   // 第三条
	
    {LT_R1_5_INDEX, LT_R1_6_INDEX, LT_R2_6_INDEX, LT_R2_7_INDEX, LT_R3_7_INDEX, LT_R5_7_INDEX, LT_R6_6_INDEX, 
	 LT_R6_7_INDEX, LT_R7_5_INDEX, LT_R7_6_INDEX, LT_R3_1_INDEX, LT_R4_1_INDEX, LT_R4_2_INDEX, LT_R5_1_INDEX},
	
    {LT_R1_6_INDEX, LT_R1_7_INDEX, LT_R2_7_INDEX, LT_R6_7_INDEX, LT_R7_6_INDEX, LT_R7_7_INDEX, LT_R2_1_INDEX, 
	 LT_R3_1_INDEX, LT_R3_2_INDEX, LT_R4_2_INDEX, LT_R4_3_INDEX, LT_R5_1_INDEX, LT_R5_2_INDEX, LT_R6_1_INDEX},
	
    {LT_R1_7_INDEX, LT_R7_7_INDEX, LT_R1_1_INDEX, LT_R2_1_INDEX, LT_R2_2_INDEX, LT_R3_2_INDEX, LT_R3_3_INDEX, 
	 LT_R4_3_INDEX, LT_R4_4_INDEX, LT_R5_2_INDEX, LT_R5_3_INDEX, LT_R6_1_INDEX, LT_R6_2_INDEX, LT_R7_1_INDEX},
	
    {LT_R1_1_INDEX, LT_R1_2_INDEX, LT_R2_2_INDEX, LT_R2_3_INDEX, LT_R3_3_INDEX, LT_R3_4_INDEX, LT_R4_4_INDEX,
	 LT_R4_5_INDEX, LT_R5_3_INDEX, LT_R5_4_INDEX, LT_R6_2_INDEX, LT_R6_3_INDEX, LT_R7_1_INDEX, LT_R7_2_INDEX},
	
    {LT_R1_2_INDEX, LT_R1_3_INDEX, LT_R2_3_INDEX, LT_R2_4_INDEX, LT_R3_4_INDEX, LT_R3_5_INDEX, LT_R4_5_INDEX,
	 LT_R4_6_INDEX, LT_R5_4_INDEX, LT_R5_5_INDEX, LT_R6_3_INDEX, LT_R6_4_INDEX, LT_R7_2_INDEX, LT_R7_3_INDEX},
	
    {LT_R1_3_INDEX, LT_R1_4_INDEX, LT_R2_4_INDEX, LT_R2_5_INDEX, LT_R3_5_INDEX, LT_R3_6_INDEX, LT_R4_6_INDEX,
	 LT_R4_7_INDEX, LT_R5_5_INDEX, LT_R5_6_INDEX, LT_R6_4_INDEX, LT_R6_5_INDEX, LT_R7_3_INDEX, LT_R7_4_INDEX},
	
	 
    {LT_R1_4_INDEX, LT_R1_5_INDEX, LT_R2_5_INDEX, LT_R2_6_INDEX, LT_R3_6_INDEX, LT_R3_7_INDEX, LT_R4_7_INDEX,
	 LT_R5_6_INDEX, LT_R5_7_INDEX, LT_R6_5_INDEX, LT_R6_6_INDEX, LT_R7_4_INDEX, LT_R7_5_INDEX, 0xFF},
	
    {LT_R1_5_INDEX, LT_R1_6_INDEX, LT_R2_6_INDEX, LT_R2_7_INDEX, LT_R3_7_INDEX, LT_R5_7_INDEX, LT_R6_6_INDEX, 
	 LT_R6_7_INDEX, LT_R7_5_INDEX, LT_R7_6_INDEX, 0xFF,          0xFF,          0xFF,          0xFF},
	
    {LT_R1_6_INDEX, LT_R1_7_INDEX, LT_R2_7_INDEX, LT_R6_7_INDEX, LT_R7_6_INDEX, LT_R7_7_INDEX, 0xFF, 
	 0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF},
	
    {LT_R1_7_INDEX, LT_R7_7_INDEX, 0xFF,          0xFF,          0xFF,          0xFF,          0xFF, 
	 0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF},
	
    {0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF, 
	 0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF},
	
    {0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF, 
	 0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF},
};

// 开机动画_re2
const uint8_t Lattice_Power_Up_ReWave_Tab[LT_POWER_UP_WAVE_GROUP][LT_POWER_UP_WAVE_SIZE] = {
    {0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,
	 0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          LT_R4_7_INDEX},

    {0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,
	 0xFF,          0xFF,          0xFF,          LT_R3_7_INDEX, LT_R4_7_INDEX, LT_R4_6_INDEX, LT_R5_7_INDEX},

    {0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          LT_R2_7_INDEX,
	 LT_R3_7_INDEX, LT_R3_6_INDEX, LT_R4_6_INDEX, LT_R4_5_INDEX, LT_R5_7_INDEX, LT_R5_6_INDEX, LT_R6_7_INDEX},

    {0xFF,          0xFF,          LT_R1_7_INDEX, LT_R2_7_INDEX, LT_R2_6_INDEX, LT_R3_6_INDEX, LT_R3_5_INDEX,
	 LT_R4_5_INDEX, LT_R4_4_INDEX, LT_R5_6_INDEX, LT_R5_5_INDEX, LT_R6_7_INDEX, LT_R6_6_INDEX, LT_R7_7_INDEX},
	 
    {LT_R1_7_INDEX, LT_R1_6_INDEX, LT_R2_6_INDEX, LT_R2_5_INDEX, LT_R3_5_INDEX, LT_R3_4_INDEX, LT_R4_4_INDEX,
	 LT_R4_3_INDEX, LT_R5_5_INDEX, LT_R5_4_INDEX, LT_R6_6_INDEX, LT_R6_5_INDEX, LT_R7_7_INDEX, LT_R7_6_INDEX},
	
    {LT_R1_6_INDEX, LT_R1_5_INDEX, LT_R2_5_INDEX, LT_R2_4_INDEX, LT_R3_4_INDEX, LT_R3_3_INDEX, LT_R4_3_INDEX,
	 LT_R4_2_INDEX, LT_R5_4_INDEX, LT_R5_3_INDEX, LT_R6_5_INDEX, LT_R6_4_INDEX, LT_R7_6_INDEX, LT_R7_5_INDEX},
	
    {LT_R1_5_INDEX, LT_R1_4_INDEX, LT_R2_4_INDEX, LT_R2_3_INDEX, LT_R3_3_INDEX, LT_R3_2_INDEX, LT_R4_2_INDEX,
	 LT_R4_1_INDEX, LT_R5_3_INDEX, LT_R5_2_INDEX, LT_R6_4_INDEX, LT_R6_3_INDEX, LT_R7_5_INDEX, LT_R7_4_INDEX},
	
	 
    {LT_R1_4_INDEX, LT_R1_3_INDEX, LT_R2_3_INDEX, LT_R2_2_INDEX, LT_R3_2_INDEX, LT_R3_1_INDEX, LT_R4_1_INDEX,
	 LT_R5_2_INDEX, LT_R5_1_INDEX, LT_R6_3_INDEX, LT_R6_2_INDEX, LT_R7_4_INDEX, LT_R7_3_INDEX, LT_R4_7_INDEX},	// 第二条
	
    {LT_R1_3_INDEX, LT_R1_2_INDEX, LT_R2_2_INDEX, LT_R2_1_INDEX, LT_R3_1_INDEX, LT_R5_1_INDEX, LT_R6_2_INDEX, 
	 LT_R6_1_INDEX, LT_R7_3_INDEX, LT_R7_2_INDEX, LT_R3_7_INDEX, LT_R4_7_INDEX, LT_R4_6_INDEX, LT_R5_7_INDEX},
	
    {LT_R1_2_INDEX, LT_R1_1_INDEX, LT_R2_1_INDEX, LT_R6_1_INDEX, LT_R7_2_INDEX, LT_R7_1_INDEX, LT_R2_7_INDEX, 
	 LT_R3_7_INDEX, LT_R3_6_INDEX, LT_R4_6_INDEX, LT_R4_5_INDEX, LT_R5_7_INDEX, LT_R5_6_INDEX, LT_R6_7_INDEX},
	
    {LT_R1_1_INDEX, LT_R7_1_INDEX, LT_R1_7_INDEX, LT_R2_7_INDEX, LT_R2_6_INDEX, LT_R3_6_INDEX, LT_R3_5_INDEX, 
	 LT_R4_5_INDEX, LT_R4_4_INDEX, LT_R5_6_INDEX, LT_R5_5_INDEX, LT_R6_7_INDEX, LT_R6_6_INDEX, LT_R7_7_INDEX},
	
    {LT_R1_7_INDEX, LT_R1_6_INDEX, LT_R2_6_INDEX, LT_R2_5_INDEX, LT_R3_5_INDEX, LT_R3_4_INDEX, LT_R4_4_INDEX,
	 LT_R4_3_INDEX, LT_R5_5_INDEX, LT_R5_4_INDEX, LT_R6_6_INDEX, LT_R6_5_INDEX, LT_R7_7_INDEX, LT_R7_6_INDEX},
	
    {LT_R1_6_INDEX, LT_R1_5_INDEX, LT_R2_5_INDEX, LT_R2_4_INDEX, LT_R3_4_INDEX, LT_R3_3_INDEX, LT_R4_3_INDEX,
	 LT_R4_2_INDEX, LT_R5_4_INDEX, LT_R5_3_INDEX, LT_R6_5_INDEX, LT_R6_4_INDEX, LT_R7_6_INDEX, LT_R7_5_INDEX},
	
    {LT_R1_5_INDEX, LT_R1_4_INDEX, LT_R2_4_INDEX, LT_R2_3_INDEX, LT_R3_3_INDEX, LT_R3_2_INDEX, LT_R4_2_INDEX,
	 LT_R4_1_INDEX, LT_R5_3_INDEX, LT_R5_2_INDEX, LT_R6_4_INDEX, LT_R6_3_INDEX, LT_R7_5_INDEX, LT_R7_4_INDEX},
	
	 
    {LT_R1_4_INDEX, LT_R1_3_INDEX, LT_R2_3_INDEX, LT_R2_2_INDEX, LT_R3_2_INDEX, LT_R3_1_INDEX, LT_R4_1_INDEX,
	 LT_R5_2_INDEX, LT_R5_1_INDEX, LT_R6_3_INDEX, LT_R6_2_INDEX, LT_R7_4_INDEX, LT_R7_3_INDEX, LT_R4_7_INDEX},	// 第三条
	
    {LT_R1_3_INDEX, LT_R1_2_INDEX, LT_R2_2_INDEX, LT_R2_1_INDEX, LT_R3_1_INDEX, LT_R5_1_INDEX, LT_R6_2_INDEX, 
	 LT_R6_1_INDEX, LT_R7_3_INDEX, LT_R7_2_INDEX, LT_R3_7_INDEX, LT_R4_7_INDEX, LT_R4_6_INDEX, LT_R5_7_INDEX},
	
    {LT_R1_2_INDEX, LT_R1_1_INDEX, LT_R2_1_INDEX, LT_R6_1_INDEX, LT_R7_2_INDEX, LT_R7_1_INDEX, LT_R2_7_INDEX, 
	 LT_R3_7_INDEX, LT_R3_6_INDEX, LT_R4_6_INDEX, LT_R4_5_INDEX, LT_R5_7_INDEX, LT_R5_6_INDEX, LT_R6_7_INDEX},
	
    {LT_R1_1_INDEX, LT_R7_1_INDEX, LT_R1_7_INDEX, LT_R2_7_INDEX, LT_R2_6_INDEX, LT_R3_6_INDEX, LT_R3_5_INDEX, 
	 LT_R4_5_INDEX, LT_R4_4_INDEX, LT_R5_6_INDEX, LT_R5_5_INDEX, LT_R6_7_INDEX, LT_R6_6_INDEX, LT_R7_7_INDEX},
	
    {LT_R1_7_INDEX, LT_R1_6_INDEX, LT_R2_6_INDEX, LT_R2_5_INDEX, LT_R3_5_INDEX, LT_R3_4_INDEX, LT_R4_4_INDEX,
	 LT_R4_3_INDEX, LT_R5_5_INDEX, LT_R5_4_INDEX, LT_R6_6_INDEX, LT_R6_5_INDEX, LT_R7_7_INDEX, LT_R7_6_INDEX},
	
    {LT_R1_6_INDEX, LT_R1_5_INDEX, LT_R2_5_INDEX, LT_R2_4_INDEX, LT_R3_4_INDEX, LT_R3_3_INDEX, LT_R4_3_INDEX,
	 LT_R4_2_INDEX, LT_R5_4_INDEX, LT_R5_3_INDEX, LT_R6_5_INDEX, LT_R6_4_INDEX, LT_R7_6_INDEX, LT_R7_5_INDEX},
	
    {LT_R1_5_INDEX, LT_R1_4_INDEX, LT_R2_4_INDEX, LT_R2_3_INDEX, LT_R3_3_INDEX, LT_R3_2_INDEX, LT_R4_2_INDEX,
	 LT_R4_1_INDEX, LT_R5_3_INDEX, LT_R5_2_INDEX, LT_R6_4_INDEX, LT_R6_3_INDEX, LT_R7_5_INDEX, LT_R7_4_INDEX},
	
	 
    {LT_R1_4_INDEX, LT_R1_3_INDEX, LT_R2_3_INDEX, LT_R2_2_INDEX, LT_R3_2_INDEX, LT_R3_1_INDEX, LT_R4_1_INDEX,
	 LT_R5_2_INDEX, LT_R5_1_INDEX, LT_R6_3_INDEX, LT_R6_2_INDEX, LT_R7_4_INDEX, LT_R7_3_INDEX, 0xFF},
	
    {LT_R1_3_INDEX, LT_R1_2_INDEX, LT_R2_2_INDEX, LT_R2_1_INDEX, LT_R3_1_INDEX, LT_R5_1_INDEX, LT_R6_2_INDEX, 
	 LT_R6_1_INDEX, LT_R7_3_INDEX, LT_R7_2_INDEX, 0xFF,          0xFF,          0xFF,          0xFF},
	
    {LT_R1_2_INDEX, LT_R1_1_INDEX, LT_R2_1_INDEX, LT_R6_1_INDEX, LT_R7_2_INDEX, LT_R7_1_INDEX, 0xFF, 
	 0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF},
	
    {LT_R1_1_INDEX, LT_R7_1_INDEX, 0xFF,          0xFF,          0xFF,          0xFF,          0xFF, 
	 0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF},
	
    {0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF, 
	 0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF},
	
    {0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF, 
	 0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF,          0xFF},
};

uint8_t Lattice_Flash_Count = 0x00;
uint8_t Lattice_Led_Count = 0x00;
uint8_t Lattice_Heart_Count = 0x00;
uint8_t Lattice_Power_Up_Count = 0x00;
uint8_t Lattice_Play_Point = 64;
uint8_t Lattice_Play_Point_1 = 4;
uint8_t Lattice_Heart_Point = 0;
uint8_t Lattice_Charge_Point = 0;
uint8_t Lattice_Pwm_R = 0;
uint8_t Lattice_Pwm_G = 0;
uint8_t Lattice_Pwm_B = 0;
uint8_t Lattice_Pwm_Colour = 0;
uint8_t Terminal_Row_Count = 0;         // 终端模式行矩阵
uint8_t Terminal_Col_Count = 0;         // 终端模式列矩阵
uint8_t Terminal_Random = 0;            // 终端模式随机换行

// 光芒四射
uint8_t Lattice_Circle_Count = 0;
uint8_t Lattice_Circle_Count_Second = 0;
uint8_t Lattice_Circle_Second_Flag = 0;
uint8_t Last_Count1 = 0;
uint8_t Last_Count2 = 0;
uint8_t Last_Second_Flag = 0;

// 开机动画
uint8_t Lattice_Power_Flag = 1;
uint8_t LATTICE_Power_Up_Heart_Point = 0;
uint8_t LATTICE_Power_Up_Wave_Point = 0;
uint8_t LATTICE_Power_Up_ReWave_Point = 0;
uint8_t Lt_Heart_Flag = 0;
uint8_t Lt_Heart_Count = 0;
uint8_t Lt_Heart_Flag_Second = 0;
uint8_t Lt_Heart_Count_Second = 0;
uint8_t Lt_Wave_Flag = 0;
uint8_t Lt_ReWave_Flag = 0;
uint8_t Lt_Last_Heart_Count = 0;
uint8_t Lt_Last_Wave_Count = 0;
uint8_t Lt_Last_ReWave_Count = 0;
/*********************************
         初始化函数
*********************************/
void Lattice_Init(void) {
    for (uint8_t i = 0; i < LATTICE_LED_GROUP; i++) {
        for (uint8_t j = 0; j < LATTICE_LED_SIZE; j++) {
            rgb_matrix_set_color(Lattice_Index_Tab[i][j], 0x00, 0x00, 0x00);
        }
    }
    Lattice_Play_Point = 0;
    Lattice_Pwm_R = 0;
    Lattice_Pwm_G = 0;
    Lattice_Pwm_B = 0;
    Lattice_Pwm_Colour = 0;
    Lattice_Heart_Point = 0;
    Lattice_Charge_Point = 0;
    Lattice_Circle_Count = 0;
    Lattice_Circle_Count_Second = 0;

    LATTICE_Power_Up_Heart_Point = 0;
    LATTICE_Power_Up_Wave_Point = 0;
    LATTICE_Power_Up_ReWave_Point = 0;
    Lt_Heart_Flag = 1;
    Lt_Heart_Count = 0;
    Lt_Heart_Flag_Second = 0;
    Lt_Heart_Count_Second = 0;
    Lt_Wave_Flag = 0;
    Lt_ReWave_Flag = 0;
    Lt_Last_Heart_Count = 0;
    Lt_Last_Wave_Count = 0;
    Lt_Last_ReWave_Count = 0;
    
    Last_Count1 = 0;
    Last_Count2 = 0;
    Last_Second_Flag = false;
}

void Lattice_Pwm_Rgb_Updata(uint8_t Pwm) {
    Lattice_Pwm_R |= Keyboard_Info.Lattice_Saturation;
    Lattice_Pwm_G |= Keyboard_Info.Lattice_Saturation;
    Lattice_Pwm_B |= Keyboard_Info.Lattice_Saturation;

    uint16_t Temp_Pwm = 0;
    Temp_Pwm = Lattice_Pwm_R * Pwm;
    Lattice_Pwm_R = (Temp_Pwm >> 8);

    Temp_Pwm = Lattice_Pwm_G * Pwm;
    Lattice_Pwm_G = (Temp_Pwm >> 8);

    Temp_Pwm = Lattice_Pwm_B * Pwm;
    Lattice_Pwm_B = (Temp_Pwm >> 8);
}

void Lattice_Pwm_Ds_Updata(uint8_t Pwm) {
    uint16_t Temp_Pwm = 0;
    Temp_Pwm = Lattice_Pwm_R * Pwm;
    Lattice_Pwm_R = (Temp_Pwm >> 8);
    Lattice_Pwm_R |= Keyboard_Info.Lattice_Saturation;
    Temp_Pwm = Lattice_Pwm_R * Keyboard_Info.Lattice_Brightness;
    Lattice_Pwm_R = (Temp_Pwm >> 8);

    Temp_Pwm = Lattice_Pwm_G * Pwm;
    Lattice_Pwm_G = (Temp_Pwm >> 8);
    Lattice_Pwm_G |= Keyboard_Info.Lattice_Saturation;
    Temp_Pwm = Lattice_Pwm_G * Keyboard_Info.Lattice_Brightness;
    Lattice_Pwm_G = (Temp_Pwm >> 8);

    Temp_Pwm = Logo_Pwm_B * Pwm;
    Lattice_Pwm_B = (Temp_Pwm >> 8);
    Lattice_Pwm_B |= Keyboard_Info.Lattice_Saturation;
    Temp_Pwm = Lattice_Pwm_B * Keyboard_Info.Lattice_Brightness;
    Lattice_Pwm_B = (Temp_Pwm >> 8);
}

/*********************************
         爱心常亮
*********************************/
void Lattice_Heart_mode_Show(void) {
    if (Lattice_Heart_Count > 20) {
        Lattice_Heart_Count = 0;
        Lattice_Heart_Point += 1;
        if (Lattice_Heart_Point >= 4) {
            Lattice_Heart_Point = 5;
        }
    }

    Lattice_Pwm_R = LED_Mix_Colour_Tab[Keyboard_Info.Lattice_Colour][0];
    Lattice_Pwm_G = LED_Mix_Colour_Tab[Keyboard_Info.Lattice_Colour][1];
    Lattice_Pwm_B = LED_Mix_Colour_Tab[Keyboard_Info.Lattice_Colour][2];
    Lattice_Pwm_Rgb_Updata(Keyboard_Info.Lattice_Brightness);

    if (Lattice_Heart_Point < 4) {
        for (uint8_t j = 0; j < 10; j++) {
            uint8_t Led_Index = Lattice_Heart_Index_Tab[Lattice_Heart_Point][j];
            if (Led_Index < RGB_MATRIX_LED_COUNT) {
                rgb_matrix_set_color(Led_Index, Lattice_Pwm_R, Lattice_Pwm_G, Lattice_Pwm_B);
            }
        }
    } else {
        for (uint8_t i = 0; i < 5; i++) {
            for (uint8_t j = 0; j < 10; j++) {
                uint8_t Led_Index = Lattice_Heart_Index_Tab[i][j];
                if (Led_Index < RGB_MATRIX_LED_COUNT) {
                    rgb_matrix_set_color(Led_Index, Lattice_Pwm_R, Lattice_Pwm_G, Lattice_Pwm_B);
                }
            }
        }
    }
}

/*********************************
         扫描模式
*********************************/
void Lattice_Scan_mode_Show(void) {
    if (Lattice_Led_Count > ((LATTICE_MAX_SPEED - Keyboard_Info.Lattice_Speed) + 1 * 4)) {
        Lattice_Led_Count = 0;
        Lattice_Play_Point += 1;
        if (Lattice_Play_Point >= LED_MAP_SIZE) {
            Lattice_Play_Point = 0;
        }
        Lattice_Pwm_Colour += 8;
    }

    for (uint8_t i = 0; i < LED_MAP_SIZE; i++) {
        for (uint8_t j = 0; j < 8; j++) {
            rgb_matrix_set_color(Lattice_Scan_Mode_Tab[i][j], 0x00, 0x00, 0x00);
        }
    }

    for (uint8_t j = 0; j < 8; j++) {
        Lattice_Pwm_R = LED_Mix_Colour_Tab[Lattice_Pwm_Colour][0];
        Lattice_Pwm_G = LED_Mix_Colour_Tab[Lattice_Pwm_Colour][1];
        Lattice_Pwm_B = LED_Mix_Colour_Tab[Lattice_Pwm_Colour][2];
        Lattice_Pwm_Rgb_Updata(Keyboard_Info.Lattice_Brightness);
        rgb_matrix_set_color(Lattice_Scan_Mode_Tab[Lattice_Play_Point][j], Lattice_Pwm_R, Lattice_Pwm_G, Lattice_Pwm_B);
    }
}

/*********************************
         雨滴模式
*********************************/
void Lattice_Rain_mode_Show(void) {
    if (Lattice_Led_Count >= ((LATTICE_MAX_SPEED - Keyboard_Info.Lattice_Speed) + 1 * 4)) {
        Lattice_Led_Count = 0;
        if (Lattice_Play_Point) {
            Lattice_Play_Point--;
        } else {
            Lattice_Play_Point = (LED_RAIN_SIZE - 1);
        }
    }

    uint8_t i = 0, j = 0, Temp_Count = 0;
    if (Lattice_Play_Point < (LATTICE_LED_SIZE - 1)) { // 0 ~ 5
        Lattice_Play_Point_1 = ((LATTICE_LED_SIZE - 1) - Lattice_Play_Point);
        
        for (i = (LED_RAIN_SIZE - Lattice_Play_Point_1); i < LED_RAIN_SIZE; i++) {
            for (j = 0; j < LATTICE_LED_SIZE; j++) {
                Lattice_Rain_Mode_Show_Tab[j][Temp_Count] = Lattice_Rain_Mode_Tab[j][i];
            }
            Temp_Count++;
        }

        for (i = 0; i < (Lattice_Play_Point + 1); i++) {
            for (j = 0; j < LATTICE_LED_SIZE; j++) {
                Lattice_Rain_Mode_Show_Tab[j][Temp_Count] = Lattice_Rain_Mode_Tab[j][i];
            }
            Temp_Count++;
        }
    } else {                                    // 6 ~ LED_MAP_SIZE
        for (i = (Lattice_Play_Point - (LATTICE_LED_SIZE - 1)); i <= Lattice_Play_Point; i++) {
            for (j = 0; j < LATTICE_LED_SIZE; j++) {
                Lattice_Rain_Mode_Show_Tab[j][Temp_Count] = Lattice_Rain_Mode_Tab[j][i];
            }
            Temp_Count++;
        }
    }

    Lattice_Pwm_R = LED_Mix_Colour_Tab[Keyboard_Info.Lattice_Colour_Blue][0];
    Lattice_Pwm_G = LED_Mix_Colour_Tab[Keyboard_Info.Lattice_Colour_Blue][1];
    Lattice_Pwm_B = LED_Mix_Colour_Tab[Keyboard_Info.Lattice_Colour_Blue][2];
    Lattice_Pwm_Rgb_Updata(Keyboard_Info.Lattice_Brightness);
    for (i = 0; i < LATTICE_LED_SIZE; i++) {
        for (j = 0; j < LATTICE_LED_SIZE; j++) {
            if (Lattice_Rain_Mode_Show_Tab[j][i]) {
                rgb_matrix_set_color(Lattice_Index_Tab[i][j], Lattice_Pwm_R, Lattice_Pwm_G, Lattice_Pwm_B);
            } else {
                rgb_matrix_set_color(Lattice_Index_Tab[i][j], 0x00, 0x00, 0x00);
            }
        }
    }
}

/*********************************
         打字机模式函数
*********************************/
void Lattice_Type_mode_Show(void) {
    Lattice_Pwm_Rgb_Updata(Keyboard_Info.Lattice_Brightness);
}

/*********************************
         终端模式函数
*********************************/
void Lattice_Terminal_mode_Show(void) {
    Lattice_Pwm_Rgb_Updata(Keyboard_Info.Lattice_Brightness);
}

/*********************************
         打字机模式触发
*********************************/
void Lattice_Type_mode_Show_Trigger(uint16_t Led_Index, uint8_t IsPressed) {
    uint8_t Lattice_Button = 0;
    if (IsPressed) {
        for (uint8_t j = 0; j < 7; j++) {
            if ((Led_Index == KC_LEFT_CTRL) || (Led_Index == KC_LEFT_SHIFT) || (Led_Index == KC_LEFT_ALT) || (Led_Index == KC_LEFT_GUI)
                || (Led_Index == KC_RIGHT_SHIFT) || (Led_Index == KC_RIGHT_CTRL) || (Led_Index == KC_RIGHT_GUI) || (Led_Index == MO(2)) || (Led_Index == MO(3))
                || (Led_Index == KC_BRIGHTNESS_DOWN) || (Led_Index == KC_BRIGHTNESS_UP) || (Led_Index == KC_WWW_HOME) || (Led_Index == KC_MAIL) || (Led_Index == KC_CALCULATOR)
                || (Led_Index == KC_MEDIA_SELECT) || (Led_Index == KC_MEDIA_PREV_TRACK) || (Led_Index == KC_MEDIA_PLAY_PAUSE) || (Led_Index == KC_MEDIA_NEXT_TRACK) || (Led_Index == KC_AUDIO_MUTE)
                || (Led_Index == KC_AUDIO_VOL_DOWN) || (Led_Index == KC_AUDIO_VOL_UP) || (Led_Index == KC_K14) || (Led_Index == KC_K56) || (Led_Index == KC_K131) || (Led_Index == KC_K132) || (Led_Index == KC_K133)
                || (Led_Index == KC_LNG2) || (Led_Index == KC_LNG1) || (Led_Index == KC_MCTL) || (Led_Index == KC_LPAD)) {
                switch (Led_Index) {
                    case KC_LEFT_CTRL:
                    case KC_RIGHT_CTRL: {
                        Lattice_Button = LED_Button_Tab[83][j];
                    } break;
                    case KC_RIGHT_SHIFT:
                    case KC_LEFT_SHIFT: {
                        Lattice_Button = LED_Button_Tab[84][j];
                    } break;
                    case KC_RIGHT_ALT:
                    case KC_LEFT_ALT: {
                        Lattice_Button = LED_Button_Tab[85][j];
                    } break;
                    case KC_RIGHT_GUI:
                    case KC_LEFT_GUI: {
                        Lattice_Button = LED_Button_Tab[86][j];
                    } break;
                    case MO(2):
                    case MO(3): {
                        Lattice_Button = LED_Button_Tab[89][j];
                    } break;
                    case KC_BRIGHTNESS_DOWN: {
                        Lattice_Button = LED_Button_Tab[97][j];
                    } break;
                    case KC_BRIGHTNESS_UP: {
                        Lattice_Button = LED_Button_Tab[96][j];
                    } break;
                    case KC_WWW_HOME: {
                        Lattice_Button = LED_Button_Tab[98][j];
                    } break;
                    case KC_MAIL: {
                        Lattice_Button = LED_Button_Tab[99][j];
                    } break;
                    case KC_CALCULATOR: {
                        Lattice_Button = LED_Button_Tab[100][j];
                    } break;
                    case KC_MEDIA_SELECT: {
                        Lattice_Button = LED_Button_Tab[101][j];
                    } break;
                    case KC_MEDIA_PREV_TRACK: {
                        Lattice_Button = LED_Button_Tab[102][j];
                    } break;
                    case KC_MEDIA_PLAY_PAUSE: {
                        Lattice_Button = LED_Button_Tab[103][j];
                    } break;
                    case KC_MEDIA_NEXT_TRACK: {
                        Lattice_Button = LED_Button_Tab[104][j];
                    } break;
                    case KC_AUDIO_MUTE: {
                        Lattice_Button = LED_Button_Tab[105][j];
                    } break;
                    case KC_AUDIO_VOL_DOWN: {
                        Lattice_Button = LED_Button_Tab[107][j];
                    } break;
                    case KC_AUDIO_VOL_UP:  {
                        Lattice_Button = LED_Button_Tab[106][j];
                    } break;
                    case KC_MCTL:  {
                        Lattice_Button = LED_Button_Tab[108][j];
                    } break;
                    case KC_LPAD:  {
                        Lattice_Button = LED_Button_Tab[90][j];
                    } break;
                    case KC_K14: {
                        Lattice_Button = LED_Button_Tab[91][j];
                    } break;
                    case KC_K56: {
                        Lattice_Button = LED_Button_Tab[92][j];
                    } break;
                    case KC_LNG2:
                    case KC_K131: {
                        Lattice_Button = LED_Button_Tab[93][j];
                    } break;
                    case KC_LNG1: {
                        if (User_Change_Flag) {
                            Lattice_Button = LED_Button_Tab[94][j];
                        } else if (User_Kana_Flag) {
                            Lattice_Button = LED_Button_Tab[95][j];
                        }
                    } break;
                    case KC_K132: {
                        Lattice_Button = LED_Button_Tab[94][j];
                    } break;
                    case KC_K133: {
                        Lattice_Button = LED_Button_Tab[95][j];
                    } break;
                    default:  break;
                }
            } else {
                Lattice_Button = LED_Button_Tab[Led_Index][j];
            }

            for (uint8_t k = 0; k < 7; k++) {
                uint8_t Index = Lattice_Index_Tab[j][k];
                
                // 设置颜色和亮度
                Lattice_Pwm_R = LED_Mix_Colour_Tab[Keyboard_Info.Lattice_Colour_Blue][0];
                Lattice_Pwm_G = LED_Mix_Colour_Tab[Keyboard_Info.Lattice_Colour_Blue][1];
                Lattice_Pwm_B = LED_Mix_Colour_Tab[Keyboard_Info.Lattice_Colour_Blue][2];
                Lattice_Pwm_Rgb_Updata(Keyboard_Info.Lattice_Brightness);

                if (Lattice_Button & (1 << k)) {
                    rgb_matrix_set_color(Index, Lattice_Pwm_R, Lattice_Pwm_G, Lattice_Pwm_B);
                } else {
                    rgb_matrix_set_color(Index, 0X00, 0x00, 0x00);
                }
            }
        }
    }
}

/*********************************
         终端模式触发
*********************************/
void Lattice_Terminal_mode_Show_Trigger(uint16_t Led_Index, uint8_t IsPressed) {
    srand(timer_read32()); // 使用QMK内置计时器初始化随机种子
    if (IsPressed) {
        Terminal_Random = rand() % 10;
        if (Terminal_Random < 2) {
            Terminal_Row_Count++;
            Terminal_Col_Count = 0;
        } else {
            // 点亮当前行列
            Lattice_Terminal_Mode_Show_Tab[Terminal_Row_Count][Terminal_Col_Count] = 1;
            Terminal_Col_Count++;
            if (Terminal_Col_Count > 6) {
                Terminal_Row_Count++;
                Terminal_Col_Count = 0;
            }
        }
        
        if (Terminal_Row_Count > 6) {
            for (uint8_t i = 0; i < (LATTICE_LED_GROUP - 1); i++) {
                for (uint8_t j = 0; j < LATTICE_LED_SIZE; j++) {
                    Lattice_Terminal_Mode_Show_Tab[i][j] = Lattice_Terminal_Mode_Show_Tab[i + 1][j];
                }
            }
            for (uint8_t k = 0; k < LATTICE_LED_SIZE; k++) {
                Lattice_Terminal_Mode_Show_Tab[6][k] = 0;
            }
            Terminal_Row_Count = 6;
        }

        // 设置颜色和亮度
        Lattice_Pwm_R = LED_Mix_Colour_Tab[Keyboard_Info.Lattice_Colour_Blue][0];
        Lattice_Pwm_G = LED_Mix_Colour_Tab[Keyboard_Info.Lattice_Colour_Blue][1]; 
        Lattice_Pwm_B = LED_Mix_Colour_Tab[Keyboard_Info.Lattice_Colour_Blue][2];
        Lattice_Pwm_Rgb_Updata(Keyboard_Info.Lattice_Brightness);
        for (uint8_t i = 0; i < LATTICE_LED_GROUP; i++) {
            for (uint8_t j = 0; j < LATTICE_LED_SIZE; j++) {
                if (Lattice_Terminal_Mode_Show_Tab[i][j]) {
                    rgb_matrix_set_color(Lattice_Index_Tab[i][j], Lattice_Pwm_R, Lattice_Pwm_G, Lattice_Pwm_B);
                } else {
                    rgb_matrix_set_color(Lattice_Index_Tab[i][j], 0x00, 0x00, 0x00);
                }
            }
        }
    }
}

/*********************************
         需要按键的灯光效果
*********************************/
void Lattice_Key_Led_Trigger_Mode(uint16_t Key_Index, uint8_t IsPressed) {
    if (Lattice_Power_Flag) {
        return;
    } if (User_Key_Batt_Num_Show) {
        return;
    } else {
        switch(Keyboard_Info.Lattice_Mode) {
            case LATTICE_TYPE_MODE:    { Lattice_Type_mode_Show_Trigger(Key_Index, IsPressed); }     break;
            case LATTICE_TER_MODE:     { Lattice_Terminal_mode_Show_Trigger(Key_Index, IsPressed); } break;
            default:                                                                                 break;
        }
    }

}

/*********************************
            光芒四射
*********************************/
void Lattice_Circle_mode_Show(void) {
    // 修改原有计数更新部分
    if (Lattice_Led_Count > ((LATTICE_MAX_SPEED - Keyboard_Info.Lattice_Speed) + 1 * 2)) {
        Lattice_Led_Count = 0;
        Lattice_Circle_Count = (Lattice_Circle_Count + 1) % LED_FIRST_CIRCLE_SIZE;
        
        if (Lattice_Circle_Second_Flag) {
            Lattice_Circle_Count_Second += 1;
            if (Lattice_Circle_Count_Second >= LED_SECOND_CIRCLE_SIZE) {
                Lattice_Circle_Count_Second = 0;
                Lattice_Circle_Second_Flag = 0;
            }
        } else if (Lattice_Circle_Count == (LED_SPILIT_FLAG + 1)) {
            Lattice_Circle_Second_Flag = 1; // 激活第二条轨迹
            Lattice_Circle_Count_Second = 0;   // 从起始位置开始
        }
    }

    // 熄灭上一帧的两条轨迹 (只熄灭变化的LED)
    for (uint8_t j = 0; j < 14; j++) {
        uint8_t led = Lattice_First_Circle_Mode_Tab[Last_Count1][j];
        if (led < RGB_MATRIX_LED_COUNT) rgb_matrix_set_color(led, 0, 0, 0);
    }
    if (Last_Second_Flag) {  // 第二条轨迹上次已激活才熄灭
        for (uint8_t j = 0; j < 14; j++) {
            uint8_t led = Lattice_Second_Circle_Mode_Tab[Last_Count2][j];
            if (led < RGB_MATRIX_LED_COUNT) rgb_matrix_set_color(led, 0, 0, 0);
        }
    }

    // 设置当前帧颜色 (保持原有逻辑)
    Lattice_Pwm_R = LED_Mix_Colour_Tab[Keyboard_Info.Lattice_Colour][0];
    Lattice_Pwm_G = LED_Mix_Colour_Tab[Keyboard_Info.Lattice_Colour][1];
    Lattice_Pwm_B = LED_Mix_Colour_Tab[Keyboard_Info.Lattice_Colour][2];
    Lattice_Pwm_Rgb_Updata(Keyboard_Info.Lattice_Brightness);

    // 点亮当前帧的第一条轨迹
    for (uint8_t j = 0; j < 14; j++) {
        uint8_t led = Lattice_First_Circle_Mode_Tab[Lattice_Circle_Count][j];
        if (led < RGB_MATRIX_LED_COUNT) rgb_matrix_set_color(led, Lattice_Pwm_R, Lattice_Pwm_G, Lattice_Pwm_B);
    }
    
    // 点亮当前帧的第二条轨迹（如果已激活）
    if (Lattice_Circle_Second_Flag) {
        for (uint8_t j = 0; j < 14; j++) {
            uint8_t led = Lattice_Second_Circle_Mode_Tab[Lattice_Circle_Count_Second][j];
            if (led < RGB_MATRIX_LED_COUNT) rgb_matrix_set_color(led, Lattice_Pwm_R, Lattice_Pwm_G, Lattice_Pwm_B);
        }
    }

    // 更新"上一帧"状态
    Last_Count1 = Lattice_Circle_Count;
    Last_Count2 = Lattice_Circle_Count_Second;
    Last_Second_Flag = Lattice_Circle_Second_Flag;
}

/*********************************
            开机动画
*********************************/
void Lt_Power_Up_Show(void) {
	if (Lattice_Power_Up_Count > 6) {
        Lattice_Power_Up_Count = 0;
        // 逻辑判定
        if (Lt_Heart_Flag || Lt_Heart_Flag_Second) {
	        Lt_Last_Heart_Count = LATTICE_Power_Up_Heart_Point;
            LATTICE_Power_Up_Heart_Point = (LATTICE_Power_Up_Heart_Point + 1) % LT_POWER_UP_HEART_GROUP;
            if (LATTICE_Power_Up_Heart_Point == (LT_POWER_UP_HEART_GROUP - 1)) {
                if (Lt_Heart_Flag) {
                    Lt_Heart_Count += 1;
                    if (Lt_Heart_Count == 2) {
                        Lt_Heart_Flag = 0;
                        Lt_Wave_Flag = 1;
                    }
                }
                if (Lt_Heart_Flag_Second) {
                    Lt_Heart_Count_Second += 1;
                    // if (Lt_Heart_Count_Second == 4) {
                    //     Lt_Heart_Flag_Second = 0;
                    //     // 开机动画播放一次后停止
                    //     Lattice_Init();
                    //     Lattice_Power_Flag = 0;
                    // }
                }
            }
        }
        
        if (Lt_Wave_Flag) {
            Lt_Last_Wave_Count = LATTICE_Power_Up_Wave_Point;
            LATTICE_Power_Up_Wave_Point = (LATTICE_Power_Up_Wave_Point + 1) % LT_POWER_UP_WAVE_GROUP;
            if (LATTICE_Power_Up_Wave_Point == (LT_POWER_UP_WAVE_GROUP - 1)) {
                Lt_Wave_Flag = 0;
                Lt_ReWave_Flag = 1;
            }
        }
        if (Lt_ReWave_Flag) {
            Lt_Last_ReWave_Count = LATTICE_Power_Up_ReWave_Point;
            LATTICE_Power_Up_ReWave_Point = (LATTICE_Power_Up_ReWave_Point + 1) % LT_POWER_UP_WAVE_GROUP;
            if (LATTICE_Power_Up_ReWave_Point == (LT_POWER_UP_WAVE_GROUP - 1)) {
                Lt_ReWave_Flag = 0;
                Lt_Heart_Flag_Second = 1;
            }
        }

    }
	
	// 熄灭上一帧动画
	if (Lt_Heart_Flag || Lt_Heart_Flag_Second) {
		for (uint8_t j = 0; j < LT_POWER_UP_HEART_SIZE; j++) {
			uint8_t Led_Index = Lattice_Power_Up_Heart_Tab[Lt_Last_Heart_Count][j];
			if (Led_Index < RGB_MATRIX_LED_COUNT) {
				rgb_matrix_set_color(Led_Index, 0, 0, 0);
			}
		}
	}
	if (Lt_Wave_Flag) {
		for (uint8_t j = 0; j < LT_POWER_UP_WAVE_SIZE; j++) {
			uint8_t Led_Index = Lattice_Power_Up_Wave_Tab[Lt_Last_Wave_Count][j];
			if (Led_Index < RGB_MATRIX_LED_COUNT) {
				rgb_matrix_set_color(Led_Index, 0, 0, 0);
			}
		}
	}
	if (Lt_ReWave_Flag) {
		for (uint8_t j = 0; j < LT_POWER_UP_WAVE_SIZE; j++) {
			uint8_t Led_Index = Lattice_Power_Up_ReWave_Tab[Lt_Last_ReWave_Count][j];
			if (Led_Index < RGB_MATRIX_LED_COUNT) {
				rgb_matrix_set_color(Led_Index, 0, 0, 0);
			}
		}
	}
	
	// 爱心
	if (Lt_Heart_Flag || Lt_Heart_Flag_Second) {
		for (uint8_t j = 0; j < LT_POWER_UP_HEART_SIZE; j++) {
			uint8_t Led_Index = Lattice_Power_Up_Heart_Tab[LATTICE_Power_Up_Heart_Point][j];
			if(Led_Index < RGB_MATRIX_LED_COUNT) {
			    rgb_matrix_set_color(Led_Index, LATTICE_MAX_BRIGHTNESS, 0, 0);
			}
		}
	}
	// 正向波浪
	if (Lt_Wave_Flag) {
		for (uint8_t j = 0; j < LT_POWER_UP_WAVE_SIZE; j++) {
			uint8_t Led_Index = Lattice_Power_Up_Wave_Tab[LATTICE_Power_Up_Wave_Point][j];
			if(Led_Index < RGB_MATRIX_LED_COUNT) {
			    rgb_matrix_set_color(Led_Index, LATTICE_MAX_BRIGHTNESS, 0, 0);
			}
		}
	}
	// 反向波浪
	if (Lt_ReWave_Flag) {
		for (uint8_t j = 0; j < LT_POWER_UP_WAVE_SIZE; j++) {
			uint8_t Led_Index = Lattice_Power_Up_ReWave_Tab[LATTICE_Power_Up_ReWave_Point][j];
			if(Led_Index < RGB_MATRIX_LED_COUNT) {
			    rgb_matrix_set_color(Led_Index, LATTICE_MAX_BRIGHTNESS, 0, 0);
			}
		}
	}

    if (Lt_Heart_Count_Second == 4) {
        Lt_Heart_Flag_Second = 0;
        // 开机动画播放一次后停止
        Lattice_Init();
        Lattice_Power_Flag = 0;
    }
}

/*********************************
         自定义效果
*********************************/
void Lattice_User_Define_mode_Show(void) {
    for (uint8_t i = 0; i < (LATTICE_LED_GROUP * LATTICE_LED_SIZE); i++) {
        Lattice_Pwm_R = Lattice_User_Mode_Show_Tab[i * 3 + 0];
        Lattice_Pwm_G = Lattice_User_Mode_Show_Tab[i * 3 + 1]; 
        Lattice_Pwm_B = Lattice_User_Mode_Show_Tab[i * 3 + 2];
        Lattice_Pwm_Rgb_Updata(Keyboard_Info.Lattice_Brightness);
        rgb_matrix_set_color(113 + i, Lattice_Pwm_R, Lattice_Pwm_G, Lattice_Pwm_B);
    }
    return;
}

/*********************************
         充电状态指示
*********************************/
void Led_Batt_Number_Show(void) {
    if (es_stdby_pin_state == 1) {                      //充电
        if (Lattice_Led_Count > 50) {
            Lattice_Led_Count = 0;
            if (Lattice_Charge_Point < 5) {
                Lattice_Charge_Point++;
            } else {
                Lattice_Charge_Point = 0;
            }
        }

        for (uint8_t i = 0; i < LATTICE_LED_GROUP; i++) {
            for (uint8_t j = 0; j < LATTICE_LED_SIZE; j++) {
                rgb_matrix_set_color(Battery_Index_Tab[i][j], 0x08, 0x00, 0X00);
            }
        }

        if (Lattice_Charge_Point == 1) {
            for (uint8_t i = 2; i <= 4; i++) {
                rgb_matrix_set_color(Charging_Index_Tab[i][1], 0X00, 0x08, 0X00);
            }
        } else if (Lattice_Charge_Point == 2) {
            for (uint8_t i = 2; i <= 4; i++) {
                rgb_matrix_set_color(Charging_Index_Tab[i][2], 0X00, 0x08, 0X00);
            }
        } else if (Lattice_Charge_Point == 3) {
            for (uint8_t i = 2; i <= 4; i++) {
                rgb_matrix_set_color(Charging_Index_Tab[i][3], 0X00, 0x08, 0X00);
            }
        } else if (Lattice_Charge_Point == 4) {
            for (uint8_t i = 2; i <= 4; i++) {
                rgb_matrix_set_color(Charging_Index_Tab[i][4], 0X00, 0x08, 0X00);
            }
        } else if (Lattice_Charge_Point == 5) {
            for (uint8_t i = 2; i <= 4; i++) {
                rgb_matrix_set_color(Charging_Index_Tab[i][5], 0X00, 0x08, 0X00);
            }
        } else {
            Lattice_Charge_Point = 0;
            for (uint8_t i = 0; i < LATTICE_LED_GROUP; i++) {
                for (uint8_t j = 0; j < LATTICE_LED_SIZE; j++) {
                    rgb_matrix_set_color(Charging_Index_Tab[i][j], 0x00, 0x00, 0X00);
                }
            }
        }
    } else if (es_stdby_pin_state == 2) {               //充满
        for (uint8_t i = 0; i < LATTICE_LED_GROUP; i++) {
            for (uint8_t j = 0; j < LATTICE_LED_SIZE; j++) {
                rgb_matrix_set_color(Battery_Index_Tab[i][j], 0x00, 0x08, 0X00);
            }
        }
        for (uint8_t i = 0; i < LATTICE_LED_GROUP; i++) {
            for (uint8_t j = 0; j < LATTICE_LED_SIZE; j++) {
                rgb_matrix_set_color(Charging_Index_Tab[i][j], 0x00, 0x08, 0X00);
            }
        }
    } else {                                            //未充电
        uint8_t Colour_R = 0, Colour_G = 0, Colour_B = 0;
        uint8_t Temp_Count = (Keyboard_Info.Batt_Number / 20);

        if (Temp_Count <= 1) {                           //红色
            Colour_R = 0XFF;    Colour_G = 0X00;    Colour_B = 0X00;
        } else if (Temp_Count <= 3) {                    //黄色
            Colour_R = 0XFF;    Colour_G = 0XFF;    Colour_B = 0X00;
        } else {                                         //绿色
            Colour_R = 0X00;    Colour_G = 0XFF;    Colour_B = 0X00;
        }

        for (uint8_t i = 0; i < LATTICE_LED_GROUP; i++) {
            for (uint8_t j = 0; j < LATTICE_LED_SIZE; j++) {
                rgb_matrix_set_color(Battery_Index_Tab[i][j], 0x08, 0x00, 0X00);
            }
        }
        
        for (uint8_t i = 2; i < 5; i++) {
            for (uint8_t j = 1; j < (Temp_Count + 1); j++) {
                rgb_matrix_set_color(Charging_Index_Tab[i][j], Colour_R, Colour_G, Colour_B);
            }
        }
    }
}

/*********************************
         低电指示
*********************************/
void Led_Power_Low_Show(void) {
    for (uint8_t i = 0; i < RGB_MATRIX_LED_COUNT; i++) {
        rgb_matrix_set_color(i, 0, 0, 0);
    }
    
    if (Systick_Led_Count < 25) {
        for (uint8_t i = 0; i < LATTICE_LED_GROUP; i++) {
            for (uint8_t j = 0; j < LATTICE_LED_SIZE; j++) {
                rgb_matrix_set_color(Battery_Index_Tab[i][j], U_PWM, 0x00, 0X00);
            }
        }
    } else {
        for (uint8_t i = 0; i < LATTICE_LED_GROUP; i++) {
            for (uint8_t j = 0; j < LATTICE_LED_SIZE; j++) {
                rgb_matrix_set_color(Battery_Index_Tab[i][j], 0x00, 0x00, 0X00);
            }
        }
    }

    if (Systick_Led_Count >= 50) {
        Systick_Led_Count = 0;
    }
}

/*********************************
            关闭
*********************************/
void Lattice_Off_mode_Show(void) {
    for (uint8_t i = 0; i < LATTICE_LED_GROUP; i++) {
        for (uint8_t j = 0; j < LATTICE_LED_SIZE; j++) {
            rgb_matrix_set_color(Lattice_Index_Tab[i][j], 0X00, 0X00, 0X00);
        }
    }
}

void Lattice_Mode_Show(void) {
    if (Lattice_Power_Flag) {
        Lt_Power_Up_Show();
    } else if (Keyboard_Info.Lattice_On_Off) {
        Lattice_Off_mode_Show();
    } else { 
        switch (Keyboard_Info.Lattice_Mode) { 
            case LATTICE_HEART_MODE:       Lattice_Heart_mode_Show();      break;
            case LATTICE_SCAN_MODE:        Lattice_Scan_mode_Show();       break;
            case LATTICE_TYPE_MODE:        Lattice_Type_mode_Show();       break;
            case LATTICE_RAIN_MODE:        Lattice_Rain_mode_Show();       break;
            case LATTICE_TER_MODE:         Lattice_Terminal_mode_Show();   break;
            case LATTICE_USER_MODE:        Lattice_User_Define_mode_Show();break;
            case LATTICE_CIRCLE_MODE:      Lattice_Circle_mode_Show();     break;
            default:                       Lattice_Off_mode_Show();        break;
        }
    }
}

void User_Via_Qmk_Lattice_Get_Value(uint8_t *data) {
    // data = [ value_id, value_data ]
    uint8_t *value_id   = &(data[0]);
    uint8_t *value_data = &(data[1]);

    switch (*value_id) {
        case id_qmk_rgb_matrix_brightness: {
            value_data[0] = Keyboard_Info.Lattice_Brightness;
        } break;
        case id_qmk_rgb_matrix_effect: {
            value_data[0] = Keyboard_Info.Lattice_Mode;
        } break;
        case id_qmk_rgb_matrix_effect_speed: {
            value_data[0] = Keyboard_Info.Lattice_Speed;
        } break;
        case id_qmk_rgb_matrix_color: {
            value_data[0] = Keyboard_Info.Lattice_Colour;
            value_data[0] = Keyboard_Info.Lattice_Colour_Blue;
            value_data[1] = (255 - Keyboard_Info.Lattice_Saturation);
        } break;
        case id_qmk_rgb_signal_user_define: {
            value_data[1] = Lattice_User_Mode_Show_Tab[value_data[0] * 3 + 0];
            value_data[2] = Lattice_User_Mode_Show_Tab[value_data[0] * 3 + 1];
            value_data[3] = Lattice_User_Mode_Show_Tab[value_data[0] * 3 + 2];
        } break;
        case id_qmk_rgb_all_user_define: {
            for (uint8_t i = 0; i < (value_data[0] / 2); i++) {
                value_data[2] = Lattice_User_Mode_Show_Tab[i * 3 + 0]; // 0 : 偏移地址 1 : 有效长度 2 : Data R
                value_data[3] = Lattice_User_Mode_Show_Tab[i * 3 + 1]; // 0 : 偏移地址 1 : 有效长度 2 : Data R
                value_data[4] = Lattice_User_Mode_Show_Tab[i * 3 + 2]; // 0 : 偏移地址 1 : 有效长度 2 : Data R
            } 
        } break;
        case id_qmk_rgb_reset_user_define: {             // 设置自定义复位模式
            for (uint8_t i = 0; i < (LATTICE_LED_GROUP * LATTICE_LED_SIZE); i++) {
                value_data[0] = Lattice_User_Mode_Show_Tab[i * 3 + 0];
                value_data[1] = Lattice_User_Mode_Show_Tab[i * 3 + 1];
                value_data[2] = Lattice_User_Mode_Show_Tab[i * 3 + 2];
            } 
            Lattice_Init();
            Keyboard_Info.Lattice_Mode = INIT_LATTICE_MODE;
        } break;
        case id_qmk_rgb_set_matrix_info: {               // 设定点阵屏矩阵信息
            value_data[0] = LATTICE_LED_GROUP;           // 点阵屏行数
            value_data[1] = LATTICE_LED_SIZE;            // 点阵屏列数
            value_data[2] = LATTICE_LIGHT_MODE_COUNT;    // 点阵屏灯光模式总数
            value_data[3] = LATTICE_MAX_BRIGHTNESS;      // 点阵屏最大亮度
            value_data[4] = LATTICE_MAX_SPEED;           // 点阵屏最大速度
            value_data[5] = INIT_LATTICE_MODE;           // 点阵屏默认模式
            value_data[6] = INIT_LATTICE_BRIGHTNESS;     // 点阵屏默认亮度
            value_data[7] = INIT_LATTICE_SPEED;          // 点阵屏默认速度
        } break;
        case id_qmk_rgb_get_matrix_info: {               // 读取点阵屏矩阵RGB信息
            for (uint8_t i = 0; i < (value_data[0] / 2); i++) {
                value_data[2] = Lattice_User_Mode_Show_Tab[i * 3 + 0];
                value_data[3] = Lattice_User_Mode_Show_Tab[i * 3 + 1];
                value_data[4] = Lattice_User_Mode_Show_Tab[i * 3 + 2];
            }
        } break;
        case id_qmk_rgb_set_matrix_color: {              // 在除自定义外的模式设置灯光颜色
            for (uint8_t i = 0; i < (value_data[0] / 2); i++) {
                value_data[2] = rgb_matrix_get_mode();
                value_data[3] = Lattice_User_Mode_Show_Tab[i * 3 + 0];
                value_data[4] = Lattice_User_Mode_Show_Tab[i * 3 + 1];
                value_data[5] = Lattice_User_Mode_Show_Tab[i * 3 + 2];
            }
        } break;
    }
}

void User_Via_Qmk_Lattice_Set_Value(uint8_t *data) {
    // data = [ value_id, value_data ]
    uint8_t *value_id   = &(data[0]);
    uint8_t *value_data = &(data[1]);
    switch (*value_id) {
        case id_qmk_rgb_matrix_brightness: {    //设置亮度 0 ~ 255
            if (value_data[0] >= LATTICE_MAX_BRIGHTNESS) {
                Keyboard_Info.Lattice_Brightness = LATTICE_MAX_BRIGHTNESS;
            } else {
                Keyboard_Info.Lattice_Brightness = value_data[0];
            }
        } break;
        case id_qmk_rgb_matrix_effect: {        //设置灯光模式
            if (value_data[0] == 0) {
                Keyboard_Info.Lattice_On_Off = LATTICE_LED_OFF;
            } else {
                Keyboard_Info.Lattice_On_Off = LATTICE_LED_ON;
                if (value_data[0] <= LATTICE_OFF_MODE) {
                    Keyboard_Info.Lattice_Mode = value_data[0];
                } else {
                    Keyboard_Info.Lattice_Mode = INIT_LATTICE_MODE;
                }
            }
            Lattice_Init();
        } break;
        case id_qmk_rgb_matrix_effect_speed: {  //设置灯光速度
            if (value_data[0] >= LATTICE_MAX_SPEED) {
                Keyboard_Info.Lattice_Speed = LATTICE_MAX_SPEED;
            } else {
                Keyboard_Info.Lattice_Speed = value_data[0];
            }
        } break;
        case id_qmk_rgb_matrix_color: {         //设置颜色和饱和度
            Keyboard_Info.Lattice_Colour = value_data[0];
            Keyboard_Info.Lattice_Colour_Blue = value_data[0];
            Keyboard_Info.Lattice_Saturation = (255 - value_data[1]); 
        } break;
        case id_qmk_rgb_signal_user_define: {     // 设置自定义单点亮模式
            Lattice_User_Mode_Show_Tab[value_data[0] * 3 + 0] = value_data[1];
            Lattice_User_Mode_Show_Tab[value_data[0] * 3 + 1] = value_data[2];
            Lattice_User_Mode_Show_Tab[value_data[0] * 3 + 2] = value_data[3]; 
        } break;
        case id_qmk_rgb_all_user_define: {        // 设置自定义全点亮模式
            for (uint8_t i = 0; i < (value_data[0] / 2); i++) {
                Lattice_User_Mode_Show_Tab[i * 3 + 0] = value_data[2]; // 0 : 偏移地址 1 : 有效长度 2 : Data R
                Lattice_User_Mode_Show_Tab[i * 3 + 1] = value_data[3]; // 0 : 偏移地址 1 : 有效长度 2 : Data R
                Lattice_User_Mode_Show_Tab[i * 3 + 2] = value_data[4]; // 0 : 偏移地址 1 : 有效长度 2 : Data R
            } 
        } break;
        case id_qmk_rgb_reset_user_define: {        // 设置自定义复位模式
            for (uint8_t i = 0; i < (LATTICE_LED_GROUP * LATTICE_LED_SIZE); i++) {
                Lattice_User_Mode_Show_Tab[i * 3 + 0] = value_data[0];
                Lattice_User_Mode_Show_Tab[i * 3 + 1] = value_data[1];
                Lattice_User_Mode_Show_Tab[i * 3 + 2] = value_data[2];
            }  
            Lattice_Init();
            Keyboard_Info.Lattice_Mode = INIT_LATTICE_MODE;
        } break;
        case id_qmk_rgb_set_matrix_info: {               // 设定点阵屏矩阵信息
            value_data[0] = LATTICE_LED_GROUP;           // 点阵屏行数
            value_data[1] = LATTICE_LED_SIZE;            // 点阵屏列数
            value_data[2] = LATTICE_LIGHT_MODE_COUNT;    // 点阵屏灯光模式总数
            value_data[3] = LATTICE_MAX_BRIGHTNESS;      // 点阵屏最大亮度
            value_data[4] = LATTICE_MAX_SPEED;           // 点阵屏最大速度
            value_data[5] = INIT_LATTICE_MODE;           // 点阵屏默认模式
            value_data[6] = INIT_LATTICE_BRIGHTNESS;     // 点阵屏默认亮度
            value_data[7] = INIT_LATTICE_SPEED;          // 点阵屏默认速度
        } break;
        case id_qmk_rgb_get_matrix_info: {              // 读取点阵屏矩阵RGB信息
            for (uint8_t i = 0; i < (value_data[0] / 2); i++) {
                Lattice_User_Mode_Show_Tab[i * 3 + 0] = value_data[2];
                Lattice_User_Mode_Show_Tab[i * 3 + 1] = value_data[3];
                Lattice_User_Mode_Show_Tab[i * 3 + 2] = value_data[4];
            }
        } break;
        case id_qmk_rgb_set_matrix_color: {              // 在除自定义外的模式设置灯光颜色
            for (uint8_t i = 0; i < (value_data[0] / 2); i++) {
                Keyboard_Info.Lattice_Mode            = value_data[2];
                Lattice_User_Mode_Show_Tab[i * 3 + 0] = value_data[3];
                Lattice_User_Mode_Show_Tab[i * 3 + 1] = value_data[4];
                Lattice_User_Mode_Show_Tab[i * 3 + 2] = value_data[5];
            }
        } break;
    }
}

void User_Via_Qmk_Lattice_Command(uint8_t *data, uint8_t length) {
    // data = [ command_id, channel_id, value_id, value_data ]
    uint8_t *command_id        = &(data[0]);
    uint8_t *value_id_and_data = &(data[2]);

    switch (*command_id) {
        case id_custom_set_value: {
            User_Via_Qmk_Lattice_Set_Value(value_id_and_data);
            break;
        }
        case id_custom_get_value: {
            User_Via_Qmk_Lattice_Get_Value(value_id_and_data);
            break;
        }
        case id_custom_save: {
            Save_Flash_Set();
            break;
        }
        default: {
            *command_id = id_unhandled;
            break;
        }
    }
}

#endif

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

led_config_t g_led_config = { {
    { 0      , 1      , 2      , 3      , 4      , 5      , 6      , 7      , 8      , 9      , 10     , 11     , 12     , NO_LED , NO_LED , NO_LED   },
    { 13     , 14     , 15     , 16     , 17     , 18     , 19     , 20     , 21     , 22     , 23     , 24     , 25     , 26     , NO_LED , NO_LED   },
    { 29     , 30     , 31     , 32     , 33     , 34     , 35     , 36     , 37     , 38     , 39     , 40     , 41     , 27     , NO_LED , 28       },
    { 43     , 44     , 45     , 46     , 47     , 48     , 49     , 50     , 51     , 52     , 53     , 54     , 55     , 56     , 42     , 57       },
    { 58     , NO_LED , 59     , 60     , 61     , 62     , 63     , 64     , 65     , 66     , 67     , 68     , 69     , 70     , 71     , 72       },
    { 73     , 74     , 75     , 76     , NO_LED , 77     , NO_LED , NO_LED , NO_LED , 78     , 79     , 80     , 81     , 82     , 83     , 84       }
},{
    { 0,  10},  { 17, 10}, { 32, 10}, { 47, 10}, { 62, 10}, { 79, 10}, { 94, 10}, {109, 10}, {124, 10}, { 141, 10}, { 156, 10}, { 171, 10}, { 186, 10}, 
    { 0,  20},  { 15, 20}, { 30, 20}, { 45, 20}, { 60, 20}, { 75, 20}, { 90, 20}, {105, 20}, {120, 20}, { 135, 20}, { 150, 20}, { 165, 20}, { 180, 20}, { 195, 20}, { 210, 20}, { 224, 20},            
    { 4,  30},  { 20, 30}, { 35, 30}, { 50, 30}, { 65, 30}, { 80, 30}, { 95, 30}, {110, 30}, {125, 30}, { 140, 30}, { 155, 30}, { 170, 30}, { 185, 30},                         { 224, 30}, 
    { 6,  40},  { 24, 40}, { 39, 40}, { 54, 40}, { 69, 40}, { 84, 40}, { 99, 40}, {114, 40}, {129, 40}, { 144, 40}, { 159, 40}, { 174, 40}, { 189, 40},             { 205, 40}, { 224, 40},
    { 8,  50},             { 28, 50}, { 43, 50}, { 58, 50}, { 73, 50}, { 88, 50}, {103, 50}, {118, 50}, { 133, 50}, { 148, 50}, { 163, 50}, { 178, 50}, { 193, 50}, { 208, 50}, { 224, 50},
    { 0,  60},  { 15, 60}, { 30, 60}, { 45, 60},            { 85, 60},                       {120, 60}, { 135, 60}, { 150, 60}, { 165, 60},             { 193, 60}, { 208, 60}, { 224, 60}, 

    // 侧灯
    { 225, 65}, { 225, 65}, { 225, 65}, { 225, 65}, { 225, 65}, { 225, 65}, { 225, 65}, { 225, 65}, { 225, 65}, { 225, 65}, { 225, 65},
    { 225, 65}, { 225, 65}, { 225, 65}, { 225, 65}, { 225, 65}, { 225, 65}, { 225, 65}, { 225, 65}, { 225, 65}, { 225, 65}, { 225, 65},
    { 225, 65}, { 225, 65}, { 225, 65}, { 225, 65}, { 225, 65}, { 225, 65}, { 225, 65}, { 225, 65}, { 225, 65}, { 225, 65}, { 225, 65},

    // 点阵
    { 225, 65}, { 225, 65}, { 225, 65}, { 225, 65}, { 225, 65}, { 225, 65}, { 225, 65},
    { 225, 65}, { 225, 65}, { 225, 65}, { 225, 65}, { 225, 65}, { 225, 65}, { 225, 65},
    { 225, 65}, { 225, 65}, { 225, 65}, { 225, 65}, { 225, 65}, { 225, 65}, { 225, 65},
    { 225, 65}, { 225, 65}, { 225, 65}, { 225, 65}, { 225, 65}, { 225, 65}, { 225, 65},
    { 225, 65}, { 225, 65}, { 225, 65}, { 225, 65}, { 225, 65}, { 225, 65}, { 225, 65},
    { 225, 65}, { 225, 65}, { 225, 65}, { 225, 65}, { 225, 65}, { 225, 65}, { 225, 65},
    { 225, 65}, { 225, 65}, { 225, 65}, { 225, 65}, { 225, 65}, { 225, 65}, { 225, 65}
}, {
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,      
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,          1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,      1,  1,
    1,      1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
    1,  1,  1,  1,      1,          1,  1,  1,  1,      1,  1,  1, 

    // 侧灯
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,

    // 点阵
    0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0
} };

void Led_Rf_Mode_Show(void) {
    uint8_t Temp_Colour = 0,Led_Index = 0;
    if (Keyboard_Info.Key_Mode == QMK_BLE_MODE) {
        if (Keyboard_Info.Ble_Channel == QMK_BLE_CHANNEL_1) {
            Temp_Colour = 5;
            Led_Index = LED_BLE_1_INDEX;
        } else if (Keyboard_Info.Ble_Channel == QMK_BLE_CHANNEL_2) {
            Temp_Colour = 5;
            Led_Index = LED_BLE_2_INDEX;
        } else if (Keyboard_Info.Ble_Channel == QMK_BLE_CHANNEL_3) {
            Temp_Colour = 5;
            Led_Index = LED_BLE_3_INDEX;
        }
    } else if (Keyboard_Info.Key_Mode == QMK_2P4G_MODE) {
        Temp_Colour = 3;
        Led_Index = LED_2P4G_INDEX;
    }
    
    for (uint8_t i = 0; i < RGB_MATRIX_LED_COUNT; i++) {
        rgb_matrix_set_color(i, 0, 0, 0);
    }

    if (Keyboard_Status.System_Connect_Status == KB_MODE_CONNECT_PAIR) {
        if (Systick_Led_Count < 10) {
            rgb_matrix_set_color(Led_Index, Led_Colour_Tab[Temp_Colour][0], Led_Colour_Tab[Temp_Colour][1], Led_Colour_Tab[Temp_Colour][2]);
        } else {
            rgb_matrix_set_color(Led_Index, 0, 0, 0);
        }

        if (Systick_Led_Count >= 20) {
            Systick_Led_Count = 0;
        }
    } else if (Keyboard_Status.System_Connect_Status == KB_MODE_CONNECT_RETURN) {
        if (Systick_Led_Count < 25) {
            rgb_matrix_set_color(Led_Index, Led_Colour_Tab[Temp_Colour][0], Led_Colour_Tab[Temp_Colour][1], Led_Colour_Tab[Temp_Colour][2]);
        } else {
            rgb_matrix_set_color(Led_Index, 0, 0, 0);
        }

        if (Systick_Led_Count >= 50) {
            Systick_Led_Count = 0;
        }
    } else {
        rgb_matrix_set_color(Led_Index, Led_Colour_Tab[Temp_Colour][0], Led_Colour_Tab[Temp_Colour][1], Led_Colour_Tab[Temp_Colour][2]);

        if (Systick_Led_Count >= 240) {
            Systick_Led_Count = 0;
            Led_Rf_Pair_Flg = false;
            if (Keyboard_Info.Key_Mode == QMK_BLE_MODE) {
                User_Batt_Send_Spi = true;
            }
        }
    }
}

void Led_Point_Flash_Show(void) {
    if (Systick_Led_Count < 25) {
        if (Led_Point_Count) {
            for(uint8_t i = 0; i < 2; i++){
                rgb_matrix_set_color(Led_Point_buf[i], U_PWM, U_PWM, U_PWM);
            }
        } else if(Mac_Win_Point_Count){
            for(uint8_t i = 0; i < 2; i++){
                rgb_matrix_set_color(Led_Point_buf[i], 0x00, U_PWM, 0X00);
            }
        } else if (Debounce_Point_Count) {
            if (!Debounce_Function_Count) {
                rgb_matrix_set_color(LED_DEB_INDEX, U_PWM, 0X00, 0X00);
            } else {
                rgb_matrix_set_color(LED_DEB_INDEX, 0x00, U_PWM, 0X00);
            }
        } else {
            switch(Keyboard_Info.User_Sleep_Time){
                case SLEEP_TIME_ONE:
                    rgb_matrix_set_color(LED_SLEEP_ST_INDEX, U_PWM, 0X00, 0X00);
                    break;
                case SLEEP_TIME_TWO:
                    rgb_matrix_set_color(LED_SLEEP_ST_INDEX, 0x00, U_PWM, 0X00);
                    break;
                case SLEEP_TIME_THREE:
                    rgb_matrix_set_color(LED_SLEEP_ST_INDEX, 0x00, 0x00, U_PWM);
                    break;
                case SLEEP_TIME_FOUR:
                    rgb_matrix_set_color(LED_SLEEP_ST_INDEX, U_PWM, U_PWM, U_PWM);
                    break;
            }
        }
    } else {
        if(Led_Point_Count || Mac_Win_Point_Count){
            for(uint8_t i = 0; i < 2; i++){
                rgb_matrix_set_color(Led_Point_buf[i], 0X00, 0X00, 0X00);
            }
        } else if (Debounce_Point_Count) {
            rgb_matrix_set_color(LED_DEB_INDEX, 0X00, 0X00, 0X00);
        } else {
            rgb_matrix_set_color(LED_SLEEP_ST_INDEX, 0x00, 0x00, 0x00);
        }
    }

    if (Systick_Led_Count >= 50) {
        Systick_Led_Count = 0;

        if (Led_Point_Count) {
            Led_Point_Count--;
        } else if (Mac_Win_Point_Count) {
            Mac_Win_Point_Count--;
        } else if (Debounce_Point_Count) {
            Debounce_Point_Count--;
        } else {
            User_Sleep_Timer_Count--;
        }
    }
}

void User_Test_Colour_Show(void){
    uint8_t Test_R = 0, Test_G = 0, Test_B = 0;
    switch(Test_Colour) {
        case 0:  Test_R = RGB_MATRIX_MAXIMUM_BRIGHTNESS; Test_G = 0;                             Test_B = 0;                             break;
        case 1:  Test_R = 0;                             Test_G = RGB_MATRIX_MAXIMUM_BRIGHTNESS; Test_B = 0;                             break;
        case 2:  Test_R = 0;                             Test_G = 0;                             Test_B = RGB_MATRIX_MAXIMUM_BRIGHTNESS; break;
        case 3:  Test_R = 80;                            Test_G = 80;                            Test_B = 80;                            break;
        default: Test_R = 80;                            Test_G = 80;                            Test_B = 80;                            break;
    }
    rgb_matrix_driver_set_color_all(Test_R, Test_G, Test_B);
}

void User_Get_Led_Power_Status(void) {
#if (LOGO_LED_ENABLE && SIDE_LED_ENABLE)
    if ( ((Keyboard_Info.Led_On_Off == INIT_LED_OFF)  || (rgb_matrix_get_val() <= 0)) &&
        ((Keyboard_Info.Logo_On_Off == LOGO_LED_OFF) || (Keyboard_Info.Logo_Brightness <= 0) || (Keyboard_Info.Logo_Mode == LOGO_OFF_MODE)) &&
        ((Keyboard_Info.Side_On_Off == SIDE_LED_OFF) || ((Keyboard_Info.Side_Brightness <= 0)) || (Keyboard_Info.Side_Mode == SIDE_OFF_MODE) )) {
        Led_Point_Sleep = true;
    } else {
        Led_Point_Sleep = false;
    }
#elif (LOGO_LED_ENABLE)
    if ( ((Keyboard_Info.Led_On_Off == INIT_LED_OFF)  || (rgb_matrix_get_val() <= 0)) &&
        ((Keyboard_Info.Logo_On_Off == LOGO_LED_OFF) || (Keyboard_Info.Logo_Brightness <= 0) || (Keyboard_Info.Logo_Mode == LOGO_OFF_MODE)) ) {
        Led_Point_Sleep = true;
    } else {
        Led_Point_Sleep = false;
    }
#else
    if ((Keyboard_Info.Led_On_Off == INIT_LED_OFF) || (rgb_matrix_get_val() <= 0)) {
        Led_Point_Sleep = true;
    } else {
        Led_Point_Sleep = false;
    }
#endif

    if (Keyboard_Info.Led_On_Off == INIT_LED_OFF) {
        for (uint8_t i = 0; i < RGB_MATRIX_LED_COUNT; i++) {
                rgb_matrix_set_color(i, 0, 0, 0);
        }
    }
}

bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    User_Get_Led_Power_Status();
    
#if LATTICE_LED_ENABLE
    if ((!User_Power_Low) && (!Test_Led) && (!User_Key_Batt_Num_Show) && (!Led_Rf_Pair_Flg)) {
        Lattice_Mode_Show();
    }
#endif

#if LOGO_LED_ENABLE
    if ((!User_Power_Low) && (!User_Key_Batt_Num_Show) && (!Test_Led) && (!Led_Rf_Pair_Flg)) {
        Logo_Mode_Show();
    }
#endif

    if (User_Power_Low) {
        Led_Point_Sleep = false;
        Led_Power_Low_Show();
    } else if (Led_Rf_Pair_Flg && (Keyboard_Info.Key_Mode != QMK_USB_MODE)) {
        Led_Point_Sleep = false;
        Led_Rf_Mode_Show();
    } else if (Test_Led) {
        Led_Point_Sleep = false;
        User_Test_Colour_Show();
    } else if (Led_Point_Count || Mac_Win_Point_Count || INIT_ALL_SIX_KEY_Count || Debounce_Point_Count || User_Sleep_Timer_Count) {
        Led_Point_Sleep = false;
        Led_Point_Flash_Show();
    } 
    // else if (Usb_If_Ok && (Keyboard_Info.Key_Mode == QMK_USB_MODE)) {
    //     Led_Point_Sleep = false;
    //     rgb_matrix_set_color(LED_USB_INDEX, 0x00, U_PWM, U_PWM);
    // } 
    else {
        Systick_Led_Count = 0;

        if (Keyboard_Info.Key_Mode == QMK_USB_MODE) {
            if (host_keyboard_led_state().caps_lock && Usb_If_Ok_Led) {
                Led_Point_Sleep = false;
                rgb_matrix_set_color(LED_CAP_INDEX, U_PWM, U_PWM, U_PWM);
            }
        } else {
            if (Keyboard_Status.System_Led_Status & 0x02) {
                Led_Point_Sleep = false;
                rgb_matrix_set_color(LED_CAP_INDEX, U_PWM, U_PWM, U_PWM);
            }
        }

        if (Keyboard_Info.Win_Lock) {
            Led_Point_Sleep = false;
            rgb_matrix_set_color(LED_WIN_L_INDEX, U_PWM, U_PWM, U_PWM);
        }

        if (Key_Fn_Status) {                            //FN 按下模式指示
            switch (Keyboard_Info.Key_Mode) {
                case QMK_BLE_MODE: {
                    switch (Keyboard_Info.Ble_Channel) {
                        case QMK_BLE_CHANNEL_1: rgb_matrix_set_color(LED_BLE_1_INDEX, U_PWM, U_PWM, U_PWM); break;
                        case QMK_BLE_CHANNEL_2: rgb_matrix_set_color(LED_BLE_2_INDEX, U_PWM, U_PWM, U_PWM); break;
                        case QMK_BLE_CHANNEL_3: rgb_matrix_set_color(LED_BLE_3_INDEX, U_PWM, U_PWM, U_PWM); break;
                        default:                                                                            break;
                    }
                } break;
                case QMK_2P4G_MODE:             rgb_matrix_set_color(LED_2P4G_INDEX, U_PWM, U_PWM, U_PWM);  break;
                // case QMK_USB_MODE:              rgb_matrix_set_color(LED_USB_INDEX,  U_PWM, U_PWM, U_PWM);  break;
                default:                                                                                    break;
            }
            Led_Point_Sleep = false;
        }
    }
    if (User_Key_Batt_Num_Show && (!User_Power_Low)) {
        Led_Point_Sleep = false;
        Led_Batt_Number_Show();
    }

#if SIDE_LED_ENABLE
    if ((!User_Power_Low) && (!User_Key_Batt_Num_Show)) {
        Side_Mode_Show();
    }
#endif
    return false;
}
/*********************************************************/

/************************ADC******************************/
const md_adc_initial adc_initStruct =    /**< ADC init structure */
{
    MD_ADC_CFG_ALIGN_RIGHT,     //Data alignment（数据对齐）
    MD_ADC_CFG_RSEL_12BIT,      //Data resolution（数据分辨率）
    MD_ADC_MODE_NCHS,           //Regular or Injected
    MD_ADC_CFG_CM_SINGLE,       //Single mode
    MD_ADC_NCHS1_NSL_1CON,      /*sample count  采样通道数量*/
    MD_ADC_SMPT1_CKDIV_DIV6,    //ADC prescale（ADC预分频）
};

void User_Adc_Init(void) {  //ES_BATT_ADC_IO
    md_gpio_inittypedef gpiox;

    gpiox.OutputType = MD_GPIO_OUTPUT_PUSHPULL;
    gpiox.Pull = MD_GPIO_PULL_FLOATING;
    gpiox.OutDrive = MD_GPIO_DRIVING_8MA;
    gpiox.Function = MD_GPIO_AF0;
    gpiox.Mode = MD_GPIO_MODE_ANALOG;
    gpiox.Pin = MD_GPIO_PIN_4;
    md_gpio_init(GPIOC, &gpiox);

    md_rcu_enable_adc(RCU);//使能ADC同步制动
    md_adc_calibration(ADC, (md_adc_initial *)(&adc_initStruct));//ADC校准
    md_adc_set_sampletime_channel_14(ADC, 0x40);            //设置ADC通道

    md_adc_init(ADC, (md_adc_initial *)(&adc_initStruct));

    while ((ADC->RIF & 0x1) == 0);

    md_adc_set_normal_sequence_length(ADC, adc_initStruct.Cnt);//采样次数
    md_adc_set_normal_sequence_selection_1th(ADC, MD_ADC_NCHS1_NS1_CH14);

    md_adc_set_start_normal(ADC, MD_ADC_CON_NSTART_START_REGULAR);//开始ADC采样
}

void User_Adc_Deinit(void) {
    md_rcu_enable_adc_reset(RCU);
    md_rcu_disable_adc_reset(RCU);
    md_rcu_disable_adc(RCU);
}
/*********************************************************/

/************************USB 插件**************************/
void User_Usb_Init(void) {  //中断频率2ms
    /*Using USB_SOF to calibrate the internal clock*/
    md_rcu_enable_csu(RCU);
    CSU->CON |= CSU_CON_AUTOEN_MSK;
    CSU->CON |= CSU_CON_CNTEN_MSK;
}

void es_restart_usb_driver(void) {
    md_rcu_enable_usb(RCU);
    ald_usb_device_components_init();
    USB->TXIER = 0x7F;
    USB->RXIER = 0x7E;
    USB->IER = 0x2F;
    usb_lld_connect_bus(0);
    ald_usb_int_register();
}

void Usb_Disconnect(void) {
    /*USB 复位*/
    ald_usb_int_unregister();
    usb_lld_disconnect_bus(0);

    md_rcu_enable_usb_reset(RCU);
    md_rcu_disable_usb_reset(RCU);
    md_rcu_disable_usb(RCU);
}

void User_Usb_Deinit(void) {
    md_rcu_enable_csu_reset(RCU);
    md_rcu_disable_csu_reset(RCU);
    md_rcu_disable_csu(RCU);
}
/*********************************************************/

void notify_usb_device_state_change_user(enum usb_device_state usb_device_state)  {
    if (Keyboard_Info.Key_Mode == QMK_USB_MODE) {
        if(usb_device_state == USB_DEVICE_STATE_CONFIGURED) {
            Usb_If_Ok = true;//usb枚举完成
            Usb_If_Ok_Led = true;
            Usb_If_Ok_Delay = 0;
        } else {
            Usb_If_Ok = false;
            Usb_If_Ok_Led = false;
        }
    } else {
        Usb_If_Ok = false;
        Usb_If_Ok_Led = false;
    }
}

/*********************************************************/

/**********************系统函数***************************/
/*  键盘扫描按键延时 */
void matrix_io_delay(void) {
}

void matrix_output_select_delay(void) {
}

void matrix_output_unselect_delay(uint8_t line, bool key_pressed) {
}

/*拨动开关*/
void Key_Switch_Mode_Scan(void) {
    Key_Switch_Scan = 0x00;
    if ((gpio_read_pin(MODE_BLE_IO)) && (!gpio_read_pin(MODE_2P4G_IO))) {
        Key_Switch_Scan = 0x01;
    } else if ((!gpio_read_pin(MODE_BLE_IO)) && (gpio_read_pin(MODE_2P4G_IO))) {
        Key_Switch_Scan = 0x02;
    } else if ((gpio_read_pin(MODE_BLE_IO)) && (gpio_read_pin(MODE_2P4G_IO))) {
        Key_Switch_Scan = 0x04;
    } else {
        Key_Switch_Scan = 0x04;
    }

    if (Key_Switch_Scan != Key_Switch_Old) {
        if (Key_Switch_Scan != Key_Switch_Check) {
            Key_Switch_delay = 4;
            Key_Switch_Check = Key_Switch_Scan;
        } else {
            if (Key_Switch_delay) {
                Key_Switch_delay--;
            } else {
                Key_Switch_Old = Key_Switch_Scan;

                Usb_Change_Mode_Delay = 0;
                Usb_Change_Mode_Wakeup = false;

                if (Key_Switch_Check == 0x01) {         //2.4G
                    if (Keyboard_Info.Key_Mode != QMK_2P4G_MODE) {
                        Usb_Disconnect();
                        Keyboard_Info.Key_Mode = QMK_2P4G_MODE;
                        Spi_Send_Commad(USER_SWITCH_2P4G_MODE);
                        Save_Flash_Set();
                        Led_Rf_Pair_Flg = true;
                    }
                } else if (Key_Switch_Check == 0x02) {  //BLE
                    if (Keyboard_Info.Key_Mode != QMK_BLE_MODE) {
                        Usb_Disconnect();
                        Keyboard_Info.Key_Mode = QMK_BLE_MODE;
                        switch (Keyboard_Info.Ble_Channel) {
                            case QMK_BLE_CHANNEL_1: Spi_Send_Commad(USER_SWITCH_BLE_1_MODE); break;
                            case QMK_BLE_CHANNEL_2: Spi_Send_Commad(USER_SWITCH_BLE_2_MODE); break;
                            case QMK_BLE_CHANNEL_3: Spi_Send_Commad(USER_SWITCH_BLE_3_MODE); break;
                            default:                                                         break;
                        }
                        Save_Flash_Set();
                        Led_Rf_Pair_Flg = true;
                    }
                } else {                                //USB
                    if (Keyboard_Info.Key_Mode != QMK_USB_MODE) {
                        Keyboard_Info.Key_Mode = QMK_USB_MODE;
                        Usb_If_Ok = true;
                        Spi_Send_Commad(USER_SWITCH_USB_MODE);
                        es_restart_usb_driver();
                        Save_Flash_Set();
                        Led_Rf_Pair_Flg = false;
                    }
                }
                
                #if LATTICE_LED_ENABLE
                    if (Lattice_Power_Flag) {
                        Lattice_Power_Flag = 0;
                    }
                    Lattice_Init();
                    Lattice_Power_Flag = 1;
                    Lt_Heart_Flag = 1;
                #endif
            }
        }
    }
}

void housekeeping_task_user(void) {
    User_Keyboard_Reset();

    if (Scan_Switch_Ok) {
        Scan_Switch_Ok = false;
        Key_Switch_Mode_Scan(); // 拨动开关轮询
    }

    if(User_EE_CLR_Start_Flag){                 //长按3s重启按键
        User_EE_CLR_Start_Flag = false;

        Keyboard_Info.Led_On_Off = INIT_LED_ON_OFF;
        Keyboard_Info.Nkro = INIT_ALL_KEY;
        Keyboard_Info.Mac_Win_Mode = INIT_WIN_MODE;
        Keyboard_Info.Win_Lock = INIT_WIN_NLOCK;
        Keyboard_Info.Debounce_Delay = DEBOUNCE_DELAY_CLASS;
        Keyboard_Info.User_Sleep_Time = SLEEP_TIME_CLASS;
        Keyboard_Info.User_DSleep_Time = USER_DSLEEP_TIME; //复位需不需要初始化无线休眠时间根据实际情况
    #if LOGO_LED_ENABLE
        Keyboard_Info.Logo_On_Off = INIT_LOGO_ON_OFF;
        Keyboard_Info.Logo_Mode = INIT_LOGO_MODE;
        Keyboard_Info.Logo_Colour = INIT_LOGO_COLOUR;
        Keyboard_Info.Logo_Saturation = INIT_LOGO_SATURATION;
        Keyboard_Info.Logo_Brightness = INIT_LOGO_BRIGHTNESS;
        Keyboard_Info.Logo_Speed = INIT_LOGO_SPEED;
        Logo_Init();
    #endif
    #if SIDE_LED_ENABLE
        Keyboard_Info.Side_On_Off = INIT_SIDE_ON_OFF;
        Keyboard_Info.Side_Mode = INIT_SIDE_MODE;
        Keyboard_Info.Side_Colour = INIT_SIDE_COLOUR;
        Keyboard_Info.Side_Colour_Blue = INIT_SIDE_COLOUR_BLUE;
        Keyboard_Info.Side_Saturation = INIT_SIDE_SATURATION;
        Keyboard_Info.Side_Brightness = INIT_SIDE_BRIGHTNESS;
        Keyboard_Info.Side_Speed = INIT_SIDE_SPEED;
        Side_Init();
    #endif
    #if LATTICE_LED_ENABLE
        Keyboard_Info.Lattice_On_Off = INIT_LATTICE_ON_OFF;
        Keyboard_Info.Lattice_Mode = INIT_LATTICE_MODE;
        Keyboard_Info.Lattice_Colour = INIT_LATTICE_COLOUR;
        Keyboard_Info.Lattice_Colour_Blue = INIT_LATTICE_COLOUR_BLUE;
        Keyboard_Info.Lattice_Saturation = INIT_LATTICE_SATURATION;
        Keyboard_Info.Lattice_Brightness = INIT_LATTICE_BRIGHTNESS;
        Keyboard_Info.Lattice_Speed = INIT_LATTICE_SPEED;
        Lattice_Init();
    #endif
        Reset_Save_Flash = true;
        /*将当前模式写入flash*/
        eeprom_write_block_user((void *)&Keyboard_Info.Key_Mode, 0, sizeof(Keyboard_Info_t));
        Reset_Save_Flash = false;

        Debounce_Delay = Keyboard_Info.Debounce_Delay;
        if (Debounce_Delay > DEBOUNCE_DELAY_ONE) {
            Debounce_Function_Count = 0;
        } else {
            Debounce_Function_Count = 1;
        }

    #ifdef NO_RESET
        eeconfig_init();
    #else
        eeconfig_disable();
        soft_reset_keyboard();
    #endif

        return;
    }
}
/*********************************************************/
void User_Adc_Batt_Power_Up_Init(void) {
    Get_User_Adc_Batt_Power_Up_Init();
}

void User_Adc_Batt_Number(void) {
    Get_User_Adc_Batt_Number();
}

void Init_Keyboard_Infomation(void){
    /*上电将flash里面的工作模式读取出来*/
    eeprom_read_block_user((void *)&Keyboard_Info, 0, sizeof(Keyboard_Info_t));

    /*如果 eeprom 里面是空数据直接初始化结构体*/
    if ((Keyboard_Info.Key_Mode == 0XFF) && (Keyboard_Info.Ble_Channel == 0XFF) && (Keyboard_Info.Batt_Number == 0XFF)  && (Keyboard_Info.Nkro == 0XFF) && (Keyboard_Info.Mac_Win_Mode == 0XFF) 
        && (Keyboard_Info.Win_Lock == 0XFF) && (Keyboard_Info.Led_On_Off == 0XFF) && (Keyboard_Info.Debounce_Delay == 0xFF)) {
        Keyboard_Info.Key_Mode = INIT_WORK_MODE;
        Keyboard_Info.Ble_Channel = INIT_BLE_CHANNEL;
        Keyboard_Info.Batt_Number = INIT_BATT_NUMBER;
        Keyboard_Info.Nkro = INIT_ALL_KEY;
        Keyboard_Info.Mac_Win_Mode = INIT_WIN_MODE;
        Keyboard_Info.Win_Lock = INIT_WIN_NLOCK;
        Keyboard_Info.Led_On_Off = INIT_LED_ON_OFF;
        Keyboard_Info.Debounce_Delay = DEBOUNCE_DELAY_CLASS;
        Keyboard_Info.User_Sleep_Time = SLEEP_TIME_CLASS;
        Keyboard_Info.User_DSleep_Time = USER_DSLEEP_TIME;
    #if LOGO_LED_ENABLE
        Keyboard_Info.Logo_On_Off = INIT_LOGO_ON_OFF;
        Keyboard_Info.Logo_Mode = INIT_LOGO_MODE;
        Keyboard_Info.Logo_Colour = INIT_LOGO_COLOUR;
        Keyboard_Info.Logo_Saturation = INIT_LOGO_SATURATION;
        Keyboard_Info.Logo_Brightness = INIT_LOGO_BRIGHTNESS;
        Keyboard_Info.Logo_Speed = INIT_LOGO_SPEED;
    #endif
    #if SIDE_LED_ENABLE
        Keyboard_Info.Side_On_Off = INIT_SIDE_ON_OFF;
        Keyboard_Info.Side_Mode = INIT_SIDE_MODE;
        Keyboard_Info.Side_Colour = INIT_SIDE_COLOUR;
        Keyboard_Info.Side_Colour_Blue = INIT_SIDE_COLOUR_BLUE;
        Keyboard_Info.Side_Saturation = INIT_SIDE_SATURATION;
        Keyboard_Info.Side_Brightness = INIT_SIDE_BRIGHTNESS;
        Keyboard_Info.Side_Speed = INIT_SIDE_SPEED;
    #endif
    #if LATTICE_LED_ENABLE
        Keyboard_Info.Lattice_On_Off = INIT_LATTICE_ON_OFF;
        Keyboard_Info.Lattice_Mode = INIT_LATTICE_MODE;
        Keyboard_Info.Lattice_Colour = INIT_LATTICE_COLOUR;
        Keyboard_Info.Lattice_Colour_Blue = INIT_LATTICE_COLOUR_BLUE;
        Keyboard_Info.Lattice_Saturation = INIT_LATTICE_SATURATION;
        Keyboard_Info.Lattice_Brightness = INIT_LATTICE_BRIGHTNESS;
        Keyboard_Info.Lattice_Speed = INIT_LATTICE_SPEED;
    #endif
    } else if ((Keyboard_Info.Key_Mode == 0) && (Keyboard_Info.Ble_Channel == 0) && (Keyboard_Info.Batt_Number == 0)  && (Keyboard_Info.Nkro == 0) && (Keyboard_Info.Mac_Win_Mode == 0) 
        && (Keyboard_Info.Win_Lock == 0) && (Keyboard_Info.Led_On_Off == 0) && (Keyboard_Info.Debounce_Delay == 0)) {
        Keyboard_Info.Key_Mode = INIT_WORK_MODE;
        Keyboard_Info.Ble_Channel = INIT_BLE_CHANNEL;
        Keyboard_Info.Batt_Number = INIT_BATT_NUMBER;
        Keyboard_Info.Nkro = INIT_ALL_KEY;
        Keyboard_Info.Mac_Win_Mode = INIT_WIN_MODE;
        Keyboard_Info.Win_Lock = INIT_WIN_NLOCK;
        Keyboard_Info.Led_On_Off = INIT_LED_ON_OFF;
        Keyboard_Info.Debounce_Delay = DEBOUNCE_DELAY_CLASS;
        Keyboard_Info.User_Sleep_Time = SLEEP_TIME_CLASS;
        Keyboard_Info.User_DSleep_Time = USER_DSLEEP_TIME;
    #if LOGO_LED_ENABLE
        Keyboard_Info.Logo_On_Off = INIT_LOGO_ON_OFF;
        Keyboard_Info.Logo_Mode = INIT_LOGO_MODE;
        Keyboard_Info.Logo_Colour = INIT_LOGO_COLOUR;
        Keyboard_Info.Logo_Saturation = INIT_LOGO_SATURATION;
        Keyboard_Info.Logo_Brightness = INIT_LOGO_BRIGHTNESS;
        Keyboard_Info.Logo_Speed = INIT_LOGO_SPEED;
    #endif
    #if SIDE_LED_ENABLE
        Keyboard_Info.Side_On_Off = INIT_SIDE_ON_OFF;
        Keyboard_Info.Side_Mode = INIT_SIDE_MODE;
        Keyboard_Info.Side_Colour = INIT_SIDE_COLOUR;
        Keyboard_Info.Side_Colour_Blue = INIT_SIDE_COLOUR_BLUE;
        Keyboard_Info.Side_Saturation = INIT_SIDE_SATURATION;
        Keyboard_Info.Side_Brightness = INIT_SIDE_BRIGHTNESS;
        Keyboard_Info.Side_Speed = INIT_SIDE_SPEED;
    #endif
    #if LATTICE_LED_ENABLE
        Keyboard_Info.Lattice_On_Off = INIT_LATTICE_ON_OFF;
        Keyboard_Info.Lattice_Mode = INIT_LATTICE_MODE;
        Keyboard_Info.Lattice_Colour = INIT_LATTICE_COLOUR;
        Keyboard_Info.Lattice_Colour_Blue = INIT_LATTICE_COLOUR_BLUE;
        Keyboard_Info.Lattice_Saturation = INIT_LATTICE_SATURATION;
        Keyboard_Info.Lattice_Brightness = INIT_LATTICE_BRIGHTNESS;
        Keyboard_Info.Lattice_Speed = INIT_LATTICE_SPEED;
    #endif
    } else {
        if (Keyboard_Info.Key_Mode > QMK_USB_MODE) {
            Keyboard_Info.Key_Mode = QMK_USB_MODE;
        }

        if (Keyboard_Info.Ble_Channel > QMK_BLE_CHANNEL_3) {
            Keyboard_Info.Ble_Channel = QMK_BLE_CHANNEL_3;
        }

        if (Keyboard_Info.Batt_Number > 100) {
            Keyboard_Info.Batt_Number = 100;
        }

        if (Keyboard_Info.Nkro > INIT_ALL_KEY) {
            Keyboard_Info.Nkro = INIT_ALL_KEY;
        }

        if (Keyboard_Info.Mac_Win_Mode > INIT_MAC_MODE) {
            Keyboard_Info.Mac_Win_Mode = INIT_WIN_MODE;
        }

        if (Keyboard_Info.Win_Lock > INIT_WIN_LOCK) {
            Keyboard_Info.Win_Lock = INIT_WIN_NLOCK;
        }

        if (Keyboard_Info.Led_On_Off > INIT_LED_OFF) {
            Keyboard_Info.Led_On_Off = INIT_LED_ON_OFF;
        }

        if (Keyboard_Info.Debounce_Delay > DEBOUNCE_DELAY_TWO) {
            Keyboard_Info.Debounce_Delay = DEBOUNCE_DELAY_CLASS;
        }
        
        if (Keyboard_Info.User_Sleep_Time == SLEEP_TIME_FOUR) {
            Keyboard_Info.User_Sleep_Time = SLEEP_TIME_CLASS;
        }
        if (Keyboard_Info.User_DSleep_Time == 0XFFFFFF) {
            Keyboard_Info.User_DSleep_Time = USER_DSLEEP_TIME;
        }
    #if LOGO_LED_ENABLE
        if (Keyboard_Info.Logo_On_Off > LOGO_LED_OFF) {
            Keyboard_Info.Logo_On_Off = INIT_LOGO_ON_OFF;
        }

        if(Keyboard_Info.Logo_Mode > LOGO_LIGHT_MODE) {
            Keyboard_Info.Logo_Mode = INIT_LOGO_MODE;
        }

        if (Keyboard_Info.Logo_Colour > LOGO_MAX_COLOUR) {
            Keyboard_Info.Logo_Colour = INIT_LOGO_COLOUR;
        }

        if (Keyboard_Info.Logo_Saturation > LOGO_MIN_SATURATION) {
            Keyboard_Info.Logo_Saturation = INIT_LOGO_SATURATION;
        }

        if (Keyboard_Info.Logo_Brightness > LOGO_MAX_BRIGHTNESS) {
            Keyboard_Info.Logo_Brightness = INIT_LOGO_BRIGHTNESS;
        }

        if (Keyboard_Info.Logo_Speed > LOGO_MAX_SPEED) {
            Keyboard_Info.Logo_Speed = INIT_LOGO_SPEED;
        }
    #endif
    #if SIDE_LED_ENABLE
        if (Keyboard_Info.Side_On_Off > SIDE_LED_OFF) {
            Keyboard_Info.Side_On_Off = INIT_SIDE_ON_OFF;
        }

        if (Keyboard_Info.Side_Mode > SIDE_OFF_MODE) {
            Keyboard_Info.Side_Mode = INIT_SIDE_MODE;
        }

        if (Keyboard_Info.Side_Colour > SIDE_MAX_COLOUR) {
            Keyboard_Info.Side_Colour = INIT_SIDE_COLOUR;
        }

        if (Keyboard_Info.Side_Colour_Blue > SIDE_MAX_COLOUR) {
            Keyboard_Info.Side_Colour_Blue = INIT_SIDE_COLOUR;
        }

        if (Keyboard_Info.Side_Saturation > SIDE_MIN_SATURATION) {
            Keyboard_Info.Side_Saturation = INIT_SIDE_SATURATION;
        }

        if (Keyboard_Info.Side_Brightness > SIDE_MAX_BRIGHTNESS) {
            Keyboard_Info.Side_Brightness = INIT_SIDE_BRIGHTNESS;
        }

        if (Keyboard_Info.Side_Speed > SIDE_MAX_SPEED) {
            Keyboard_Info.Side_Speed = INIT_SIDE_SPEED;
        }
    #endif
    #if LATTICE_LED_ENABLE
        if (Keyboard_Info.Lattice_On_Off > LATTICE_LED_OFF) {
            Keyboard_Info.Lattice_On_Off = INIT_LATTICE_ON_OFF;
        }

        if (Keyboard_Info.Lattice_Mode > LATTICE_OFF_MODE) {
            Keyboard_Info.Lattice_Mode = INIT_LATTICE_MODE;
        }

        if (Keyboard_Info.Lattice_Colour > LATTICE_MAX_COLOUR) {
            Keyboard_Info.Lattice_Colour = INIT_LATTICE_COLOUR;
        }
        
        if (Keyboard_Info.Lattice_Colour_Blue > LATTICE_MAX_COLOUR) {
            Keyboard_Info.Lattice_Colour_Blue = INIT_LATTICE_COLOUR_BLUE;
        }

        if (Keyboard_Info.Lattice_Saturation > LATTICE_MIN_SATURATION) {
            Keyboard_Info.Lattice_Saturation = INIT_LATTICE_SATURATION;
        }

        if (Keyboard_Info.Lattice_Brightness > LATTICE_MAX_BRIGHTNESS) {
            Keyboard_Info.Lattice_Brightness = INIT_LATTICE_BRIGHTNESS;
        }

        if (Keyboard_Info.Lattice_Speed > LATTICE_MAX_SPEED) {
            Keyboard_Info.Lattice_Speed = INIT_LATTICE_SPEED;
        }
    #endif
    }
    Debounce_Delay = Keyboard_Info.Debounce_Delay;
    if (Debounce_Delay == DEBOUNCE_DELAY_ONE) {
        Debounce_Function_Count = 0;
    } else {
        Debounce_Function_Count = 1;
    }
}

void Key_Switch_Mode_Power(void) {  /*拨动开关在波动的瞬间会断电，所以在上电的时候需要特殊处理*/
    uint8_t Scan_Delay = 0;
    uint8_t Power_Up_Mode = QMK_USB_MODE;
    uint8_t Power_UP_BLE = 0;
    uint8_t Power_UP_2P4G = 0;
    uint8_t Power_UP_USB = 0;

    for(uint8_t i = 0; i < 11; i++) {
        if ((gpio_read_pin(MODE_BLE_IO)) && (!gpio_read_pin(MODE_2P4G_IO))) {
            Power_UP_2P4G++;
        } else if ((!gpio_read_pin(MODE_BLE_IO)) && (gpio_read_pin(MODE_2P4G_IO))) {
            Power_UP_BLE++;
        } else if ((gpio_read_pin(MODE_BLE_IO)) && (gpio_read_pin(MODE_2P4G_IO))) {
            Power_UP_USB++;
        } else {
            Power_UP_USB++;
        }

        Scan_Delay = 200;
        while(Scan_Delay--);
    }

    uint8_t MAX = Power_UP_USB;
    if (Power_UP_BLE > MAX) MAX = Power_UP_BLE;
    if (Power_UP_2P4G > MAX) MAX = Power_UP_2P4G;

    if (MAX == Power_UP_USB) {
        Power_Up_Mode = QMK_USB_MODE;
    } else if (MAX == Power_UP_BLE) {
        Power_Up_Mode = QMK_BLE_MODE;
    } else if (MAX == Power_UP_2P4G) {
        Power_Up_Mode = QMK_2P4G_MODE;
    } else {
        Power_Up_Mode = QMK_USB_MODE;
    }

    if (Power_Up_Mode != Keyboard_Info.Key_Mode) {
        Keyboard_Info.Key_Mode = Power_Up_Mode;
        Save_Flash_Set();
    }
}

void board_init(void) {
    es_ble_spi_init();          //SPI

    User_Adc_Init();            //ADC

    eeprom_driver_init();       //EEPROM

    rgb_matrix_driver_init();   //PWM DMA 初始化

    Init_Gpio_Infomation();     //GPIO

    Init_Keyboard_Infomation(); //初始化键盘基本信息

    Key_Switch_Mode_Power();    //拨动开关强制切换模式

    Init_Batt_Infomation();     //初始化电池电量

    User_Systime_Init();        //time定时器

    if (Keyboard_Info.Key_Mode == QMK_USB_MODE) {
        User_Usb_Init();        //USB 插件
        Led_Rf_Pair_Flg = false;
    } else {
        Usb_Disconnect();
    }

    Init_Spi_Power_Up = true;
    Init_Spi_100ms_Delay = 0;
    Spi_Interval = SPI_DELAY_RF_TIME;
    NVIC_SetPriority(PendSV_IRQn, 3);
    NVIC_SetPriority(SysTick_IRQn, 3);

    Usb_If_Ok = false;
    Usb_If_Ok_Led = false;
    Led_Power_Up = false;
    Emi_Test_Start = false;
    
    Spi_Send_Recv_count = 0;

#if LOGO_LED_ENABLE
    Logo_Init();
#endif

#if SIDE_LED_ENABLE
    Side_Init();
#endif

#if LATTICE_LED_ENABLE
    Lattice_Power_Flag = 1;
    Lt_Heart_Flag = 1;
    Lattice_Init();
#endif
}

void keyboard_post_init_user(void) {
    User_Keyboard_Post_Init();
}

void es_change_qmk_nkro_mode_enable(void) {  /*六键 、全键无冲*/
    if(!keymap_config.nkro) {
        clear_keyboard(); // clear first buffer to prevent stuck keys
        keymap_config.nkro = true;

        Keyboard_Info.Nkro = keymap_config.nkro;
        Save_Flash_Set();
    }
}

void es_change_qmk_nkro_mode_disable(void) {
    if(keymap_config.nkro) {
        clear_keyboard(); // clear first buffer to prevent stuck keys
        keymap_config.nkro = false;

        Keyboard_Info.Nkro = keymap_config.nkro;
        Save_Flash_Set();
    }
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {   /*键盘只要有按键按下就会调用此函数*/
    Usb_Change_Mode_Delay = 0;                                      /*只要有按键就不会进入休眠*/
    Usb_Change_Mode_Wakeup = false;

    /* Mac下変換/かな同为KC_LNG1，必须先按矩阵位置置位，再刷新点阵图案 */
    if (keycode == KC_LNG1) {
        if (record->event.pressed) {
            User_Change_Flag = ((record->event.key.col == CHANGE_COL) && (record->event.key.row == CHANGE_ROL));
            User_Kana_Flag   = ((record->event.key.col == KANA_COL) && (record->event.key.row == KANA_ROL));
        } else {
            User_Change_Flag = false;
            User_Kana_Flag   = false;
        }
    }

    Lattice_Key_Led_Trigger_Mode(keycode, record->event.pressed);

    if (Test_Led) {
        if ((keycode != KC_SPC) && (keycode != MO(2)) && (keycode != MO(3)) && (keycode != KC_LCTL)) {
            Test_Led = false;
            Lattice_Init();
        }
    }

    switch (keycode) {
        case QMK_KB_MODE_2P4G: {                                    //2.4G
            if (Keyboard_Info.Key_Mode != QMK_2P4G_MODE) {
                return true;
            }
            if (record->event.pressed) {
                Key_2p4g_Status = true;
                Usb_Disconnect();
                if (Keyboard_Info.Key_Mode != QMK_2P4G_MODE) {      /*如果当前模式不是2.4G模式则切换为2.4G*/
                    Keyboard_Info.Key_Mode = QMK_2P4G_MODE;
                    User_Clear_Board_2ms();
                    Spi_Send_Commad(USER_SWITCH_2P4G_MODE);         /*发送SPI命令*/
                    Save_Flash_Set();
                    Led_Rf_Pair_Flg = true;

                }
            } else {
                Key_2p4g_Status = false;
            }
            Time_3s_Count = 0;
        } return true;
        case QMK_KB_MODE_BLE1: {                                    //BLE 1
            if (Keyboard_Info.Key_Mode != QMK_BLE_MODE) {
                return true;
            }
            if (record->event.pressed) {
                Key_Ble_1_Status = true;
                Usb_Disconnect();
                if (Keyboard_Info.Ble_Channel != QMK_BLE_CHANNEL_1) {
                    Keyboard_Info.Ble_Channel = QMK_BLE_CHANNEL_1;
                    User_Clear_Board_2ms();
                    Spi_Send_Commad(USER_SWITCH_BLE_1_MODE);        /*发送SPI命令*/
                    Save_Flash_Set();
                    Led_Rf_Pair_Flg = true;
                }
            } else {
                Key_Ble_1_Status = false;
            }
            Time_3s_Count = 0;
        } return true;
        case QMK_KB_MODE_BLE2: {                                    //BLE 2
            if (Keyboard_Info.Key_Mode != QMK_BLE_MODE) {
                return true;
            }
            if (record->event.pressed) {
                Key_Ble_2_Status = true;
                Usb_Disconnect();
                if (Keyboard_Info.Ble_Channel != QMK_BLE_CHANNEL_2) {
                    Keyboard_Info.Ble_Channel = QMK_BLE_CHANNEL_2;
                    User_Clear_Board_2ms();
                    Spi_Send_Commad(USER_SWITCH_BLE_2_MODE);        /*发送SPI命令*/
                    Save_Flash_Set();
                    Led_Rf_Pair_Flg = true;
                }
            } else {
                Key_Ble_2_Status = false;
            }
            Time_3s_Count = 0;
        } return true;
        case QMK_KB_MODE_BLE3: {                                    //BLE 3
            if (Keyboard_Info.Key_Mode != QMK_BLE_MODE) {
                return true;
            }
            if (record->event.pressed) {
                Key_Ble_3_Status = true;
                Usb_Disconnect();
                if (Keyboard_Info.Ble_Channel != QMK_BLE_CHANNEL_3) {
                    Keyboard_Info.Ble_Channel = QMK_BLE_CHANNEL_3;
                    User_Clear_Board_2ms();
                    Spi_Send_Commad(USER_SWITCH_BLE_3_MODE);        /*发送SPI命令*/
                    Save_Flash_Set();
                    Led_Rf_Pair_Flg = true;
                }
            } else {
                Key_Ble_3_Status = false;
            }
            Time_3s_Count = 0;
        } return true;
        case QMK_KB_MODE_USB: {                                     //USB
            if (record->event.pressed) {
                if (Keyboard_Info.Key_Mode != QMK_USB_MODE) {
                    Keyboard_Info.Key_Mode = QMK_USB_MODE;
                    Spi_Send_Commad(USER_SWITCH_USB_MODE);          /*发送SPI命令*/
                    es_restart_usb_driver();
                    Save_Flash_Set();
                    Led_Rf_Pair_Flg = false;
                }
            }
        } return true;
        case QMK_BATT_NUM: {                                        //电池状态显示
            if (record->event.pressed) {
                User_Key_Batt_Num_Show = true;
                Lattice_Init();
                User_Key_Batt_Count = 0;
            } else {
                User_Key_Batt_Num_Show = false;
                Lattice_Init();
                User_Key_Batt_Count = 0;
            }
        } return true;
        case QMK_WIN_LOCK: {                                        //锁WIN
            if (!record->event.pressed) {
                if (Keyboard_Info.Mac_Win_Mode == INIT_MAC_MODE) {
                    if (Keyboard_Info.Win_Lock == INIT_WIN_LOCK) {
                        Keyboard_Info.Win_Lock = INIT_WIN_NLOCK;
                        Save_Flash_Set();
                    }
                } else {
                    if (Keyboard_Info.Win_Lock == INIT_WIN_NLOCK) {
                        Keyboard_Info.Win_Lock = INIT_WIN_LOCK;
                        unregister_code(KC_LGUI); unregister_code(KC_RGUI); unregister_code(KC_APP);
                    } else {
                        Keyboard_Info.Win_Lock = INIT_WIN_NLOCK;
                    }
                    Save_Flash_Set();
                }
            }
        } return true;
        case QMK_DEBOUNCE: {                                        //按键消抖功能
            if (record->event.pressed) {
                Debounce_Function_Status = true;
            } else {
                Debounce_Function_Status = false;
            }
            User_Key_3s_Count = 0;
        } return true; 
        case QMK_TIME_SET: {                                        //一级休眠
            if (!record->event.pressed) {
                if (Keyboard_Info.User_Sleep_Time == SLEEP_TIME_ONE) {
                    Keyboard_Info.User_Sleep_Time = SLEEP_TIME_TWO;
                } else if(Keyboard_Info.User_Sleep_Time == SLEEP_TIME_TWO) {
                    Keyboard_Info.User_Sleep_Time = SLEEP_TIME_THREE;
                } else if(Keyboard_Info.User_Sleep_Time == SLEEP_TIME_THREE) {
                    Keyboard_Info.User_Sleep_Time = SLEEP_TIME_FOUR;
                } else if(Keyboard_Info.User_Sleep_Time == SLEEP_TIME_FOUR){
                    Keyboard_Info.User_Sleep_Time = SLEEP_TIME_ONE;
                } else {
                    Keyboard_Info.User_Sleep_Time = SLEEP_TIME_CLASS;
                }
                Spi_Send_Commad(USER_SLEEP_TIME_WRITE);
                User_Sleep_Time_Send = false;
                User_Sleep_Timer_Count = 3;
                Save_Flash_Set();
            }
        } return true;
        case QMK_DTIME_SET: {                                       //二级休眠
            if (!record->event.pressed) {
                Keyboard_Info.User_DSleep_Time += 15;               //调节多少根据客户的需求来
                Spi_Send_Commad(USER_DSLEEP_TIME_WRITE);
                User_DSleep_Time_Send = false;
                Save_Flash_Set();
            }
        } return true;
        case QMK_TEST_COLOUR: {                                     //键盘灯光颜色测试
            if (!record->event.pressed) {
                if (Test_Led == false) {
                    Test_Led = true;
                    Test_Colour = 0;
                }
            }
        } return true;
        case KC_SPC: {                                              //测试灯光颜色切换
            if (!record->event.pressed) {
                if (Test_Led) {
                    Test_Colour++;
                    if (Test_Colour >= 4) {
                        Test_Colour = 0;
                    }
                }
            }
        } return true;
        case KC_LGUI: {                                             //key_win_l
            if (Keyboard_Info.Win_Lock) {
                record->event.pressed = false;
            }
        } return true;
        case KC_RGUI: {                                             //key_win_r
            if (Keyboard_Info.Win_Lock) {
                record->event.pressed = false;
            }
        } return true;
        case KC_APP: {                                              //key_app
            if (Keyboard_Info.Win_Lock) {
                record->event.pressed = false;
            }
        } return true;
        case RGB_VAI: {                                             //亮度加
            if (!record->event.pressed) {
                if (rgb_matrix_get_val() >= (RGB_MATRIX_MAXIMUM_BRIGHTNESS - RGB_MATRIX_VAL_STEP)) {
                    Led_Point_Count = 3;
                }
            }
        } return true;
        case RGB_VAD: {                                             //亮度减
            if (!record->event.pressed) {
                if (rgb_matrix_get_val() <= RGB_MATRIX_VAL_STEP) {
                    Led_Point_Count = 3;
                }
            }
        } return true;
        case RGB_SPI: {                                             //速度加
            if (!record->event.pressed) {
                if (rgb_matrix_get_speed() >= (255 - RGB_MATRIX_SPD_STEP)) {
                    Led_Point_Count = 3;
                }
            }
        } return true;
        case RGB_SPD: {                                             //速度减
            if (!record->event.pressed) {
                if (rgb_matrix_get_speed() <= RGB_MATRIX_SPD_STEP) {
                    Led_Point_Count = 3;
                }
            }
        } return true;
        case RGB_RTOG: {                                            //开关闭背光
            if (!record->event.pressed) {
                if (Keyboard_Info.Led_On_Off) {
                    Keyboard_Info.Led_On_Off = INIT_LED_ON;
                    if (rgb_matrix_get_val() <= 0) {                // 获取当前指示灯亮度
                        rgb_matrix_sethsv_noeeprom(rgb_matrix_get_hue(), rgb_matrix_get_sat(), RGB_MATRIX_MAXIMUM_BRIGHTNESS);//设置RGB灯的色相（hue）、饱和度(sat)、亮度
                    }
                } else {
                    Keyboard_Info.Led_On_Off = INIT_LED_OFF;

                }
                Lattice_Init();
                Logo_Init();
                Save_Flash_Set();
            }
        } return true;
        case MO(2): {                                               //FN
            if (record->event.pressed) {
                Key_Fn_Status = true;
            } else {
                Key_Fn_Status = false;
            }
        } return true;
        case MO(3): {                                               //FN
            if (record->event.pressed) {
                Key_Fn_Status = true;
            } else {
                Key_Fn_Status = false;
            }
        } return true;
        case QMK_MAC_WIN_CH: {                                     //MAC/WIN系统切换
            if (record->event.pressed) {
                Key_Sys_Mode_Status = true;
            } else {
                Key_Sys_Mode_Status = false;
                Win_Mac_Key_3s_Count = 0;
            }
        } return true;
        case U_EE_CLR: {                                             //复位
            if (record->event.pressed) {
                User_QMK_EE_CLR_Flag = true;
            }else{
                User_QMK_EE_CLR_Flag = false;
                Time_3s_EE_CLR_Count = 0;
            }
        } return true;
    #if LOGO_LED_ENABLE
        case LOGO_TOG: {                                            //logo 灯光开关
            if (!record->event.pressed) {
                if (Keyboard_Info.Logo_On_Off) {
                    Keyboard_Info.Logo_On_Off = LOGO_LED_ON;
                    if (Keyboard_Info.Logo_Brightness <= LOGO_MIN_BRIGHTNESS) {
                        Keyboard_Info.Logo_Brightness = LOGO_MAX_BRIGHTNESS;
                    }
                } else {
                    Keyboard_Info.Logo_On_Off = LOGO_LED_OFF;
                }
                Logo_Init();
                Save_Flash_Set();
            }
        } return true;
        case LOGO_MOD: {                                            //logo 模式切换
            if (!record->event.pressed) {
                if (Keyboard_Info.Logo_On_Off) {
                    return true;
                } else {
                    if (Keyboard_Info.Logo_Mode < LOGO_OFF_MODE) {
                        Keyboard_Info.Logo_Mode ++;
                    } else {
                        Keyboard_Info.Logo_Mode = INIT_LOGO_MODE;
                    }
                }
                Lattice_Power_Flag = 0;
                Logo_Init();
                Save_Flash_Set();
            }
        } return true;
        case LOGO_RMOD: {                                           //logo 模式切换
            if (!record->event.pressed) {
                if (Keyboard_Info.Logo_On_Off) {
                    return true;
                } else {
                    if (Keyboard_Info.Logo_Mode > INIT_LOGO_MODE) {
                        Keyboard_Info.Logo_Mode--;
                    } else {
                        Keyboard_Info.Logo_Mode = LOGO_LIGHT_MODE;
                    }
                }
                Logo_Init();
                Save_Flash_Set();
            }
        } return true;
        case LOGO_HUI: {                                            //logo 颜色增加
            if (!record->event.pressed) {
                if (Keyboard_Info.Logo_On_Off || (Keyboard_Info.Logo_Mode == LOGO_OFF_MODE) || (Keyboard_Info.Logo_Mode == LOGO_WAVE_RGB_MODE) || (Keyboard_Info.Logo_Mode == LOGO_SPECTRUM_MODE)) {
                    return true;
                } else {
                    if (Keyboard_Info.Logo_Colour < (LOGO_MAX_COLOUR - COLOUR_LEVEL)) {
                        Keyboard_Info.Logo_Colour += COLOUR_LEVEL;
                    } else {
                        Keyboard_Info.Logo_Colour = ((Keyboard_Info.Logo_Colour + COLOUR_LEVEL) - LOGO_MAX_COLOUR);
                    }
                }
                Save_Flash_Set();
            }
        } return true;
        case LOGO_HUD: {                                            //logo 颜色减小
            if (!record->event.pressed) {
                if (Keyboard_Info.Logo_On_Off || (Keyboard_Info.Logo_Mode == LOGO_OFF_MODE) || (Keyboard_Info.Logo_Mode == LOGO_WAVE_RGB_MODE) || (Keyboard_Info.Logo_Mode == LOGO_SPECTRUM_MODE)) {
                    return true;
                } else {
                    if ((Keyboard_Info.Logo_Colour - COLOUR_LEVEL) > LOGO_MIN_COLOUR) {
                        Keyboard_Info.Logo_Colour -= COLOUR_LEVEL;
                    } else {
                        Keyboard_Info.Logo_Colour = ((LOGO_MAX_COLOUR - COLOUR_LEVEL) + Keyboard_Info.Logo_Colour);
                    }
                }
                Save_Flash_Set();
            }
        } return true;
        case LOGO_VAI: {                                            //logo 亮度增加
            if (!record->event.pressed) {
                if (Keyboard_Info.Logo_On_Off || (Keyboard_Info.Logo_Mode == LOGO_OFF_MODE)) {
                    return true;
                } else {
                    if (Keyboard_Info.Logo_Brightness < (LOGO_MAX_BRIGHTNESS - BRIGHTNESS_LEVEL)) {
                        Keyboard_Info.Logo_Brightness += BRIGHTNESS_LEVEL;
                    } else {
                        Keyboard_Info.Logo_Brightness = LOGO_MAX_BRIGHTNESS;
                        Led_Point_Count = 3;
                    }
                }
                Save_Flash_Set();
            }
        } return true;
        case LOGO_VAD: {                                            //logo 亮度减小
            if (!record->event.pressed) {
                if (Keyboard_Info.Logo_On_Off || (Keyboard_Info.Logo_Mode == LOGO_OFF_MODE)) {
                    return true;
                } else {
                    if ((Keyboard_Info.Logo_Brightness - BRIGHTNESS_LEVEL) > LOGO_MIN_BRIGHTNESS) {
                        Keyboard_Info.Logo_Brightness -= BRIGHTNESS_LEVEL;
                    } else {
                        Keyboard_Info.Logo_Brightness = LOGO_MIN_BRIGHTNESS;
                        Led_Point_Count = 3;
                    }
                }
                Save_Flash_Set();
            }
        } return true;
        case LOGO_SPI: {                                            //logo 速度增加
            if (!record->event.pressed) {
                if (Keyboard_Info.Logo_On_Off || (Keyboard_Info.Logo_Mode == LOGO_OFF_MODE)) {
                    return true;
                } else {
                    if (Keyboard_Info.Logo_Speed < (LOGO_MAX_SPEED - SPEED_LEVEL)) {
                        Keyboard_Info.Logo_Speed += SPEED_LEVEL;
                    } else {
                        Keyboard_Info.Logo_Speed = LOGO_MAX_SPEED;
                        Led_Point_Count = 3;
                    }
                }
                Save_Flash_Set();
            }
        } return true;
        case LOGO_SPD: {                                            //logo 速度减小
            if (!record->event.pressed) {
                if (Keyboard_Info.Logo_On_Off || (Keyboard_Info.Logo_Mode == LOGO_OFF_MODE)) {
                    return true;
                } else {
                    if ((Keyboard_Info.Logo_Speed - SPEED_LEVEL) > LOGO_MIN_SPEED) {
                        Keyboard_Info.Logo_Speed -= SPEED_LEVEL;
                    } else {
                        Keyboard_Info.Logo_Speed = LOGO_MIN_SPEED;
                        Led_Point_Count = 3;
                    }
                }
                Save_Flash_Set();
            }
        } return true;
    #endif
    
    #if LATTICE_LED_ENABLE
        case LATTICE_MOD: {                                            //矩阵 模式切换
            if (!record->event.pressed) {
                if (Lattice_Power_Flag) {
                    Lattice_Power_Flag = 0;
                } else if ((Keyboard_Info.Lattice_On_Off) || (User_Key_Batt_Num_Show)) {
                    return true;
                } else {
                    if (Keyboard_Info.Lattice_Mode < LATTICE_OFF_MODE) {
                        Keyboard_Info.Lattice_Mode ++;
                    } else {
                        Keyboard_Info.Lattice_Mode = INIT_LATTICE_MODE;
                    }
                }
                Lattice_Init();
                Save_Flash_Set();
            }
        } return true;
        case LATTICE_HUI: {                                            //矩阵 颜色增加
            if (!record->event.pressed) {
                if (Lattice_Power_Flag) {
                    Lattice_Power_Flag = 0;
                    Lattice_Init();
                } else if (Keyboard_Info.Lattice_On_Off || (Keyboard_Info.Lattice_Mode == LATTICE_OFF_MODE) || (Keyboard_Info.Lattice_Mode == LATTICE_SCAN_MODE)) {
                    return true;
                } else {
                    if (Keyboard_Info.Lattice_Colour < (LATTICE_MAX_COLOUR - LATTICE_COLOUR_LEVEL)) {
                        Keyboard_Info.Lattice_Colour += LATTICE_COLOUR_LEVEL;
                    } else {
                        Keyboard_Info.Lattice_Colour = ((Keyboard_Info.Lattice_Colour + LATTICE_COLOUR_LEVEL) - LATTICE_MAX_COLOUR);
                    }
                    if (Keyboard_Info.Lattice_Colour_Blue < (LATTICE_MAX_COLOUR - LATTICE_COLOUR_LEVEL)) {
                        Keyboard_Info.Lattice_Colour_Blue += LATTICE_COLOUR_LEVEL;
                    } else {
                        Keyboard_Info.Lattice_Colour_Blue = ((Keyboard_Info.Lattice_Colour_Blue + LATTICE_COLOUR_LEVEL) - LATTICE_MAX_COLOUR);
                    }
                }
                Save_Flash_Set();
            }
        } return true;
        case LATTICE_VAI: {                                            //矩阵 亮度增加
            if (!record->event.pressed) {
                if (Lattice_Power_Flag) {
                    Lattice_Power_Flag = 0;
                    Lattice_Init();
                } else if (Keyboard_Info.Lattice_On_Off || (Keyboard_Info.Lattice_Mode == LATTICE_OFF_MODE)) {
                    return true;
                } else {
                    if (Keyboard_Info.Lattice_Brightness < (LATTICE_MAX_BRIGHTNESS - LATTICE_BRIGHTNESS_LEVEL)) {
                        Keyboard_Info.Lattice_Brightness += LATTICE_BRIGHTNESS_LEVEL;
                    } else {
                        Keyboard_Info.Lattice_Brightness = LATTICE_MAX_BRIGHTNESS;
                        Led_Point_Count = 3;
                    }
                }
                Save_Flash_Set();
            }
        } return true;
        case LATTICE_VAD: {                                            //矩阵 亮度减小
            if (!record->event.pressed) {
                if (Lattice_Power_Flag) {
                    Lattice_Power_Flag = 0;
                    Lattice_Init();
                } else if (Keyboard_Info.Lattice_On_Off || (Keyboard_Info.Lattice_Mode == LATTICE_OFF_MODE)) {
                    return true;
                } else {
                    if ((Keyboard_Info.Lattice_Brightness - LATTICE_BRIGHTNESS_LEVEL) > LATTICE_MIN_BRIGHTNESS) {
                        Keyboard_Info.Lattice_Brightness -= LATTICE_BRIGHTNESS_LEVEL;
                    } else {
                        Keyboard_Info.Lattice_Brightness = LATTICE_MIN_BRIGHTNESS;
                        Led_Point_Count = 3;
                    }
                }
                Save_Flash_Set();
            }
        } return true;
        case LATTICE_SPI: {                                            //矩阵 速度增加
            if (!record->event.pressed) {
                if (Lattice_Power_Flag) {
                    Lattice_Power_Flag = 0;
                    Lattice_Init();
                } else if (Keyboard_Info.Lattice_On_Off || (Keyboard_Info.Lattice_Mode == LATTICE_OFF_MODE)) {
                    return true;
                } else {
                    if (Keyboard_Info.Lattice_Speed < (LATTICE_MAX_SPEED - LATTICE_SPEED_LEVEL)) {
                        Keyboard_Info.Lattice_Speed += LATTICE_SPEED_LEVEL;
                    } else {
                        Keyboard_Info.Lattice_Speed = LATTICE_MAX_SPEED;
                        Led_Point_Count = 3;
                    }
                }
                Save_Flash_Set();
            }
        } return true;
        case LATTICE_SPD: {                                            //矩阵 速度减小
            if (!record->event.pressed) {
                if (Lattice_Power_Flag) {
                    Lattice_Power_Flag = 0;
                    Lattice_Init();
                } else if (Keyboard_Info.Lattice_On_Off || (Keyboard_Info.Lattice_Mode == LATTICE_OFF_MODE)) {
                    return true;
                } else {
                    if ((Keyboard_Info.Lattice_Speed - LATTICE_SPEED_LEVEL) > LATTICE_MIN_SPEED) {
                        Keyboard_Info.Lattice_Speed -= LATTICE_SPEED_LEVEL;
                    } else {
                        Keyboard_Info.Lattice_Speed = LATTICE_MIN_SPEED;
                        Led_Point_Count = 3;
                    }
                }
                Save_Flash_Set();
            }
        } return true;
    #endif
        default:    return true; // Process all other keycodes normally
    }
}
