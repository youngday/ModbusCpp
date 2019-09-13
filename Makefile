CC=g++
CFLAGS= -O3 -Wall -Werror -Wextra -pedantic -std=c++11 -g -fsanitize=address
LIBS = -lmodbus
SRCS = client_demo.cpp server_m.cpp parser.cpp
CLIOBJS = client_demo.o modbus.o parser.o
SEROBJS = server_m.o modbus.o
#MAIN = test
DEPS = 
INCLUDES=-I/usr/lib/

.PHONY: clean

all: client server
	@echo  Simple modbus client and server has been compiled

server: $(SEROBJS) 
	$(CC) $(CFLAGS) $(INCLUDES) -o server_m.run $(SEROBJS) $(LFLAGS) $(LIBS)
	
client: $(CLIOBJS) 
	$(CC) $(CFLAGS) $(INCLUDES) -o client.run $(CLIOBJS) $(LFLAGS) $(LIBS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $<  -o $@ 

clean:
	$(RM) *.o *~ *.run

# DO NOT DELETE THIS LINE -- make depend needs it
