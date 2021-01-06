socket-server: main.c json.c utils.c ini.c logger.c
	gcc -g -std=c99 -Wall -o $@ $^ -lcrypto -ljson-c -pthread

clean:
	rm socket-server
