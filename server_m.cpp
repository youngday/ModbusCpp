#include <iostream>
#include "includes/modbus.h"

int main(void)
{
    /* modbus server instance, bind to 0.0.0.0:1502 
       the number of coil bits, input bits, holding registers and input registers are 9999
    */
    static ModBusServer server("0.0.0.0", 1502, 9999, 9999, 9999, 9999);

    /* start to listen to incomming connections
       max number of pending connections waiting
       for the server to accept in queue is 5 */
    server.listen(5);

    for (;;)
    {
        /* wait for any connections to be ready for I/O,
           connection_count is number of connections ready for I/O
        */
        int connection_count = server.wait();

        /* iterate over the ready connections to handle the request */
        for (int n = 0; n < connection_count; ++n)
        {
            if (server.process(n)) /* true if we build a new connection */
                std::cout << "New Connection" << std::endl;
        }
    }
}