OBJS=utils.o db.o auth.o menu.o form.o game.o main.o 

all: exec

run: exec
	./exec

exec: $(OBJS) Makefile
	gcc $(OBJS) -fsanitize=address -g -lncursesw -lSDL2 -lSDL2_mixer -lpq -o exec

%.o: %.c Makefile
	gcc -c -lncursesw -o $@ $<