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
static const int HEADER_SIZE_BYTES = 2;
static const int HEADER_SIZE_BITS = HEADER_SIZE_BYTES * 8;
static const int DATA_SIZE_BYTES = 4;
static const int DATA_SIZE_BITS = DATA_SIZE_BYTES * 8;
static const int TRAILER_SIZE_BYTES = 2;
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
	//data to be sent
	bitset<HEADER_SIZE_BITS> header;
	bitset<DATA_SIZE_BITS> data;
	bitset<TRAILER_SIZE_BITS> trailer;
	
	//reference value for debug
	string header_string;
	string data_string;
	string trailer_string;
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

bitset<HEADER_SIZE_BITS> generate_header(int sequenceNumber)
{
	return bitset<HEADER_SIZE_BITS>(sequenceNumber);
}

bitset<DATA_SIZE_BITS> generate_data(string data)
{
	char charArray[data.length()];
	data.copy(charArray, data.length());

	string bits;

	for(int i = 0; i < data.length(); i++)
	{	
		bitset<8> x(charArray[i]);
		bits += x.to_string();
	}

	cout << data << " represented as " << bits << endl;

	return bitset<DATA_SIZE_BITS>(bits);
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
	
	//handle each data packet and parse accordingly to strings
	for(int i = 0; i < ARRAY_SIZE; i++)
	{
		cout << i << ": " << get_padding(dataStream[i], DATA_SIZE_BYTES) << endl;
	}

	//set up sockets

	//parse data to packets if successful in binary from ascii chars
	DATA_PACKET dataPacket;
	for(int i = 0; i < ARRAY_SIZE; i++)
	{
		dataPacket.header_string = i;
		dataPacket.data_string = dataStream[i];
		dataPacket.trailer_string = dataStream[i];

		dataPacket.header = generate_header(i);
		dataPacket.data = generate_data(dataStream[i]);

		//dataPacket.header = generate_header(i);
		cout << "HEADER " << dataPacket.header.to_string() << endl;
		cout << "DATA " << dataPacket.data.to_string() << endl << endl;
	}

	//send

	//wait and receive ack

	//iff has next packet, send data

	//reset
	
	return 0;
}
