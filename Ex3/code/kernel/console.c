
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			      console.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/*
	回车键: 把光标移到第一列
	换行键: 把光标前进到下一行
*/


#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "keyboard.h"
#include "proto.h"

PRIVATE void set_cursor(unsigned int position);
PRIVATE void set_video_start_addr(u32 addr);
PRIVATE void flush(CONSOLE* p_con);
PRIVATE void reset_stay_time();
PRIVATE void clean(CONSOLE* p_con);
PRIVATE void search(CONSOLE* p_con);
PRIVATE void reset_text_color(CONSOLE* p_con);

PRIVATE int stay_time = 0;
EXTERN int mode;

/*======================================================================*
			   init_screen
 *======================================================================*/
PUBLIC void init_screen(TTY* p_tty)
{
	int nr_tty = p_tty - tty_table;
	p_tty->p_console = console_table + nr_tty;

	int v_mem_size = V_MEM_SIZE >> 1;	/* 显存总大小 (in WORD) */

	int con_v_mem_size                   = v_mem_size / NR_CONSOLES;
	p_tty->p_console->original_addr      = nr_tty * con_v_mem_size;
	p_tty->p_console->v_mem_limit        = con_v_mem_size;
	p_tty->p_console->current_start_addr = p_tty->p_console->original_addr;

	/* 默认光标位置在最开始处 */
	p_tty->p_console->cursor = p_tty->p_console->original_addr;

	if (nr_tty == 0) {
		/* 第一个控制台沿用原来的光标位置 */
		p_tty->p_console->cursor = disp_pos / 2;
		disp_pos = 0;
	}
	else {
		out_char(p_tty->p_console, nr_tty + '0');
		out_char(p_tty->p_console, '#');
	}

	set_cursor(p_tty->p_console->cursor);
	clean(p_tty->p_console);
}


/*======================================================================*
			   is_current_console
*======================================================================*/
PUBLIC int is_current_console(CONSOLE* p_con)
{
	return (p_con == &console_table[nr_current_console]);
}


/*======================================================================*
			   out_char
 *======================================================================*/
PUBLIC void out_char(CONSOLE* p_con, char ch)
{
	if(mode == AFTER_SEARCH) return;

	u8* p_vmem = (u8*)(V_MEM_BASE + p_con->cursor * 2);
	
	switch(ch) {
	case '\n':
		if(mode == INSERT){
			if (p_con->cursor < p_con->original_addr +
		    	p_con->v_mem_limit - SCREEN_WIDTH) {
				unsigned int cursor = p_con->original_addr + SCREEN_WIDTH * 
					((p_con->cursor - p_con->original_addr) /
					 SCREEN_WIDTH + 1);
				while(p_con->cursor != cursor){
					*(p_vmem++) = ' ';
					*(p_vmem++) = BLACK;
					p_con->cursor++;
				}	
			
			}
		}else{
			mode = AFTER_SEARCH;
			search(p_con);
		}
		break;
	case '\b':
		if(mode == INSERT){
			if (p_con->cursor > p_con->original_addr) {
				if(*(p_vmem-1) == BLACK){
					int cnt = 0;
					while(*(p_vmem - 1) == BLACK && cnt < SCREEN_WIDTH){
						p_con->cursor--;
						*(p_vmem-2) = ' ';
						*(p_vmem-1) = DEFAULT_CHAR_COLOR;
						p_vmem -= 2;
						cnt++;
					}
				}else if(*(p_vmem-1) == BLUE){
					int cnt = 0;
					while(cnt < 4){
						p_con->cursor--;
						*(p_vmem-2) = ' ';
						*(p_vmem-1) = DEFAULT_CHAR_COLOR;
						p_vmem -= 2;
						cnt++;
					}
				}else{
					p_con->cursor--;
					*(p_vmem-2) = ' ';
					*(p_vmem-1) = DEFAULT_CHAR_COLOR;
				}

			}
		}else{
			if (p_con->cursor > p_con->search_cursor) {
				if(*(p_vmem-1) == BLACK){
					int cnt = 0;
					while(*(p_vmem - 1) == BLACK && cnt < SCREEN_WIDTH){
						p_con->cursor--;
						*(p_vmem-2) = ' ';
						*(p_vmem-1) = DEFAULT_CHAR_COLOR;
						p_vmem -= 2;
						cnt++;
					}
				}else if(*(p_vmem-1) == BLUE){
					int cnt = 0;
					while(cnt < 4){
						p_con->cursor--;
						*(p_vmem-2) = ' ';
						*(p_vmem-1) = DEFAULT_CHAR_COLOR;
						p_vmem -= 2;
						cnt++;
					}
				}else{
					p_con->cursor--;
					*(p_vmem-2) = ' ';
					*(p_vmem-1) = DEFAULT_CHAR_COLOR;
				}

			}
		}
		break;
	case '\t':
		if (p_con->cursor <
		    p_con->original_addr + p_con->v_mem_limit - 1) {
			int cnt = 0;
			while(cnt < 4){
				*p_vmem++ = ' ';
				*p_vmem++ = BLUE;
				p_con->cursor++;
				cnt++;
			}
			
		}
		break;
	default:
		if (p_con->cursor <
		    p_con->original_addr + p_con->v_mem_limit - 1) {
			*p_vmem++ = ch;
			if(mode == INSERT || ch == ' '){
				*p_vmem++ = DEFAULT_CHAR_COLOR;
			}else{
				*p_vmem++ = RED;
			}

			p_con->cursor++;
		}
		break;
	}

	while (p_con->cursor >= p_con->current_start_addr + SCREEN_SIZE) {
		scroll_screen(p_con, SCR_DN);
	}
	reset_stay_time();
	flush(p_con);
}

