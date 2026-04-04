#include "sched.h"

/* 100 ms time slice -- same as RR_TIMESLICE in include/linux/sched/rt.h */
#define FREEZER_TIMESLICE (100 * HZ / 1000)

/* sched_class_freezer methods */


static void enqueue_task_freezer(struct rq *rq, struct task_struct *p, int flags)
{
	// TODO
}

static void dequeue_task_freezer(struct rq *rq, struct task_struct *p, int flags)
{
	// TODO
}

static void yield_task_freezer(struct rq *rq)
{
	// TODO
}

static void wakeup_preempt_freezer(struct rq *rq, struct task_struct *p, int flags)
{
    // TODO
}

static struct task_struct *pick_next_task_freezer(struct rq *rq)
{
    // TODO
}

static void put_prev_task_freezer(struct rq *rq, struct task_struct *p)
{
    // TODO
}

static void set_next_task_freezer(struct rq *rq, struct task_struct *p, bool first)
{
    // TODO
}

static void task_tick_freezer(struct rq * rq, struct task_struct *p, int queued)
{
    // TODO
}

// TODO: Part 7 SMP
/* SMP methods */
// static int balance_freezer(struct rq *rq, struct task_struct *prev, struct rq_flags *rf)
// {
//     // TODO
// }

// static int select_task_rq_freezer(struct task_struct *p, int task_cpu, int flags)
// {
//     // TODO
// }

// static void migrate_task_rq_freezer(struct task_struct *p, int new_cpu)
// {
//     // TODO
// }

DEFINE_SCHED_CLASS(freezer) = {
    .enqueue_task = enqueue_task_freezer,
    .dequeue_task = dequeue_task_freezer,
    .yield_task = yield_task_freezer,
    .wakeup_preempt = wakeup_preempt_freezer,
    .pick_next_task = pick_next_task_freezer,
    .put_prev_task = put_prev_task_freezer,
    .set_next_task = set_next_task_freezer,
    .task_tick = task_tick_freezer,

    //TODO: Part 7 SMP
    // .balance = balance_freezer,
    // .select_task_rq = select_task_rq_freezer,
    // .migrate_task_rq = migrate_task_rq_freezer,
};
