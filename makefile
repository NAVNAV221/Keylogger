all:
	echo "run, build, clear"
run:
	./keylogger
build:
	gcc keylogger.c -o keylogger_spy
clear:
	rm keylogger_spy