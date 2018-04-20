#include <unordered_map>
#include <utility>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <locale>

class ModbusConfigParser
{
	public:
		static void parse(const std::string& config_file, std::string& ip, int& port, 
			std::unordered_map<std::string, std::pair<std::string, std::vector<int>>>& data_map);
};

