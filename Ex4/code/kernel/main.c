
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            main.c
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
                            kernel_main
 *======================================================================*/
PUBLIC int kernel_main()
{
	disp_str("-----\"kernel_main\" begins-----\n");

	TASK*		p_task		= task_table;
	PROCESS*	p_proc		= proc_table;
	char*		p_task_stack	= task_stack + STACK_SIZE_TOTAL;
	u16		selector_ldt	= SELECTOR_LDT_FIRST;
	int i;
	for (i = 0; i < NR_TASKS; i++) {
		strcpy(p_proc->p_name, p_task->name);	// name of the process
		p_proc->pid = i;			// pid

		p_proc->ldt_sel = selector_ldt;

		memcpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3],
		       sizeof(DESCRIPTOR));
		p_proc->ldts[0].attr1 = DA_C | PRIVILEGE_TASK << 5;
		memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3],
		       sizeof(DESCRIPTOR));
		p_proc->ldts[1].attr1 = DA_DRW | PRIVILEGE_TASK << 5;
		p_proc->regs.cs	= ((8 * 0) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.ds	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.es	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.fs	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.ss	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.gs	= (SELECTOR_KERNEL_GS & SA_RPL_MASK)
			| RPL_TASK;

		p_proc->regs.eip = (u32)p_task->initial_eip;
		p_proc->regs.esp = (u32)p_task_stack;
		p_proc->regs.eflags = 0x1202; /* IF=1, IOPL=1 */

		p_task_stack -= p_task->stacksize;
		p_proc++;
		p_task++;
		selector_ldt += 1 << 3;
	}

	proc_table[0].ticks = proc_table[0].priority = 1;
	proc_table[1].ticks = proc_table[1].priority = 1;
	proc_table[2].ticks = proc_table[2].priority = 1;
	proc_table[3].ticks = proc_table[3].priority = 1;
	proc_table[4].ticks = proc_table[4].priority = 1;
	proc_table[5].ticks = proc_table[5].priority = 1;

	proc_table[0].sleep = 0;
	proc_table[1].sleep = 0;
	proc_table[2].sleep = 0;
	proc_table[3].sleep = 0;
	proc_table[4].sleep = 0;
	proc_table[5].sleep = 0;

	k_reenter = 0;
	ticks = 0;

	/* init the semaphore */
	rmutex.value = 1;
	rmutex.length = 0;
	memset(rmutex.queue, -1, 10 * sizeof(int));
	
	wmutex.value = 1;
	wmutex.length = 0;
	memset(wmutex.queue, -1, 10 * sizeof(int));

	x.value = 1;
	x.length = 0;
	memset(x.queue, -1, 10 * sizeof(int));


	y.value = 1;
	y.length = 0;
	memset(y.queue, -1, 10 * sizeof(int));

	z.value = 1;
	z.length = 0;
	memset(z.queue, -1, 10 * sizeof(int));

	max_read.value = 2;
	max_read.length = 0;
	memset(max_read.queue, -1, 10 * sizeof(int));

	readcount = 0;
	writecount = 0;

	// read first
	p_proc_ready	= proc_table;

	// write first
	//p_proc_ready	= proc_table+NR_TASKS-1;

	clean_screen();

        /* 初始化 8253 PIT */
        out_byte(TIMER_MODE, RATE_GENERATOR);
        out_byte(TIMER0, (u8) (TIMER_FREQ/HZ) );
        out_byte(TIMER0, (u8) ((TIMER_FREQ/HZ) >> 8));

        put_irq_handler(CLOCK_IRQ, clock_handler); /* 设定时钟中断处理程序 */
        enable_irq(CLOCK_IRQ);                     /* 让8259A可以接收时钟中断 */

	restart();

	while(1){}
}

/*======================================================================*
                               TestA
 *======================================================================*/
void TestA()
{
	read("A.");
}

/*======================================================================*
                               TestB
 *======================================================================*/
void TestB()
{
	read("B.");
}

/*======================================================================*
                               TestC
 *======================================================================*/
void TestC()
{
	read("C.");
}

/*======================================================================*
                               TestD
 *======================================================================*/
void TestD()
{
	write("D.");
}

/*======================================================================*
                               TestE
 *======================================================================*/
void TestE()
{
	write("E.");
}

/*======================================================================*
                               TestF
 *======================================================================*/
