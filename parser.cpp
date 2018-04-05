#include <unordered_map>
#include <utility>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <locale>

void parse(const std::string& config_file, std::string& ip, int& port, 
			std::unordered_map<std::string, std::pair<std::string, std::vector<int>>>& data_map)
{
	std::ifstream ifs(config_file);
	
	if (ifs.is_open())
	{
		std::string line;
		while (std::getline(ifs,line))
		{
			if (line.empty())
			{
				continue;
			}
			
			std::stringstream ss(line);
			std::string item,name;
			
			if (!(ss>>item) || !item.size() || item[0] == '#')
			{
				continue;
			}		
			
			
			if (item == "connection_params")
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
			else if (item == "coil")
			{
				int coils_addr[2]; 
				//coils_addr[0]: start address
				//coils_addr[1]: number of values
				ss>>name>>coils_addr[0]>>coils_addr[1];
				
				if (!ss)
				{
					std::cerr<<"The data parameters are not correct! "
							 <<"FORMAT: coil NAME START_ADDR NUMBER_OF_VALUES"<<std::endl;
					throw std::runtime_error("Configuration file parsing failed. " + line);
				}
				
				std::vector<int> coils_params(coils_addr,coils_addr+2);
				std::pair<std::string,std::vector<int>> data_params("coil", coils_params);
				data_map[name] = data_params;	
			}
			else if (item == "inputbit")
			{
				int inputbits_addr[2]; 
				//coils_addr[0]: start address
				//coils_addr[1]: number of values
				ss>>name>>inputbits_addr[0]>>inputbits_addr[1];
				
				if (!ss)
				{
					std::cerr<<"The data parameters are not correct! "
							 <<"FORMAT: inputbit NAME START_ADDR NUMBER_OF_VALUES"<<std::endl;
					throw std::runtime_error("Configuration file parsing failed. " + line);
				}
				
				std::vector<int> inputbits_params(inputbits_addr, inputbits_addr+2);
				std::pair<std::string,std::vector<int>> data_params("input_bit", inputbits_params);
				data_map[name] = data_params;	
			}
			else if (item == "register")
			{
				int registers_addr[2]; 
				//coils_addr[0]: start address
				//coils_addr[1]: number of values
				ss>>name>>registers_addr[0]>>registers_addr[1];
				
				if (!ss)
				{
					std::cerr<<"The data parameters are not correct! "
							 <<"FORMAT: register NAME START_ADDR NUMBER_OF_VALUES"<<std::endl;
					throw std::runtime_error("Configuration file parsing failed. " + line);
				}
				
				std::vector<int> registers_params(registers_addr,registers_addr+2);
				std::pair<std::string,std::vector<int>> data_params("holding_register", registers_params);
				data_map[name] = data_params;	
			}
			else if (item == "inputreg")
			{
				int inputregs_addr[2]; 
				//inputregs_addr[0]: start address
				//inputregs_addr[1]: number of values
				ss>>name>>inputregs_addr[0]>>inputregs_addr[1];
				
				if (!ss)
				{
					std::cerr<<"The data parameters are not correct! "
							 <<"FORMAT: coil NAME START_ADDR NUMBER_OF_VALUES"<<std::endl;
					throw std::runtime_error("Configuration file parsing failed. " + line);
				}
				
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