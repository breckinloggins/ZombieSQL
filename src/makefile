CC=gcc
CFLAGS=-c -std=c99 -g -Wall
LDFLAGS=

SOURCES=message.c types.c engine.c query.c main.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=zsql

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(EXECUTABLE) $(OBJECTS)

