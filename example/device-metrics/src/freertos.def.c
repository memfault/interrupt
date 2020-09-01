// the FreeRTOS heap
uint8_t ucHeap[configTOTAL_HEAP_SIZE];

void vAssertCalled(const char *file, int line) {
  EXAMPLE_ASSERT(0);
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
  EXAMPLE_ASSERT(0);
}

void *_sbrk(int incr) {
  static int s_call_count = 0;
  EXAMPLE_ASSERT(s_call_count == 0);
  s_call_count++;

  static uint8_t danger_zone[2048];
  return danger_zone;
}

static void vPortEnableVFP( void ){
#if defined(__ARM_ARCH) && (__ARM_ARCH != 6) || defined(__TI_ARM__)
  volatile uint32_t *cpacr = (volatile uint32_t *)0xE000ED88;
  *cpacr |= 0xf << 20;
#endif
}