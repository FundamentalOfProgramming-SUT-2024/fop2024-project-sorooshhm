CFLAGS=-lncurses -O3
OBJS=main.o

all: exec

run: exec
	./exec

exec: $(OBJS) Makefile
	gcc $(OBJS) -o exec

%.o: %.c Makefile
	gcc -c -o $@ $< $(CFLAGS)
