/* 
 * modbus.cpp
 *
 * Description:
 * MODBUS connection class.
 *
 * Parameters:
 *     (none)
 *
 * Return Values:
 *     (none)
 *
 */

#include "includes/modbus.h"
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>

/* Function codes */
/* 
   https://github.com/stephane/libmodbus/blob/v3.0.X/src/modbus-rtu-private.h
   Because libmodbus library doesn't provide API to retrieve received information, 
   this is a trick to get the received message from PLC client. It works with
   libmodbus V3.0.X. However, the function codes may change in the future release
   of the library since they are the library's internal definition.
*/
#define _FC_READ_COILS 0x01
#define _FC_READ_DISCRETE_INPUTS 0x02
#define _FC_READ_HOLDING_REGISTERS 0x03
#define _FC_READ_INPUT_REGISTERS 0x04
#define _FC_WRITE_SINGLE_COIL 0x05
#define _FC_WRITE_SINGLE_REGISTER 0x06
#define _FC_READ_EXCEPTION_STATUS 0x07
#define _FC_WRITE_MULTIPLE_COILS 0x0F
#define _FC_WRITE_MULTIPLE_REGISTERS 0x10
#define _FC_REPORT_SLAVE_ID 0x11
#define _FC_WRITE_AND_READ_REGISTERS 0x17

ModBusConnector::ModBusConnector(const std::string &ip, const int &port)
{
	/* create modbux context */
	this->ctx = modbus_new_tcp(ip.c_str(), port);

	if (!this->ctx) /* Unable to allocate libmodbus context  */
	{
		throw std::runtime_error("Unable to allocate libmodbus context" + std::string(modbus_strerror(errno)));
	}
}

ModBusConnector::~ModBusConnector() noexcept
{
	if (this->is_connected) /* disconnect if connected; */
	{
		modbus_close(this->ctx); /* call libmodbus to close the modbus connection */
		this->is_connected = false;
	}

	if (this->ctx) // free modbus context
	{
		modbus_free(this->ctx); /* call libmodbus to destroy the modbus context */
	}
}

/** create a modbus connection
 * \throw: std::system_error if when errors occur, including errors from the
 *         underlying operating system that would prevent lock from meeting its
 *         specifications */
void ModBusConnector::connect()
{
	std::lock_guard<std::mutex> lk(modbus_lock); /* acquire lock, lock_guard will auto unlock when out of scope */
	if (this->is_connected)						 /* Do nothing if already connected */
		return;
	if (modbus_connect(this->ctx) == -1) /* connect to modbus server */
	{									 //libmodbus
		throw std::runtime_error("Connection failed: " + std::string(modbus_strerror(errno)));
	}
	this->is_connected = true;
}

/** disconnect a modbus connection
 *  Though without calling disconnect(), the connection will be disconnected
 *  once the class instance is destructed but it's good behaviour to disconnect
 *  before the program exit since the destructor is not always on program exit
 * \throw: std::system_error if when errors occur, including errors from the
 *         underlying operating system that would prevent lock from meeting its
 *         specifications
*/
void ModBusConnector::disconnect()
{
	std::lock_guard<std::mutex> lk(modbus_lock); /* acquire lock, lock_guard will auto unlock when out of scope */
	if (this->is_connected)
	{
		modbus_close(this->ctx); /* close the connection to modbus server */
		this->is_connected = false;
	}
}

/** Test if modbus connection is ready
 * \throw: std::system_error if when errors occur, including errors from the
 *         underlying operating system that would prevent lock from meeting its
 *         specifications
*/
bool ModBusConnector::is_connect()
{
	std::lock_guard<std::mutex> lk(modbus_lock);
	return this->is_connected;
}

/** enable modbus verbose message mode
 * \throw: std::system_error if when errors occur, including errors from the
 *         underlying operating system that would prevent lock from meeting its
 *         specifications */
void ModBusConnector::set_debug(bool flag)
{
	std::lock_guard<std::mutex> lk(modbus_lock); /* acquire lock, lock_guard will auto unlock when out of scope */
	modbus_set_debug(this->ctx, flag);			 /* enable modbus verbose message mode */
}

