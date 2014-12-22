main: main.c interface.glade
	gcc -g -rdynamic -o main main.c `pkg-config --cflags --libs gtk+-3.0`
