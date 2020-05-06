#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>

#include "stm32f4xx_conf.h"
#include "stm32f4xx.h"
/* #include "main.h" */

#include "gpio_mod.h"
#include "sys_init.h"
#include "ad_mod.h"
#include "button.h"
#include "pins.h"
#include "stasks_mod.h"
#include "timer_mod.h"
#include "spi_mod.h"
#include "uart_mod.h"
#include "controller.h"
#include "interpreter.h"

#include "usbd_cdc_core.h"
#include "usbd_usr.h"
#include "usbd_desc.h"
#include "usbd_cdc_vcp.h"

// Private variables
volatile uint32_t time_var1, time_var2;
__ALIGN_BEGIN USB_OTG_CORE_HANDLE  USB_OTG_dev __ALIGN_END;

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
  stasks_replace_current(led6_off_task, TIMING_ONCE, 0, (opmode) ? 50 : 200);
}

void led6_off_task(void) {
  LED6_off;
  stasks_replace_current(led6_on_task, TIMING_ONCE, 0, (opmode) ? 100 : 200);
}

/* blue button (user button) from board */
void user_button_fcn(void *data) {
  opmode = !opmode;
}

extern float Vbus;

void background_task(void) {
  /* Read sensor x (Sx) and put in LED(x+2) */
  if (READBIT(S1)) LED3_on; else LED3_off;
  /* if (READBIT(S2)) LED4_on; else LED4_off; */
  /* LED5 will be used to indicate Vbus > 140V */
  /* if (READBIT(S3)) LED5_on; else LED5_off; */
  if (Vbus > 140) {
    /* turn SCR on - inverse logic */
    LOW(TX485);
    LED5_on;
  }
  else {
    LED5_off;
    /* turn SCR off - inverse logic */
    HIGH(TX485);
  }
}

void serial_ticks_task(void) {
  static int i;
  char s[20];
  snprintf(s, 20, "# bobo %d\r\n", i++);
  uart_puts(s);
}

void usb_task() {
  static int iteration = 0, n;
  char s[20];
  time_var2 = 0;
  n=read(0, &s, 19);
  s[n]='\0';
  printf("str=%s\nTime:      %lu ms\n\r", s, time_var2);
  printf("Iteration: %i\n\r", iteration);
  iteration++;
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

  /* ------------- USB -------------- */
  USBD_Init(&USB_OTG_dev,
            USB_OTG_FS_CORE_ID,
            &USR_desc,
            &USBD_CDC_cb,
            &USR_cb);

  spi_init(NULL);
  stasks_init();
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
  stasks_add(buttons_task, READY, 0, 0);

  stasks_add(led6_on_task, READY, 0, 0);
  /* interpret UART commands */
  stasks_add(interpret_rx, READY, 0, 0);
  stasks_add(background_task, READY, 0, 0);
  stasks_add(serial_ticks_task, TIMING, 500, 500);

  stasks_add(usb_task, TIMING, 1000, 1000);

  /* main loop */
  systick_cntr = 0;
  for(;;) {
    /* S. O. Functions */
    stasks_run();
    flag=TIM3->CNT;
  }
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* Infinite loop */
  while (1)
  {
  }
}
#endif


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
