CC=gcc
CFLAGS=-g -std=c99 -Wall
CLIBS=-lcrypto -ljson-c -pthread
SOURCES=main.c json.c utils.c ini.c logger.c
EXECUTABLE=astroclime_server

LOG_PATH = /var/log/astroclime/

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(SOURCES)
	$(CC) $(CFLAGS) -o $@ $^ $(CLIBS)

clean:
	rm $(EXECUTABLE)
