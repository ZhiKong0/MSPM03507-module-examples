/*
 * Copyright (c) 2021-2025, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <ti/eeprom/emulation_type_b/eeprom_emulation_type_b.h>
#include "ti_msp_dl_config.h"

/* The starting emulation address is defined in the eeprom_emulation_type_b.h
 * header file. It is 0x2000. If the example needs to change its location, it is
 * necessary to define EEPROM_EMULATION_ADDRESS at the project level or change
 * the value in the header file.
 */

/* Data and identifier used for test */
uint16_t var1_id       = 0x0011;
uint32_t var1          = 0xaaaa1234;
uint16_t var2_id       = 0x0022;
uint32_t var2          = 0xbbbb4321;
uint16_t var3_id       = 0x0033;
uint32_t var3          = 0xcccc0000;
uint16_t var4_id       = 0x0044;
uint32_t var4          = 0xddddffff;
uint16_t var_id        = 0;
uint32_t var_data      = 0;
uint32_t var_data_read = 0;

/* 64bytes data for unexpected case*/
const uint32_t UnexpectedDataArray[16] = {0xABCDEF00, 0x12345678, 0x00FEDCBA,
    0x87654321, 0xABCDEF00, 0x12345678, 0x00FEDCBA, 0x87654321, 0xABCDEF00,
    0x12345678, 0x00FEDCBA, 0x87654321, 0xABCDEF00, 0x12345678, 0x00FEDCBA,
    0x87654321};

