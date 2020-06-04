# multiplayer-pacman
Compiling commands:
* `make server` - compiles the server
* `make client` - compiles the client
* `make clean-server` - deletes every `*.o` file and the server executable
* `make clean-client` - deletes every `*.o` file and the client executable

Running commands:
* `./server [filename.txt]` -- run the server with the board configuration in `filename.txt`. If none is provided, the file used is named `board.txt`. If this does not exist, default dimensions are used.
* `./client server-ip server-port color-r color-g color-b` -- run a client with the server at `server-ip:server-port` and with the color with the given rgb.