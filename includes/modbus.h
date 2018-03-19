#ifndef __MODBUS_CPP_
#define __MODBUS_CPP_

#include <modbus/modbus.h>
#include <errno.h>
#include <iostream>
#include <vector>

using namespace std;

class ModBusConnector
{
	private:
	
		modbus_t * ctx;  //libmodbus context
	
	public:
	
		ModBusConnector(const string & ip, const int & port);
	
		~ModBusConnector();
	
		int read_bits(const int & addr, const int & num_of_bits, vector<uint8_t> & values);
	
		int read_input_bits(const int & addr, const int & num_of_bits, vector<uint8_t> & values);
	
		int read_registers(const int & addr, const int & num_of_registers, vector<uint16_t> & values);
	
		int read_input_registers(const int & addr, const int & num_of_registers, vector<uint16_t> & values);
	
		int write_bit(const int & addr, const uint8_t& value);
	
		int write_bits(const int & addr, const int & num_of_bits, const vector<uint8_t> & values);
	
		int write_register(const int & addr, const uint16_t& value);
	
		int write_registers(const int & addr, const int & num_of_registers, const vector<uint16_t> & values);
	
		int write_and_read_registers(const int & write_addr, const int & num_of_registers_to_write,
									 const vector<uint16_t> & values_to_write,
							 		 const int & read_addr, const int & num_registers_to_read,
							 		 vector<uint16_t> & values_to_read);
		
		int isConnected();
							 
		void disconnect();
	
		void set_debug(bool flag);	
	
};

#endif
