# ModbusCpp
A Simple C++ Lib for ModBus

## Use the simple modbus client and server demo

### Prerequisite

- Install libmodbus:

  For Debian/Ubuntu
   ```
   $ sudo apt update
   $ sudo apt install libmodbus-dev
   ```

### Compile

1. Enter the directory of `./ModbusCpp`
2. Type the command in terminal
   ```
   $ make
   ```
3. You should see two executables produced, where client.run is the modbus client and server_m.run is modbus server

### Use the server
1. In terminal, type the command below where the server_m.run is located
   ```
   $ ./server_m.run
   ```
2. you can exit the server by "ctrl+C" keyboard combo 

### Use the client
1. Prepare a conf file called `PLC.conf` containing the modbus server ip and port
   and the definitions of modbus variables
   for the client in the same directory with `client.run`
   
   **The `PLC.conf` for SMA also works for this client**
2. In terminal, type the command below where the client.run is located
   ```
   $ ./client.run
   ```
3. You should see the welcome message in the terminal
   
   if the welcome message doesn't display nicely, 
   please adjust the width of the terminal window
4. Type the command the send/read value to/from Modbus Server
5. Commands Usage:

   Format:
   ```
   COMMAND [OPTION_1] [OPTION_2]
   ```
   available commands
   ```
   Command usage                 Description
   ```
   ```
   help                          display help information
   exit                          exit program
   v                             display available variables
   r VAR_NAME                    read value of VAR_NAME
   w VAR_NAME                    write random numbers into VAR_NAME
   rw VAR_NAME                   write random numbers into VAR_NAME and read the values
   rr VAR_NAME                   read real value of VAR_NAME
   wr VAR_NAME                   write random real numbers into VAR_NAME
   rwr VAR_NAME                  write random real numbers into VAR_NAME and read the values
   wr VAR_NAME REAL_VALUE        write the real number REAL_VALUE into VAR_NAME
   rwr VAR_NAME REAL_VALUE       write real number REAL_VALUE into VAR_NAME and then read the values from it
   ```   
   where `VAR_NAME` is the modbus variable name defined in `PLC.conf` and `REAL_VALUE` is a real number
