#include "includes/parser.h"

void ModbusConfigParser::parse(const std::string& config_file, std::string& ip, int& port, 
			std::unordered_map<std::string, std::pair<std::string, std::vector<int>>>& data_map)
{
	std::ifstream ifs(config_file); //open config file
	
	if (ifs.is_open())
	{
		std::string line;
		while (std::getline(ifs,line))  //read line
		{
			if (line.empty())
			{
				continue;
			}
			
			std::stringstream ss(line); //read line into stream
			std::string item,name;
			
			if (!(ss>>item) || !item.size() || item[0] == '#') //skip line begining with #
			{
				continue;
			}			
			
			if (item == "connection_params") //connection parameters, server ip and port
			{
				ss>>ip>>port;
				if (!ss)
				{
					std::cerr<<"The connection parameters are not correct! "
							 <<"FORMAT: connection_params IP_ADDRESS PORT"<<std::endl;
					ip="";
					port=0;
					throw std::runtime_error("Configuration file parsing failed. " + line);
				}
			}
			else if (item == "coil") //coil type
			{
				int coils_addr[2]; 
				//coils_addr[0]: start address
				//coils_addr[1]: number of values
				ss>>name>>coils_addr[0]>>coils_addr[1];
				
				if (!ss || coils_addr[0] < 1 || coils_addr[1] < 1)
				{
					std::cerr<<"The data parameters are not correct! "
							 <<"FORMAT: coil NAME START_ADDR NUMBER_OF_VALUES"<<std::endl;
					throw std::runtime_error("Configuration file parsing failed. " + line);
				}
				
				coils_addr[0]--; //convert PLC address space mapping to true address space
								 //e.g In PLC, address 1-999 -----> true address 0-998
				
				//put varable into map
				std::vector<int> coils_params(coils_addr,coils_addr+2);
				std::pair<std::string,std::vector<int>> data_params("coil", coils_params);
				data_map[name] = data_params;	
			}
			else if (item == "inputbit") //discrete input
			{
				int inputbits_addr[2]; 
				//inputbits_addr[0]: start address
				//inputbits_addr[1]: number of values
				ss>>name>>inputbits_addr[0]>>inputbits_addr[1];
				
				if (!ss || inputbits_addr[0] < 1 || inputbits_addr[1] < 1)
				{
					std::cerr<<"The data parameters are not correct! "
							 <<"FORMAT: inputbit NAME START_ADDR NUMBER_OF_VALUES"<<std::endl;
					throw std::runtime_error("Configuration file parsing failed. " + line);
				}
				
				inputbits_addr[0]--; //convert PLC address space mapping to true address space
								 	 //e.g In PLC, address 1-999 -----> true address 0-998
								 	 
				//put varable into map
				std::vector<int> inputbits_params(inputbits_addr, inputbits_addr+2);
				std::pair<std::string,std::vector<int>> data_params("input_bit", inputbits_params);
				data_map[name] = data_params;	
			}
			else if (item == "register") //holding register
			{
				int registers_addr[2]; 
				//registers_addr[0]: start address
				//registers_addr[1]: number of values
				ss>>name>>registers_addr[0]>>registers_addr[1];
				
				if (!ss || registers_addr[0] < 1 || registers_addr[1] < 1)
				{
					std::cerr<<"The data parameters are not correct! "
							 <<"FORMAT: register NAME START_ADDR NUMBER_OF_VALUES"<<std::endl;
					throw std::runtime_error("Configuration file parsing failed. " + line);
				}
				
				registers_addr[0]--; //convert PLC address space mapping to true address space
								 	 //e.g In PLC, address 1-999 -----> true address 0-998
				
				//put varable into map
				std::vector<int> registers_params(registers_addr,registers_addr+2);
				std::pair<std::string,std::vector<int>> data_params("holding_register", registers_params);
				data_map[name] = data_params;	
			}
			else if (item == "inputreg") //input registers
			{
				int inputregs_addr[2]; 
				//inputregs_addr[0]: start address
				//inputregs_addr[1]: number of values
				ss>>name>>inputregs_addr[0]>>inputregs_addr[1];
				
				if (!ss || inputregs_addr[0] < 1 || inputregs_addr[1] < 1)
				{
					std::cerr<<"The data parameters are not correct! "
							 <<"FORMAT: coil NAME START_ADDR NUMBER_OF_VALUES"<<std::endl;
					throw std::runtime_error("Configuration file parsing failed. " + line);
				}
				
				inputregs_addr[0]--; //convert PLC address space mapping to true address space
								 	 //e.g In PLC, address 1-999 -----> true address 0-998
				
				//put varable into map
				std::vector<int> inputregs_params(inputregs_addr,inputregs_addr+2);
				std::pair<std::string,std::vector<int>> data_params("input_register", inputregs_params);
				data_map[name] = data_params;	
			}
			else
			{
				std::cerr<<"failed to parse config file in line: "<<line<<std::endl;
				throw std::runtime_error("Configuration file parsing failed. " + line);
			}
		}
	}
	else
	{
		std::cerr<<"The config file "<<config_file<<" cannot be opened"<<std::endl;
		throw std::runtime_error("Configuration file cannot be opened. ");
	}
    
}
