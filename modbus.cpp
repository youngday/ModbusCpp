/* 
 * modbus.cpp
 *
 * Description:
 * MODBUS connection class.
 *
 * Parameters:
 *	(none)
 *
 * Return Values:
 *	(none)
 *
 */
/* =========================================================================
 * Author(s):            M.S. Billah, J.A.Farrell
 * Last committed:       05/07/2018
 * Last changed by:      $Author: mbillah $
 * email:                farrell@ece.ucr.edu
 * Website:              http://www.ee.ucr.edu/~farrell/
 * 
 * This program carries no warranty, not even the implied
 * warranty of merchantability or fitness for a particular purpose.
 * 
 * Please email bug reports or suggestions for improvements to:
 * farrell@ece.ucr.edu or mbill002@ucr.edu
 * ========================================================================= */
#include "includes/modbus.h"
#include <cstdlib>
#include <cstring>


ModBusConnector::ModBusConnector(const std::string & ip, const int & port)
{
	this->ctx = modbus_new_tcp(ip.c_str(), port);
	//create modbux context
	
	if (this->ctx == NULL) {
    	std::cerr << "modbus.cpp: Unable to allocate libmodbus context" << std::endl;
    	throw std::runtime_error("Unable to allocate libmodbus context");
	}

}

ModBusConnector::~ModBusConnector()
{
	if (this->is_connected) //disconnect if connected;
	{
		modbus_close(this->ctx);  //libmodbus
		this->is_connected = false;
	}	
	
	if (this->ctx != NULL) // free modbus context
	{
		modbus_free(this->ctx);
		this->ctx = NULL;
	}
	
}

void ModBusConnector::connect() //build a modbus connection
{
	if (modbus_connect(this->ctx) == -1) {   //libmodbus 
    	//std::cerr << "modbus.cpp: Connection failed: " << modbus_strerror(errno) << std::endl;
    	//modbus_free(ctx);
    	//this->ctx = NULL; 	
    	throw std::runtime_error("Connection failed: " + std::string(modbus_strerror(errno)));
	}
	
	this->is_connected = true;
}

// disconnect a modbus connection
// Even without calling disconnect(), the connection will be disconnected
// when the class instance is destructed
void ModBusConnector::disconnect()
{
	if (this->is_connected)
	{
		modbus_close(this->ctx); //libmodbus 
		this->is_connected = false;
	}
}

void ModBusConnector::set_debug(bool flag)  // enable modbus verbose message mode
{
	modbus_set_debug(ctx, flag); //libmodbus 
}

//read a number of coils
int ModBusConnector::read_bits(const int & addr, const int & num_of_bits, std::vector<std::uint8_t> & values)
{
	std::uint8_t* tmp_values = (std::uint8_t *) malloc(num_of_bits * sizeof(std::uint8_t));
	//c libmodbus only takes array, temp array to store the data 
	if (tmp_values == NULL)
	{
		std::cerr << "modbus.cpp read_bits: Unable to allocate memory" << std::endl;
		return -1;
	}
	memset(tmp_values, 0, num_of_bits * sizeof(std::uint8_t));	
	int rc = modbus_read_bits(this->ctx, addr, num_of_bits, tmp_values); //libmodbus
	
	if (rc == num_of_bits) //if successfully
	{
		values = std::vector<std::uint8_t>(tmp_values,tmp_values+num_of_bits); // convert to c++ vector
	}
	
	free(tmp_values);
	
	return rc;
}

//read a number of discrete inputs
int ModBusConnector::read_input_bits(const int & addr, const int & num_of_bits, 
	std::vector<std::uint8_t> & values)
{
	std::uint8_t* tmp_values = (std::uint8_t *) malloc(num_of_bits * sizeof(std::uint8_t));
	//c libmodbus only takes array, temp array to store the data 
	if (tmp_values == NULL)
	{
		std::cerr << "modbus.cpp: Unable to allocate memory" << std::endl;
		return -1;
	}
	memset(tmp_values, 0, num_of_bits * sizeof(std::uint8_t));	
	int rc = modbus_read_input_bits(this->ctx, addr, num_of_bits, tmp_values); //libmodbus
	
	if (rc == num_of_bits) //if successfully
	{
		values = std::vector<std::uint8_t>(tmp_values,tmp_values+num_of_bits); // convert to c++ vector
	}
	
	free(tmp_values);
	
	return rc;
}

//read a number of holding registers
int ModBusConnector::read_registers(const int & addr, const int & num_of_registers, std::vector<std::uint16_t> & values)
{
	std::uint16_t* tmp_values = (std::uint16_t *) malloc(num_of_registers * sizeof(std::uint16_t));
	//c libmodbus only takes array, temp array to store the data 
	
	if (tmp_values == NULL)
	{
		std::cerr << "modbus.cpp: Unable to allocate memory" << std::endl;
		return -1;
	}
	memset(tmp_values, 0, num_of_registers * sizeof(std::uint16_t));	
	int rc = modbus_read_registers(this->ctx, addr, num_of_registers, tmp_values); //libmodbus
	
	if (rc == num_of_registers) //if successfully
	{
		values = std::vector<std::uint16_t>(tmp_values,tmp_values+num_of_registers); // convert to c++ vector
	}
	
	free(tmp_values);
	
	return rc;
}

