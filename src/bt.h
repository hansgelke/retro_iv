//
// Created by Hans Gelke on 08.09.2026.
//

#ifndef SRC_BT_H
#define SRC_BT_H



#include "stdint.h"
#include <zephyr/device.h>

#define BT_CLR_RST (0x10) // Clear BT reset
#define BT_RST (0x10) // Mask for Bluetooth reset bit
#define BT_SET_MFB (0x20) // Set MFB pin
#define BT_MFB (0x20) // Mask for MFB pin

#define BT_DIR_A (0x00) //All outputs
#define BT_DIR_B (0x00) //All outputs

#define BT_DIR (0xff) //Unmask all

int init_bt(void);

#endif //SRC_BT_H