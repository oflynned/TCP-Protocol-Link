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
static const int HEADER_SIZE_BYTES = 1;
static const int HEADER_SIZE_BITS = HEADER_SIZE_BYTES * 8;
static const int DATA_SIZE_BYTES = 4;
static const int DATA_SIZE_BITS = DATA_SIZE_BYTES * 8;
static const int TRAILER_SIZE_BYTES = 2;
static const int TRAILER_SIZE_BITS = TRAILER_SIZE_BYTES * 8;
static const int MAX_DATA_PACKET_SIZE_BYTES = 8;
static const int MAX_DATA_PACKET_SIZE_BITS = MAX_DATA_PACKET_SIZE_BYTES * 8;

static const int SERVER_PORT = 2016;
static const int CLIENT_PORT = 2015;
static const char* IP_CLIENT = "127.0.0.1";
static const char* IP_HOST = "127.0.0.1";

static const string regex = "011111";

struct sockaddr_in client_addr, server_addr;
int client_len = sizeof(client_addr);
int server_len = sizeof(server_addr);
socklen_t addr_len = sizeof(struct sockaddr);
int client_socket;
int req_send, req_recv;

const static int BUFFER_SIZE = 1024;
char buffer[BUFFER_SIZE];
const int MAX_PACKETS = 255;

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

char generate_random_ascii();
int generate_random_number(int low, int high);

string generate_random_string();
string gremlin_checksum(bitset<TRAILER_SIZE_BITS> data);
string get_allocated_ip(struct sockaddr_in addr);
string get_data();

bitset<HEADER_SIZE_BITS> generate_header(int sequenceNumber);
bitset<DATA_SIZE_BITS> generate_data(string data);
bitset<TRAILER_SIZE_BITS> generate_checksum(string data, int length);
bitset<TRAILER_SIZE_BITS> generate_trailer(string data);

void generate_output_file();
void split_data(string data);
void open_socket();
void close_connection();
void display_connection_info();
void send_data(int sequenceNumber, string data_packet);
void receive_echo();
void receive_ack(int sequenceNumber, string data);

bool hasEnding (string const &data, string const &regex);
string stuff_bits(string data);
string pad(string data);

//<----- end of data transmission 

int main()
{
	//generate and parse data
	generate_output_file();
	split_data(get_data());

	//parse data to packets if successful in binary from ascii chars
	DATA_PACKET dataPacket;
	for(int i = 0; i <= MAX_PACKETS; i++)
	{
		//set up sockets
		cout << "Packet #" << i << endl;
		open_socket();
		display_connection_info();

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
		receive_ack(i, dataPacket.data_string);
		usleep(2000);

		//reset
		close_connection();	

		cout << endl << "************************************************************" << endl << endl;
	}

  cout << "Original message: " << endl;
  cout << get_data() << endl << endl;
  
	return 0;
}


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