/** receive a serial of coil-type modbus objects from modbus server
 * \addr: the start address of a serial of coil-type modbus objects
 * \num_of_bits: the number of coil-type modbus objects
 * \values: vector of the fetched value of each coil-type object
 * \return: -1 on failure, or the number of coil-type objects received on success
 * \throw: std::system_error if when errors occur, including errors from the
 *         underlying operating system that would prevent lock from meeting its
 *         specifications
*/
int ModBusConnector::read_bits(const int &addr, const int &num_of_bits, std::vector<std::uint8_t> &values)
{
	std::unique_lock<std::mutex> ulk(modbus_lock); /* acquire lock, unique_lock will auto unlock when out of scope */
	if (!this->is_connected)					   /* return failure if connection not yet established */
		return -1;
	ulk.unlock(); /* release lock when it's not required */

	/* c libmodbus only takes c-style array, temp array to store the data */
	std::uint8_t *tmp_values = (std::uint8_t *)malloc(num_of_bits * sizeof(std::uint8_t));
	if (!tmp_values) /* Unable to allocate memory */
	{
		std::cerr << "modbus.cpp read_bits: Unable to allocate memory" << std::endl;
		return -1;
	}
	/* init each bit of the tmp array to 0 */
	memset(tmp_values, 0, num_of_bits * sizeof(std::uint8_t));

	int rc = -1; /* return value */
	try
	{
		ulk.lock();														 /* acquire lock */
		rc = modbus_read_bits(this->ctx, addr, num_of_bits, tmp_values); /* call libmodbus to do the reading */
		ulk.unlock();													 /* release the lock */
	}
	catch (const std::exception &ex) /* catch the exception from acquiring the lock to give the chance to clean memory */
	{
		free(tmp_values); /* in case of exception from lock(), release memory to avoid leaking */
		throw ex;		  /* re-throw the exception */
	}

	if (rc == num_of_bits) /* successfully */
	{
		values.clear();
		for (int i = 0; i < num_of_bits; ++i)
		{
			values.push_back(tmp_values[i]); // copy value to the result vector
		}
	}

	free(tmp_values); /* free tmp array */

	return rc;
}

/** receive a number of discrete inputs
 * \addr: the start address of a serial of discrete-input-type modbus objects
 * \num_of_bits: the number of discrete-input-type modbus objects
 * \values: vector of the fetched value of each discrete-input-type object
 * \return: -1 on failure, or the number of discrete-input-type objects received on success
 * \throw: std::system_error if when errors occur, including errors from the
 *         underlying operating system that would prevent lock from meeting its
 *         specifications
*/
int ModBusConnector::read_input_bits(const int &addr, const int &num_of_bits,
									 std::vector<std::uint8_t> &values)
{
	std::unique_lock<std::mutex> ulk(modbus_lock); /* acquire lock, unique_lock will auto unlock when out of scope */
	if (!this->is_connected)					   /* return failure if connection not yet established */
		return -1;
	ulk.unlock(); /* release lock when it's not required */

	/* c libmodbus only takes c-style array, temp array to store the data */
	std::uint8_t *tmp_values = (std::uint8_t *)malloc(num_of_bits * sizeof(std::uint8_t));
	if (!tmp_values) /* Unable to allocate memory */
	{
		std::cerr << "modbus.cpp: Unable to allocate memory" << std::endl;
		return -1;
	}
	/* init each bit of the tmp array to 0 */
	memset(tmp_values, 0, num_of_bits * sizeof(std::uint8_t));

	int rc = -1; /* return value */
	try
	{
		ulk.lock();															   /* acquire lock */
		rc = modbus_read_input_bits(this->ctx, addr, num_of_bits, tmp_values); /* call libmodbus to do the reading */
		ulk.unlock();														   /* release the lock */
	}
	catch (const std::exception &ex) /* catch the exception from acquiring the lock to give the chance to clean memory */
	{
		free(tmp_values); /* in case of exception from lock(), release memory to avoid leaking */
		throw ex;		  /* re-throw the exception */
	}

	if (rc == num_of_bits) /* successfully */
	{
		values.clear();
		for (int i = 0; i < num_of_bits; ++i)
		{
			values.push_back(tmp_values[i]); // copy value to the result vector
		}
	}

	free(tmp_values); /* free tmp array */

	return rc;
}

