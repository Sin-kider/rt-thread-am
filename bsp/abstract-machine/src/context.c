#include <am.h>
#include <klib.h>
#include <rtthread.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <threads.h>

volatile rt_ubase_t rt_from = 0;
volatile rt_ubase_t rt_to = 0;

static Context* ev_handler(Event e, Context *c) {
  assert(rt_to != 0);
  switch (e.event) {
    case EVENT_YIELD:
      if (rt_from != 0) {
        *(Context **)rt_from = c;
      }
      assert(rt_to != 0);
      c = *(Context **)rt_to;
      break;
    case EVENT_IRQ_TIMER:
      break;
    default: printf("Unhandled event ID = %d\n", e.event); assert(0);
  }
  return c;
}

void __am_cte_init() {
  cte_init(ev_handler);
}

void rt_hw_context_switch_to(rt_ubase_t to) {
  rt_from = 0;
  rt_to = to;
  yield();
}

void rt_hw_context_switch(rt_ubase_t from, rt_ubase_t to) {
  rt_from = from;
  rt_to = to;
  yield();
}

void rt_hw_context_switch_interrupt(void *context, rt_ubase_t from, rt_ubase_t to, struct rt_thread *to_thread) {
  assert(0);
}


typedef void (*tentry_t) (void *);
typedef void (*texit_t) (void);

typedef struct {
  tentry_t entry;
  texit_t exit;
  void *args;
} Args;

void wrap_func(void * arg) {
  ((Args *)arg)->entry(((Args *)arg)->args);
  ((Args *)arg)->exit();
  assert(0);
}

rt_uint8_t *rt_hw_stack_init(void *tentry, void *parameter, rt_uint8_t *stack_addr, void *texit) {
  Area area = {.start = NULL, .end = (void *)((rt_uint64_t)(stack_addr + (((rt_uint64_t)stack_addr & 7) ? 8 : 0)) & (~7))};
  Args *args = rt_malloc(sizeof(Args));
  args->entry = tentry;
  args->exit = texit;
  args->args = parameter;
  Context *context = kcontext(area, wrap_func, args);
  return (rt_uint8_t *)context;
}
