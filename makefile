CC=gcc
CFLAGS= -O0 -c -Wall
LDFLAGS=

SOURCES=$ dspp.c convert_byteLE_int16.c
OBJECTS=$(SOURCES:.c=.o)
#LIBS=$ -lX11
DEPTS=$ 

EXECUTABLE=siggen mag

all: $(DEPTS) $(SOURCES) $(OBJECTS) $(EXECUTABLE)

$(EXECUTABLE): $(SOURCES) $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o dspp -lm $(LIBS)
$(SOURCES):
	$(CC) $(CFLAGS) $< -lm -o $@

test:
	$(CC) $(CFLAGS) testIn.c -o testIn.o
	$(CC) $(CFLAGS) testOut.c -o testOut.o
	$(CC) $(LDFLAGS) testIn.o -o testIn -lm $(LIBS)
	$(CC) $(LDFLAGS) testOut.o -o testOut -lm $(LIBS)
clean:
	rm -fr $(OBJECTS) $(EXECUTABLE)
