#include "includes/parser.h"
#include "includes/modbus.h"
#include <iomanip>
#include <vector>

void displayhelp();
void displayvar(const std::unordered_map<std::string, std::pair<std::string, std::vector<int>>>& data_map);
void oper_write(ModBusConnector & conn, std::stringstream& ss, const bool is_float,
				std::unordered_map<std::string, std::pair<std::string, std::vector<int>>> & data_map);
void oper_read(ModBusConnector & conn, std::stringstream& ss, const bool is_float,
				std::unordered_map<std::string, std::pair<std::string, std::vector<int>>> & data_map);
void oper_read_write(ModBusConnector & conn, std::stringstream& ss, const bool is_float,
				std::unordered_map<std::string, std::pair<std::string, std::vector<int>>> & data_map);

int main(int argc, char ** argv)
{
	
	std::unordered_map<std::string, std::pair<std::string, std::vector<int>>> data_map;
	
	std::string ip;
	int port;
	
	ModbusConfigParser::parse("./demo.conf", ip, port, data_map);
	
	std::cout<<"Connection to be established"<<std::endl;
	
	std::cout<<"ip: "<<ip<<" port: "<<port<<std::endl<<std::endl;
	
	ModBusConnector conn = ModBusConnector(ip, port);
	
	std::cout<<"Available Variables List: "<<std::endl;
	
	displayvar(data_map);
	
	displayhelp();
	
	std::string oper;
	
	while(1)
	{
		conn.disconnect();
		std::cout<<std::endl<<"Please Type Command:"<<std::endl<<"command>";
						
		std::string input;
		std::getline(std::cin,input);
		
		if (input.empty())
		{
			continue;
		}
		std::stringstream ss(input);
		
		if (!(ss>>oper) || !oper.size())
		{
			continue;
		}
		
		if (oper == "exit")
		{
			break;
		}
		
		if (oper == "help")
		{
			displayhelp();
			continue;
		}
		
		if (oper == "var")
		{
			displayvar(data_map);
			continue;
		}
		
		try
		{
		
			conn.connect();
		
		}
		catch(std::exception & ex)
		{
			std::cout<<ex.what()<<std::endl;
			continue;
		}
		
		if (oper == "write")
		{
			oper_write(conn, ss, false, data_map);
			continue;		
		}
		
		else if (oper == "write_real")
		{
			oper_write(conn, ss, true, data_map);
			continue;
		}
		
		else if (oper == "read")
		{
			oper_read(conn, ss, false, data_map);
			continue;	
		}
		
		else if (oper == "read_real")
		{
			oper_read(conn, ss, true, data_map);
			continue;
		}
		
		else if (oper == "read_write")
		{
			oper_read_write(conn, ss, false, data_map);
		}
		
		else if (oper == "read_write_real")
		{
			oper_read_write(conn, ss, true, data_map);
		}
		else
		{
			std::cerr<<"No such operation"<<std::endl;
			continue;
		}
	}
}

void displayhelp()
{
	std::cout<<"Avaiable Command: "<<std::endl;
		std::cout<<std::left<<std::setw(30)<<"help"<<std::right
			<<"display this information"<<std::endl;
		std::cout<<std::left<<std::setw(30)<<"exit"<<std::right
			<<"exit program"<<std::endl;
		std::cout<<std::left<<std::setw(30)<<"var"<<std::right
			<<"display available variables"<<std::endl;
		std::cout<<std::left<<std::setw(30)<<"read VAR_NAME"<<std::right
			<<"read value of VAR_NAME"<<std::endl;
		std::cout<<std::left<<std::setw(30)<<"write VAR_NAME"<<std::right
			<<"write random numbers into VAR_NAME"<<std::endl;
		std::cout<<std::left<<std::setw(30)<<"read_write VAR_NAME"<<std::right
			<<"write random numbers into VAR_NAME and read the values"<<std::endl;
		std::cout<<std::left<<std::setw(30)<<"read_real VAR_NAME"<<std::right
			<<"read real value of VAR_NAME"<<std::endl;
		std::cout<<std::left<<std::setw(30)<<"write_real VAR_NAME"<<std::right
			<<"write random real numbers into VAR_NAME"<<std::endl;
		std::cout<<std::left<<std::setw(30)<<"read_write_real VAR_NAME"<<std::right
			<<"write random real numbers into VAR_NAME and read the values"<<std::endl;
}

