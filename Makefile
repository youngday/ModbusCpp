CC=g++
CFLAGS= -O3 -Wall -std=c++11
LIBS = -lmodbus
SRCS = client_demo.cpp server.cpp parser.cpp
CLIOBJS = client_demo.o modbus.o parser.o
SEROBJS = server.o
#MAIN = test
DEPS = 
INCLUDES=/usr/lib/

.PHONY: clean

all: client
	@echo  Simple compiler has been compiled

server: $(SEROBJS) 
	$(CC) $(CFLAGS) -I $(INCLUDES) -o server $(SEROBJS) $(LFLAGS) $(LIBS)
	
client: $(CLIOBJS) 
	$(CC) $(CFLAGS) -I $(INCLUDES) -o client $(CLIOBJS) $(LFLAGS) $(LIBS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $<  -o $@ 

clean:
	$(RM) *.o *~ $(MAIN)

# DO NOT DELETE THIS LINE -- make depend needs it
