#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>

#include "stm32f4xx.h"

#include "gpio_mod.h"
#include "sys_init.h"
#include "ad_mod.h"
#include "button.h"
#include "pins.h"
#include "cotask.h"
#include "timer_mod.h"
#include "spi_mod.h"
#include "uart_mod.h"
#include "controller.h"
#include "interpreter_stack.h"

// Private variables
volatile uint32_t time_var1, time_var2;

// Private function prototypes
void Delay(volatile uint32_t nCount);
void init();
void calculation_test();

//target remote | openocd -c "gdb_port pipe; log_output openocd.log" -f openocd.cfg

/* buttons */
/* it can be malloced, but malloc uses a lot of RAM */
#define NUMBUTTONS 1
button_t buttons[NUMBUTTONS];

/* If opmode = 0 => inverter bridge is off, else is on. */
int opmode = 0;

/* TODO: to know what for */
uint32_t flag=0xFFFF;
float    fltflag=0;
extern float speed;

void _init(){}

void led6_off_task(void);

void led6_on_task(void) {
  LED6_on;
  cotask_replace_current(led6_off_task, TIMING_ONCE, 0, (opmode) ? 50 : 200);
}

void led6_off_task(void) {
  LED6_off;
  cotask_replace_current(led6_on_task, TIMING_ONCE, 0, (opmode) ? 100 : 200);
}

/* blue button (user button) from board */
void user_button_fcn(void *data) {
  opmode = !opmode;
}

void serial_ticks_task(void) {
  static int i;
  char s[20];
  snprintf(s, 20, "# bobo %d\r\n", i++);
  uart_puts(s);
}

/*
 * Called from systick handler
 */
void timing_handler() {
  if (time_var1) {
    time_var1--;
  }

  time_var2++;
}

/*
 * Delay a number of systick cycles (1ms)
 */
void Delay(volatile uint32_t nCount) {
  time_var1 = nCount;
  while(time_var1){};
}

int main(void)
{
  /* volatile int d; */
  /* uint32_t reg; */

  /* RCC init */
  /* Clock Enable */
  RCC->APB1ENR |= RCC_APB1ENR_PWREN;

  /* Enable the clock for GPIO port A, B, C, D and E */
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN | RCC_AHB1ENR_GPIOCEN | RCC_AHB1ENR_GPIODEN | RCC_AHB1ENR_GPIOEEN;

  /* Enable SYSCFG */
  RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

  SysTick_Config(SystemCoreClock / 1000);

  pins_init();

  spi_init(NULL);
  cotask_init();
  LSE_init();
  DA_init();
  AD_init();
  /* PWM in TIMER 1 */
  timer1_set_fcn(controller_i_pi);
  TIM1_init();
  /* Encoder in Timer 4 */
  TIM4_init();
  /* Speed controller in TIMER 3 -> TODO: put in timer 1 */
  timer3_set_fcn(NULL);
  TIM3_init();
  
  uart_init(/* TODO: this is actualy 9600bps */ 38400 * 3.55, rx_hook); \

  /* External interrupt for INT0 from slave SPI */
  NVIC_SetPriority(EXTI3_IRQn, 3);
  NVIC_EnableIRQ(EXTI3_IRQn);

  button_array_init(buttons, NUMBUTTONS);
  button_add(1, PIN(USER_BUTTON), PORT(USER_BUTTON), user_button_fcn, (void*) 500);
  cotask_add(buttons_task, READY, 0, 0);

  cotask_add(led6_on_task, READY, 0, 0);
  /* interpret UART commands */
  cotask_add(interpret_rx, READY, 0, 0);
  cotask_add(serial_ticks_task, TIMING, 500, 500);

  /* main loop */
  systick_cntr = 0;
  for(;;) {
    /* S. O. Functions */
    cotask_run();
    flag=TIM3->CNT;
  }
}

