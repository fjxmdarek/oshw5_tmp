/*
 * Freezer Scheduling Class
 * - implements a simple round-robin scheduling algorithm 
 *   (use a simple linux circular linked list as the queue)
 * - supports SMP (Part 7)
 * - every task has the same time slice of 100 ms
 * - a task should be assigned to the CPU with the fewest Freezer tasks (Part 7)
 * - when a CPU becomes idle, it steals a task from another CPU (Part 7)
 * - SCHED_FREEZER takes priority over SCHED_NORMAL but not over SCHED_RR or 
 *   SCHED_FIFO (Part 6)
 */

#include "sched.h"

/* initialize the freezer run queue */
void init_freezer_rq(struct freezer_rq *fr)
{
	INIT_LIST_HEAD(&fr->tasks);
	/* number of runnable tasks on that run queue */
	fr->nr_running = 0;
}

/* sched_class_freezer methods */

/* enqueue the sched_freezer_entity */
static void enqueue_entity_freezer(struct rq *rq,
				   struct sched_freezer_entity *fr_se)
{
	struct freezer_rq *fr = &rq->fr;

	list_add_tail(&fr_se->run_list, &fr->tasks);
	/* increment number of runnable freezer tasks on this rq,
	 * protected by rq lock for enqueue
	 */
	fr->nr_running++;
}

/* add task to freezer run queue */
static void enqueue_task_freezer(struct rq *rq, struct task_struct *p,
				 int flags)
{
	// TODO: comment out for Part 6
	trace_printk("freezer: enqueue_task called for pid %d\n", p->pid);

	struct sched_freezer_entity *fr_se = &p->freezer;

	/* avoid double enqueue for the same task */
	if (!list_empty(&fr_se->run_list))
		return;

	enqueue_entity_freezer(rq, fr_se);
	/* increment number of total runnable tasks on this CPU */
	add_nr_running(rq, 1);
}

/* dequeue the sched_freezer_entity*/
static void dequeue_entity_freezer(struct rq *rq,
				   struct sched_freezer_entity *fr_se)
{
	struct freezer_rq *fr = &rq->fr;

	list_del_init(&fr_se->run_list);
	/* decrement number of runnable freezer tasks on this rq,
	 * protected by rq lock for dequeue
	 */
	fr->nr_running--;
}

/* remove task from freezer run queue */
static void dequeue_task_freezer(struct rq *rq, struct task_struct *p,
				 int flags)
{
	// TODO: comment out for Part 6
	trace_printk("freezer: dequeue_task called for pid %d\n", p->pid);

	struct sched_freezer_entity *fr_se = &p->freezer;

	dequeue_entity_freezer(rq, fr_se);
	/* decrement number of total runnable tasks on this CPU */
	sub_nr_running(rq, 1);
}

/* move task to end of freezer run queue */
static void yield_task_freezer(struct rq *rq)
{
	struct task_struct *p = rq->curr;
	struct sched_freezer_entity *fr_se = &p->freezer;
	struct freezer_rq *fr = &rq->fr;

	list_move_tail(&fr_se->run_list, &fr->tasks);
}

/* pick the next task to run from the freezer rq 
 * -> for simple RR just pick the head of the list
 */
static struct task_struct *pick_next_task_freezer(struct rq *rq)
{
	// TODO: comment out for Part 6
	trace_printk("freezer: pick_next_task called, nr_running=%d\n",
		     rq->fr.nr_running);

	struct task_struct *next;
	struct freezer_rq *fr = &rq->fr;
	struct sched_freezer_entity *fr_se;

	if (list_empty(&fr->tasks))
		return NULL;

	fr_se = list_first_entry(&fr->tasks, struct sched_freezer_entity,
				 run_list);
	/* use container_of to get the task struct surrounding the sched_entity */
	next = container_of(fr_se, struct task_struct, freezer);

	return next;
}

static void update_curr_freezer(struct rq *rq)
{
}

static void put_prev_task_freezer(struct rq *rq, struct task_struct *p)
{
}

static void set_next_task_freezer(struct rq *rq, struct task_struct *p,
				  bool first)
{
}

static void wakeup_preempt_freezer(struct rq *rq, struct task_struct *p,
				   int flags)
{
}

static void prio_changed_freezer(struct rq *rq, struct task_struct *p,
				 int oldprio)
{
}

static void switched_to_freezer(struct rq *rq, struct task_struct *p)
{
	/* re-initialize fields for new freezer task */
	struct sched_freezer_entity *fr_se = &p->freezer;

	fr_se->time_slice = FREEZER_TIMESLICE;
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
#ifdef CONFIG_SMP
static int balance_freezer(struct rq *rq, struct task_struct *prev,
			   struct rq_flags *rf)
{
	// TODO
	return 0;
}

/* choose the cpu with the fewest freezer tasks */
static int select_task_rq_freezer(struct task_struct *p, int cpu, int flags)
{
	int min_freezer_cpu = cpu, min_running = INT_MAX, n;

	/* read the value for nr_running under a protected rcu lock */
	rcu_read_lock();
	for_each_online_cpu(n) {
		int nr = cpu_rq(n)->fr.nr_running;

		if (nr < min_running) {
			min_running = nr;
			min_freezer_cpu = n;
		}
	}
	rcu_read_unlock();

	// TODO: comment out for Part 6
	trace_printk(
		"freezer: select_task_rq called for pid %d, selected cpu %d\n",
		p->pid, min_freezer_cpu);

	return min_freezer_cpu;
}

static void migrate_task_rq_freezer(struct task_struct *p, int new_cpu)
{
	// TODO
}
#endif

DEFINE_SCHED_CLASS(freezer) = { .enqueue_task = enqueue_task_freezer,
				.dequeue_task = dequeue_task_freezer,
				.yield_task = yield_task_freezer,
				.wakeup_preempt = wakeup_preempt_freezer,
				.pick_next_task = pick_next_task_freezer,
				.put_prev_task = put_prev_task_freezer,
				.update_curr = update_curr_freezer,
				.set_next_task = set_next_task_freezer,
				.task_tick = task_tick_freezer,
				.prio_changed = prio_changed_freezer,
				.switched_to = switched_to_freezer,

//TODO: Part 7 SMP
#ifdef CONFIG_SMP
				.balance = balance_freezer,
				.select_task_rq = select_task_rq_freezer,
				.migrate_task_rq = migrate_task_rq_freezer
#endif
};