void displayvar(const std::unordered_map<std::string, std::pair<std::string, std::vector<int>>>& data_map)
{
	for (auto& x: data_map)
		
		std::cout << "Type: "<<std::left <<std::setw(17)<< x.second.first 
				  << " Name: "<< std::left <<std::setw(15) << x.first  << "\tStart Addr: " 
				  << x.second.second[0]+1 <<"\tNum of Value: " << x.second.second[1] << std::endl<<std::endl;
}

void oper_write(ModBusConnector & conn, std::stringstream& ss, const bool is_float,
				std::unordered_map<std::string, std::pair<std::string, std::vector<int>>> & data_map)
{
	std::string name;
	ss>>name;
	if(!ss)
	{
		std::cerr<<"Error Input"<<std::endl;
		return;
	}
			
	std::unordered_map<std::string, std::pair<std::string, std::vector<int>>>
		::const_iterator got = data_map.find(name);
	if (got != data_map.end())
	{
		std::string type = (got->second).first;
		int start_addr = (got->second).second[0];
		int num = (got->second).second[1];
	
		if (type == "input_register" || type == "input_bit")
		{
			std::cerr<<"Input bits or registers are written forbiden"<<std::endl;
			return;
		}
		else if (type == "holding_register")
		{
			std::cout<<"Writing random numbers into the selected holding registers..."<<std::endl;
			std::vector<uint16_t> tab_rq_registers(num,0);
			if (is_float)
			{
				float value = 0.0;
				for (int i=0; i<num-1; i += 2) {	
					value = (float) ((double) rand() / ((double)RAND_MAX/65535.0));
               		ModBusConnector::set_float(value, tab_rq_registers[i], tab_rq_registers[i+1]);
               		std::cout<<std::setprecision(7)<<value<<"\t";
            	}
			}
			else
			{
				for (int i=0; i<num; i++) {
               		tab_rq_registers[i] = (uint16_t) (65535.0*rand() / (RAND_MAX + 1.0));
               		std::cout<<(int16_t)tab_rq_registers[i]<<"\t";
            	}            	
			}
			std::cout<<std::endl;
			
            	
            int rc = conn.write_registers(start_addr, num, tab_rq_registers);
            		
            if (rc == num)
            {
            	std::cout<<"Writing finished successfully"<<std::endl;
            }
            else
            {
            	std::cout<<"Writing finished unsuccessfully"<<std::endl;
            }            		
		}
		else if (type == "coil")
		{
			if (is_float)
			{
				std::cout<<"Cannot write real number into coils"<<std::endl;
				return;
			}
			std::cout<<"Writing random bits into the selected coils..."<<std::endl;
			std::vector<uint8_t> tab_rq_bits(num,0);
			for (int i=0; i<num; i++) {
               	tab_rq_bits[i] = ((uint16_t) (65535.0*rand() / (RAND_MAX + 1.0))) % 2;
                std::cout<<(int)tab_rq_bits[i]<<"\t";
            }
           	std::cout<<std::endl;
            		
            int rc = conn.write_bits(start_addr, num, tab_rq_bits);
            		
            if (rc == num)
            {
            	std::cout<<"Writing finished successfully"<<std::endl;
            }
            else
            {
            	std::cout<<"Writing finished unsuccessfully"<<std::endl;
            } 
		}
		else
		{
			std::cerr<<"Error defined data type"<<std::endl;
			return;
		}
	}
	else
	{
		std::cerr<<"No such data name"<<std::endl;
		return;
	}
}

