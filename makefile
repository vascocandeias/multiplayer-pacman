CC = gcc										# compilador a usar
CFLAGS = -g -Wall -O3 -pedantic					# opções de compilação
LFLAGS = -lSDL2 -lSDL2_image -lpthread			# opções de linkagem
OBJS = $(exec).c list.o  UI_library.o 			# ficheiros objecto
EXEC = $(exec) 									# nome do executável

$(EXEC): $(OBJS)
	@$(CC) $(LFLAGS) -o $(EXEC) $(OBJS) 

list.o: list.c list.h
	@$(CC) $(CFLAGS) -c list.c

UI_library.o: UI_library.c
	@$(CC) $(CFLAGS) -c UI_library.c
# valgrind:
# 	@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(EXEC) $(file)

clean:
	@rm -f *.o $(EXEC)