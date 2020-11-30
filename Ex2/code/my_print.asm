section .data
	fileColor db 27, '[1;31m', 0 
	fileColorLen equ $ - fileColor
	warnColor db 27, '[0m', 0 
	warnColorLen equ $ - warnColor

section .text
	global my_print
	global prepareOutputDirectory
	global prepareOutputFile

	;my_print(char* c, int length);
	;利用栈传递参数
	
	my_print:
		mov	edx,[esp+8]						;[esp+8] = Q - 04h, 得到length
		mov	ecx,[esp+4]						;[esp+4] = Q - 08h, 得到c
		mov	ebx,1
		mov	eax,4 
		int	80h
		ret
		
	
	prepareOutputFile:
		mov eax, 4
		mov ebx, 1
		mov ecx, warnColor
		mov edx, warnColorLen
		int 80h
		ret

	prepareOutputDirectory:
		mov eax, 4
		mov ebx, 1
		mov ecx, fileColor
		mov edx, fileColorLen
		int 80h
		ret