int main(void)
{
    uint32_t EEPROMEmulationState;
    uint16_t count, num;

    SYSCFG_DL_init();

    /*
     *
     * 1. Test EEPROM_TypeB_init with previous unused data present.
     *
     */
    DL_FlashCTL_unprotectSector(
        FLASHCTL, EEPROM_EMULATION_ADDRESS, DL_FLASHCTL_REGION_SELECT_MAIN);
    DL_FlashCTL_eraseMemoryFromRAM(
        FLASHCTL, EEPROM_EMULATION_ADDRESS, DL_FLASHCTL_COMMAND_SIZE_SECTOR);

    DL_FlashCTL_unprotectSector(
        FLASHCTL, EEPROM_EMULATION_ADDRESS, DL_FLASHCTL_REGION_SELECT_MAIN);
    DL_FlashCTL_programMemoryBlockingFromRAM64WithECCGenerated(FLASHCTL,
        EEPROM_EMULATION_ADDRESS, (uint32_t *) &UnexpectedDataArray[0], 16,
        DL_FLASHCTL_REGION_SELECT_MAIN);
    __BKPT(0);
    /* In memory browser, search the address 0x00002000
     * Before EEPROM_TypeB_init, Flash is populated with the following 64-byte irrelevant data:
     * 0xABCDEF00, 0x12345678, 0x00FEDCBA, 0x87654321,
     * 0xABCDEF00, 0x12345678, 0x00FEDCBA, 0x87654321,
     * 0xABCDEF00, 0x12345678, 0x00FEDCBA, 0x87654321,
     * 0xABCDEF00, 0x12345678, 0x00FEDCBA, 0x87654321
     *
     * EEPROM_TypeB_init will erase all the irrelevant data
     * */

    EEPROMEmulationState = EEPROM_TypeB_init();
    if (EEPROMEmulationState == EEPROM_EMULATION_INIT_ERROR ||
        EEPROMEmulationState == EEPROM_EMULATION_TRANSFER_ERROR) {
        __BKPT(0);
    }
    __BKPT(0);
    /*
     * After EEPROM_TypeB_init with invalid data:
     * checkFormat() finds no valid group header, so eraseAllGroups() is called.
     * Returns EEPROM_EMULATION_INIT_OK_ALL_ERASE
     * The group header is now in the confirmed erased state (0xFF00FFFF 0xFFFFFFFF)
     * global variables after init:
     *      gActiveGroupNum = 1
     *      gActiveDataItemNum = 0
     *      gEEPROMTypeBSearchFlag = 0
     *      gEEPROMTypeBEraseFlag = 0
     * */

    /*
     *
     * 2. Test write operations - data accumulates in group without transfer
     *
     */
    EEPROMEmulationState = EEPROM_TypeB_write(var1_id, var1);
    if (EEPROMEmulationState != EEPROM_EMULATION_WRITE_OK) {
        __BKPT(0);
    }
    EEPROMEmulationState = EEPROM_TypeB_write(var2_id, var2);
    if (EEPROMEmulationState != EEPROM_EMULATION_WRITE_OK) {
        __BKPT(0);
    }
    EEPROMEmulationState = EEPROM_TypeB_write(var3_id, var3);
    if (EEPROMEmulationState != EEPROM_EMULATION_WRITE_OK) {
        __BKPT(0);
    }
    EEPROMEmulationState = EEPROM_TypeB_write(var3_id, var4);
    if (EEPROMEmulationState != EEPROM_EMULATION_WRITE_OK) {
        __BKPT(0);
    }
    __BKPT(0);
    /* After 4 EEPROM_TypeB_write calls:
     * 4 data items written to group1, identifiers: 0x0011, 0x0022, 0x0033
     * (0x0033 written twice - latest value is 0xDDDDFFFF)
     *
     * Flash state:
     *      group1 (0x00002000):
     *          Header: 0x00000000, 0xFFFFFFFF (Active state)
     *          Item 0: ID=0x0011, data=0xAAAA1234
     *          Item 1: ID=0x0022, data=0xBBBB4321
     *          Item 2: ID=0x0033, data=0xCCCC0000
     *          Item 3: ID=0x0033, data=0xDDDDFFFF (latest for this ID)
     *      group2, group3: All erased (0xFF...)
     *
     * global variables:
     *      gActiveGroupNum = 1
     *      gActiveDataItemNum = 4
     *      gEEPROMTypeBSearchFlag = 0
     *      gEEPROMTypeBEraseFlag = 0
     * */

    EEPROMEmulationState = EEPROM_TypeB_init();
    if (EEPROMEmulationState == EEPROM_EMULATION_INIT_ERROR ||
        EEPROMEmulationState == EEPROM_EMULATION_TRANSFER_ERROR) {
        __BKPT(0);
    }
    __BKPT(0);
    /* After EEPROM_TypeB_init on existing data:
     * checkFormat() finds group1 with Active header
     * updateDataItemNum(1) scans from end and finds 4 items (not full)
     * Group is NOT full (max 255 items), so NO transfer occurs
     * Init resumes at item slot 4, ready for next write
     * Returns EEPROM_EMULATION_INIT_OK
     *
     * Flash state UNCHANGED - no transfer:
     *      group1: Still contains same 4 items at slots 0-3
     *      group2, group3: Still all erased
     *
     * global variables after init:
     *      gActiveGroupNum = 1
     *      gActiveDataItemNum = 4
     *      gEEPROMTypeBSearchFlag = 0
     *      gEEPROMTypeBEraseFlag = 0
     * */

    /*
     *
     * 3. Test accumulating more data without triggering transfer
     *
     */
    for (count = 0; count < 62; count++) {
        for (num = 0; num < 4; num++) {
            var_id               = num;
            EEPROMEmulationState = EEPROM_TypeB_write(var_id, var_data);
            if (EEPROMEmulationState != EEPROM_EMULATION_WRITE_OK) {
                __BKPT(0);
            }
        }
        var_data++;
    }
    __BKPT(0);
    /* After 248 more EEPROM_TypeB_write calls (62 iterations × 4 items):
     * Total items in group1: 4 (from Test 2) + 248 = 252 items
     * Still under 255 max, so NO transfer triggered during writes
     *
     * Identifiers written: 0x0000-0x0003 (var_data 0x00 to 0x3D)
     * Latest values for each ID: var_data = 0x3D (61 decimal)
     *
     * Flash state:
     *      group1 (0x00002000):
     *          Header: 0x00000000, 0xFFFFFFFF (Active)
     *          Item 0-3: Original items from Test 2
     *          Item 4-251: New items from this loop
     *      group2, group3: All erased
     *
     * global variables:
     *      gActiveGroupNum = 1
     *      gActiveDataItemNum = 252
     *      gEEPROMTypeBSearchFlag = 0
     *      gEEPROMTypeBEraseFlag = 0
     * */

    /*
     *
     * 4. Test transfer - fill group1 to trigger automatic transfer
     *
     */
    for (num = 0; num < 3; num++) {
        var_id               = num;
        EEPROMEmulationState = EEPROM_TypeB_write(var_id, 0xFFFFFFFF);
        if (EEPROMEmulationState != EEPROM_EMULATION_WRITE_OK) {
            __BKPT(0);
        }
    }
    __BKPT(0);
    /* After 3 more writes (items 252-254), group1 now has 255 items (full).
     * gActiveDataItemNum = 255
     * */

    EEPROMEmulationState = EEPROM_TypeB_write(0, 0x99999999);
    if (EEPROMEmulationState != EEPROM_EMULATION_WRITE_OK) {
        __BKPT(0);
    }
    __BKPT(0);
    /* The 255th write triggers transfer:
     * - gActiveDataItemNum becomes 255, which equals EEPROM_EMULATION_ACTIVE_DATAITEM_NUM_MAX
     * - transferDataItem(1) is called automatically
     * - group2 marked "Receiving" state (0x0000FFFF / 0xFFFFFFFF)
     * - Latest data for each unique ID transferred to group2 (deduplication)
     *   Only 4 unique IDs: 0x0000, 0x0001, 0x0002, 0x0003
     * - group1 marked "Erasing" state (0x00000000 / 0x00000000)
     * - gEEPROMTypeBEraseFlag = 1
     * - gActiveGroupNum = 2, gActiveDataItemNum = 4
     *
     * Flash state after transfer:
     *      group1: Marked Erasing, waiting to be erased
     *      group2: Active with 4 items (latest values for each ID)
     *      group3: All erased
     * */

    if (gEEPROMTypeBEraseFlag == 1) {
        EEPROM_TypeB_eraseGroup();
        __BKPT(0);
        /* eraseGroup() finds group1 marked Erasing and erases it.
         * After erase, group1 is marked with Erased header (0xFF00FFFF / 0xFFFFFFFF)
         *
         * Flash state after erase:
         *      group1: Marked Erased
         *      group2: Active with 4 items
         *      group3: All erased (Unknown state)
         *
         * global variables:
         *      gActiveGroupNum = 2
         *      gActiveDataItemNum = 4
         *      gEEPROMTypeBSearchFlag = 0
         *      gEEPROMTypeBEraseFlag = 0 (cleared by eraseGroup)
         * */
    }

    /*
     *
     * 5. Test continuous write with multiple transfers and cycles
     *
     */

    uint32_t outerCounter = 0;
    for (outerCounter = 0; outerCounter < 2; outerCounter++) {
        for (count = 0; count < 31; count++) {
            for (num = 0; num < 4; num++) {
                var_id               = num;
                EEPROMEmulationState = EEPROM_TypeB_write(var_id, var_data);
                if (EEPROMEmulationState != EEPROM_EMULATION_WRITE_OK) {
                    __BKPT(0);
                }
            }
            var_data++;
        }
        __BKPT(0);
        /* Each outer loop iteration: 31 inner loops × 4 writes = 124 writes
         * Starting from gActiveGroupNum = 2, gActiveDataItemNum = 4
         *
         * Outer loop 1:
         *   - Writes 124 items to group2 (4 + 124 = 128 items, under 255 max)
         *   - var_data incremented from 0x3D to 0x5C
         *   - No transfer triggered
         *
         * Outer loop 2:
         *   - Writes 124 more items to group2 (128 + 124 = 252 total, still under 255)
         *   - But after ~3 writes, gActiveDataItemNum reaches 255
         *   - Transfer triggered automatically: group2 → group3
         *   - group2 marked Erasing, gEEPROMTypeBEraseFlag = 1
         *   - Remaining writes continue in group3 (newly Active)
         *   - var_data incremented from 0x5C to 0x7D
         *
         * Result: Data cycles through groups, latest values for each ID preserved
         * */

        if (gEEPROMTypeBEraseFlag == 1) {
            /* Group marked Erasing after transfer. Erase it before groups fill up */
            EEPROM_TypeB_eraseGroup();
            __BKPT(0);
        }
    }

    /*
     *
     * 6. Test fresh start and read operations
     *
     */
    EEPROM_TypeB_eraseAllGroups();
    __BKPT(0);
    /* All groups erased and marked with Erased header */

    EEPROMEmulationState = EEPROM_TypeB_init();
    __BKPT(0);
    /* Init on fresh erased state. Returns EEPROM_EMULATION_INIT_OK_ALL_ERASE
     * gActiveGroupNum = 1, gActiveDataItemNum = 0 */

    EEPROMEmulationState = EEPROM_TypeB_write(var1_id, 0x11111111);
    __BKPT(0);
    /* Write var1_id (0x0011) with value 0x11111111 */

    EEPROMEmulationState = EEPROM_TypeB_write(var1_id, 0x22222222);
    __BKPT(0);
    /* Write var1_id again (update) with value 0x22222222 */

    EEPROMEmulationState = EEPROM_TypeB_write(var2_id, 0x33333333);
    __BKPT(0);
    /* Write var2_id (0x0022) with value 0x33333333 */

    EEPROMEmulationState = EEPROM_TypeB_write(var2_id, 0x44444444);
    __BKPT(0);
    /* Write var2_id again (update) with value 0x44444444 */

    var_data_read = EEPROM_TypeB_readDataItem(var1_id);
    __BKPT(0);
    /* Read var1_id. Check var_data_read (should be 0x22222222)
     * and gEEPROMTypeBSearchFlag (should be 1 if found) */

    var_data_read = EEPROM_TypeB_readDataItem(var2_id);
    __BKPT(0);
    /* Read var2_id. Check var_data_read (should be 0x44444444)
     * and gEEPROMTypeBSearchFlag (should be 1 if found) */

    while (1) {
        __WFI();
    }
}