//read a number of input registers
int ModBusConnector::read_input_registers(const int & addr, const int & num_of_registers, std::vector<std::uint16_t> & values)
{
	std::uint16_t* tmp_values = (std::uint16_t *) malloc(num_of_registers * sizeof(std::uint16_t));
	//c libmodbus only takes array, temp array to store the data 
	
	if (tmp_values == NULL)
	{
		std::cerr << "modbus.cpp: Unable to allocate memory" << std::endl;
		return -1;
	}
	memset(tmp_values, 0, num_of_registers * sizeof(std::uint16_t));	
	int rc = modbus_read_input_registers(this->ctx, addr, num_of_registers, tmp_values); //libmodbus
	
	if (rc == num_of_registers)  //if successfully
	{
		values = std::vector<std::uint16_t>(tmp_values,tmp_values+num_of_registers); // convert to c++ vector
	}
	
	free(tmp_values);
	
	return rc;
}

//write value into a single coil
int ModBusConnector::write_bit(const int & addr, const std::uint8_t& value)
{
	return modbus_write_bit(this->ctx, addr, value); //libmodbus
}

//write values into a number of coils
int ModBusConnector::write_bits(const int & addr, const int & num_of_bits, 
	const std::vector<std::uint8_t> & values)
{
	int num = values.size(); 
	
	//if num_of_bits is larger than the actual vector size, overwrite it with vector size
	
	if ( num > num_of_bits)
	{
		num = num_of_bits; 
	}
	
	std::uint8_t const* tmp_values = values.data(); //convert to array
	
	return modbus_write_bits(this->ctx, addr, num, tmp_values); //libmodbus
	
}

//write value into a single holding register
int ModBusConnector::write_register(const int & addr, const std::uint16_t& value)
{
	return modbus_write_register(this->ctx, addr, value); //libmodbus
}

//write values into a number of holding registers
int ModBusConnector::write_registers(const int & addr, const int & num_of_registers, 
									 const std::vector<std::uint16_t> & values)
{
	int num = values.size();
	
	//if num_of_bits is larger than the actual vector size, overwrite it with vector size
	
	if ( num > num_of_registers)
	{
		num = num_of_registers;
	}
	
	std::uint16_t const* tmp_values = values.data(); //convert to array
	
	return modbus_write_registers(this->ctx, addr, num, tmp_values); //libmodbus
	
}

//write values into a number of holding registers and then read values back from those registers
int ModBusConnector::write_and_read_registers(const int & write_addr, const int & num_of_registers_to_write,
											  const std::vector<std::uint16_t> & values_to_write,
									 		  const int & read_addr, const int & num_registers_to_read,
									 		  std::vector<std::uint16_t> & values_to_read)
{
	int num_to_write = values_to_write.size();
	
	//if num_of_bits is larger than the actual vector size, overwrite it with vector size

	if ( num_to_write > num_of_registers_to_write)
	{
		num_to_write = num_of_registers_to_write;
	}
	
	std::uint16_t const* tmp_values_to_write = values_to_write.data(); //convert to array
	
	std::uint16_t* tmp_values_to_read = (std::uint16_t *) malloc(num_registers_to_read * sizeof(std::uint16_t));
	if (tmp_values_to_read == NULL)
	{
		std::cerr << "modbus.cpp: Unable to allocate memory" << std::endl;
		return -1;
	}
	memset(tmp_values_to_read, 0, num_registers_to_read * sizeof(std::uint16_t));
	
	//libmodbus
	int rc = modbus_write_and_read_registers(this->ctx, write_addr, num_to_write, tmp_values_to_write,
										     read_addr, num_registers_to_read, tmp_values_to_read);
	if (rc == num_registers_to_read) //if read successfully
	{
		values_to_read = 
			std::vector<std::uint16_t>(tmp_values_to_read,tmp_values_to_read+num_registers_to_read); 
			// convert to c++ vector
	}
	
	free(tmp_values_to_read);
	return rc;
}

//convert a float type value to two holding registers
void ModBusConnector::set_float(const float & f, std::uint16_t& register0, std::uint16_t & register1)
{
	std::uint16_t tmp_values[2]={0};
	modbus_set_float(f, tmp_values); //libmodbus
	register0 = tmp_values[0];
	register1 = tmp_values[1];
}

//get a float type value from two holding registers
float ModBusConnector::get_float(const std::uint16_t& register0, const std::uint16_t & register1)
{
	std::uint16_t tmp_values[2];
	tmp_values[0] = register0;
	tmp_values[1] = register1;
	return modbus_get_float(tmp_values); //libmodbus
}

//convert a float type value to two holding registers
void ModBusConnector::set_float_swap(const float & f, std::uint16_t& register0, std::uint16_t & register1)
{
	std::uint16_t tmp_values[2]={0};
	modbus_set_float(f, tmp_values); //libmodbus
	register0 = tmp_values[1];
	register1 = tmp_values[0];
}

//get a float type value from two holding registers
float ModBusConnector::get_float_swap(const std::uint16_t& register0, const std::uint16_t & register1)
{
	std::uint16_t tmp_values[2];
	tmp_values[0] = register1;
	tmp_values[1] = register0;
	return modbus_get_float(tmp_values); //libmodbus
}


