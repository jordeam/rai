#ifndef _button_h
#define _button_h

#include <stdint.h>

#include "stm32f4xx_gpio.h"

#define BUTTON_CNT_MAX 50

struct button_type {
  /* counter is incremented while button is pressed */
  uint16_t cnt;
  /* if button is already pressed */
  uint16_t flag;
  /* if button is pressed in level 0 or 1 */
  uint16_t level;
  uint16_t bit;
  GPIO_TypeDef* gpio;
  void *data;
  /* called once when pressed */
  void (*f)(void *data);
  /* the next button */
  struct button_type *next;
};

typedef struct button_type button_t;

void button_array_init(button_t * buttons_array, int num_of_buttons);
int button_add(int level, uint16_t bit, GPIO_TypeDef* gpio, void (*f)(void*), void *data);
int button_pressed(button_t * b, int switch_state);
void button_process(button_t *b);
void buttons_task(void);

#endif
