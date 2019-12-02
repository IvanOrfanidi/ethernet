/**
 ******************************************************************************
 * @file    main.hpp
 * @author  Ivan Orfanidi
 * @version V1.0.0
 * @date    28-March-2018
 * @brief   
 ******************************************************************************
 * @attention
 *
 *
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_HPP
#define __MAIN_HPP

/* Driver MCU */
#include "stm32f10x.h"

/* Standart lib */
#include <iostream>
#include <time.h>

/* Driver lib */
#include "spi.hpp"
#include "ethernet/enc28j60.hpp"
#include "hd44780/hd44780.hpp"
#include "exti.hpp"
#include "gpio.hpp"

#ifdef __cplusplus

class Main {
  public:
    Main();

  private:
    void initNet();

    // Drivers interface
    Systick& _systick;

    Hd44780 _lcd;

    Enc28j60* _net;
};

extern "C" {
}

#endif

#endif