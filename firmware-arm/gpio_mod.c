#include <stdint.h>

#include "pbit.h"
#include "stm32f4xx.h"

int GPIO_state(GPIO_TypeDef* GPIO, int pin){
  int pos = (1<<pin);
  if(GPIO->ODR & pos)
    return (1);
  else
    return (0);
}

void GPIO_toggle(GPIO_TypeDef* GPIO, int pin, uint32_t count){
  int pos = (1<<pin);
  if(GPIO->ODR & pos)
    GPIO->BSRRH|=pos;
  else
    GPIO->BSRRL|=pos;
}


void GPIO_config_mode(GPIO_TypeDef* GPIO, int pin, uint32_t mode){
  int pos;
  uint32_t reg;
  pos=pin*2;
  reg = GPIO->MODER;
  reg &= ~(3 << pos);
  reg |= (mode<<pos);
  GPIO->MODER = reg;
}

void GPIO_config_out_speed(GPIO_TypeDef* GPIO, int pin, uint32_t speed){
  int pos;
  uint32_t reg;
  pos=pin*2;
  reg = GPIO->OSPEEDR;
  reg &= ~(3 << pos);
  reg |= (speed<<pos);
  GPIO->OSPEEDR = reg;
}

void GPIO_config_out_type(GPIO_TypeDef* GPIO, int pin, uint32_t type){
  int pos;
  uint32_t reg;
  pos=pin;
  reg = GPIO->OTYPER;
  reg &= ~(1 << pos);
  reg |= (type<<pos);
  GPIO->OTYPER = reg;
}

void GPIO_config_pupd(GPIO_TypeDef* GPIO, int pin, uint32_t pupd){
  int pos;
  uint32_t reg;
  pos=pin*2;
  reg = GPIO->PUPDR;
  reg &= ~(3 << pos);
  reg |= (pupd<<pos);
  GPIO->PUPDR = reg;
}

void GPIO_config_af(GPIO_TypeDef* GPIO, int pin, uint32_t mode) {
  int pos;
  uint32_t reg;
  if (pin < 8) {
    pos = pin * 4;
    reg = GPIO->AFR[0] & ~(uint32_t)(0x0f << pos);
    reg |= (mode << pos);
    GPIO->AFR[0] = reg;
  }
  else if (pin < 16) {
    pos = (pin - 8) * 4;
    reg = GPIO->AFR[1] & ~(uint32_t)(0x0f << pos);
    reg |= (mode << pos);
    GPIO->AFR[1] = reg;
  }
}
