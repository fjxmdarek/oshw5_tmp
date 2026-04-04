/*
 * Freezer Scheduling Class
 * - implements a simple round-robin scheduling algorithm 
 *   (use a simple linux circular linked list as the queue)
 * - supports SMP (Part 7)
 * - every task has the same time slice of 100 ms
 * - a task should be assigned to the CPU with the fewest Freezer tasks (Part 7)
 * - when a CPU becomes idle, it steals a task from another CPU (Part 7)
 * - SCHED_FREEZER takes priority over SCHED_NORMAL but not over SCHED_RR or 
 *   SCHED_FIFO
 */

#include "sched.h"

/* initialize the freezer run queue */
void init_freezer_rq(struct freezer_rq *fr)
{
	INIT_LIST_HEAD(&fr->tasks);
	fr->nr_running = 0;
}

/* sched_class_freezer methods */

/* add task to freezer run queue */
static void enqueue_task_freezer(struct rq *rq, struct task_struct *p,
				 int flags)
{
	pr_info("freezer: enqueue_task called for pid %d\n", p->pid);
	WARN_ON_ONCE(!list_empty(&p->freezer.run_list));
	if (flags &
	    ENQUEUE_WAKEUP) /* set the timeslice if it is the first time enqueuing this task */
		p->freezer.time_slice = FREEZER_TIMESLICE;
	list_add_tail(&p->freezer.run_list, &rq->fr.tasks);
	rq->fr.nr_running++;
}

/* remove task from freezer run queue */
static void dequeue_task_freezer(struct rq *rq, struct task_struct *p,
				 int flags)
{
	WARN_ON_ONCE(list_empty(&p->freezer.run_list));
	list_del_init(&p->freezer.run_list);
	rq->fr.nr_running--;
}

/* move task to end of freezer run queue */
static void yield_task_freezer(struct rq *rq)
{
	list_move_tail(&rq->curr->freezer.run_list, &rq->fr.tasks);
}

/* decide whether to preempt the current task 
 * -> preempt if its scheduling policy is lower than freezer (fair or idle)
 */
static void wakeup_preempt_freezer(struct rq *rq, struct task_struct *p,
				   int flags)
{
	if (rq->curr->sched_class == &fair_sched_class ||
	    rq->curr->sched_class == &idle_sched_class)
		resched_curr(rq);
}

/* pick the next task to run from the freezer rq 
 * -> for simple RR just pick the head of the list
 */
static struct task_struct *pick_next_task_freezer(struct rq *rq)
{
	if (list_empty(&rq->fr.tasks))
		return NULL;
	return list_first_entry(&rq->fr.tasks, struct task_struct,
				freezer.run_list);
}

static void put_prev_task_freezer(struct rq *rq, struct task_struct *p)
{
	// TODO: Part 7 SMP - cleanup load balancing state
}

static void set_next_task_freezer(struct rq *rq, struct task_struct *p,
				  bool first)
{
	// TODO: Part 7 SMP - update load balancing state
}

static void task_tick_freezer(struct rq *rq, struct task_struct *p, int queued)
{
	if (--p->freezer.time_slice)
		return; /* we have not reached the end of this task's time slice */

	p->freezer.time_slice = FREEZER_TIMESLICE; /* reset the time slice */

	/* move task to the end of the run queue if we have more than 1 task, 
     * otherwise just keep running it
     */
	if (rq->fr.nr_running > 1) {
		list_move_tail(&p->freezer.run_list, &rq->fr.tasks);
		resched_curr(rq);
	}
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
