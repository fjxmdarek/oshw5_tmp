/*
 * Heater Scheduling Class
 * - implements a non-preemptive scheduling policy
 * - each CPU's local run queue holds at most 1 task at a time
 * - overflow tasks go to a global run queue shared by all CPUs
 * - new tasks are enqueued at the tail of the global run queue
 * - when a CPU needs a new task, it pulls from the head of the global run queue
 * - SCHED_HEATER sits below SCHED_FREEZER in priority
 */

#include "sched.h"

/*
 * Global run queue shared by all CPUs.
 * Protected by global_rq_lock.
 */
static struct heater_rq global_rq = {
	.tasks      = LIST_HEAD_INIT(global_rq.tasks),
	.nr_running = 0,
};

DEFINE_RAW_SPINLOCK(global_rq_lock);

/* initialize a per-CPU heater run queue */
void init_heater_rq(struct heater_rq *hr)
{
	INIT_LIST_HEAD(&hr->tasks);
	hr->nr_running = 0;
}

/*
 * enqueue_task_heater - add a task to the heater run queue
 *
 * If the local per-CPU queue is empty, add the task there.
 * Otherwise send it to the tail of the global run queue.
 */
static void enqueue_task_heater(struct rq *rq, struct task_struct *p, int flags)
{
	struct sched_heater_entity *he = &p->heater;

	/* avoid double enqueue */
	if (!list_empty(&he->run_list))
		return;

	raw_spin_lock(&global_rq_lock);

	if (rq->hr.nr_running == 0) {
		/* local slot is free: place the task here */
		list_add_tail(&he->run_list, &rq->hr.tasks);
		rq->hr.nr_running++;
	} else {
		/* local is full: overflow to global queue (at tail) */
		list_add_tail(&he->run_list, &global_rq.tasks);
		global_rq.nr_running++;
	}

	raw_spin_unlock(&global_rq_lock);

	add_nr_running(rq, 1);
}

/*
 * dequeue_task_heater - remove a task from whichever queue it is on
 *
 * Because local holds at most 1 task, we can tell where the task lives
 * by checking whether it is the first entry in the local list.
 */
static void dequeue_task_heater(struct rq *rq, struct task_struct *p, int flags)
{
	struct sched_heater_entity *he = &p->heater;

	raw_spin_lock(&global_rq_lock);

	if (!list_empty(&rq->hr.tasks) &&
	    list_first_entry(&rq->hr.tasks,
			     struct sched_heater_entity, run_list) == he) {
		/* task is in the local queue */
		list_del_init(&he->run_list);
		rq->hr.nr_running--;
	} else {
		/* task is in the global queue */
		list_del_init(&he->run_list);
		global_rq.nr_running--;
	}

	raw_spin_unlock(&global_rq_lock);

	sub_nr_running(rq, 1);
}

/*
 * pick_next_task_heater - select the next task to run
 *
 * Check the local queue first.  If empty, pull from the head of
 * the global queue into the local slot.
 */
static struct task_struct *pick_next_task_heater(struct rq *rq)
{
	struct sched_heater_entity *he;

	raw_spin_lock(&global_rq_lock);

	/* prefer the local queue */
	if (!list_empty(&rq->hr.tasks)) {
		he = list_first_entry(&rq->hr.tasks,
				      struct sched_heater_entity, run_list);
		raw_spin_unlock(&global_rq_lock);
		return container_of(he, struct task_struct, heater);
	}

	/* local is empty: pull from the head of the global queue */
	if (!list_empty(&global_rq.tasks)) {
		he = list_first_entry(&global_rq.tasks,
				      struct sched_heater_entity, run_list);
		list_move_tail(&he->run_list, &rq->hr.tasks);
		global_rq.nr_running--;
		rq->hr.nr_running++;
		raw_spin_unlock(&global_rq_lock);
		return container_of(he, struct task_struct, heater);
	}

	raw_spin_unlock(&global_rq_lock);
	return NULL;
}

/*
 * yield_task_heater - voluntarily give up the CPU
 *
 * Move the current task from the local queue to the tail of the global
 * queue so that other heater tasks get a chance to run.
 * pick_next_task_heater will pull the next candidate from the global head.
 */
static void yield_task_heater(struct rq *rq)
{
	struct task_struct *p = rq->curr;
	struct sched_heater_entity *he = &p->heater;

	raw_spin_lock(&global_rq_lock);

	list_del_init(&he->run_list);
	rq->hr.nr_running--;

	list_add_tail(&he->run_list, &global_rq.tasks);
	global_rq.nr_running++;

	raw_spin_unlock(&global_rq_lock);
}

/*
 * task_tick_heater - periodic tick handler
 *
 * Heater is non-preemptive: a running task is never preempted by the
 * timer tick.  Do nothing.
 */
static void task_tick_heater(struct rq *rq, struct task_struct *p, int queued)
{
}

/*
 * wakeup_preempt_heater - check whether a waking task should preempt curr
 *
 * Heater is non-preemptive: never preempt a running heater task.
 */
static void wakeup_preempt_heater(struct rq *rq, struct task_struct *p,
				   int flags)
{
}

static void put_prev_task_heater(struct rq *rq, struct task_struct *p)
{
}

static void set_next_task_heater(struct rq *rq, struct task_struct *p,
				  bool first)
{
}

static void update_curr_heater(struct rq *rq)
{
}

static void prio_changed_heater(struct rq *rq, struct task_struct *p,
				 int oldprio)
{
}

static void switched_to_heater(struct rq *rq, struct task_struct *p)
{
}

#ifdef CONFIG_SMP
static int balance_heater(struct rq *rq, struct task_struct *prev,
			   struct rq_flags *rf)
{
	return 0;
}

/* keep the task on the CPU it was enqueued on */
static int select_task_rq_heater(struct task_struct *p, int cpu, int flags)
{
	return cpu;
}

static void migrate_task_rq_heater(struct task_struct *p, int new_cpu)
{
}
#endif

DEFINE_SCHED_CLASS(heater) = {
	.enqueue_task    = enqueue_task_heater,
	.dequeue_task    = dequeue_task_heater,
	.yield_task      = yield_task_heater,
	.wakeup_preempt  = wakeup_preempt_heater,
	.pick_next_task  = pick_next_task_heater,
	.put_prev_task   = put_prev_task_heater,
	.update_curr     = update_curr_heater,
	.set_next_task   = set_next_task_heater,
	.task_tick       = task_tick_heater,
	.prio_changed    = prio_changed_heater,
	.switched_to     = switched_to_heater,

#ifdef CONFIG_SMP
	.balance         = balance_heater,
	.select_task_rq  = select_task_rq_heater,
	.migrate_task_rq = migrate_task_rq_heater,
#endif
};
