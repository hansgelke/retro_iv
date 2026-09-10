//
// Created by Hans Gelke on 08.09.2026.
//

#include "bt.h"
#include "slic.h"
#include "gpio_i2c.h"


int init_bt(void)
{
    set_slic(i2c_bus0,PERIPH_ADDR_22, MCPREG_IODIR_A,BT_DIR_A,BT_DIR);
    set_slic(i2c_bus0,PERIPH_ADDR_22, MCPREG_IODIR_B,BT_DIR_B,BT_DIR);

    set_slic_txmux(i2c_bus0,PERIPH_ADDR_22, SLIC_MUX_DIS);
    set_slic_rxmux(i2c_bus0,PERIPH_ADDR_22, SLIC_MUX_DIS);
    set_slic(i2c_bus0,PERIPH_ADDR_22,MCPREG_GPIO_A,BT_CLR_RST, BT_RST);
    set_slic(i2c_bus0,PERIPH_ADDR_22,MCPREG_GPIO_A,BT_SET_MFB, BT_MFB);

return 0;
}