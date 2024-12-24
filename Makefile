OBJS=auth.o menu.o main.o 

all: exec

run: exec
	./exec

exec: $(OBJS) Makefile
	gcc $(OBJS) -lncurses -o exec

%.o: %.c Makefile
	gcc -lncurses -c -o $@ $<
