#include <stdlib.h>

#include "stasks_mod.h"
#include "button.h"

static button_t * button_array;
static int num_buttons;

void button_array_init(button_t * buttons_array, int num_of_buttons) {
  button_array = buttons_array;
  num_buttons = num_of_buttons;
  int i;
  for(i = 0; i < num_of_buttons; i++) {
    button_array[i].f = NULL;
  }
}

/**
   Add one button in the button_array
   Returns the position of button in the array if success, -1 if it is full.
*/
int button_add(int level, uint16_t bit, GPIO_TypeDef* gpio, void (*f)(void*), void *data) {
  int i;
  /* seek for last button */
  for (i = 0; i < num_buttons && button_array[i].f != NULL; i++);
  /* if it is not empty, return fail */
  if (i == num_buttons)
    return -1;
  button_array[i].cnt = 0;
  button_array[i].flag = 0;
  button_array[i].level = level;
  button_array[i].bit = bit;
  button_array[i].gpio = gpio;
  button_array[i].f = f;
  button_array[i].data = data;
  button_array[i].next = NULL;
  return i;
}

int button_pressed(button_t * b, int switch_state) {
  if (switch_state) {
    if (b->cnt == BUTTON_CNT_MAX) {
      if (b->flag == 0) {
        b->flag = 1;
        return 1;
      }
    }
    else 
      b->cnt++;
  }
  else {
    /* switch state = 0 */
      if (b->cnt == 0)
        b->flag = 0;
      else
        b->cnt--;
  }
  return 0;
}    

void button_process(button_t *b) {
  if(button_pressed(b, ((b->gpio->IDR & b->bit) != 0) == b->level) && b->f)
      b->f(b->data);
}

/**
  Process all buttons.
*/
void buttons_task(void) {
  int i, n = 0;
  for (i = 0; i < num_buttons; i++) {
    button_t * b = &button_array[i];
    if (b->f) {
      button_process(b);
      n++;
    }
  }
}
