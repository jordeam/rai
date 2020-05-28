#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdint.h>
#include <assert.h>

/* interval between interruptions in us */
#define INTERVAL 1000

/* integrative constant for the control of interval between interrupts */
#define ki 0.1

static void interrupt_1ms(int delta_t) {
  /* ARM code executed each 1ms goes here*/
  
}

void * interval_code(int *max_counter) {
  int c = *max_counter;
  struct timespec tr, last_tp, tp, last_ti, ti;
  uint32_t delta_t, delta_ti;
  int32_t uk = INTERVAL, delta_uk;
  
  clock_getres(CLOCK_REALTIME, &tr);
  clock_gettime(CLOCK_REALTIME, &last_tp);
  clock_gettime(CLOCK_REALTIME, &last_ti);
  printf("tr{tv_sec=%ld,tv_nsec=%ld}\n", tr.tv_sec, tr.tv_nsec);
  for (;;) {
    usleep(uk);
    clock_gettime(CLOCK_REALTIME, &tp);
    delta_t = ((tp.tv_sec - last_tp.tv_sec) * 1000000000 + tp.tv_nsec - last_tp.tv_nsec) / 1000;
    last_tp.tv_sec = tp.tv_sec;
    last_tp.tv_nsec = tp.tv_nsec;
    /* PI control to adjust time*/
    delta_uk = ki * (INTERVAL - (int)delta_t);
    /* saturate delta_uk */
    if (delta_uk > 200)
      delta_uk = 200;
    else if (delta_uk < -200)
      delta_uk = -200;
    uk += delta_uk;
    if (uk < 0)
      uk = 0;
 
    /* calls the interrupt */
    interrupt_1ms(delta_t);

    /* some log */
    if (--c == 0) {
      c = *max_counter;
      clock_gettime(CLOCK_REALTIME, &ti);
      delta_ti = ((ti.tv_sec - last_ti.tv_sec) * 1000000000 + ti.tv_nsec - last_ti.tv_nsec) / 1000;
      last_ti.tv_sec = ti.tv_sec;
      last_ti.tv_nsec = ti.tv_nsec;
      
      printf("Tick 1s: uk=%d delta_uk=%d ti = %ld.%09ld last_ti = %ld.%09ld delta_ti=%dus\n", uk, delta_uk, ti.tv_sec, ti.tv_nsec, last_ti.tv_sec, ti.tv_nsec, delta_ti);
    }
  }
  return NULL;
}

