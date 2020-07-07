#include "stm32f4xx.h"
#include "cotask.h"
#include "pins.h"

#define  LSE_FAIL_FLAG  0x80
#define  LSE_PASS_FLAG  0x100

u32 LSE_Delay = 0;
int LSE_init_failed = 1;

void LSE_activate_task(void);

/**
   Enables LSE.
   Sets LSE_init_failed if fail, 0 if success
*/
int LSE_init(void) {
  /* Enable access to the backup register => LSE can be enabled */
  asm("@ begin here");
  PWR->CR = PWR_CR_DBP | (PWR->CR & (~(PWR_CR_CSBF | PWR_CR_CWUF)));
  /* PWR_BackupAccessCmd(ENABLE); */
  asm("@ end");
  

#define RCC_LSE_OFF                      ((uint8_t)0x00)
#define RCC_LSE_ON                       ((uint8_t)0x01)
#define RCC_OFFSET                (RCC_BASE - PERIPH_BASE)
#define BDCR_OFFSET               (RCC_OFFSET + 0x20)

  /* Enable LSE (Low Speed External Oscillation) */
  /* Reset LSEON bit */
   *(__IO uint8_t *) (PERIPH_BASE + BDCR_OFFSET) = RCC_LSE_OFF;
  /* Reset LSEBYP bit */
  *(__IO uint8_t *) (PERIPH_BASE + BDCR_OFFSET) = RCC_LSE_OFF;
  /* Set LSEON bit */
  *(__IO uint8_t *) (PERIPH_BASE + BDCR_OFFSET) = RCC_LSE_ON;

  cotask_add(LSE_activate_task, TIMING, 500, 500);
  return 1;
}

/* 
   Check whether LSE is ready, within 4 seconds of timeout
*/
void LSE_activate_task(void) {
  if (LSE_Delay < LSE_FAIL_FLAG) {
    LSE_Delay += 0x10;
    /* if(RCC_GetFlagStatus(RCC_FLAG_LSERDY) != RESET) { */
    if (RCC->BDCR & RCC_BDCR_LSERDY) {
      /* Set flag: LSE PASS */
      LSE_Delay |= LSE_PASS_FLAG;
      /* Disable LSE */
      /* RCC_LSEConfig(RCC_LSE_OFF); */
      *(__IO uint8_t *) (PERIPH_BASE + BDCR_OFFSET) = RCC_LSE_OFF;
      LSE_init_failed = 0;
      cotask_killme();
    }        
  }
  else if (LSE_Delay >= LSE_FAIL_FLAG) {          
    /* if(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET) { */
    if ((RCC->BDCR & RCC_BDCR_LSERDY) == 0) {
      /* Set flag: LSE FAIL */
      LSE_Delay |= LSE_FAIL_FLAG;
    }        
    /* Disable LSE */
    /* RCC_LSEConfig(RCC_LSE_OFF); */
    *(__IO uint8_t *) (PERIPH_BASE + BDCR_OFFSET) = RCC_LSE_OFF;
    LSE_init_failed = 1;
    cotask_killme();
  }
}
