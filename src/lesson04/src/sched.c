#include "sched.h"
#include "irq.h"
#include "printf.h"

static struct task_struct init_task = INIT_TASK;
struct task_struct *current = &(init_task);
struct task_struct * task[NR_TASKS] = {&(init_task), };
int nr_tasks = 1;

void preempt_disable(void)
{
	current->preempt_count++;
}

void preempt_enable(void)
{
	current->preempt_count--;
}

const unsigned int intervalo = 3000;		// tiempo CPU de cada proceso

void temporizador ( void* p )
{
	curVal = get32(p->priority);   //Lee el valor del contador actual
	curVal += interval;			//aumenta su valor
	put32(TIMER_C1, curVal);	//ajusta el valor curVal al registro para la interrupcion 1
}


void _schedule(void)
{
	preempt_disable();
	int next,c;
	struct task_struct * p;
	int burstTime;
	while (1) {
		int burstTime=0;
		//c = -1;
		//next = 0;
		for (int i = 0; i < NR_TASKS; i++){
			p = task[i];
			burstTime = p->priority; //priority se asigna cuando se crea el proceso
			if (p && p->state == TASK_RUNNING) {
				for (int j = 0; j < burstTime; j++){				// Para ejecutar un proceso un numero de priority veces
					if((burstTime - p->counter) == intervalo || p->counter == 0){ //al restar el burstime con el counter tengo el tiempo de proceso en CPU
						switch_to(task[i+1]);			//cambio a siguiente proceso una vez que un proceso alcanzo el quantum
						break;
					}
				}
				//c = p->counter;
				//next = i;
			}
		}
		/*if (c) {
			break;
		}*/
		for (int i = 0; i < NR_TASKS; i++) {
			p = task[i];
			if (p) {
				p->counter = (p->counter >> 1) + p->priority;
			}
		}
	}
//	switch_to(task[next]);
	preempt_enable();
}

void schedule(void)
{
	current->counter = 0;
	_schedule();
}

void switch_to(struct task_struct * next)
{
	if (current == next)
		return;
	struct task_struct * prev = current;
	current = next;
	cpu_switch_to(prev, next);
}

void schedule_tail(void) {
	preempt_enable();
}


void timer_tick()
{
	--current->counter;
	if (current->counter>0 || current->preempt_count >0) {
		return;
	}
	current->counter=0;
	enable_irq();
	_schedule();
	disable_irq();
}