/** receive a serial of holding registers
 * \addr: the start address of a serial of holding registers
 * \num_of_bits: the number of holding registers
 * \values: vector of the fetched value of each holding register
 * \return: -1 on failure, or the number of holding registers received on success
 * \throw: std::system_error if when errors occur, including errors from the
 *         underlying operating system that would prevent lock from meeting its
 *         specifications
*/
int ModBusConnector::read_registers(const int &addr, const int &num_of_registers, std::vector<std::uint16_t> &values)
{
	std::unique_lock<std::mutex> ulk(modbus_lock); /* acquire lock, unique_lock will auto unlock when out of scope */
	if (!this->is_connected)					   /* return failure if connection not yet established */
		return -1;
	ulk.unlock(); /* release lock when it's not required */

	/* c libmodbus only takes c-style array, temp array to store the data */
	std::uint16_t *tmp_values = (std::uint16_t *)malloc(num_of_registers * sizeof(std::uint16_t));
	if (!tmp_values)
	{
		std::cerr << "modbus.cpp: Unable to allocate memory" << std::endl;
		return -1;
	}
	/* init each bit of the tmp array to 0 */
	memset(tmp_values, 0, num_of_registers * sizeof(std::uint16_t));

	int rc = -1; /* return value */
	try
	{
		ulk.lock();																   /* acquire lock */
		rc = modbus_read_registers(this->ctx, addr, num_of_registers, tmp_values); /* call libmodbus to do the reading */
		ulk.unlock();															   /* release the lock */
	}
	catch (const std::exception &ex) /* catch the exception from acquiring the lock to give the chance to clean memory */
	{
		free(tmp_values); /* in case of exception from lock(), release memory to avoid leaking */
		throw ex;		  /* re-throw the exception */
	}

	if (rc == num_of_registers) /* successfully */
	{
		values.clear();
		for (int i = 0; i < num_of_registers; ++i)
		{
			values.push_back(tmp_values[i]); // copy value to the result vector
		}
	}

	free(tmp_values); /* free tmp array */

	return rc;
}

/** receive a serial of input registers
 * \addr: the start address of a serial of input registers
 * \num_of_bits: the number of input registers
 * \values: vector of the fetched value of each input register
 * \return: -1 on failure, or the number of input registers received on success
 * \throw: std::system_error if when errors occur, including errors from the
 *         underlying operating system that would prevent lock from meeting its
 *         specifications
*/
int ModBusConnector::read_input_registers(const int &addr, const int &num_of_registers, std::vector<std::uint16_t> &values)
{
	std::unique_lock<std::mutex> ulk(modbus_lock); /* acquire lock, unique_lock will auto unlock when out of scope */
	if (!this->is_connected)					   /* return failure if connection not yet established */
		return -1;
	ulk.unlock(); /* release lock when it's not required */

	/* c libmodbus only takes c-style array, temp array to store the data */
	std::uint16_t *tmp_values = (std::uint16_t *)malloc(num_of_registers * sizeof(std::uint16_t));
	if (!tmp_values)
	{
		std::cerr << "modbus.cpp: Unable to allocate memory" << std::endl;
		return -1;
	}
	/* init each bit of the tmp array to 0 */
	memset(tmp_values, 0, num_of_registers * sizeof(std::uint16_t));

	int rc = -1; /* return value */
	try
	{
		ulk.lock();																		 /* return value */
		rc = modbus_read_input_registers(this->ctx, addr, num_of_registers, tmp_values); /* call libmodbus to do the reading */
		ulk.unlock();																	 /* release the lock */
	}
	catch (const std::exception &ex) /* catch the exception from acquiring the lock to give the chance to clean memory */
	{
		free(tmp_values); /* in case of exception from lock(), release memory to avoid leaking */
		throw ex;		  /* re-throw the exception */
	}

	if (rc == num_of_registers) /* successfully */
	{
		values.clear();
		for (int i = 0; i < num_of_registers; ++i)
		{
			values.push_back(tmp_values[i]); // copy value to the result vector
		}
	}

	free(tmp_values); /* free tmp array */

	return rc;
}

/** send value into a single coil
 * \addr: the address of coil modbus object
 * \value: the value to send, either 1 or 0
 * \return: -1 on failure, or 1 on success
 * \throw: std::system_error if when errors occur, including errors from the
 *         underlying operating system that would prevent lock from meeting its
 *         specifications
*/
int ModBusConnector::write_bit(const int &addr, const std::uint8_t &value)
{
	std::lock_guard<std::mutex> lk(modbus_lock); /* acquire lock, lock_guard will auto unlock when out of scope */
	if (!this->is_connected)					 /* return failure if connection not yet established */
		return -1;

	int rc = modbus_write_bit(this->ctx, addr, value); /* call libmodbus to do the writing */
	return rc;
}

