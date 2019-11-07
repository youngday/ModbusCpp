#include "includes/parser.h"

/* Put varable into map
 * name: varable name
 * type: type of the modbus object this varable take
 * var_addr: the address of the varable, two element array:
 *           var_addr[0]: start address;
 *           var_addr[1]: number of modbus object this varable takes;
 * data_map: the structure to store varable info
 * if name already exists in data_map, the existing one will be replaced and return true,
 * otherwise add the new varable and return false
 */
inline bool _store_params(const std::string &name, const std::string &type, const int *__restrict var_addr,
						  std::unordered_map<std::string, std::pair<std::string, std::vector<int>>> &data_map)
{
	// duplicate name found
	bool ret = data_map.find(name) != data_map.end();
	data_map.emplace(std::piecewise_construct,
					 std::forward_as_tuple(name),
					 std::forward_as_tuple(std::piecewise_construct,
										   std::forward_as_tuple(type),
										   std::forward_as_tuple(var_addr, var_addr + 2)));
	return ret;
}

/**Parse modbus connection parameters
 * config_file: configuration file name
 * ip: modbus server ip address
 * port: modbus server port number
 * send_rate: data sending rate to modbus server
 * Exception: Configuration file parsing failed/Configuration file cannot be opened.
 */
void ModbusConfigParser::parse(const std::string &config_file, std::string &ip, int &port,
							   std::unordered_map<std::string, std::pair<std::string, std::vector<int>>> &data_map)
{
	std::ifstream ifs(config_file); //open config file

	if (ifs.is_open())
	{
		std::string line;
		while (std::getline(ifs, line)) //read line
		{
			if (line.empty()) // skip empty line
			{
				continue;
			}

			std::stringstream ss(line); //read line into stream
			std::string item, name;

			if (!(ss >> item) || !item.size() || item[0] == '#') //skip line begining with #
			{
				continue;
			}

			if (item == "connection_params") //connection parameters, server ip and port
			{
				ss >> ip >> port;
				if (!ss)
				{
					std::cerr << "CnfPrsr: The connection parameters are not correct! "
													   << "CnfPrsr: FORMAT: connection_params IP_ADDRESS PORT SENDING_RATE"
													   << std::endl;
					ip = "";
					port = 0;
					throw std::runtime_error("CnfPrsr: Configuration file parsing failed. " + config_file + ":" + line);
				}
			}
			else if (item == "coil") //coil type
			{
				int coils_addr[2];
				//coils_addr[0]: start address
				//coils_addr[1]: number of values
				ss >> name >> coils_addr[0] >> coils_addr[1];

				if (!ss || coils_addr[0] < 1 || coils_addr[1] < 1)
				{
					std::cerr << "CnfPrsr: The data parameters are not correct! "
													   << "CnfPrsr: FORMAT: coil NAME START_ADDR NUMBER_OF_VALUES" << std::endl;
					throw std::runtime_error("CnfPrsr: Configuration file parsing failed. " + config_file + ":" + line);
				}

				coils_addr[0]--; //convert PLC address space mapping to true address space
								 //e.g In PLC, address 1-999 -----> true address 0-998

				//put varable into map, return true if duplicate found
				if (_store_params(name, "coil", coils_addr, data_map))
					std::cerr << "confparser: Warning! Duplicate variable found, "
													   << "stored value will be replaced by this. "
													   << config_file << ":" << line << std::endl;
			}
			else if (item == "inputbit") //discrete input
			{
				int inputbits_addr[2];
				//inputbits_addr[0]: start address
				//inputbits_addr[1]: number of values
				ss >> name >> inputbits_addr[0] >> inputbits_addr[1];

				if (!ss || inputbits_addr[0] < 1 || inputbits_addr[1] < 1)
				{
					std::cerr << "CnfPrsr: The data parameters are not correct! "
													   << "FORMAT: inputbit NAME START_ADDR NUMBER_OF_VALUES" << std::endl;
					throw std::runtime_error("CnfPrsr: Configuration file parsing failed. " + config_file + ":" + line);
				}

				inputbits_addr[0]--; //convert PLC address space mapping to true address space
									 //e.g In PLC, address 1-999 -----> true address 0-998

				//put varable into map, return true if duplicate found
				if (_store_params(name, "input_bit", inputbits_addr, data_map))
					std::cerr << "confparser: Warning! Duplicate variable found, "
													   << "stored value will be replaced by this. "
													   << config_file << ":" << line << std::endl;
			}
			else if (item == "register") //holding register
			{
				int registers_addr[2];
				//registers_addr[0]: start address
				//registers_addr[1]: number of values
				ss >> name >> registers_addr[0] >> registers_addr[1];

				if (!ss || registers_addr[0] < 1 || registers_addr[1] < 1)
				{
					std::cerr << "CnfPrsr: The data parameters are not correct! "
													   << "FORMAT: register NAME START_ADDR NUMBER_OF_VALUES" << std::endl;
					throw std::runtime_error("CnfPrsr: Configuration file parsing failed. " + config_file + ":" + line);
				}

				registers_addr[0]--; //convert PLC address space mapping to true address space
									 //e.g In PLC, address 1-999 -----> true address 0-998

				//put varable into map, return true if duplicate found
				if (_store_params(name, "holding_register", registers_addr, data_map))
					std::cerr << "confparser: Warning! Duplicate variable found, "
													   << "stored value will be replaced by this. "
													   << config_file << ":" << line << std::endl;
			}
			else if (item == "inputreg") //input registers
			{
				int inputregs_addr[2];
				//inputregs_addr[0]: start address
				//inputregs_addr[1]: number of values
				ss >> name >> inputregs_addr[0] >> inputregs_addr[1];

				if (!ss || inputregs_addr[0] < 1 || inputregs_addr[1] < 1)
				{
					std::cerr << "CnfPrsr: The data parameters are not correct! "
													   << "FORMAT: coil NAME START_ADDR NUMBER_OF_VALUES" << std::endl;
					throw std::runtime_error("CnfPrsr: Configuration file parsing failed. " + config_file + ":" + line);
				}

				inputregs_addr[0]--; //convert PLC address space mapping to true address space
									 //e.g In PLC, address 1-999 -----> true address 0-998

				//put varable into map, return true if duplicate found
				if (_store_params(name, "input_register", inputregs_addr, data_map))
					std::cerr << "confparser: Warning! Duplicate variable found, "
													   << "stored value will be replaced by this. "
													   << config_file << ":" << line << std::endl;
			}
			else
			{
				std::cerr << "CnfPrsr: failed to parse config file in line: " << line << std::endl;
				throw std::runtime_error("CnfPrsr: Configuration file parsing failed. " + config_file + ":" + line);
			}
		} // while
		ifs.close();
	} // if
	else
	{
		std::cerr << "CnfPrsr: The config file " << config_file << " cannot be opened" << std::endl;
		throw std::runtime_error("CnfPrsr: Configuration file cannot be opened. " + config_file);
	}
} //ModbusConfigParser::parse
