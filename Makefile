all:
	gcc -lcrypto -ljson-c -o tcp_server main.c
