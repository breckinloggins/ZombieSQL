CC=gcc
CFLAGS=-std=c99 -g -Wall
LDFLAGS=

SOURCES=main.c types.c query.c message.c engine.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=zsql

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(EXECUTABLE) $(OBJECTS)

