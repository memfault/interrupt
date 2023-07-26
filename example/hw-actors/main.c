#include <stdint.h>
#include <stdalign.h>
#include <stdnoreturn.h>
#include "stm32f1xx.h"
#include "hw_actors.h"

#define EXAMPLE_VECTOR 20

static noreturn void Error_Handler(void) {
    __disable_irq();
    GPIOC->BSRR = GPIO_BSRR_BR13;
    for(;;);
}

void HardFault_Handler(void) {
    Error_Handler();
}

void __assert_func(const char *file, int line, const char *func, const char *expr) {
    Error_Handler();
}

static struct example_msg_t {
    struct message_t header;
    unsigned int led_state;
} g_msgs[1];

static struct message_pool_t g_pool;
static struct queue_t g_queue;
static struct actor_t g_handler;
struct context_t g_context;

/* This is name for 20th IRQ which we use for actor execution. Any unused vector may be used. */
void USB_LP_CAN1_RX0_IRQHandler(void) {
    context_schedule(EXAMPLE_VECTOR);
}

/* Systick sends new LED state to queue at every tick. */
void SysTick_Handler(void) {
    static unsigned int led_state = 0;

    led_state ^= 1;
    struct example_msg_t* msg = message_alloc(&g_pool);
    msg->led_state = led_state;
    queue_push(&g_queue, &msg->header);
}

/* Actor reads new LED state from the message and programs GPIO. */
static struct queue_t* actor(struct actor_t* self, struct message_t* m) {
    const struct example_msg_t* msg = (struct example_msg_t*) m;

    if (msg->led_state == 0)
        GPIOC->BSRR = GPIO_BSRR_BR13;
    else
        GPIOC->BSRR = GPIO_BSRR_BS13;

    message_free(m);
    return &g_queue;
}

int main(void) {
    RCC->CR |= RCC_CR_HSEON;            
    while(!(RCC->CR & RCC_CR_HSERDY))
        ;
    
    FLASH->ACR = FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY_1;

    RCC->CFGR |= RCC_CFGR_SW_HSE;
    RCC->CFGR |= RCC_CFGR_PLLMULL9;
    RCC->CFGR |= RCC_CFGR_PLLSRC;

    RCC->CR |= RCC_CR_PLLON;
    while(!(RCC->CR & RCC_CR_PLLRDY))
        ;
    
    RCC->CFGR = (RCC->CFGR | RCC_CFGR_SW_PLL) & ~RCC_CFGR_SW_HSE;
    while(!(RCC->CFGR & RCC_CFGR_SWS_PLL))
        ;

    RCC->CR &= ~RCC_CR_HSION;

    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;
    GPIOC->CRH |= GPIO_CRH_CNF13_0 | GPIO_CRH_MODE13_1;

    NVIC_SetPriorityGrouping(3);

    /* Actors and queues initialization... */

    context_init();
    message_pool_init(&g_pool, g_msgs, sizeof(g_msgs), sizeof(g_msgs[0]));
    queue_init(&g_queue);
    actor_init(&g_handler, &actor, EXAMPLE_VECTOR, &g_queue);

    NVIC_EnableIRQ(EXAMPLE_VECTOR);

    /* Enable Systick to trigger interrupt every 100ms. */
    SysTick->LOAD  = 72000U * 100 - 1U;
    SysTick->VAL   = 0;
    SysTick->CTRL  = 7;

    for (;;) __WFI();
    return 0; /* make compiler happy. */
}
