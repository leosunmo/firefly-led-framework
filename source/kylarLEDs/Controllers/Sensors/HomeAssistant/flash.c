#include <stdio.h>
#include <string.h>
#include "flash.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "pico/time.h"
#include "pico/multicore.h"

device_info_t device_info;

void print_flash_sector(uint32_t offset) {
    const uint8_t *flash_data = (const uint8_t *)(XIP_BASE + offset);

    printf("Flash sector at offset 0x%08X:\n", offset);
    for (int i = 0; i < FLASH_SECTOR_SIZE; i += 16) {
        printf("0x%08X: ", offset + i);
        for (int j = 0; j < 16; j++) {
            printf("%02X ", flash_data[i + j]);
        }
        printf(" | ");
        for (int j = 0; j < 16; j++) {
            uint8_t ch = flash_data[i + j];
            if (ch >= 32 && ch <= 126) {
                printf("%c", ch); // Print ASCII character if printable
            } else {
                printf(".");
            }
        }
        printf("\n");
    }
}

// Function to save device information to flash memory
void flash_write_device_info() {
    //printf("Flash before: ");
    //print_flash_sector(FLASH_TARGET_OFFSET);
    uint8_t flash_data[FLASH_SECTOR_SIZE];
    memset(flash_data, 0xFF, FLASH_SECTOR_SIZE);
    device_info.is_valid = DATA_MARKER;
    memcpy(flash_data, &device_info, sizeof(device_info_t));

    uint32_t ints = save_and_disable_interrupts();
    multicore_lockout_start_blocking();
    flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(FLASH_TARGET_OFFSET, flash_data, FLASH_SECTOR_SIZE);
    multicore_lockout_end_blocking();
    restore_interrupts(ints);

    printf("new device_info.entity %s\n\r", device_info.entity);
    printf("new device_info.name %s\n\r", device_info.name);

    //printf("Flash after: ");
    //print_flash_sector(FLASH_TARGET_OFFSET);
}

// Function to read device information from flash memory
void flash_read_device_info() {
    const uint8_t *flash_data = (const uint8_t *)(XIP_BASE + FLASH_TARGET_OFFSET);
    memcpy(&device_info, flash_data, sizeof(device_info_t));
    //printf("Flash read:");
    //print_flash_sector(FLASH_TARGET_OFFSET);

    // If the flash data is invalid, use default values
    if( DATA_MARKER != device_info.is_valid ) {
        snprintf(device_info.name, sizeof(device_info.name), DEFAULT_DEVICE_NAME);
        snprintf(device_info.entity, sizeof(device_info.entity), DEFAULT_ENTITY_NAME);
    }
    printf("device_info.entity %s\n\r", device_info.entity);
    printf("device_info.name %s\n\r", device_info.name);
}
