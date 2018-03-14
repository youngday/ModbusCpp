#include <includes/modbus.h>
#include <exception>

using namespace std;


ModBusConnector::ModBusConnector(string & ip, int & port)
{
	this->ctx = modbus_new_tcp(ip.c_str(), port);
	
	if (ctx == NULL) {
    	cerr << "Unable to allocate libmodbus context" << endl;
	}

	if (modbus_connect(ctx) == -1) {
    	cerr << "Connection failed: " << modbus_strerror(errno) << endl;
    	modbus_free(ctx);
	}
}

ModBusConnector::~ModBusConnector()
{
	if (modbus_connect(ctx) != -1)
	{
		modbus_close(this->ctx);
	}
	
	modbus_free(this->ctx);
}

void ModBusConnector::disconnect()
{
	if (modbus_connect(ctx) != -1)
	{
		modbus_close(this->ctx);
	}
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

int ModBusConnector::read_registers(const int & addr, const int & num_of_bits, vector<uint16_t> & values)
{
	uint16_t* tmp_values = (uint16_t *) malloc(num_of_bits * sizeof(uint16_t));
	memset(tmp_values, 0, num_of_bits * sizeof(uint16_t));	
	int rc = modbus_read_registers(this->ctx, addr, num_of_bits, tmp_values);
	
	if (rc == 1)
	{
		values = vector<uint16_t>(tmp_values,tmp_values+num_of_bits);
	}
	
	free(tmp_values);
	
	return rc;
}

int ModBusConnector::read_input_read_registers(const int & addr, const int & num_of_bits, vector<uint16_t> & values)
{
	uint16_t* tmp_values = (uint16_t *) malloc(num_of_bits * sizeof(uint16_t));
	memset(tmp_values, 0, num_of_bits * sizeof(uint16_t));	
	int rc = modbus_read_input_read_registers(this->ctx, addr, num_of_bits, tmp_values);
	
	if (rc == 1)
	{
		values = vector<uint16_t>(tmp_values,tmp_values+num_of_bits);
	}
	
	free(tmp_values);
	
	return rc;
}

int ModBusConnector::write_bits(const int & addr, const vector<uint8_t> & values)
{
	int num_of_bits = values.size();
	
	uint8_t* tmp_values = values.data();	
	
	return modbus_write_bits(this->ctx, addr, num_of_bits, tmp_values);
	
}

int ModBusConnector::write_registers(const int & addr, const vector<uint16_t> & values)
{
	int num_of_bits = values.size();
	
	uint16_t* tmp_values = values.data();	
	
	return modbus_write_registers(this->ctx, addr, num_of_bits, tmp_values);
	
}

int ModBusConnector::write_and_read_registers(const int & write_addr, const vector<uint16_t> & values_to_write,
									 const int & read_addr, const int & num_registers_to_read,
									 vector<uint16_t> & values_to_read)
{

	int num_of_bits_to_write = values_to_write.size();
	
	uint16_t* tmp_values_to_write = values_to_write.data();
	
	uint16_t* tmp_values_to_read = (uint16_t *) malloc(num_registers_to_read * sizeof(uint16_t));
	memset(tmp_values_to_read, 0, num_registers_to_read * sizeof(uint16_t));
	
	int rc = modbus_write_and_read_registers(this->ctx, write_addr, num_of_bits_to_write, tmp_values_to_write,
										     read_addr, num_registers_to_read, tmp_values_to_read);
	if (rc == 1)
	{
		values_to_read = vector<uint16_t>(tmp_values_to_read,tmp_values+tmp_values_to_read);
	}
	
	free(tmp_values_to_read);
	return rc;
}


