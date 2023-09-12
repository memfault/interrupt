/** 
  * @file  hw_actors.h
  * License: Public domain. The code is provided as is without any warranty.
  */

#ifndef _HW_ACTORS_H_
#define _HW_ACTORS_H_

#include <stddef.h>
#include <stdbool.h>
#include <assert.h>

/* NVIC porting layer */
#define context_lock(p) { asm volatile ("cpsid i"); }
#define context_unlock(p) { asm volatile ("cpsie i"); }
#define queue_lock(p) { asm volatile ("cpsid i"); }
#define queue_unlock(p) { asm volatile ("cpsie i"); }
#define pic_vect2prio(v) \
    ((((volatile unsigned char*)0xE000E400)[v]) >> (8 - __NVIC_PRIO_BITS))
#define STIR_ADDR ((volatile unsigned int*) 0xE000EF00)
#define pic_interrupt_request(v) ((*STIR_ADDR) = v)

struct list_t {
    struct list_t* next;
    struct list_t* prev;
};

#define list_init(head) ((head)->next = (head)->prev = (head))
#define list_empty(head) ((head)->next == (head))
#define list_first(head) ((head)->next)
#define list_entry(p, type, member) \
    ((type*)((char*)(p) - (size_t)(&((type*)0)->member)))

static inline void list_append(
    struct list_t* head, 
    struct list_t* node) {

    node->next = head;
    node->prev = head->prev;
    node->prev->next = node;
    head->prev = node;
}

static inline void list_remove(struct list_t* node) {
    node->prev->next = node->next;
    node->next->prev = node->prev;
    node->next = node->prev = 0;
}

struct queue_t {
    struct list_t items;
    bool item_type_is_msg;
};

struct message_pool_t {
    struct queue_t queue;
    unsigned char* array;
    size_t total_length;
    size_t block_sz;
    size_t offset;
    bool array_space_available;
};

_Static_assert(
    offsetof(struct message_pool_t, queue) == 0U, 
    "queue must be the first member of the pool"
);

struct message_t {
    struct message_pool_t* parent;
    struct list_t link;
};

struct actor_t {
    struct context_t* parent;
    struct queue_t* (*func)(struct actor_t* self, struct message_t* m);
    unsigned int vect;
    unsigned int prio;
    struct message_t* mailbox;
    struct list_t link;
};

struct context_t {
    struct list_t runq[PRIO_MAX];
};

extern struct context_t g_context;
#define get_context() (&g_context)

static inline void context_init(void) {
    struct context_t* context = get_context();
    const size_t runq_num = (sizeof(context->runq) / sizeof(context->runq[0])); 

    for (size_t i = 0; i < runq_num; ++i) {
        list_init(&context->runq[i]);
    }
}

static inline void queue_init(struct queue_t* q) {
    list_init(&q->items);
    q->item_type_is_msg = true;
}

static inline void message_pool_init(
    struct message_pool_t* pool, 
    void* mem, 
    size_t total_len, 
    size_t block_sz) {

    assert(total_len >= block_sz);
    assert(block_sz >= sizeof(struct message_t));
    queue_init(&pool->queue);
    pool->array = mem;
    pool->total_length = total_len;
    pool->block_sz = block_sz;
    pool->offset = 0;
    pool->array_space_available = true;
}

static inline struct message_t* queue_pop(
    struct queue_t* q, 
    struct actor_t* subscriber) {

    struct message_t* msg = 0;
    queue_lock(q);

    if (!list_empty(&q->items) && q->item_type_is_msg) {
        struct list_t* const head = list_first(&q->items);
        list_remove(head);
        msg = list_entry(head, struct message_t, link);
    } 
    else if (subscriber != 0) {
        list_append(&q->items, &subscriber->link);
        q->item_type_is_msg = false;
    }

    queue_unlock(q);

    return msg;
}

static inline void queue_push(
    struct queue_t* q, 
    struct message_t* msg) {

    struct actor_t* actor = 0;
    queue_lock(q);

    if (q->item_type_is_msg) {
        list_append(&q->items, &msg->link);
    } 
    else {
        struct list_t* const head = list_first(&q->items);
        list_remove(head);
        actor = list_entry(head, struct actor_t, link);
        actor->mailbox = msg;

        if (list_empty(&q->items)) {
            q->item_type_is_msg = true;
        }
    }

    queue_unlock(q);

    if (actor) {
        struct context_t* const context = actor->parent;
        context_lock(context);
        list_append(&context->runq[actor->prio], &actor->link);
        pic_interrupt_request(actor->vect);
        context_unlock(context);
    }
}

static inline void actor_init(
    struct actor_t* actor, 
    struct queue_t* (*func)(struct actor_t*, struct message_t*), 
    unsigned int vect, 
    struct queue_t* q) {

    struct context_t* const context = get_context();
    const unsigned int prio = pic_vect2prio(vect);
    assert(prio < PRIO_MAX);
    actor->func = func;
    actor->vect = vect;
    actor->prio = prio;
    actor->parent = context;
    actor->mailbox = 0;
    const struct message_t* const msg = queue_pop(q, actor);
    assert(msg == 0);
}

static inline void* message_alloc(struct message_pool_t* pool) {
    struct message_t* msg = 0;
    queue_lock(q);

    if (pool->array_space_available) {
        msg = (void*)(pool->array + pool->offset);
        pool->offset += pool->block_sz;

        if ((pool->offset + pool->block_sz) >= pool->total_length) {
            pool->array_space_available = false;
        }

        msg->parent = pool;
    }

    queue_unlock(q);

    if (!msg) {
        msg = queue_pop(&pool->queue, 0);
    }  

    return msg;
}

static inline void message_free(struct message_t* msg) {
    struct message_pool_t* const pool = msg->parent;
    queue_push(&pool->queue, msg);
}

static inline void context_schedule(unsigned int vect) {
    struct context_t* const context = get_context();
    const unsigned int prio = pic_vect2prio(vect);
    assert(prio < PRIO_MAX);
    struct list_t* const runq = &context->runq[prio];
    context_lock(context);

    while (!list_empty(runq)) {
        struct list_t* const head = list_first(runq);
        struct actor_t* const actor = list_entry(
            head, 
            struct actor_t, 
            link
        );
        list_remove(head);
        context_unlock(context);

        do {
            struct queue_t* const q = actor->func(actor, actor->mailbox);
            assert(q != 0);
            actor->mailbox = queue_pop(q, actor);
        } while (actor->mailbox != 0);

        context_lock(context);
    }

    context_unlock(context);
}

#endif