/** send values into a serial of coils
 * \addr: the start address of the serial of coil modbus objects
 * \num_of_bits: the number of coil-type modbus objects to be sent
 * \values: the vector of values to send, each value should be either 1 or 0
 * \return: -1 on failure, or the number of coil-type objects sent on success
 * \throw: std::system_error if when errors occur, including errors from the
 *         underlying operating system that would prevent lock from meeting its
 *         specifications
*/
int ModBusConnector::write_bits(const int &addr, const int &num_of_bits,
								const std::vector<std::uint8_t> &values)
{
	std::unique_lock<std::mutex> ulk(modbus_lock); /* acquire lock, unique_lock will auto unlock when out of scope */
	if (!this->is_connected)					   /* return failure if connection not yet established */
		return -1;
	ulk.unlock(); /* release lock when it's not required */

	int num_to_write = values.size(); /* get the size of values vector */

	/* if num_of_bits is larger than the actual vector size, overwrite it with vector size 
	   so that the Min<num_of_bits, size> values will be sent */
	if (num_to_write > num_of_bits)
	{
		num_to_write = num_of_bits;
	}

	ulk.lock();																/* acquire lock */
	return modbus_write_bits(this->ctx, addr, num_to_write, values.data()); /* call libmodbus to do the writing */
}

/** send value into a single holding register
 * \addr: the address of holding register
 * \value: the value to send
 * \return: -1 on failure, or 1 on success
 * \throw: std::system_error if when errors occur, including errors from the
 *         underlying operating system that would prevent lock from meeting its
 *         specifications 
*/
int ModBusConnector::write_register(const int &addr, const std::uint16_t &value)
{
	std::lock_guard<std::mutex> lk(modbus_lock); /* acquire lock, lock_guard will auto unlock when out of scope */
	if (!this->is_connected)					 /* return failure if connection not yet established */
		return -1;

	int rc = modbus_write_register(this->ctx, addr, value); /* call libmodbus to do the writing */
	return rc;
}

/** send values into a number of holding registers
 * \addr: the start address of the serial of holding registers
 * \num_of_bits: the number of holding registers to be sent
 * \values: the vector of values to send
 * \return: -1 on failure, or the number of holding registers sent on success
 * \throw: std::system_error if when errors occur, including errors from the
 *         underlying operating system that would prevent lock from meeting its
 *         specifications
*/
int ModBusConnector::write_registers(const int &addr, const int &num_of_registers,
									 const std::vector<std::uint16_t> &values)
{
	std::unique_lock<std::mutex> ulk(modbus_lock); /* acquire lock, unique_lock will auto unlock when out of scope */
	if (!this->is_connected)					   /* return failure if connection not yet established */
		return -1;
	ulk.unlock(); /* release lock when it's not required */

	int num_to_write = values.size(); /* get the size of values vector */

	/* if num_of_registers is larger than the actual vector size, overwrite it with vector size 
	   so that the Min<num_of_registers, size> values will be sent */
	if (num_to_write > num_of_registers)
	{
		num_to_write = num_of_registers;
	}

	ulk.lock();																	 /* acquire lock */
	return modbus_write_registers(this->ctx, addr, num_to_write, values.data()); /* call libmodbus to do the writing */
}

