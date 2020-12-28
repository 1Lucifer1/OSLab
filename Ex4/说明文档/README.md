# 说明文档

## 基本思路

### 睡眠

1. 对进程添加一个变量sleep，并且在sleep不等于0时，schedule()函数不分配时间片给这个进程。

### 打印字符串

1. 调用disp_color_str()函数，并且定义color数组使得每一个进程对应的颜色不同。
2. 在kliba.asm里面对disp_color_str()函数添加esi、edi的进栈出栈，避免系统因为时钟中断而导致字符串打印不全。


### PV操作

1. 当P的时候，信号量小于等于0时，表示没有可用资源，将进程放进队列等待，并且重新进行进程调度。
2. 当V的时候，信号量小于等于0时，表示仍有进程等待，将队列最开头的进程唤醒，并且重新进行进程调度。

### 读者写者问题

1. 读者优先与写者优先时考虑的进程饿死问题，我将读者和写者的后面都加上睡眠时间，让读者在读完一轮之后，写者可以写，避免写者在读者优先时进程饿死，反之同理。
2. 读数据时，加入PV操作控制读的人数。
3. 修改schedule()函数使得读者优先与写者优先遍历进程的方向不一样。

## 添加操作

### 添加系统调用

1. 添加系统调用处理函数体
2. 在system.call中添加相应的用户调用函数体
3. 在proto.h中声明
4. 如果有参数，则要修改系统调用中的寄存器进栈出栈(kernel.asm)

### 添加进程

1. 添加进程体
2. 在task_table中增加一项(global.c)
3. 让nr_tasks加1(proc.h)
4. 定义任务堆栈(proc.h)
5. 修改stack_size_total (proc.h)

## 关键代码

### main.c

#### 读者代码

```c
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
```

#### 写者代码

```c
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
```

#### 每个时间片打印状态

```c
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
```

#### 进程等待与唤醒

```c
PUBLIC void wait(SEMAPHORE* s){
	*(s->queue + s->length) = p_proc_ready - proc_table;
	p_proc_ready->wait = TRUE;
	s->length++;
	// disp_int(p_proc_ready - proc_table);
}

PUBLIC void wake_up(SEMAPHORE* s){
	p_proc_ready = proc_table + s->queue[0];
	p_proc_ready->wait = FALSE;
	for(int i = 0; i < s->length - 1; i++){
		s->queue[i] = s->queue[i + 1];
	}
	s->queue[s->length] = -1;
	s->length--;
}
```

### proc.c

#### 系统调用：睡眠

```c
PUBLIC int sys_process_sleep(int milli_seconds)
{
	p_proc_ready->sleep = milli_seconds;
	// disp_int(milli_seconds);
	return 0;
}
```

#### 系统调用：打印字符串

```c
PUBLIC int sys_print_str(char* str)
{
	int color = p_colors[p_proc_ready - proc_table];
	disp_color_str(str, color);
	return 0;
}
```

#### 系统调用：PV操作

```c
PUBLIC int sys_P(SEMAPHORE* s)
{
	s->value--;
	if(s->value < 0) wait(s);
	schedule();
	return 0;
}

PUBLIC int sys_V(SEMAPHORE* s)
{
	s->value++;
	if(s->value <= 0) wake_up(s);
	schedule();
	return 0;
}
```

### global.c

#### 变量声明

```c
PUBLIC	system_call	sys_call_table[NR_SYS_CALL] = {sys_get_ticks, sys_process_sleep, sys_print_str, sys_P, sys_V};

PUBLIC	int		p_colors[NR_TASKS] = {0x1, 0x2, 0x3, 0x4, 0x5, 0x6};

PUBLIC	SEMAPHORE	wmutex, rmutex, x, y, z, max_read;
PUBLIC	int		readcount = 0;
PUBLIC	int		writecount = 0;
PUBLIC	int		rw = WRITE;
PUBLIC	int		readers[3] = {FALSE, FALSE, FALSE};
```

