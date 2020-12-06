all:
	echo "run, build, clear"
run:
	sudo ./keylogger_spy
build:
	gcc keylogger.c -o keylogger_spy
clear:
	rm keylogger_spy
debug:
	gcc -g keylogger.c
	sudo gdb a.out