/** send values into a serial of holding registers and then read values back from those registers
 * \write_addr: the start address of the serial of holding registers
 * \num_of_registers_to_write: the number of holding registers to send values into
 * \values_to_write: the vector of values to send
 * \num_registers_to_read: the number of holding registers to receive values from
 * \values_to_read: the vector to hold the received values
 * \return: -1 on failure, or the number of holding registers received on success
 * \throw: std::system_error if when errors occur, including errors from the
 *         underlying operating system that would prevent lock from meeting its
 *         specifications
*/
int ModBusConnector::write_and_read_registers(const int &write_addr, const int &num_of_registers_to_write,
											  const std::vector<std::uint16_t> &values_to_write,
											  const int &read_addr, const int &num_registers_to_read,
											  std::vector<std::uint16_t> &values_to_read)
{
	std::unique_lock<std::mutex> ulk(modbus_lock); /* acquire lock, unique_lock will auto unlock when out of scope */
	if (!this->is_connected)					   /* return failure if connection not yet established */
		return -1;
	ulk.unlock(); /* release lock when it's not required */

	int num_to_write = values_to_write.size(); /* get the size of values vector */

	/* if num_of_registers is larger than the actual vector size, overwrite it with vector size 
	   so that the Min<num_of_registers, size> values will be sent */
	if (num_to_write > num_of_registers_to_write)
	{
		num_to_write = num_of_registers_to_write;
	}
	/* c libmodbus only takes c-style array, temp array to store the received data */
	std::uint16_t *tmp_values_to_read = (std::uint16_t *)malloc(num_registers_to_read * sizeof(std::uint16_t));
	if (!tmp_values_to_read)
	{
		std::cerr << "modbus.cpp: Unable to allocate memory" << std::endl;
		return -1;
	}
	memset(tmp_values_to_read, 0, num_registers_to_read * sizeof(std::uint16_t));

	int rc = -1; /* return value */
	try
	{
		ulk.lock(); /* acquire lock */
		/* call libmodbus to do the jobs */
		rc = modbus_write_and_read_registers(this->ctx, write_addr, num_to_write, values_to_write.data(),
											 read_addr, num_registers_to_read, tmp_values_to_read);
		ulk.unlock(); /* release the lock */
	}
	catch (std::exception &ex)
	{
		free(tmp_values_to_read); /* in case of exception from lock(), release memory to avoid leaking */
		throw ex;				  /* re-throw the exception */
	}

	if (rc == num_registers_to_read) /* read successfully */
	{
		values_to_read.clear();
		for (int i = 0; i < num_registers_to_read; ++i)
		{
			values_to_read.push_back(tmp_values_to_read[i]); // copy value to the result vector
		}
	}

	free(tmp_values_to_read); /* free tmp array */
	return rc;
}

/** convert a float type value to two holding registers
 * \f the float value to be converted
 * \register0: the first of the registers the float converted to
 * \register1: the second of the registers the float converted to
*/
void ModBusConnector::set_float(const float &f, std::uint16_t &register0, std::uint16_t &register1) noexcept
{
	std::uint16_t tmp_values[2] = {0};
	modbus_set_float(f, tmp_values); //libmodbus
	register0 = tmp_values[0];
	register1 = tmp_values[1];
}

/** get a float type value from two holding registers
 * \register0: the first of the registers to be converted
 * \register1: the second of the registers to be converted
 * \return: the converted float
*/
float ModBusConnector::get_float(const std::uint16_t &register0, const std::uint16_t &register1) noexcept
{
	std::uint16_t tmp_values[2] = {0};
	tmp_values[0] = register0;
	tmp_values[1] = register1;
	return modbus_get_float(tmp_values); //libmodbus
}

/** convert a float type value to two holding registers(reversed order)
 * \f the float value to be converted
 * \register0: the first of the registers the float converted to
 * \register1: the second of the registers the float converted to
*/
void ModBusConnector::set_float_swap(const float &f, std::uint16_t &register0, std::uint16_t &register1) noexcept
{
	std::uint16_t tmp_values[2] = {0};
	modbus_set_float(f, tmp_values); //libmodbus
	register0 = tmp_values[1];
	register1 = tmp_values[0];
}

/** get a float type value from two holding registers(reversed order)
 * \register0: the first of the registers to be converted
 * \register1: the second of the registers to be converted
 * \return: the converted float
*/
float ModBusConnector::get_float_swap(const std::uint16_t &register0, const std::uint16_t &register1) noexcept
{
	std::uint16_t tmp_values[2] = {0};
	tmp_values[0] = register1;
	tmp_values[1] = register0;
	return modbus_get_float(tmp_values); //libmodbus
}

inline bool epoll_add(const int &epollfd, const int &socket);