/*======================================================================*
                           flush
*======================================================================*/
PRIVATE void flush(CONSOLE* p_con)
{
        set_cursor(p_con->cursor);
        set_video_start_addr(p_con->current_start_addr);
}

/*======================================================================*
			    set_cursor
 *======================================================================*/
PRIVATE void set_cursor(unsigned int position)
{
	disable_int();
	out_byte(CRTC_ADDR_REG, CURSOR_H);
	out_byte(CRTC_DATA_REG, (position >> 8) & 0xFF);
	out_byte(CRTC_ADDR_REG, CURSOR_L);
	out_byte(CRTC_DATA_REG, position & 0xFF);
	enable_int();
}

/*======================================================================*
			  set_video_start_addr
 *======================================================================*/
PRIVATE void set_video_start_addr(u32 addr)
{
	disable_int();
	out_byte(CRTC_ADDR_REG, START_ADDR_H);
	out_byte(CRTC_DATA_REG, (addr >> 8) & 0xFF);
	out_byte(CRTC_ADDR_REG, START_ADDR_L);
	out_byte(CRTC_DATA_REG, addr & 0xFF);
	enable_int();
}



/*======================================================================*
			   select_console
 *======================================================================*/
PUBLIC void select_console(int nr_console)	/* 0 ~ (NR_CONSOLES - 1) */
{
	if ((nr_console < 0) || (nr_console >= NR_CONSOLES)) {
		return;
	}

	nr_current_console = nr_console;

	set_cursor(console_table[nr_console].cursor);
	set_video_start_addr(console_table[nr_console].current_start_addr);
}

/*======================================================================*
			   scroll_screen
 *----------------------------------------------------------------------*
 滚屏.
 *----------------------------------------------------------------------*
 direction:
	SCR_UP	: 向上滚屏
	SCR_DN	: 向下滚屏
	其它	: 不做处理
 *======================================================================*/
PUBLIC void scroll_screen(CONSOLE* p_con, int direction)
{
	if (direction == SCR_UP) {
		if (p_con->current_start_addr > p_con->original_addr) {
			p_con->current_start_addr -= SCREEN_WIDTH;
		}
	}
	else if (direction == SCR_DN) {
		if (p_con->current_start_addr + SCREEN_SIZE <
		    p_con->original_addr + p_con->v_mem_limit) {
			p_con->current_start_addr += SCREEN_WIDTH;
		}
	}
	else{
	}

	set_video_start_addr(p_con->current_start_addr);
	set_cursor(p_con->cursor);
}
/*======================================================================*
			   clean
 *======================================================================*/
