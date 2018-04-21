CC=g++
CFLAGS= -O3 -Wall
LIBS = -lmodbus
SRCS = client.c server.c server_m.c
CLIOBJS = client.o
SEROBJS = server.o
SER_MOBJS = server_m.o
#MAIN = test
DEPS = 
INCLUDES=/usr/lib/

.PHONY: clean

all:  server client server_m
	@echo  Simple compiler has been compiled

server_m: $(SER_MOBJS) 
	$(CC) $(CFLAGS) -I $(INCLUDES) -o server_m $(SEROBJS) $(LFLAGS) $(LIBS)

server: $(SEROBJS) 
	$(CC) $(CFLAGS) -I $(INCLUDES) -o server $(SEROBJS) $(LFLAGS) $(LIBS)
	
client: $(CLIOBJS) 
	$(CC) $(CFLAGS) -I $(INCLUDES) -o client $(CLIOBJS) $(LFLAGS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<  -o $@ 

clean:
	$(RM) *.o *~ $(MAIN)

# DO NOT DELETE THIS LINE -- make depend needs it
