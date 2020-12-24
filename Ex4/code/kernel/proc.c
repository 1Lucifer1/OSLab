
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               proc.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "string.h"
#include "proc.h"
#include "global.h"

/*======================================================================*
                              schedule
 *======================================================================*/
PUBLIC void schedule()
{
	PROCESS* p;
	int	 greatest_ticks = 0;

	// read first
	while (!greatest_ticks) {
		for (p = proc_table; p < proc_table+NR_TASKS; p++) {
			if(p->sleep > 0) p->sleep--;
			if (p->ticks > greatest_ticks) {
				if(p->sleep > 0 || p->wait) continue;
				greatest_ticks = p->ticks;
				p_proc_ready = p;
			}
			// if(p->sleep > 0) disp_int(p->sleep);
			//disp_str(".");
		}

		if (!greatest_ticks) {
			for (p = proc_table; p < proc_table+NR_TASKS; p++) {
				if(p->sleep > 0 || p->wait) continue;
				p->ticks = p->priority;
			}
		}
	}

	// write first
	/*while (!greatest_ticks) {
		for (p = proc_table+NR_TASKS-1; p >= proc_table; p--) {
			if(p->sleep > 0) p->sleep--;
			if (p->ticks > greatest_ticks) {
				if(p->sleep > 0 || p->wait) continue;
				greatest_ticks = p->ticks;
				p_proc_ready = p;
			}
			// if(p->sleep > 0) disp_int(p->sleep);
			//disp_str(".");
		}

		if (!greatest_ticks) {
			for (p = proc_table+NR_TASKS-1; p >= proc_table; p--) {
				if(p->sleep > 0 || p->wait) continue;
				p->ticks = p->priority;
			}
		}
	}*/
}

/*======================================================================*
                           sys_get_ticks
 *======================================================================*/
PUBLIC int sys_get_ticks()
{
	return ticks;
}

/*======================================================================*
                           sys_process_sleep
 *======================================================================*/
PUBLIC int sys_process_sleep(int milli_seconds)
{
	p_proc_ready->sleep = milli_seconds;
	// disp_int(milli_seconds);
	return 0;
}

/*======================================================================*
                           sys_print_str
 *======================================================================*/
PUBLIC int sys_print_str(char* str)
{
	int color = p_colors[p_proc_ready - proc_table];
	disp_color_str(str, color);
	return 0;
}
/*======================================================================*
                           sys_P
 *======================================================================*/
PUBLIC int sys_P(SEMAPHORE* s)
{
	s->value--;
	if(s->value < 0) wait(s);
	schedule();
	return 0;
}

/*======================================================================*
                           sys_V
 *======================================================================*/
PUBLIC int sys_V(SEMAPHORE* s)
{
	s->value++;
	if(s->value <= 0) wake_up(s);
	schedule();
	return 0;
}


