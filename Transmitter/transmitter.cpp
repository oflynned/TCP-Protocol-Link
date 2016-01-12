#include <iostream>
#include <string>
#include <ctime>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <bitset>

//start of consts ----->
static const char ascii_chars[] = 
"!$%^&*()_+-={}[];'#:@~,./<>?"
"abcdefghijklmnopqrstuvwxyz"
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"0123456789";

static const int RAW_FILE_SIZE = 1024;
static const int ARRAY_SIZE = RAW_FILE_SIZE / 4;
std::string dataStream[ARRAY_SIZE];

//size of payload parts in bytes
static const int HEADER_SIZE_BYTES = 3;
static const int HEADER_SIZE_BITS = HEADER_SIZE_BYTES * 8;
static const int DATA_SIZE_BYTES = 4;
static const int DATA_SIZE_BITS = DATA_SIZE_BYTES * 8;
static const int TRAILER_SIZE_BYTES = 1;
static const int TRAILER_SIZE_BITS = TRAILER_SIZE_BYTES * 8;

static const int ORIGIN_PORT = 1994;
std::string ORIGIN_IP = "127.0.0.1";
static const std::string DESTINATION_IP = "127.0.0.1";
//<----- end of consts

//begin random file generation ----->

//generate a random character from the selection
char generate_random_ascii()
{
	return ascii_chars[rand() % (sizeof(ascii_chars) - 1)];
}

//generate a 1024 alphanumeric string
std::string generate_random_string()
{
	std::string holder;
	srand(time(0));

	for(int i = 0; i < 1024; i++)
	{
		holder += generate_random_ascii();
	}
	return holder;
}

//write the generated string to a txt file
void generate_output_file()
{
	std::string message = generate_random_string();

	std::cout << "String generated: " << message << std::endl;
	std::cout << "Chars generated: " << message.size() << std::endl;

	std::ofstream out;
	out.open("generated.txt");
	
	if(out.fail()){
		std::cout << "Error in opening file" << std::endl;
	} else {
		std::cout << "File created successfully" << std::endl;
		out << message;
		out.close();
	}
}

//<---- end file generation

//start of data transmission ----->
std::string get_data()
{
	std::string output;
	std::ifstream in;
	in.open("generated.txt");
	
	while(!in.eof()){
		getline(in, output);
	}
	
	in.close();
	return output;
}

struct DATA_PACKET 
{
	std::string header;
	std::string data;
	std::string trailer;
};

void split_data(std::string data)
{	
	//split each 4 characters into array elements
	//[0:xxxx][1:xxxx]...[255:xxxx]
	for(int i = 0; i < RAW_FILE_SIZE / 4; i++){
		dataStream[i] = data.substr(i * 4, 4);
	}
}

std::string get_padding(std::string data, int size)
{
	if(data.length() < size)
	{
		std::cout << "data size is less than the determined size, attempting to pad" << std::endl;
		if(data.length() < size)
		{
			for(int i = data.length(); i < size; i++)
			{
				data = "0" + data;
			}	
		}
	}
	else if(data.length() == size)
	{
		std::cout << "data is equivalent to size, do nothing" << std::endl;
	}
	return data;
}

std::string generate_header(int sequenceNumber)
{
	return get_padding(std::to_string(sequenceNumber), HEADER_SIZE_BYTES);
}

std::string generate_data_packet_data(std::string data)
{
	return data;
}

std::string generate_trailer(std::string checksum)
{	
	return checksum;
}

void generate_checksum(){}
void open_socket(){}
void gremlin_function(){}
void send_data(){}
void receive_ack(){}

//<----- end of data transmission 

int main()
{
	//generate and parse data
	generate_output_file();
	split_data(get_data());

	//set up sockets
	
	
	//handle each data packet and parse accordingly
	for(int i = 0; i < ARRAY_SIZE; i++)
	{
		std::cout << i << ": " << get_padding(dataStream[i], DATA_SIZE_BYTES) << std::endl;
	}

	//parse data to packets if successful
	DATA_PACKET dataPacket;
	int i = 0;
	

	for(int i = 0; i < ARRAY_SIZE; i++)
	{
		dataPacket.header = generate_header(i);
		dataPacket.data = generate_data_packet_data(dataStream[i]);
		std::cout << "HEADER \"" << dataPacket.header  << "\" with size in bytes " << dataPacket.header.length() << std::endl;
		std::cout << "DATA \"" << dataPacket.data  << "\" with size in bytes " << dataPacket.data.length() << std::endl << std::endl;
	}
	
	//std::cout << "TRAILER \"" << dataPacket.trailer << "\" with size in bytes " << dataPacket.trailer.length() << std::endl;

	//send

	//wait and receive ack

	//iff has next packet, send data

	//reset
	
	return 0;
}
