CC=gcc
CFLAGS= -O0 -c -Wall -DLE_MACHINE
LDFLAGS=

SOURCES=$ dspp.c convert_byteLE_int16.c convert_aByte_f.c
OBJECTS=$(SOURCES:.c=.o)
#LIBS=$ -lX11
DEPTS=$ 

EXECUTABLE=dspp

all: $(DEPTS) $(SOURCES) $(OBJECTS) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o dspp -lm $(LIBS)
$(SOURCES):
	$(CC) $(CFLAGS) $< -lm -o $@

test:
	$(CC) $(CFLAGS) testIn1.c -o testIn1.o
	$(CC) $(CFLAGS) testOut1.c -o testOut1.o
	$(CC) $(LDFLAGS) testIn1.o -o testIn1 -lm $(LIBS)
	$(CC) $(LDFLAGS) testOut1.o -o testOut1 -lm $(LIBS)
	$(CC) $(CFLAGS) testIn2.c -o testIn2.o
	$(CC) $(CFLAGS) testOut2.c -o testOut2.o
	$(CC) $(LDFLAGS) testIn2.o -o testIn2 -lm $(LIBS)
	$(CC) $(LDFLAGS) testOut2.o -o testOut2 -lm $(LIBS)
clean:
	rm -fr $(OBJECTS) $(EXECUTABLE)
