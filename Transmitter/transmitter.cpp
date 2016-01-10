#include <iostream>
#include <string>
#include <ctime>
#include <cstdlib>
#include <fstream>
#include <sstream>

//start of consts ----->
static const char ascii_chars[] = 
"!$%^&*()_+-={}[];'#:@~,./<>?"
"abcdefghijklmnopqrstuvwxyz"
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"0123456789";

static const int RAW_FILE_SIZE = 1024;
static const int ARRAY_SIZE = RAW_FILE_SIZE / 4;
std::string dataStream[RAW_FILE_SIZE / 4];
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
	std::string checksum;
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

void parse_data_packet(){}
void generate_checksum(){}
void open_socket(){}
void gremlin_function(){}
void send_data(){}
void receive_ack(){}

//<----- end of data transmission 

int main()
{
	generate_output_file();
	split_data(get_data());
	for(int i = 0; i < RAW_FILE_SIZE / 4; i++){
		std::cout << i << ": " << dataStream[i] << std::endl;
	}
	return 0;
}