void split_data(string data)
{	
	//split each 4 characters into array elements
	//[0:xxxx][1:xxxx]...[255:xxxx]
	for(int i = 0; i < RAW_FILE_SIZE / 4; i++){
		dataStream[i] = data.substr(i * 4, 4);
	}
	dataStream[MAX_PACKETS] = data.substr(1020, 4);
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
  else return low + (rand() % (high - low + 1));
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

bool hasEnding(string const &data, string const &regex) {
    return data.size() >= regex.size() &&
           data.compare(data.size() - regex.size(), regex.size(), regex) == 0;
}

string stuff_bits(string data)
{
  //0111111... -> 01111101...
  cout << "Stuffing bits..." << endl;
  
  char stuffedArray[data.length()];
  data.copy(stuffedArray, data.length());
  
  string currSequence = "", stuffedSequence = "";
  
  for(int i = 0; i < data.length(); i++)
  {
    currSequence += stuffedArray[i];
    stuffedSequence += stuffedArray[i];
    
    if(hasEnding(currSequence, regex))
    {
      currSequence = "";
      stuffedSequence += "0";
    }
  }
  return stuffedSequence;
}

string pad(string data)
{
  cout << "Padding data from " << data.length() << " to " << MAX_DATA_PACKET_SIZE_BITS << " bits..." << endl;
  if(data.length() < MAX_DATA_PACKET_SIZE_BITS)
  {
    while(data.length() < MAX_DATA_PACKET_SIZE_BITS)
      data+="0";
  }
  return data;
}

//<----- end of data encapsulation

//start of data transmission ----->

string get_allocated_ip(struct sockaddr_in addr)
{
	char address[INET_ADDRSTRLEN];
  return inet_ntop(AF_INET, &(addr.sin_addr.s_addr), address, INET_ADDRSTRLEN);
}

void close_connection()
{
	close(client_socket);
}

void open_socket()
{
  close_connection();
	client_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	inet_aton(IP_HOST, &server_addr.sin_addr);
}

void display_connection_info()
{
	cout << endl;
	cout << "Opening connection on " << get_allocated_ip(server_addr) << ":" << SERVER_PORT << "..." << endl;
}

void send_data(int sequenceNumber, string data_packet)
{
	open_socket();
	usleep(100);
	cout << "Sending packet #" << sequenceNumber << "..." << endl;
	const char* data = pad(stuff_bits(data_packet)).c_str();
	strcpy(buffer, data);
	
	req_send = sendto(client_socket, &buffer, BUFFER_SIZE, 0, (struct sockaddr*) &server_addr, sizeof(server_addr));
	if(req_send == -1)
	{
		cout << "Send error!" << endl;
  	close_connection();
	}
	else
	{
	 	cout << "Packet sent successfully..." << endl << endl;
	}
}

void receive_echo()
{
	close(client_socket);
	client_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	client_addr.sin_family = AF_INET;
	client_addr.sin_port = htons(CLIENT_PORT);
	client_addr.sin_addr.s_addr = inet_addr(IP_CLIENT);
	cout << "Listening for echo on " << get_allocated_ip(client_addr) << ":" << CLIENT_PORT << endl;
	
	int result = bind(client_socket, (struct sockaddr*) &client_addr, sizeof(struct sockaddr));
	if(result == -1)
	{
		cout << "Error on binding!" << endl;
  	close_connection();
	}
	else
	{
		cout << "Bind successful, listening on " << get_allocated_ip(client_addr) << ":" << CLIENT_PORT << endl;
	}
}

void receive_ack(int sequenceNumber, string data)
{
	cout << "Waiting for ack..." << endl;
	receive_echo();
	//clear the buffer before proceding to continue as not to overflow it
	memset(buffer, 0, sizeof(buffer));
	req_recv = recvfrom(client_socket, &buffer, BUFFER_SIZE, 0, (struct sockaddr*) &client_addr, (socklen_t*) &client_addr);

	string ack = buffer;	
	if(ack == "GOOD" && sequenceNumber != MAX_PACKETS)
	{				
		cout << "Server echo: GOOD integrity in data, sending next packet..." << endl;
	}
	else if(ack == "BAD")
	{
		cout << "Server echo: BAD integrity in data, regenerate packet..." << endl;
		
		//regenerate datapacket as before with a chance of the gremlin function		
		string header = generate_header(sequenceNumber).to_string();
		string data_packet = generate_data(dataStream[sequenceNumber]).to_string();
		string trailer = generate_trailer(dataStream[sequenceNumber]).to_string();
		string data_frame = header + data_packet + trailer;
		cout << endl << "dataframe for regenerated packet (" << sequenceNumber << ")" << endl;
		cout << data_frame << endl << endl;

		//send the ith packet
		cout << "Retry sending data..." << endl;	
		send_data(sequenceNumber, data_frame);
	}
	else if(ack == "GOOD" && sequenceNumber == MAX_PACKETS)
	{
		cout << "Server echo: GOOD integrity in data, finishing transmission..." << endl;
	}
	close_connection();
}

//<----- end of data transmission 
