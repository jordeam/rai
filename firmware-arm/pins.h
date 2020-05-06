#ifndef _pins_h
#define _pins_h

#include "pbit.h"

#define USER_BUTTON GPIOA,0
#define USER_BUTTON_pressed (READ(USER_BUTTON) != 0)

/* Discovery LEDS */

/* ENCODERS */
#define ENC1A GPIOD,12
#define ENC1B GPIOB,7
#define ENC1R GPIOE,4

/* HALL SENSORS */
#define S1   GPIOD,0
#define S2   GPIOD,1
#define S3   GPIOD,2

/* INT */
#define INT0 GPIOD,3

/* LED3 */
#define LED3 GPIOD,13
#define LED3_on HIGH(LED3)
#define LED3_off LOW(LED3)

/* Used by ENCA */
/* /\* LED4 *\/ */
/* #define LED4 GPIOD,12 */
/* #define LED4_on HIGH(LED4) */
/* #define LED4_off LOW(LED4) */

/* LED5 */
#define LED5 GPIOD,14
#define LED5_on HIGH(LED5)
#define LED5_off LOW(LED5)

/* LED6 */
#define LED6 GPIOD,15
#define LED6_on HIGH(LED6)
#define LED6_off LOW(LED6)

/* SPI 3 */
#define NSS GPIOA,15
#define MOSI GPIOB,5
#define MISO GPIOB,4
#define SCK GPIOC,10

/* RS485 */
#define RE485 GPIOD,7
#define DE485 GPIOD,10
#define TX485 GPIOA,2
#define RX485 GPIOD,6

/* ENALL# */
/* Modify board: using DUPLICATED RE485 in PD11 */
#define ENALL GPIOD,11

/* PWM */
#define PWM1  GPIOE,9
#define PWM1N GPIOE,8
#define PWM2  GPIOE,11
#define PWM2N GPIOE,10
#define PWM3  GPIOE,13
#define PWM3N GPIOE,12
#define PWM4  GPIOC,8
#define PWM4N GPIOB,1

/* SIGI pin, used to signal beginning and ending of 1ms interval */
#define SIGI GPIOD,4

/* SIGAD pin, used to signal beginning and ending of AD interrupt */
#define SIGAD GPIOD,5

/* control flag when in control ? TODO */
#define SIGCONTROL GPIOB,6

/* AD channels */
/* ADC channels: 0, 3, 7, 8, 11, 12, 14, 15 */
#define ADC_IN1 GPIOA,1
#define ADC_IN3 GPIOA,3
#define ADC_IN7 GPIOA,7
#define ADC_IN8 GPIOB,0
#define ADC_IN11 GPIOC,1
#define ADC_IN12 GPIOC,2
#define ADC_IN14 GPIOC,4
#define ADC_IN15 GPIOC,5

/* DA channels */
#define DA1 GPIOA,4
#define DA2 GPIOA,5

/* RS 232 USART 3 */
#define TX232 GPIOD,8
#define RX232 GPIOD,9

#define RELAY GPIOB,8

void pins_init(void);

#endif
