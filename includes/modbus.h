#ifndef __MODBUS_CPP_
#define __MODBUS_CPP_

#include <cerrno>
#include <iostream>
#include <vector>
#include <cstdint>
#include <modbus/modbus.h>


class ModBusConnector
{
	private:
	
		modbus_t * ctx;  //libmodbus context
	
	public:
	
		ModBusConnector(const std::string & ip, const int & port);
	
		~ModBusConnector();
	
		int read_bits(const int & addr, const int & num_of_bits, std::vector<std::uint8_t> & values);
	
		int read_input_bits(const int & addr, const int & num_of_bits, std::vector<std::uint8_t> & values);
	
		int read_registers(const int & addr, const int & num_of_registers, std::vector<std::uint16_t> & values);
	
		int read_input_registers(const int & addr, const int & num_of_registers, std::vector<std::uint16_t> & values);
	
		int write_bit(const int & addr, const std::uint8_t& value);
	
		int write_bits(const int & addr, const int & num_of_bits, const std::vector<std::uint8_t> & values);
	
		int write_register(const int & addr, const std::uint16_t& value);
	
		int write_registers(const int & addr, const int & num_of_registers, const std::vector<std::uint16_t> & values);
	
		int write_and_read_registers(const int & write_addr, const int & num_of_registers_to_write,
									 const std::vector<std::uint16_t> & values_to_write,
							 		 const int & read_addr, const int & num_registers_to_read,
							 		 std::vector<std::uint16_t> & values_to_read);
		
		
		int isConnected();
							 
		void disconnect();
	
		void set_debug(bool flag);	
	
};

#endif
