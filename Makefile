OBJS=utils.o db.o auth.o menu.o form.o main.o 

all: exec

run: exec
	./exec

exec: $(OBJS) Makefile
	gcc $(OBJS) -lncurses -lSDL2 -lSDL2_mixer -o exec

%.o: %.c Makefile
	gcc -c -lncurses -o $@ $<