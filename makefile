CC=gcc
CFLAGS=-c -std=c99 -g -Wall
LDFLAGS=

SOURCES=src/types.c src/engine.c src/query.c src/main.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=zsql

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	\$(CC) $(CFLAGS) $< -o $@

clean:
	\rm -f $(EXECUTABLE) $(OBJECTS)

