
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            proto.h
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#define	READ 0
#define WRITE 1
typedef struct semaphore{
	int	value;
	int	queue[10];
	int	length;
}SEMAPHORE;

#define TIME_SLICE 1000

/* klib.asm */
PUBLIC void	out_byte(u16 port, u8 value);
PUBLIC u8	in_byte(u16 port);
PUBLIC void	disp_str(char * info);
PUBLIC void	disp_color_str(char * info, int color);

/* protect.c */
PUBLIC void	init_prot();
PUBLIC u32	seg2phys(u16 seg);

/* klib.c */
PUBLIC void	delay(int time);

/* kernel.asm */
void restart();

/* main.c */
void TestA();
void TestB();
void TestC();
void TestD();
void TestE();
void TestF();
PUBLIC void wait(SEMAPHORE* s);
PUBLIC void wake_up(SEMAPHORE* s);
PUBLIC void read(char* name);
PUBLIC void write(char* name);

/* i8259.c */
PUBLIC void put_irq_handler(int irq, irq_handler handler);
PUBLIC void spurious_irq(int irq);

/* clock.c */
PUBLIC void clock_handler(int irq);


/* 以下是系统调用相关 */

/* proc.c */
PUBLIC  int     sys_get_ticks();        /* sys_call */
PUBLIC  int     sys_process_sleep(int milli_seconds);    
PUBLIC  int     sys_print_str(char* str);      
PUBLIC	int	sys_P(SEMAPHORE* s);
PUBLIC	int	sys_V(SEMAPHORE* s);

/* syscall.asm */
PUBLIC  void    sys_call();             /* int_handler */
PUBLIC  int     get_ticks();
PUBLIC  int     process_sleep(int milli_seconds);    
PUBLIC  int     print_str(char* str);
PUBLIC	int	P(SEMAPHORE* s);
PUBLIC	int	V(SEMAPHORE* s);

