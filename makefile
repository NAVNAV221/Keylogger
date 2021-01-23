all:
	echo "run, build, clear"
run:
	sudo ./keylogger_spy
build:
	gcc main.c -o keylogger_spy
clear:
	rm keylogger_spy
debug:
	gcc -g main.c
	sudo gdb a.out
