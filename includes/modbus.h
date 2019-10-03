#ifndef __MODBUS_CPP_
#define __MODBUS_CPP_

#include <cerrno>
#include <iostream>
#include <vector>
#include <cstdint>
#include <exception>
#include <stdexcept>
#include <mutex>
#include <sys/epoll.h>
#include <modbus/modbus.h>
#include <unordered_set>

class ModBusConnector
{
private:
	modbus_t *ctx;			  // libmodbus context
	bool is_connected;		  // connection state;
	std::mutex modbus_lock{}; // mutex lock

public:
	/* No default constructor */
	ModBusConnector() = delete;
	/* default constructor for Modbus connector */
	ModBusConnector(const std::string &ip, const int &port);
	/* Non-Copyable */
	ModBusConnector(const ModBusConnector &) = delete;
	ModBusConnector &operator=(const ModBusConnector &) = delete;
	/* Non-Movable */
	ModBusConnector(ModBusConnector &&) = delete;
	ModBusConnector &operator=(ModBusConnector &&) = delete;

	/* default destructor for Modbus connector */
	~ModBusConnector() noexcept;

	/* read a number of coils */
	int read_bits(const int &addr, const int &num_of_bits, std::vector<std::uint8_t> &values);

	/* read a number of discrete inputs */
	int read_input_bits(const int &addr, const int &num_of_bits, std::vector<std::uint8_t> &values);

	/* read a number of holding registers */
	int read_registers(const int &addr, const int &num_of_registers, std::vector<std::uint16_t> &values);

	/* read a number of input registers */
	int read_input_registers(const int &addr, const int &num_of_registers, std::vector<std::uint16_t> &values);

	/* write value into a single coil */
	int write_bit(const int &addr, const std::uint8_t &value);

	/* write values into a number of coils */
	int write_bits(const int &addr, const int &num_of_bits, const std::vector<std::uint8_t> &values);

	/* write value into a single holding register */
	int write_register(const int &addr, const std::uint16_t &value);

	/* write values into a number of holding registers */
	int write_registers(const int &addr, const int &num_of_registers, const std::vector<std::uint16_t> &values);

	/* write values into a number of holding registers and then read values back from those registers */
	int write_and_read_registers(const int &write_addr, const int &num_of_registers_to_write,
								 const std::vector<std::uint16_t> &values_to_write,
								 const int &read_addr, const int &num_registers_to_read,
								 std::vector<std::uint16_t> &values_to_read);

	/* convert a float type value to two holding registers */
	static void set_float(const float &f, std::uint16_t &register0, std::uint16_t &register1) noexcept;

	/* get a float type value from two holding registers */
	static float get_float(const std::uint16_t &register0, const std::uint16_t &register1) noexcept;

	/* convert a float type value to two holding registers */
	static void set_float_swap(const float &f, std::uint16_t &register0, std::uint16_t &register1) noexcept;

	/* get a float type value from two holding registers */
	static float get_float_swap(const std::uint16_t &register0, const std::uint16_t &register1) noexcept;

	/* create a modbus connection */
	void connect();

	/* 
	   disconnect a modbus connection
	   Even without calling disconnect(), the connection will be disconnected
	   when the class instance is destructed 
	*/
	void disconnect();

	/* Test if modbus connection is ready */
	bool is_connect();

	/* enable modbus verbose message mode */
	void set_debug(bool flag);
};

/* 
   The max number of available connection returned by ModBusServer::wait()
   per calling the functin, though ModBusServer::wait() can be called multiple 
   times to handle the rest of the available connections in the I/O queue.
*/
#define MAX_EPOLL_EVENTS 10

class ModBusServer
{
private:
	/* modbus context */
	modbus_t *ctx = nullptr;
	/* modbus server data structure */
	modbus_mapping_t *mb_mapping = nullptr;
	/* epoll file descriptor */
	int epollfd = -1;
	/* epoll event data structure */
	struct epoll_event *events = nullptr;
	/* current received modbus query */
	uint8_t *query = nullptr;
	/* the set to hold the active sockets associated instance
	   used to track open sockets in order to close on destruction*/
	std::unordered_set<int> *active_socket_set = nullptr;
	/* modbus server socket */
	int server_socket = -1;
	/* store the number of the epoll events returned by ModBusServer::wait()
	   for error-proof purpose in ModBusServer::process() */
	int eventcount = -1;
	/* indicate the status of each epoll event in the epoll events array returned by ModBusServer::wait()
	   for error-proof purpose in ModBusServer::process()
	   true: the epoll event is ready for processing 
	   false: the epoll event has already been processed */
	bool *event_valid = nullptr;

public:
	/* default constructor for Modbus server */
	ModBusServer(const std::string &ip, const int &port, const int &nb_coil_status, const int &nb_input_status,
				 const int &nb_holding_registers, const int &nb_input_registers);

	/* Not copyable or movable*/
	ModBusServer(const ModBusServer &) = delete;
	ModBusServer &operator=(const ModBusServer &) = delete;
	ModBusServer(ModBusServer &&) = delete;
	ModBusServer &operator=(ModBusServer &&) = delete;

	/* default destructor for Modbus server */
	~ModBusServer();

	/* 
	   start listening to incoming connection 
	   max_number_pending_connection:
	   The allowed maximum number of pending 
	   incomming new connection not yet accepted
	*/
	void listen(const int &max_number_pending_connection);

	/* 
	   wait for client to connect
	   return: number of connections ready to receive data
	*/
	int wait();

	/* receive data from [index]th connection */
	bool process(const int &index);
};

#endif
