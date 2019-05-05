CC=gcc
CFLAGS= -O0 -c -Wall
LDFLAGS=

SOURCES=$ dspp.c
OBJECTS=$(SOURCES:.c=.o)
#LIBS=$ -lX11
DEPTS=$ 

EXECUTABLE=siggen mag

all: $(DEPTS) $(SOURCES) $(OBJECTS) $(EXECUTABLE)

$(EXECUTABLE): $(SOURCES) $(OBJECTS)
	$(CC) $(LDFLAGS) dspp.o -o dspp -lm $(LIBS)
$(SOURCES):
	$(CC) $(CFLAGS) $< -lm -o $@

clean:
	rm -fr $(OBJECTS) $(EXECUTABLE)