PRIVATE void clean(CONSOLE* p_con)
{
	u8* p_vmem = (u8*)(V_MEM_BASE + p_con->cursor * 2);
	while(p_con->cursor > p_con->original_addr){
		p_con->cursor--;
		*(p_vmem-2) = ' ';
		*(p_vmem-1) = DEFAULT_CHAR_COLOR;
		p_vmem -= 2;
	}
	flush(p_con);
}
/*======================================================================*
			   clean_screen
 *======================================================================*/
PUBLIC void clean_screen(CONSOLE* p_con)
{
	if(mode == SEARCH){
		return;
	}
	if(((get_ticks() - stay_time) * 1000 / HZ) > 200000){
		clean(p_con);
	}
}
/*======================================================================*
			   reset_stay_time
 *======================================================================*/
PRIVATE void reset_stay_time()
{
	stay_time = get_ticks();
}
/*======================================================================*
			   search
 *======================================================================*/
PRIVATE void search(CONSOLE* p_con)
{
	reset_text_color(p_con);
	unsigned int cursor = p_con->original_addr;
	while(cursor < p_con->search_cursor){
		u8* p_vmem = (u8*)(V_MEM_BASE + cursor * 2);
		u8* p_vmem_start = (u8*)(V_MEM_BASE + p_con->search_cursor * 2);

		if(*p_vmem == *p_vmem_start){
			if(*p_vmem == ' ' && *(p_vmem + 1) != *(p_vmem_start + 1)){
				//disp_str("#");
				cursor++;
				continue;
			}
			unsigned int s_cursor = cursor;
			unsigned int search_cursor = p_con->search_cursor;
			int judge = TRUE;
			int len = 0;
			while(search_cursor < p_con->cursor){
				u8* s_p_vmem = (u8*)(V_MEM_BASE + s_cursor * 2);
				u8* search_p_vmem = (u8*)(V_MEM_BASE + search_cursor * 2);
				if(*s_p_vmem == *search_p_vmem){
					if(*s_p_vmem == ' ' && *(s_p_vmem + 1) != *(search_p_vmem + 1)){
						//disp_str("#");
						judge = FALSE;
						break;
					}
				}else{
					judge = FALSE;
					break;
				}
				s_cursor++;
				search_cursor++;
				len++;
			}

			if(judge == TRUE){
				s_cursor = cursor;
				int cnt = 0;
				while(cnt < len){
					//disp_str("$");
					u8* s_p_vmem = (u8*)(V_MEM_BASE + s_cursor * 2);
					s_cursor++;
					cnt++;
					if(*s_p_vmem == ' ') continue;
					*(s_p_vmem + 1) = RED;
				}
			}
		}
		cursor++;
	}
}
/*======================================================================*
			   leave_search_mode
 *======================================================================*/
PUBLIC void leave_search_mode(CONSOLE* p_con)
{
	u8* p_vmem = (u8*)(V_MEM_BASE + p_con->cursor * 2);
	while(p_con->cursor > p_con->search_cursor){
		p_con->cursor--;
		*(p_vmem-2) = ' ';
		*(p_vmem-1) = DEFAULT_CHAR_COLOR;
		p_vmem -= 2;
	}
	flush(p_con);
	unsigned int cursor = p_con->original_addr;
	reset_text_color(p_con);
}
/*======================================================================*
			   reset_text_color
 *======================================================================*/
PRIVATE void reset_text_color(CONSOLE* p_con)
{
	unsigned int cursor = p_con->original_addr;
	while(cursor < p_con->search_cursor){
		u8* p_vmem = (u8*)(V_MEM_BASE + cursor * 2);
		cursor++;
		if(*p_vmem == ' ') continue;
		*(p_vmem + 1) = DEFAULT_CHAR_COLOR;
	}
}
