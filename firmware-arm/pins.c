/* #include <stdlib.h> */
#include <stdint.h>

#include "stm32f4xx.h"
#include "gpio_mod.h"
#include "pins.h"

void pins_init(void) {
  /* LD3 F4DISCOVERY board */
  GPIO_config_mode(LED3,GPIO_MODE_OUT);
  GPIO_config_out_type(LED3,GPIO_OUT_TYPE_PP);
  GPIO_config_out_speed(LED3,GPIO_SPEED_HIGH);
  GPIO_config_pupd(LED3,GPIO_PORT_PUPD_PU);
  LED3_off;

  /* LD4 F4DISCOVERY board */
  /* GPIO_config_mode(LED4,GPIO_MODE_OUT); */
  /* GPIO_config_out_type(LED4,GPIO_OUT_TYPE_PP); */
  /* GPIO_config_out_speed(LED4,GPIO_SPEED_HIGH); */
  /* GPIO_config_pupd(LED4,GPIO_PORT_PUPD_PU); */
  /* LED4_off; */
  
  /* LD5 F4DISCOVERY board */
  GPIO_config_mode(LED5,GPIO_MODE_OUT);
  GPIO_config_out_type(LED5,GPIO_OUT_TYPE_PP);
  GPIO_config_out_speed(LED5,GPIO_SPEED_HIGH);
  GPIO_config_pupd(LED5,GPIO_PORT_PUPD_PU);
  LED5_off;

  /* LD6 F4DISCOVERY board */
  GPIO_config_mode(LED6,GPIO_MODE_OUT);
  GPIO_config_out_type(LED6,GPIO_OUT_TYPE_PP);
  GPIO_config_out_speed(LED6,GPIO_SPEED_HIGH);
  GPIO_config_pupd(LED6,GPIO_PORT_PUPD_PU);
  LED6_off;

  /* USIN RS485 as output and not as UART */
  /* RE485 */
  GPIO_config_mode(RE485, GPIO_MODE_OUT);
  GPIO_config_out_type(RE485, GPIO_OUT_TYPE_PP);
  GPIO_config_out_speed(RE485 ,GPIO_SPEED_HIGH);
  GPIO_config_pupd(RE485, GPIO_PORT_PUPD_PU);
  LOW(RE485);

  /* DE485 */
  GPIO_config_mode(DE485, GPIO_MODE_OUT);
  GPIO_config_out_type(DE485, GPIO_OUT_TYPE_PP);
  GPIO_config_out_speed(DE485, GPIO_SPEED_HIGH);
  GPIO_config_pupd(DE485, GPIO_PORT_PUPD_PU);
  LOW(DE485);

  /* TX485 */
  GPIO_config_mode(TX485, GPIO_MODE_OUT);
  GPIO_config_out_type(TX485, GPIO_OUT_TYPE_PP);
  GPIO_config_out_speed(TX485, GPIO_SPEED_HIGH);
  GPIO_config_pupd(TX485, GPIO_PORT_PUPD_PU);
  /* turn SCR off - inverse logic */
  HIGH(TX485);

  /* enable RS485 driver output */
  HIGH(DE485);

  /* ENALL# */
  /* Modify board: using DUPLICATED RE485 in PD11 */
  HIGH(ENALL);
  GPIO_config_mode(ENALL, GPIO_MODE_OUT);
  GPIO_config_out_type(ENALL, GPIO_OUT_TYPE_PP);
  GPIO_config_out_speed(ENALL, GPIO_SPEED_HIGH);
  GPIO_config_pupd(ENALL, GPIO_PORT_PUPD_PU);
  HIGH(ENALL);

  /* User button */
  /* Configure vldiscovery User Button pin as input floating */
  //GPIO_config(USER_BUTTON, /* In floating */ 1, /* input */ 0);
  GPIO_config_mode(USER_BUTTON,GPIO_MODE_IN);
  GPIO_config_pupd(USER_BUTTON,GPIO_PORT_PUPD_NO);

  /* SIGI signaling bit */
  //GPIO_config(SIGI, 0, 3);
  GPIO_config_mode(SIGI,GPIO_MODE_OUT);
  GPIO_config_out_type(SIGI,GPIO_OUT_TYPE_PP);
  GPIO_config_out_speed(SIGI,GPIO_SPEED_HIGH);
  GPIO_config_pupd(SIGI,GPIO_PORT_PUPD_PU);
  LOW(SIGI);

  /* SIGAD signaling bit */
  //GPIO_config(SIGAD, 0, 3);
  GPIO_config_mode(SIGAD,GPIO_MODE_OUT);
  GPIO_config_out_type(SIGAD,GPIO_OUT_TYPE_PP);
  GPIO_config_out_speed(SIGAD,GPIO_SPEED_HIGH);
  GPIO_config_pupd(SIGAD,GPIO_PORT_PUPD_PU);
  LOW(SIGAD);

  /* RELAY */
  GPIO_config_mode(RELAY, GPIO_MODE_OUT);
  GPIO_config_out_type(RELAY, GPIO_OUT_TYPE_PP);
  GPIO_config_out_speed(RELAY, GPIO_SPEED_HIGH);
  GPIO_config_pupd(RELAY, GPIO_PORT_PUPD_PU);
  LOW(RELAY);
 
  /* SIGCONTROL */
  GPIO_config_mode(SIGCONTROL, GPIO_MODE_OUT);
  GPIO_config_out_type(SIGCONTROL, GPIO_OUT_TYPE_PP);
  GPIO_config_out_speed(SIGCONTROL, GPIO_SPEED_HIGH);
  GPIO_config_pupd(SIGCONTROL, GPIO_PORT_PUPD_NO);

  /* INT0 @ PD3 */
  /* EXTI3_IRQn LINE 3 CONFIGURATION*/
  GPIO_config_mode(INT0, GPIO_MODE_IN);
  GPIO_config_pupd(INT0, GPIO_PORT_PUPD_PD);
  SYSCFG->EXTICR[0] = SYSCFG_EXTICR1_EXTI3_PC;
  /* Mask register */
  EXTI->IMR |= EXTI_IMR_MR3;
  /* rising edge */
  EXTI->RTSR |= EXTI_RTSR_TR3;

  /* HALL SENSORS INPUTS */
  GPIO_config_mode(S1,GPIO_MODE_IN);
  GPIO_config_pupd(S1,GPIO_PORT_PUPD_NO);
  GPIO_config_mode(S2,GPIO_MODE_IN);
  GPIO_config_pupd(S2,GPIO_PORT_PUPD_NO);
  GPIO_config_mode(S3,GPIO_MODE_IN);
  GPIO_config_pupd(S3,GPIO_PORT_PUPD_NO);

  /* DA OUTPUTS */
  GPIO_config_mode(DA1,GPIO_MODE_ANALOG);
  GPIO_config_pupd(DA1,GPIO_PORT_PUPD_NO);
  GPIO_config_mode(DA2,GPIO_MODE_ANALOG);
  GPIO_config_pupd(DA2,GPIO_PORT_PUPD_NO);

  /* PWM TIMER 1 Port E */
  /* All pins must be put in low state until CPU is ready for PWM */
  GPIO_config_mode(PWM1, GPIO_MODE_OUT);
  GPIO_config_out_type(PWM1, GPIO_OUT_TYPE_PP);
  GPIO_config_out_speed(PWM1, GPIO_SPEED_HIGH);
  GPIO_config_pupd(PWM1,GPIO_PORT_PUPD_PU);
  LOW(PWM1);
  
  GPIO_config_mode(PWM1N, GPIO_MODE_OUT);
  GPIO_config_out_type(PWM1N, GPIO_OUT_TYPE_PP);
  GPIO_config_out_speed(PWM1N, GPIO_SPEED_HIGH);
  GPIO_config_pupd(PWM1N,GPIO_PORT_PUPD_PU);
  LOW(PWM1N);

  GPIO_config_mode(PWM2, GPIO_MODE_OUT);
  GPIO_config_out_type(PWM2, GPIO_OUT_TYPE_PP);
  GPIO_config_out_speed(PWM2, GPIO_SPEED_HIGH);
  GPIO_config_pupd(PWM2,GPIO_PORT_PUPD_PU);
  LOW(PWM2);

  GPIO_config_mode(PWM2N, GPIO_MODE_OUT);
  GPIO_config_out_type(PWM2N, GPIO_OUT_TYPE_PP);
  GPIO_config_out_speed(PWM2N, GPIO_SPEED_HIGH);
  GPIO_config_pupd(PWM2N,GPIO_PORT_PUPD_PU);
  LOW(PWM2N);

  GPIO_config_mode(PWM3, GPIO_MODE_OUT);
  GPIO_config_out_type(PWM3, GPIO_OUT_TYPE_PP);
  GPIO_config_out_speed(PWM3, GPIO_SPEED_HIGH);
  GPIO_config_pupd(PWM3,GPIO_PORT_PUPD_PU);
  LOW(PWM3);

  GPIO_config_mode(PWM3N, GPIO_MODE_OUT);
  GPIO_config_out_type(PWM3N, GPIO_OUT_TYPE_PP);
  GPIO_config_out_speed(PWM3N, GPIO_SPEED_HIGH);
  GPIO_config_pupd(PWM3N,GPIO_PORT_PUPD_PU);
  LOW(PWM3N);

  GPIO_config_mode(PWM4, GPIO_MODE_OUT);
  GPIO_config_out_type(PWM4, GPIO_OUT_TYPE_PP);
  GPIO_config_out_speed(PWM4, GPIO_SPEED_HIGH);
  GPIO_config_pupd(PWM4,GPIO_PORT_PUPD_PU);
  LOW(PWM4);

  GPIO_config_mode(PWM4N, GPIO_MODE_OUT);
  GPIO_config_out_type(PWM4N, GPIO_OUT_TYPE_PP);
  GPIO_config_out_speed(PWM4N, GPIO_SPEED_HIGH);
  GPIO_config_pupd(PWM4N,GPIO_PORT_PUPD_PU);
  LOW(PWM4N);

  /* All phases are off: turn on ENALL */
  LOW(ENALL);
}
