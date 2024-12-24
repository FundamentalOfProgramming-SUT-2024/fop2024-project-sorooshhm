OBJS=main.o menu.o

all: exec

run: exec
	./exec

exec: $(OBJS) Makefile
	gcc $(OBJS) -lncurses -o exec

%.o: %.c Makefile
	gcc -lncurses -c -o $@ $<
