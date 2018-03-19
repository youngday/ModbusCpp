#include "includes/modbus.h"
#include <stdexcept>
#include <cstdlib>
#include <cstring>

using namespace std;


ModBusConnector::ModBusConnector(const string & ip, const int & port)
{
	this->ctx = modbus_new_tcp(ip.c_str(), port);
	
	if (ctx == NULL) {
    	cerr << "Unable to allocate libmodbus context" << endl;
    	throw runtime_error("Unable to allocate libmodbus context");
	}

	if (modbus_connect(ctx) == -1) {
    	cerr << "Connection failed: " << modbus_strerror(errno) << endl;
    	modbus_free(ctx);
    	this->ctx = NULL;
    	throw runtime_error("Connection failed. ");
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

int ModBusConnector::read_bits(const int & addr, const int & num_of_bits, vector<uint8_t> & values)
{
	uint8_t* tmp_values = (uint8_t *) malloc(num_of_bits * sizeof(uint8_t));
	memset(tmp_values, 0, num_of_bits * sizeof(uint8_t));	
	int rc = modbus_read_bits(this->ctx, addr, num_of_bits, tmp_values);
	
	if (rc == 1)
	{
		values = vector<uint8_t>(tmp_values,tmp_values+num_of_bits);
	}
	
	free(tmp_values);
	
	return rc;
}

int ModBusConnector::read_input_bits(const int & addr, const int & num_of_bits, vector<uint8_t> & values)
{
	uint8_t* tmp_values = (uint8_t *) malloc(num_of_bits * sizeof(uint8_t));
	memset(tmp_values, 0, num_of_bits * sizeof(uint8_t));	
	int rc = modbus_read_input_bits(this->ctx, addr, num_of_bits, tmp_values);
	
	if (rc == 1)
	{
		values = vector<uint8_t>(tmp_values,tmp_values+num_of_bits);
	}
	
	free(tmp_values);
	
	return rc;
}

int ModBusConnector::read_registers(const int & addr, const int & num_of_registers, vector<uint16_t> & values)
{
	uint16_t* tmp_values = (uint16_t *) malloc(num_of_registers * sizeof(uint16_t));
	memset(tmp_values, 0, num_of_registers * sizeof(uint16_t));	
	int rc = modbus_read_registers(this->ctx, addr, num_of_registers, tmp_values);
	
	if (rc == 1)
	{
		values = vector<uint16_t>(tmp_values,tmp_values+num_of_registers);
	}
	
	free(tmp_values);
	
	return rc;
}

int ModBusConnector::read_input_registers(const int & addr, const int & num_of_registers, vector<uint16_t> & values)
{
	uint16_t* tmp_values = (uint16_t *) malloc(num_of_registers * sizeof(uint16_t));
	memset(tmp_values, 0, num_of_registers * sizeof(uint16_t));	
	int rc = modbus_read_input_registers(this->ctx, addr, num_of_registers, tmp_values);
	
	if (rc == 1)
	{
		values = vector<uint16_t>(tmp_values,tmp_values+num_of_registers);
	}
	
	free(tmp_values);
	
	return rc;
}

int ModBusConnector::write_bit(const int & addr, const uint8_t& value)
{
	return modbus_write_bit(this->ctx, addr, value);
}

int ModBusConnector::write_bits(const int & addr, const int & num_of_bits, const vector<uint8_t> & values)
{
	int num = values.size();
	
	if ( num > num_of_bits)
	{
		num = num_of_bits;
	}
	
	uint8_t const* tmp_values = values.data();
	
	cout<<"write bits: "<< values[0]<<endl;
	
	return modbus_write_bits(this->ctx, addr, num, tmp_values);
	
}

int ModBusConnector::write_register(const int & addr, const uint16_t& value)
{
	return modbus_write_register(this->ctx, addr, value);
}

int ModBusConnector::write_registers(const int & addr, const int & num_of_registers, 
									 const vector<uint16_t> & values)
{
	int num = values.size();
	
	if ( num > num_of_registers)
	{
		num = num_of_registers;
	}
	
	uint16_t const* tmp_values = values.data();	
	
	return modbus_write_registers(this->ctx, addr, num, tmp_values);
	
}

int ModBusConnector::write_and_read_registers(const int & write_addr, const int & num_of_registers_to_write,
											  const vector<uint16_t> & values_to_write,
									 		  const int & read_addr, const int & num_registers_to_read,
									 		  vector<uint16_t> & values_to_read)
{
	int num_to_write = values_to_write.size();

	if ( num_to_write > num_of_registers_to_write)
	{
		num_to_write = num_of_registers_to_write;
	}
	
	uint16_t const* tmp_values_to_write = values_to_write.data();
	
	uint16_t* tmp_values_to_read = (uint16_t *) malloc(num_registers_to_read * sizeof(uint16_t));
	memset(tmp_values_to_read, 0, num_registers_to_read * sizeof(uint16_t));
	
	int rc = modbus_write_and_read_registers(this->ctx, write_addr, num_to_write, tmp_values_to_write,
										     read_addr, num_registers_to_read, tmp_values_to_read);
	if (rc == 1)
	{
		values_to_read = vector<uint16_t>(tmp_values_to_read,tmp_values_to_read+num_registers_to_read);
	}
	
	free(tmp_values_to_read);
	return rc;
}


