#include <iostream>
#include <string>
#include <ctime>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <bitset>
#include <algorithm>
#include <iterator>

using namespace std;

//start of consts ----->
static const char ascii_chars[] = 
"!$%^&*()_+-={}[];'#:@~,./<>?"
"abcdefghijklmnopqrstuvwxyz"
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"0123456789";

static const int RAW_FILE_SIZE = 1024;
static const int ARRAY_SIZE = RAW_FILE_SIZE / 4;
string dataStream[ARRAY_SIZE];

//size of payload parts in bytes
static const int HEADER_SIZE_BYTES = 3;
static const int HEADER_SIZE_BITS = HEADER_SIZE_BYTES * 8;
static const int DATA_SIZE_BYTES = 4;
static const int DATA_SIZE_BITS = DATA_SIZE_BYTES * 8;
static const int TRAILER_SIZE_BYTES = 1;
static const int TRAILER_SIZE_BITS = TRAILER_SIZE_BYTES * 8;

static const int ORIGIN_PORT = 1994;
string ORIGIN_IP = "127.0.0.1";
static const string DESTINATION_IP = "127.0.0.1";
//<----- end of consts

//begin random file generation ----->

//generate a random character from the selection
char generate_random_ascii()
{
	return ascii_chars[rand() % (sizeof(ascii_chars) - 1)];
}

//generate a 1024 alphanumeric string
string generate_random_string()
{
	string holder;
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
	string message = generate_random_string();

	cout << "String generated: " << message << endl;
	cout << "Chars generated: " << message.size() << endl;

	ofstream out;
	out.open("generated.txt");
	
	if(out.fail()){
		cout << "Error in opening file" << endl;
	} else {
		cout << "File created successfully" << endl;
		out << message;
		out.close();
	}
}

//<---- end file generation

//start of data transmission ----->
string get_data()
{
	string output;
	ifstream in;
	in.open("generated.txt");
	
	while(!in.eof()){
		getline(in, output);
	}
	
	in.close();
	return output;
}

struct DATA_PACKET 
{
	string header;
	string data;
	string trailer;
};

void split_data(string data)
{	
	//split each 4 characters into array elements
	//[0:xxxx][1:xxxx]...[255:xxxx]
	for(int i = 0; i < RAW_FILE_SIZE / 4; i++){
		dataStream[i] = data.substr(i * 4, 4);
	}
}

string get_padding(string data, int size)
{
	if(data.length() < size)
	{
		cout << "data size is less than the determined size, attempting to pad" << endl;
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
		cout << "data is equivalent to size, do nothing" << endl;
	}
	return data;
}

string generate_header(int sequenceNumber)
{
	return get_padding(to_string(sequenceNumber), HEADER_SIZE_BYTES);
}

string generate_data_packet_data(string data)
{
	return data;
}

string convert_ascii_to_binary(char data)
{
	//convert each char in string to binary representation of ascii
	int bits[8];
	string binary;
	//ensure to reverse representation
	for(int i = 0; i < 8; i++)
	{
		bits[i] = ((1<<i) & data) !=0 ? 1 : 0;
	}

	for(int i = 8; i > 0; i--)
	{
		binary += bits[i];
	}

	return binary;
}

string generate_trailer(string data)
{	
	char *dataArray = new char[data.size() + 1];
	memcpy(dataArray, data.c_str(), data.size());

	cout << dataArray << endl;

	return "0";
}

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
		cout << i << ": " << get_padding(dataStream[i], DATA_SIZE_BYTES) << endl;
	}

	//parse data to packets if successful
	DATA_PACKET dataPacket;
	for(int i = 0; i < ARRAY_SIZE; i++)
	{
		dataPacket.header = generate_header(i);
		dataPacket.data = generate_data_packet_data(dataStream[i]);
		dataPacket.trailer = generate_trailer(dataStream[i]);
		cout << "HEADER \"" << dataPacket.header  << "\" with size in bytes " << dataPacket.header.length() << endl;
		cout << "DATA \"" << dataPacket.data  << "\" with size in bytes " << dataPacket.data.length() << endl;
		cout << "TRAILER \"" << dataPacket.trailer  << "\" with size in bytes " << dataPacket.trailer.length() << endl << endl;
	}
	
	//cout << "TRAILER \"" << dataPacket.trailer << "\" with size in bytes " << dataPacket.trailer.length() << endl;

	//send

	//wait and receive ack

	//iff has next packet, send data

	//reset
	
	return 0;
}
