CC=gcc
CFLAGS=-g -std=c99 -Wall -D_GNU_SOURCE
CLIBS=-lcrypto -ljson-c -pthread -lhiredis
SOURCES=main.c json.c utils.c ini.c logger.c
EXECUTABLE=astroclime_server
CONFIG=cnf.ini
FW_PATH=/var/www/astroclime/fw/

LOG_PATH = /var/log/astroclime/

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(SOURCES)
	$(CC) $(CFLAGS) -o $@ $^ $(CLIBS)
	
copy:
	cp $(EXECUTABLE) /usr/sbin/
	cp $(CONFIG) /etc/astroclime/
	cp ./fw/firmware $(FW_PATH)

clean:
	rm /usr/sbin/$(EXECUTABLE)
	rm -rf /etc/astroclime
	rm -rf $(FW_PATH)
