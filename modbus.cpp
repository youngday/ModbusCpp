#include "includes/modbus.h"
#include <stdexcept>
#include <cstdlib>
#include <cstring>

ModBusConnector::ModBusConnector(const std::string & ip, const int & port)
{
	this->ctx = modbus_new_tcp(ip.c_str(), port);
	
	if (ctx == NULL) {
    	std::cerr << "Unable to allocate libmodbus context" << std::endl;
    	throw std::runtime_error("Unable to allocate libmodbus context");
	}

	if (modbus_connect(ctx) == -1) {
    	std::cerr << "Connection failed: " << modbus_strerror(errno) << std::endl;
    	modbus_free(ctx);
    	this->ctx = NULL;
    	throw std::runtime_error("Connection failed. ");
	}
}

ModBusConnector::~ModBusConnector()
{
	if (modbus_connect(ctx) != -1)
	{
		modbus_close(this->ctx);
	}
	
	if (this->ctx != NULL)
	{
		modbus_free(this->ctx);	
	}
	
}

int ModBusConnector::isConnected()
{
	return modbus_connect(ctx);
}

void ModBusConnector::disconnect()
{
	if (modbus_connect(ctx) != -1)
	{
		modbus_close(this->ctx);
	}
}

void ModBusConnector::set_debug(bool flag)
{
	modbus_set_debug(ctx, flag);
}

int ModBusConnector::read_bits(const int & addr, const int & num_of_bits, std::vector<std::uint8_t> & values)
{
	std::uint8_t* tmp_values = (std::uint8_t *) malloc(num_of_bits * sizeof(std::uint8_t));
	memset(tmp_values, 0, num_of_bits * sizeof(std::uint8_t));	
	int rc = modbus_read_bits(this->ctx, addr, num_of_bits, tmp_values);
	
	if (rc == num_of_bits)
	{
		values = std::vector<std::uint8_t>(tmp_values,tmp_values+num_of_bits);
	}
	
	free(tmp_values);
	
	return rc;
}

int ModBusConnector::read_input_bits(const int & addr, const int & num_of_bits, std::vector<std::uint8_t> & values)
{
	std::uint8_t* tmp_values = (std::uint8_t *) malloc(num_of_bits * sizeof(std::uint8_t));
	memset(tmp_values, 0, num_of_bits * sizeof(std::uint8_t));	
	int rc = modbus_read_input_bits(this->ctx, addr, num_of_bits, tmp_values);
	
	if (rc == num_of_bits)
	{
		values = std::vector<std::uint8_t>(tmp_values,tmp_values+num_of_bits);
	}
	
	free(tmp_values);
	
	return rc;
}

int ModBusConnector::read_registers(const int & addr, const int & num_of_registers, std::vector<std::uint16_t> & values)
{
	std::uint16_t* tmp_values = (std::uint16_t *) malloc(num_of_registers * sizeof(std::uint16_t));
	memset(tmp_values, 0, num_of_registers * sizeof(std::uint16_t));	
	int rc = modbus_read_registers(this->ctx, addr, num_of_registers, tmp_values);
	
	if (rc == num_of_registers)
	{
		values = std::vector<std::uint16_t>(tmp_values,tmp_values+num_of_registers);
	}
	
	free(tmp_values);
	
	return rc;
}

int ModBusConnector::read_input_registers(const int & addr, const int & num_of_registers, std::vector<std::uint16_t> & values)
{
	std::uint16_t* tmp_values = (std::uint16_t *) malloc(num_of_registers * sizeof(std::uint16_t));
	memset(tmp_values, 0, num_of_registers * sizeof(std::uint16_t));	
	int rc = modbus_read_input_registers(this->ctx, addr, num_of_registers, tmp_values);
	
	if (rc == num_of_registers)
	{
		values = std::vector<std::uint16_t>(tmp_values,tmp_values+num_of_registers);
	}
	
	free(tmp_values);
	
	return rc;
}

int ModBusConnector::write_bit(const int & addr, const std::uint8_t& value)
{
	return modbus_write_bit(this->ctx, addr, value);
}

int ModBusConnector::write_bits(const int & addr, const int & num_of_bits, const std::vector<std::uint8_t> & values)
{
	int num = values.size();
	
	if ( num > num_of_bits)
	{
		num = num_of_bits;
	}
	
	std::uint8_t const* tmp_values = values.data();
	
	return modbus_write_bits(this->ctx, addr, num, tmp_values);
	
}

int ModBusConnector::write_register(const int & addr, const std::uint16_t& value)
{
	return modbus_write_register(this->ctx, addr, value);
}

int ModBusConnector::write_registers(const int & addr, const int & num_of_registers, 
									 const std::vector<std::uint16_t> & values)
{
	int num = values.size();
	
	if ( num > num_of_registers)
	{
		num = num_of_registers;
	}
	
	std::uint16_t const* tmp_values = values.data();	
	
	return modbus_write_registers(this->ctx, addr, num, tmp_values);
	
}

int ModBusConnector::write_and_read_registers(const int & write_addr, const int & num_of_registers_to_write,
											  const std::vector<std::uint16_t> & values_to_write,
									 		  const int & read_addr, const int & num_registers_to_read,
									 		  std::vector<std::uint16_t> & values_to_read)
{
	int num_to_write = values_to_write.size();

	if ( num_to_write > num_of_registers_to_write)
	{
		num_to_write = num_of_registers_to_write;
	}
	
	std::uint16_t const* tmp_values_to_write = values_to_write.data();
	
	std::uint16_t* tmp_values_to_read = (std::uint16_t *) malloc(num_registers_to_read * sizeof(std::uint16_t));
	memset(tmp_values_to_read, 0, num_registers_to_read * sizeof(std::uint16_t));
	
	int rc = modbus_write_and_read_registers(this->ctx, write_addr, num_to_write, tmp_values_to_write,
										     read_addr, num_registers_to_read, tmp_values_to_read);
	if (rc == num_registers_to_read)
	{
		values_to_read = std::vector<std::uint16_t>(tmp_values_to_read,tmp_values_to_read+num_registers_to_read);
	}
	
	free(tmp_values_to_read);
	return rc;
}


