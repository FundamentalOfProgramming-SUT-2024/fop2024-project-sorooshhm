OBJS=utils.o db.o auth.o menu.o form.o main.o 

all: exec

run: exec
	./exec

exec: $(OBJS) Makefile
	gcc $(OBJS) -lncurses -o exec

%.o: %.c Makefile
	gcc -c -lncurses -o $@ $<