void oper_read(ModBusConnector & conn, std::stringstream & ss, const bool is_float,
	std::unordered_map<std::string, std::pair<std::string, std::vector<int>>> & data_map)
{
	std::string name;
	ss>>name;
	if(!ss)
	{
		std::cerr<<"Error Input"<<std::endl;
		return;
	}
	
	std::unordered_map<std::string, std::pair<std::string, std::vector<int>>>
		::const_iterator got = data_map.find(name);
	if (got != data_map.end())
	{
		std::string type = (got->second).first;
		int start_addr = (got->second).second[0];
		int num = (got->second).second[1];
		
		if (type == "input_register" || type == "holding_register")
		{
			std::vector<uint16_t> tab_rp_registers(num,0);
			int rc = 0;
			
			if (type == "holding_register")
			{
				std::cout<<"Reading from holding registers"<<std::endl;
				rc = conn.read_registers(start_addr,num,tab_rp_registers);
			}
			
			if (type == "input_register")
			{
				std::cout<<"Reading from input registers"<<std::endl;
				rc = conn.read_input_registers(start_addr,num,tab_rp_registers);
			}
			
			if (rc == num)
			{
				if (is_float)
				{
					float value = 0.0;
					for (int i = 0; i < num-1; i += 2)
					{
						value = ModBusConnector::get_float(tab_rp_registers[i], tab_rp_registers[i+1]);
						std::cout<<std::setprecision(7)<<value<<"\t";
					}
				}
				else
				{
					for (int i = 0; i < num; i += 1)
					{
						std::cout<<(int16_t)tab_rp_registers[i]<<"\t";
					}
				}
				
				std::cout<<std::endl;
				std::cout<<"Reading finished successfully"<<std::endl;
			}
			else
			{
				std::cout<<"Reading finished unsuccessfully"<<std::endl;
			}
		}
		else if (type == "input_bit" || type == "coil")
		{
			if (is_float)
			{
				std::cout<<"Cannot read real number from coils or discrete input"<<std::endl;
				return;
			}
			std::vector<uint8_t> tab_rp_bits(num,0);
			int rc = 0;
			
			if (type == "coil")
			{
				std::cout<<"Reading from coils"<<std::endl;
				rc = conn.read_bits(start_addr,num,tab_rp_bits);
			}
			
			if (type == "input_bit")
			{
				std::cout<<"Reading from input registers"<<std::endl;
				rc = conn.read_input_bits(start_addr,num,tab_rp_bits);
			}
			
			if (rc == num)
			{
				for (int i = 0; i < num; i += 1)
				{
					std::cout<<(int)tab_rp_bits[i]<<"\t";
				}
				std::cout<<std::endl;
				std::cout<<"Reading finished successfully"<<std::endl;
			}
			else
			{
				std::cout<<"Reading finished unsuccessfully"<<std::endl;
			}
		}
		else
		{
			std::cerr<<"Error defined data type"<<std::endl;
			return;
		}
	}
	else
	{
		std::cerr<<"No such data name"<<std::endl;
		return;
	}
}

void oper_read_write(ModBusConnector & conn, std::stringstream& ss, const bool is_float,
				std::unordered_map<std::string, std::pair<std::string, std::vector<int>>> & data_map)
{
	std::string name;
	ss>>name;
	if(!ss)
	{
		std::cerr<<"Error Input"<<std::endl;
		return;
	}
	
	std::unordered_map<std::string, std::pair<std::string, std::vector<int>>>
		::const_iterator got = data_map.find(name);
	if (got != data_map.end())
	{
		std::string type = (got->second).first;
		int start_addr = (got->second).second[0];
		int num = (got->second).second[1];
		
		if (type != "holding_register")
		{
			std::cerr<<"Only holding register is allowed for this operation"<<std::endl;
			return;
		}
				
		std::cout<<"Writing random numbers into and reading the selected holding registers..."<<std::endl;
		std::vector<uint16_t> tab_rw_rq_registers(num,0);
		std::vector<uint16_t> tab_rp_registers(num,0);
		
		if (is_float)
		{
			float value = 0.0;
			for (int i=0; i<num-1; i += 2) {
				value = (float) ((double) rand() / ((double)RAND_MAX/65535.0));
            	ModBusConnector::set_float(value, tab_rw_rq_registers[i], tab_rw_rq_registers[i+1]);
            	std::cout<<std::setprecision(7)<<value<<"\t";
        	}
		}
		else
		{
			for (int i=0; i<num; i++) {
               tab_rw_rq_registers[i] = ~ ((uint16_t) (65535.0*rand() / (RAND_MAX + 1.0)));
               std::cout<<(int16_t)tab_rw_rq_registers[i]<<"\t";
        	}
		}
		
        std::cout<<std::endl;
        
        int rc = conn.write_and_read_registers(start_addr, num, tab_rw_rq_registers,
                                               start_addr, num, tab_rp_registers);
        if (rc == num)
        {
            std::cout<<"Read:"<<std::endl;
            if (is_float)
            {
            	float value_read = 0.0;
            	for (int i = 0; i < num-1; i += 2)
            	{
            		value_read = ModBusConnector::get_float(tab_rp_registers[i],tab_rp_registers[i+1]);
            		std::cout<<std::setprecision(7)<<value_read<<"\t";
            	}
            }
            else
            {
            	for (int i = 0; i < num; i += 1)
            	{
            		std::cout<<(int16_t)tab_rp_registers[i]<<"\t";
            	}
            }
            
            std::cout<<std::endl;
            std::cout<<"Finished successfully"<<std::endl;
        }
        else
        {
            std::cout<<"Finished unsuccessfully"<<std::endl;
        }
	}
	else
	{
		std::cerr<<"No such data name"<<std::endl;
		return;
	}
}


