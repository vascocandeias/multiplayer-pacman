CC = gcc																			# compilador a usar
# CFLAGS = -g -Wall -O3 -pedantic													# opções de compilação
CFLAGS = -g -Wall -pedantic															# opções de compilação
LFLAGS = -lSDL2 -lSDL2_image -lpthread												# opções de linkagem
OBJS = server.c list.o UI_library.o communication.o board.o players.o fruits.o		# ficheiros objecto
EXEC = server																		# nome do executável

server: $(OBJS)
	@$(CC) $(LFLAGS) -o $(EXEC) $(OBJS) 

client: client.c UI_library.o
	@$(CC) $(LFLAGS) -o client client.c UI_library.o

board.o: board.c board.h
	@$(CC) $(CFLAGS) -c board.c

list.o: list.c list.h
	@$(CC) $(CFLAGS) -c list.c

fruits.o: fruits.c fruits.h
	@$(CC) $(CFLAGS) -c fruits.c

communication.o: communication.c communication.h
	@$(CC) $(CFLAGS) -c communication.c

players.o: players.c players.h
	@$(CC) $(CFLAGS) -c players.c

UI_library.o: UI_library.c
	@$(CC) $(CFLAGS) -c UI_library.c

valgrind:
	@valgrind --leak-check=full --show-leak-kinds=all --suppressions="./darwin9.supp" --undef-value-errors=no ./$(EXEC) 

clean-server:
	@rm -f *.o server

clean-client:
	@rm -f *.o client