/** constructor for Modbus server
 * \ip: the ip which the server to bind with
 * \port: the port which the server to bind with
 * \nb_coil_status: the number of coil-type modbus objects in the server
 * \nb_input_status: the number of discrete-input-type modbus objects in the server
 * \nb_holding_registers: the number of holding-register-type modbus objects in the server
 * \nb_input_registers: the number of input-register-type modbus objects in the server
 * \throw: runtime_error when unable to allocate enough memory resources
*/
ModBusServer::ModBusServer(const std::string &ip, const int &port, const int &nb_coil_status, const int &nb_input_status,
						   const int &nb_holding_registers, const int &nb_input_registers)
{
	this->ctx = modbus_new_tcp(ip.c_str(), port); /* create modbux context */
	if (!this->ctx)
	{
		throw std::runtime_error("[ModBusServer::ModBusServer]Unable to allocate libmodbus context: " + std::string(modbus_strerror(errno)));
	}
	/* Allocate server data structure to hold the modbus objects */
	this->mb_mapping = modbus_mapping_new(nb_coil_status, nb_input_status, nb_holding_registers, nb_input_registers);
	if (__glibc_unlikely(!this->mb_mapping))
	{
		modbus_free(this->ctx); /* cleanup on failure */
		throw std::runtime_error("[ModBusServer::ModBusServer]Failed to allocate the mapping: " + std::string(modbus_strerror(errno)));
	}
	/* creates a new epoll instance, note: the parameter of epoll_create1() is for backward has no meaning is ignored */
	this->epollfd = epoll_create1(0);
	if (__glibc_unlikely(this->epollfd == -1))
	{
		modbus_mapping_free(this->mb_mapping); /* cleanup on failure */
		modbus_free(this->ctx);				   /* cleanup on failure */
		throw std::runtime_error("[ModBusServer::ModBusServer]Failed to create epoll: " + std::string(strerror(errno)));
	}

	this->events = (struct epoll_event *)calloc(MAX_EPOLL_EVENTS, sizeof(struct epoll_event));
	if (__glibc_unlikely(!this->events))
	{
		close(this->epollfd);				   /* cleanup on failure */
		modbus_mapping_free(this->mb_mapping); /* cleanup on failure */
		modbus_free(this->ctx);				   /* cleanup on failure */
		throw std::runtime_error("[ModBusServer::ModBusServer]Failed to allocate epoll event arrays");
	}

	this->query = (uint8_t *)calloc(MODBUS_TCP_MAX_ADU_LENGTH, sizeof(uint8_t));
	if (__glibc_unlikely(!this->query))
	{
		free(this->events);					   /* cleanup on failure */
		close(this->epollfd);				   /* cleanup on failure */
		modbus_mapping_free(this->mb_mapping); /* cleanup on failure */
		modbus_free(this->ctx);				   /* cleanup on failure */
		throw std::runtime_error("[ModBusServer::ModBusServer]Failed to allocate query");
	}

	this->active_socket_set = new std::unordered_set<int>();
	if (__glibc_unlikely(!this->active_socket_set))
	{
		free(this->query);					   /* cleanup on failure */
		free(this->events);					   /* cleanup on failure */
		close(this->epollfd);				   /* cleanup on failure */
		modbus_mapping_free(this->mb_mapping); /* cleanup on failure */
		modbus_free(this->ctx);				   /* cleanup on failure */
		throw std::runtime_error("[ModBusServer::ModBusServer]Failed to allocate active_socket_set");
	}

	this->event_valid = (bool *)calloc(MAX_EPOLL_EVENTS, sizeof(bool));
	if (__glibc_unlikely(!this->event_valid))
	{
		delete this->active_socket_set;		   /* cleanup on failure */
		free(this->query);					   /* cleanup on failure */
		free(this->events);					   /* cleanup on failure */
		close(this->epollfd);				   /* cleanup on failure */
		modbus_mapping_free(this->mb_mapping); /* cleanup on failure */
		modbus_free(this->ctx);				   /* cleanup on failure */
		throw std::runtime_error("[ModBusServer::ModBusServer]Failed to allocate event_valid arrays");
	}
	memset(this->event_valid, 0, MAX_EPOLL_EVENTS * sizeof(bool)); /* init each bit of event_valid is 0 */
}

// default destructor for Modbus server
ModBusServer::~ModBusServer()
{
	if (event_valid)
		free(event_valid);
	if (active_socket_set)
	{
		/* close all remaining active sockets */
		for (auto &sock : *active_socket_set)
		{
			/* removing active socket from epoll interest list */
			if (__glibc_unlikely(epoll_ctl(this->epollfd, EPOLL_CTL_DEL, sock, NULL) == -1))
			{
				/* sanity check */
				std::cerr << "[ModBusServer::process] removing socket " << sock
						  << " from epoll interest list fails, "
						  << strerror(errno) << std::endl;
			}
			if (__glibc_unlikely(close(sock) == -1)) /* close all active sockets */
			{
				/* sanity check, never happen if code is correct */
				std::cerr << "[ModBusServer::process] closing socket fails when adding to epoll interest list fails, "
						  << strerror(errno) << std::endl;
			}
		}
		delete active_socket_set;
	}
	if (query)
		free(query);
	if (events)
		free(events);
	if (server_socket != -1)
		close(server_socket); /* close main server socket */
	if (epollfd != -1)
		close(epollfd);
	if (mb_mapping)
		modbus_mapping_free(mb_mapping);
	if (ctx)
		modbus_free(ctx);
}

