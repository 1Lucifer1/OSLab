
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            global.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#define GLOBAL_VARIABLES_HERE

#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "proc.h"
#include "global.h"


PUBLIC	PROCESS			proc_table[NR_TASKS];

PUBLIC	char			task_stack[STACK_SIZE_TOTAL];

PUBLIC	TASK	task_table[NR_TASKS] = {{TestA, STACK_SIZE_TESTA, "TestA"},
					{TestB, STACK_SIZE_TESTB, "TestB"},
					{TestC, STACK_SIZE_TESTC, "TestC"},
					{TestD, STACK_SIZE_TESTD, "TestD"},
					{TestE, STACK_SIZE_TESTE, "TestE"},
					{TestF, STACK_SIZE_TESTF, "TestF"},};

PUBLIC	irq_handler		irq_table[NR_IRQ];

PUBLIC	system_call	sys_call_table[NR_SYS_CALL] = {sys_get_ticks, sys_process_sleep, sys_print_str, sys_P, sys_V};

PUBLIC	int		p_colors[NR_TASKS] = {0x1, 0x2, 0x3, 0x4, 0x5, 0x6};

PUBLIC	SEMAPHORE	wmutex, rmutex, x, y, z, max_read;
PUBLIC	int		readcount = 0;
PUBLIC	int		writecount = 0;
PUBLIC	int		rw = WRITE;
PUBLIC	int		readers[3] = {FALSE, FALSE, FALSE};
