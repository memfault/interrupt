#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

extern void Irq1_Handler(void);

// the FreeRTOS heap
uint8_t ucHeap[configTOTAL_HEAP_SIZE];

/* Priorities at which the tasks are created. */
#define mainQUEUE_RECEIVE_TASK_PRIORITY (tskIDLE_PRIORITY + 2)
#define mainQUEUE_SEND_TASK_PRIORITY (tskIDLE_PRIORITY + 1)

/* The rate at which data is sent to the queue.  The 200ms value is converted
to ticks using the portTICK_RATE_MS constant. */
#define mainQUEUE_SEND_FREQUENCY_MS (1)

#define mainQUEUE_LENGTH (1)

#define mainQUEUE_SEND_PARAMETER (0x1111UL)
#define mainQUEUE_RECEIVE_PARAMETER (0x22UL)

static QueueHandle_t xQueue = NULL;

// Eight crash modes:
//  0: Bad instruction execution
//  1: Bad Address Read
//  2: Disabled Coprocessor Access
//  3: Bad Memory Write
//  4: Unaligned 8 byte read
//  5: Exception Entry Fault
//  6: Bad 4 byte read
//  7: Illegal EXC_RETURN
//  Anything Else: No crashes enabled
#ifndef FAULT_EXAMPLE_CONFIG
#define FAULT_EXAMPLE_CONFIG 0
#endif

// Make the variable global so the compiler doesn't optimize away the setting
// and the crash taken can be overriden from gdb without having to recompile
// the app. For example, to cause a stack overflow crash:
//
// (gdb) break main
// (gdb) continue
// (gdb) set g_crash_config=1
// (gdb) continue
int g_crash_config = FAULT_EXAMPLE_CONFIG;

void trigger_irq(void) {
  volatile uint32_t *nvic_iser = (void *)0xE000E100;
  *nvic_iser |= (0x1 << 1);

  // Pend an interrupt
  volatile uint32_t *nvic_ispr = (void *)0xE000E200;
  *nvic_ispr |= (0x1 << 1);

  // flush pipeline to ensure exception takes effect before we
  // return from this routine
  __asm("isb");
}

void stkerr_from_psp(void) {
  extern uint32_t _start_of_ram[];
  uint8_t dummy_variable;
  const size_t distance_to_ram_bottom = (uint32_t)&dummy_variable - (uint32_t)_start_of_ram;
  volatile uint8_t big_buf[distance_to_ram_bottom - 8];
  for (size_t i = 0; i < sizeof(big_buf); i++) {
    big_buf[i] = i;
  }
  
  trigger_irq();
}

int bad_memory_access_crash(void) {
  volatile uint32_t *bad_access = (volatile uint32_t *)0xdeadbeef;
  return *bad_access;
}

int illegal_instruction_execution(void) {
  int (*bad_instruction)(void) = (void *)0xE0000000;
  return bad_instruction();
}

void unaligned_double_word_read(void) {
  extern void *g_unaligned_buffer;
  uint64_t *buf = g_unaligned_buffer;
  *buf = 0x1122334455667788;
}

void bad_addr_double_word_write(void) {
  volatile uint64_t *buf = (volatile uint64_t *)0x30000000;
  *buf = 0x1122334455667788;
}

void access_disabled_coprocessor(void) {
  // FreeRTOS will automatically enable the FPU co-processor.
  // Let's disable it for the purposes of this example
  __asm volatile(
      "ldr r0, =0xE000ED88 \n"
      "mov r1, #0 \n"
      "str r1, [r0]	\n"
      "dsb \n"
      "vmov r0, s0 \n"
      );
}

uint32_t read_from_bad_address(void) {
  return *(volatile uint32_t *)0xbadcafe;
}

__attribute__((naked))
void Irq1_Handler(void) {
  __asm volatile(
      "ldr r0, =0xFFFFFFE0 \n"
      "bx r0 \n"
                 );
}

void trigger_crash(int crash_id) {
  switch (crash_id) {
    case 0:
      illegal_instruction_execution();      
      break;
    case 1:
      read_from_bad_address();
      break;
    case 2:
      access_disabled_coprocessor();
      break;
    case 3:
      bad_addr_double_word_write();
      break;
    case 4:
      stkerr_from_psp();
      break;
    case 5:
      unaligned_double_word_read();      
      break;
    case 6:
      bad_memory_access_crash();
      break;
    case 7:
      trigger_irq();
      break;
    default:
      break;
  }
}

static void prvQueuePingTask(void *pvParameters) {
  TickType_t xNextWakeTime;
  const unsigned long ulValueToSend = 100UL;

  configASSERT(((unsigned long)pvParameters) == mainQUEUE_SEND_PARAMETER);

  xNextWakeTime = xTaskGetTickCount();

  while (1) {
    vTaskDelayUntil(&xNextWakeTime, mainQUEUE_SEND_FREQUENCY_MS);
    xQueueSend(xQueue, &ulValueToSend, 0U);

    trigger_crash(g_crash_config);
  }
}

static void prvQueuePongTask(void *pvParameters) {
  while (1) {
    unsigned long ulReceivedValue;
    xQueueReceive(xQueue, &ulReceivedValue, portMAX_DELAY);

    if (ulReceivedValue == 100UL) {
      ulReceivedValue = 0U;
    }
  }
}

void vAssertCalled(const char *file, int line) {
  __asm("bkpt 3");
}

void recover_from_task_fault(void) {
  while (1) {
    vTaskDelay(1);
  }
}

int main(void) {
  xQueue = xQueueCreate(mainQUEUE_LENGTH, sizeof(unsigned long));
  configASSERT(xQueue != NULL);

  xTaskCreate(prvQueuePongTask,
              "Pong", configMINIMAL_STACK_SIZE,
              (void *) mainQUEUE_RECEIVE_PARAMETER,
              mainQUEUE_RECEIVE_TASK_PRIORITY,
              NULL);

  xTaskCreate(prvQueuePingTask, "Ping", configMINIMAL_STACK_SIZE,
              (void *)mainQUEUE_SEND_PARAMETER, mainQUEUE_SEND_TASK_PRIORITY,
              NULL);
  vTaskStartScheduler();

  // should be unreachable
  configASSERT(0);
  return -1;
}