/** start listening to incoming connection
 * \max_number_pending_connection: the maximum number of pending connection in queue waiting for server to accept.
 *                                 Details: this argument defines the maximum length to which the queue of
 *                                 pending connections for sockfd may grow.  If a connection request
 *                                 arrives when the queue is full, the client may receive an error with
 *                                 an indication of ECONNREFUSED or, if the underlying protocol supports
 *                                 retransmission, the request may be ignored so that a later reattempt
 *                                 at connection succeeds.
 * \throw: runtime_error when
 *         1. it's already listening to incoming connection
 *         2. Unable to listen to incoming connection
*/
void ModBusServer::listen(const int &max_number_pending_connection)
{
	if (this->server_socket != -1) /* already started listening to incoming connection */
	{
		throw std::runtime_error("[ModBusServer::listen]Already start listening to incomming connections!");
	}

	this->server_socket = modbus_tcp_listen(ctx, max_number_pending_connection); /* start to listen to incoming modbus connection */
	if (this->server_socket == -1)
	{
		throw std::runtime_error("[ModBusServer::listen]Unable to listen TCP connection: " + std::string(modbus_strerror(errno)));
	}

	if (__glibc_unlikely(!epoll_add(this->epollfd, this->server_socket)))
	{
		/* sanity check */
		close(this->server_socket);				  /* cleanup on failure */
		close(this->epollfd);					  /* cleanup on failure */
		this->epollfd = this->server_socket = -1; /* reset to default on failure */
		throw std::runtime_error("[ModBusServer::listen]Unable to listen TCP connection (epoll_ctl): " + std::string(strerror(errno)));
	}
}

/** wait for client to connect, blocking until a connection is ready for ModBusServer::receive()
 * // TODO: add a timeout for it
 * \return the number of connections ready for ModBusServer::receive()
 * \throw runtime_error if unable to wait for incoming connection
*/
int ModBusServer::wait()
{
	/* wait for an I/O event on an epoll file descriptor
	   The epoll_wait() system call waits for events on the epoll instance
	   referred to by the file descriptor epollfd. The memory area pointed
	   to by events will contain the events that will be available for the
	   caller. Up to MAX_EPOLL_EVENTS are returned by epoll_wait(). The
       MAX_EPOLL_EVENTS argument must be greater than zero. -1 causes
	   epoll_wait() to block indefinitely
	*/
	this->eventcount = epoll_wait(epollfd, events, MAX_EPOLL_EVENTS, -1);
	if (this->eventcount == -1)
	{
		/*
		   The call was interrupted by a signal handler before either 
		   (1) any of the requested events occurred or
		   (2) the timeout expired 
		   Not a fatal error
		*/
		if (errno == EINTR)
			return this->eventcount = 0;
		else
			throw std::runtime_error("[ModBusServer::wait]Unable to wait for incoming connection: " + std::string(strerror(errno)));
	}
	for (std::size_t i = 0; i < MAX_EPOLL_EVENTS; ++i)
		event_valid[i] = true;

	return this->eventcount;
}

