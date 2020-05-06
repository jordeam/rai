#ifndef _gpio_mod_h
#define _gpio_mod_h

#include <stdint.h>

#include "stm32f4xx.h"

#define GPIO_MODE_IN 0
#define GPIO_MODE_OUT 1
#define GPIO_MODE_AF 2
#define GPIO_MODE_ANALOG 3

#define GPIO_SPEED_LOW 0
#define GPIO_SPEED_MEDIUM 1
#define GPIO_SPEED_FAST 2
#define GPIO_SPEED_HIGH 3

#define GPIO_OUT_TYPE_PP 0
#define GPIO_OUT_TYPE_OD 1

#define GPIO_PORT_PUPD_NO 0
#define GPIO_PORT_PUPD_PU 1
#define GPIO_PORT_PUPD_PD 2

//void GPIO_config(GPIO_TypeDef* GPIO, int pin, uint32_t cnf, uint32_t mode);

int GPIO_state(GPIO_TypeDef* GPIO, int pin);

void GPIO_toggle(GPIO_TypeDef* GPIO, int pin, uint32_t count);

void GPIO_config_mode(GPIO_TypeDef* GPIO, int pin, uint32_t mode);

void GPIO_config_out_speed(GPIO_TypeDef* GPIO, int pin, uint32_t speed);

void GPIO_config_out_type(GPIO_TypeDef* GPIO, int pin, uint32_t type);

void GPIO_config_pupd(GPIO_TypeDef* GPIO, int pin, uint32_t pupd);

void GPIO_config_af(GPIO_TypeDef* GPIO, int pin, uint32_t mode);

#endif
