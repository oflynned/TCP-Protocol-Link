//data and bit manipulation
#include <iostream>
#include <cstring>
#include <ctime>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <bitset>

//TCP connection handling
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

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
static const int MAX_DATA_PACKET_SIZE_BYTES = HEADER_SIZE_BYTES + DATA_SIZE_BYTES + TRAILER_SIZE_BYTES;
static const int MAX_DATA_PACKET_SIZE_BITS = MAX_DATA_PACKET_SIZE_BYTES * 8;

static const int PORT = 2016;
string ORIGIN_IP = "127.0.0.1";
string DESTINATION_IP = "127.0.0.1";

struct sockaddr_in client_addr;
socklen_t addr_len = sizeof(struct sockaddr);
int client_socket;
int count;

const static int BUFFER_SIZE = 1024;
char buffer[BUFFER_SIZE];

bool _ack = false;
string _echo = "";
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

//start of data encapsulation ----->
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
	string data_frame;

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

	return bitset<DATA_SIZE_BITS>(bits);
}

bitset<TRAILER_SIZE_BITS> generate_checksum(string data, int length){
	bitset<16> polynomial(65535);
	bitset<16> remainder(data);
	bitset<16> divisor(128);

	for(unsigned int bit = 8; bit > 0; bit--)
	{
		//check if uppermost bit is 1
		if((remainder & divisor) == 1)
		{
			//XOR remainder with divisor
			remainder ^= polynomial;
		}
		//shift bit into remainder
		remainder = (remainder << 1);
	}
	
	cout << "Poly " << polynomial << ", Divisor " << divisor << ", CRC " << (remainder >> 4) << endl; 
	
    	return (remainder >> 4);
}

int generate_random_number(int low, int high) {
    if (low > high) 
	return high;
    return low + (rand() % (high - low + 1));
}

string gremlin_checksum(bitset<TRAILER_SIZE_BITS> data)
{	
	bitset<16> remainder(data);
	int random = generate_random_number(0,99);
	if(random < 20)
	{
		//invoked -- flip checksum bits	
		remainder = ~remainder;
		cout << "Gremlin function invoked! CRC is now " << remainder.to_string() << endl;
	}
	return remainder.to_string();
}

bitset<TRAILER_SIZE_BITS> generate_trailer(string data)
{	
	char charArray[data.length()];
	data.copy(charArray, data.length());

	string bits;

	for(int i = 0; i < data.length(); i++)
	{	
		bitset<8> x(charArray[i]);
		bits += x.to_string();
	}

	string data_concat = bits.substr(0, 8);
	
	return bitset<TRAILER_SIZE_BITS>(gremlin_checksum(generate_checksum(data_concat, data_concat.length())));
}

//<----- end of data encapsulation

//start of data transmission ----->

void setAck(bool ack){ _ack = ack; }
bool isAck(){ return _ack; }
void setEcho(string echo){ _echo = echo; }
string getEcho(){ return _echo; }

string get_allocated_ip(struct sockaddr_in client_addr)
{
	char address[INET_ADDRSTRLEN];
  return inet_ntop(AF_INET, &(client_addr.sin_addr.s_addr), address, INET_ADDRSTRLEN);
}

void open_socket()
{
	cout << "Opening connection..." << endl;
	socket(AF_INET, SOCK_DGRAM, 0);
	client_addr.sin_family = AF_INET;
	client_addr.sin_port = htons(PORT);
	client_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	cout << "Allocated IP " << get_allocated_ip(client_addr) << endl;
}

void send_data(int sequenceNumber, string data_packet)
{
	cout << "Sending packet #" << sequenceNumber << "..." << endl;
	const char* data = data_packet.c_str();
	strcpy(buffer, data);
	count = sendto(client_socket, &buffer, BUFFER_SIZE, 0, (struct sockaddr*) &client_addr, sizeof(client_addr));
	if(count == -1)
	{
	  cout << "Send error" << endl;
	}
	else
	{
	  cout << "Sent successfully" << endl;
	}
}

void receive_ack()
{
	cout << "Acknowledgement received..." << endl;

	//echo ack from server
  
	//handle ack accordingly
	if(!isAck())
	{
		cout << "Echo frame corruption, retrying..." << endl;
	}
	else {
		cout << "Echo good data integrity, continuing..." << endl;
	}
}

void close_socket()
{
	cout << "Closing connection..." << endl;
	close(client_socket);
}

//<----- end of data transmission 

int main()
{
	//generate and parse data
	generate_output_file();
	split_data(get_data());

	//parse data to packets if successful in binary from ascii chars
	DATA_PACKET dataPacket;
	//ARRAY_SIZE
	for(int i = 0; i < 2; i++)
	{
		//set up sockets
		cout << "Packet #" << i << endl;
		open_socket();
		cout << endl;

		//data processing
		
		dataPacket.header_string = to_string(i);
		dataPacket.data_string = dataStream[i];
		dataPacket.trailer_string = dataStream[i];

		dataPacket.header = generate_header(i);
		dataPacket.data = generate_data(dataStream[i]);
		dataPacket.trailer = generate_trailer(dataStream[i]);

		cout << "HEADER: " << dataPacket.header_string << " parsed to " << dataPacket.header  << 
		" (" << dataPacket.header.size() << " bits)" << endl;
		cout << "DATA: " << dataPacket.data_string << " parsed to " << dataPacket.data << 
		" (" << dataPacket.data.size() << " bits)" << endl;
		cout << "TRAILER: checksum for " << dataPacket.trailer_string << " parsed to " << dataPacket.trailer << 
		" (" << dataPacket.trailer.size() << " bits)" << endl << endl; 
		
		dataPacket.data_frame = dataPacket.header.to_string() + dataPacket.data.to_string() + dataPacket.trailer.to_string();
		cout << "dataframe for packet #" << dataPacket.header_string << endl;
		cout << dataPacket.data_frame << endl << endl;

		//send the ith packet	
		send_data(i, dataPacket.data_frame);

		//wait and receive ack, if echo has integrity, follow through with next packet
		receive_ack();

		//reset
		close_socket();	

		cout << endl << "************************************************************" << endl << endl;
	}

	return 0;
}
