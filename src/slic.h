//
// Created by Hans Gelke on 22.06.2026.
//

#ifndef SRC_SLIC_H
#define SRC_SLIC_H

#define SLIC_LPSB (0x00) // Low Power Standby
#define SLIC_FORA (0x10) // Forward active
#define SLIC_URNG (0x20) // unballanced ringing
#define SLIC_REVA (0x30)//Reverse active
#define SLIC_RING (0x40)//Ringing
#define SLIC_LOOP (0x50)// forward loop
#define SLIC_TIPO (0x60) //Tip Open
#define SLIC_POWD (0x70) //Power denial
#define SLIC_MODE (0x70)

#define BATSEL (0x10)
#define BATHI (0x10)
#define BATLO (0x00)

#define SLIC_DIR_B (0xc0)
#define SLIC_DIR_A (0x00)
#define SLIC_DIR (0xff)

#define SLIC_MUX (0x0f)
#define SLIC_MUX_DIS (0x80)

#define SLIC_E0 (0x80)
#define SLIC_E0_MASK (0x80)

#define SWC_N (0x20)
#define SWC_N_MASK (0x20)

#endif //SRC_SLIC_H

int init_slic(void);

int set_slic_mode(const struct device* bus,
             uint8_t device, //Is the subscriber number
             uint8_t mode);

int set_slic(const struct device *bus,
                    uint8_t dev_addr,
                    uint8_t reg_addr,
                    uint8_t mode,//Is the subscriber number
                    uint8_t mask);  //Is the mode to be set

int set_slic_txmux(const struct device* bus,
             uint8_t device, //Is the subscriber number
             uint8_t val);

int set_slic_rxmux(const struct device* bus,
             uint8_t device, //Is the subscriber number
             uint8_t val);