/** receive, process and reply the request from the [index]th available connections
 * \index: the index of the selected one of sockets ready for ModBusServer::receive()
 * \return: true if a new connection is established, false if request is processed on an existing connection
 * \throw: runtime_error when:
 *         1. \index exceeds the number of available connections returned by ModBusServer::wait()
 *         3. the selected connection with the \index is already closed
 *         4. Unable to accept a new connection
*/
bool ModBusServer::process(const int &index)
{
	if (index >= this->eventcount) /* the conection is not ready */
	{
		throw std::runtime_error("[ModBusServer::process] index:" + std::to_string(index) + ", this connection is not ready");
	}

	if (!this->event_valid[index])
	{
		throw std::runtime_error("[ModBusServer::process] index:" + std::to_string(index) + ", this connection has already been processed by ModBusServer::process");
	}

	if (events[index].data.fd == -1) /* the event is not associated with a valid connection */
	{
		throw std::runtime_error("[ModBusServer::process] The connection(index:" + std::to_string(index) + ") was already closed");
	}

	if (events[index].data.fd == this->server_socket) /* A client is asking for a new connection */
	{
		socklen_t addrlen; /* length of sockaddr_in struct */
		struct sockaddr_in clientaddr;

		/* Handle new connections */
		addrlen = sizeof(clientaddr);
		memset(&clientaddr, 0, sizeof(clientaddr));
		/* accept the new connection */
		int new_sock = accept(this->server_socket, (struct sockaddr *)&clientaddr, &addrlen);
		if (new_sock == -1)
		{
			throw std::runtime_error("[ModBusServer::process]Unable to accept new incoming connection: " + std::string(strerror(errno)));
		}
		/* add the new connection to epoll interest list */
		if (__glibc_unlikely(!epoll_add(this->epollfd, new_sock)))
		{
			auto tmp_error = errno;
			if (__glibc_unlikely(close(new_sock) == -1)) /* sanity check, never happen if code is correct */
			{
				std::cerr << "[ModBusServer::process] closing socket fails when adding to epoll interest list fails, "
						  << strerror(errno) << std::endl;
				errno = tmp_error;
			}
			throw std::runtime_error("[ModBusServer::process]Unable to accept new incoming connection (epoll_ctl): " + std::string(strerror(errno)));
		}
		auto insert_result = active_socket_set->insert(new_sock);
		if (__glibc_unlikely(!insert_result.second)) /* sanity check, never happen unless something is wrong */
		{
			/* insert fails due to duplicate key */
			if (__glibc_unlikely(close(new_sock) == -1)) /* sanity check, never happen if code is correct */
			{
				std::cerr << "[ModBusServer::process] closing socket fails when duplicate socket number found, "
						  << strerror(errno) << std::endl;
			}
			throw std::runtime_error("[ModBusServer::process]uplicate socket number found: " + std::to_string(new_sock));
		}
		this->event_valid[index] = false;
		return true;
	}
	else
	{
		modbus_set_socket(ctx, events[index].data.fd);

		/* call libmodbus to receive modbus query */
		int rc = modbus_receive(ctx, query);
		if (rc > 0)
		{
			/* call libmodbus to reply the query */
			modbus_reply(ctx, query, rc, mb_mapping);
		}
		else if (rc == -1) /* connection failure or reset by peer */
		{
			/* Remove from epoll interest list */
			epoll_ctl(this->epollfd, EPOLL_CTL_DEL, events[index].data.fd, &events[index]);
			/* close socket */
			close(events[index].data.fd);
			/* remove from active sockets set */
			active_socket_set->erase(events[index].data.fd);
			/* prevent user from trying to access a closed connection */
			events[index].data.fd = -1;
		}

		if (__glibc_unlikely(!rc)) /* sanity check, not possible as stated by the doc file of libmodbus*/
		{
			std::cerr << " ModBusServer::process: rc == 0!" << std::endl;
		}
		this->event_valid[index] = false;
		return false;
	}
}

/** add socket to the interest list of epoll instance
 * \epollfd: the epollfd file descriptor
 * \socket: the socket to be added
 * \return: true on success, false on failure
*/
inline bool epoll_add(const int &epollfd, const int &socket)
{
	/** Note: epoll_event describes the object linked to the epoll file descriptor
	 * defined as (epoll.h)
	 *   typedef union epoll_data {
     *       void        *ptr;
     *       int          fd;
     *       uint32_t     u32;
     *       uint64_t     u64;
     *   } epoll_data_t;
     *   struct epoll_event {
     *       uint32_t     events;      //Epoll events 
     *       epoll_data_t data;        //User data variable
     *   }; 
	*/
	struct epoll_event ev = {0, 0}; /* init the epoll event structure for epoll() */
	ev.events = EPOLLIN;			/* The associated socket is available for read(2) operations. */
	ev.data.fd = socket;			/* associate the socket to the epoll event */
	/* 
	   This system call is used to add, modify, or remove entries in the
       interest list of the epoll instance referred to by the file descriptor
	   epollfd.
	   EPOLL_CTL_ADD
           Add server_socket to the interest list and associate the settings
           specified in ev with the internal file linked to server_socket.
	*/
	if (__glibc_unlikely(epoll_ctl(epollfd, EPOLL_CTL_ADD, socket, &ev) == -1))
	{
		return false;
	}
	return true;
}