void TestF()
{
	while(1){
		if(rw == WRITE) {
			print_str("writer run.\n");
		}else{
			for(int i = 0; i < 3; i++){
				if(readers[i] == TRUE){
					char str[3];
					str[0] = 'A' + i;
					str[1] = ' ';
					str[2] = '\0';
					print_str(str);
				}
			}
			print_str("reader run.\n");
		}
		milli_delay(TIME_SLICE);
		if(disp_pos >= 80 * 25 * 2) clean_screen();
	}
	//write("F.");
}

/*======================================================================*
                               clean_screen
 *======================================================================*/
PUBLIC void clean_screen(){
	memset(0xB8000, 0, 80 * 25 * 2);
	disp_pos = 0;
}

/*======================================================================*
                               wait
 *======================================================================*/
PUBLIC void wait(SEMAPHORE* s){
	*(s->queue + s->length) = p_proc_ready - proc_table;
	p_proc_ready->wait = TRUE;
	s->length++;
	// disp_int(p_proc_ready - proc_table);
}

/*======================================================================*
                               wake_up
 *======================================================================*/
PUBLIC void wake_up(SEMAPHORE* s){
	p_proc_ready = proc_table + s->queue[0];
	p_proc_ready->wait = FALSE;
	for(int i = 0; i < s->length - 1; i++){
		s->queue[i] = s->queue[i + 1];
	}
	s->queue[s->length] = -1;
	s->length--;
}

/*======================================================================*
                               read_file
 *======================================================================*/
PRIVATE void read_file(char* name){
	rw = READ;
	readers[name[0] - 'A'] = TRUE;
	print_str(name);
	print_str(" start read.\n");
	int cnt = 0;
	switch(name[0]){
		case 'A':
			cnt = 2;
			break;
		case 'B':
			cnt = 3;
			break;
		case 'C':
			cnt = 3;
			break;
	}

	while(cnt > 0){
		print_str(name);
		print_str(" is reading.\n");
		milli_delay(TIME_SLICE);
		cnt--;
	}

	print_str(name);
	print_str(" end read.\n");
	readers[name[0] - 'A'] = FALSE;
}
/*======================================================================*
                               read
 *======================================================================*/
PUBLIC void read(char* name){
	// reader first
	while(1){
		P(&rmutex);
		if(readcount == 0) P(&wmutex);
		readcount++;
		V(&rmutex);

		P(&max_read);
		read_file(name);
		V(&max_read);

		P(&rmutex);
		readcount--;
		if(readcount == 0) V(&wmutex);
		V(&rmutex);

		milli_delay(10 * TIME_SLICE);
	}

	// writer first
	/*while(1){
		P(&z);
		P(&rmutex);
		P(&x);
		if(readcount >= max_read) {
			milli_delay(TIME_SLICE);
		}
		readcount++;
		if(readcount == 1) P(&wmutex);

		V(&x);
		V(&rmutex);
		V(&z);

		read_file(name);

		P(&x);
		readcount--;
		if(readcount == 0) V(&wmutex);
		V(&x);

		//process_sleep(TIME_SLICE);
	}*/
}

/*======================================================================*
                               write_file
 *======================================================================*/
PRIVATE void write_file(char* name){
	rw = WRITE;
	print_str(name);
	print_str(" start write.\n");

	int cnt = 0;
	switch(name[0]){
		case 'D':
			cnt = 3;
			break;
		case 'E':
			cnt = 4;
			break;
	}

	while(cnt > 0){
		print_str(name);
		print_str(" is writing.\n");
		milli_delay(TIME_SLICE);
		cnt--;
	}

	print_str(name);
	print_str(" end write.\n");
}

/*======================================================================*
                               write
 *======================================================================*/
PUBLIC void write(char* name){
	// reader first
	while(1){
		P(&wmutex);

		write_file(name);

		V(&wmutex);

		//process_sleep(TIME_SLICE);
	}

	// writer first
	/*while(1){
		P(&y);
		writecount++;
		if(writecount == 1) P(&rmutex);
		V(&y);
		P(&wmutex);

		write_file(name);

		V(&wmutex);
		P(&y);
		writecount--;
		if(writecount == 0) V(&rmutex);
		V(&y);

		milli_delay(5 * TIME_SLICE);

	}*/
}


