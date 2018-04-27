#ifndef __MODBUS_CPP_
#define __MODBUS_CPP_

#include <cerrno>
#include <iostream>
#include <vector>
#include <cstdint>
#include <exception>
#include <modbus/modbus.h>


class ModBusConnector
{
	private:
	
		modbus_t * ctx;  //libmodbus context
		bool is_connected;  //record connection state;
	
	public:

		ModBusConnector(const std::string & ip, const int & port);
	
		~ModBusConnector();
	
		int read_bits(const int & addr, const int & num_of_bits, std::vector<std::uint8_t> & values);
		//read a number of coils
	
		int read_input_bits(const int & addr, const int & num_of_bits, std::vector<std::uint8_t> & values);
		//read a number of discrete inputs
	
		int read_registers(const int & addr, const int & num_of_registers, std::vector<std::uint16_t> & values);
		//read a number of holding registers
	
		int read_input_registers(const int & addr, const int & num_of_registers, std::vector<std::uint16_t> & values);
		//read a number of input registers
	
		int write_bit(const int & addr, const std::uint8_t& value);
		//write value into a single coil
	
		int write_bits(const int & addr, const int & num_of_bits, const std::vector<std::uint8_t> & values);
		//write values into a number of coils
	
		int write_register(const int & addr, const std::uint16_t& value);
		//write value into a single holding register
	
		int write_registers(const int & addr, const int & num_of_registers, const std::vector<std::uint16_t> & values);
		//write values into a number of holding registers
	
		int write_and_read_registers(const int & write_addr, const int & num_of_registers_to_write,
									 const std::vector<std::uint16_t> & values_to_write,
							 		 const int & read_addr, const int & num_registers_to_read,
							 		 std::vector<std::uint16_t> & values_to_read);
		//write values into a number of holding registers and then read values back from those registers
		
		static void set_float(const float& f, std::uint16_t& register0, std::uint16_t & register1);
		//convert a float type value to two holding registers
		
		static float get_float(const std::uint16_t& register0, const std::uint16_t & register1);
		//get a float type value from two holding registers
		
		void connect();
		//build a modbus connection
							 
		void disconnect();
		// disconnect a modbus connection
		// Even without calling disconnect(), the connection will be disconnected
		// when the class instance is destructed
	
		void set_debug(bool flag);	
		// enable modbus verbose message mode
};

#endif
