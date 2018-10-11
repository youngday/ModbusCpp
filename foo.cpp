#include "parser.cpp"
#include <iomanip>



int main(int argc, char ** argv)
{
	
	std::unordered_map<std::string, std::pair<std::string, std::vector<int>>> data_map;
	
	std::string ip;
	int port;
	
	parse("./test.conf", ip, port, data_map);
	
	std::cout<<"ip: "<<ip<<" port: "<<port<<std::endl;
	
	for (auto& x: data_map)
		
		std::cout << "Type: "<<std::left <<std::setw(17)<< x.second.first 
				  << " Name: "<< std::left <<std::setw(30) << x.first  << "\tStart Addr: " 
				  << x.second.second[0] <<"\tNum of Value: " << x.second.second[1] << std::endl;	
}
