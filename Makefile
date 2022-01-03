nsh:*.c
	gcc -O3 *.c -o nsh
debug:*.c
	gcc -g *.c -o nsh
	gdb nsh
