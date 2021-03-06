//data handling
#include <iostream>
#include <cstring>
#include <ctime>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <bitset>

//UDP connection
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

static const int SERVER_PORT = 2016;
static const int CLIENT_PORT = 2015;
static const int BUFFER_SIZE = 1024;
char buffer[BUFFER_SIZE];
const int MAX_PACKETS = 255;

static const char* IP_CLIENT = "127.0.0.1";
static const char* IP_HOST = "127.0.0.1";

static const string regex = "011111";

int server_socket, result;
int req_send, req_recv;
struct sockaddr_in server_addr, client_addr;

bool isEnd = false;
string message;

string get_allocated_ip(struct sockaddr_in addr);
string generate_trailer(string data);
string analyse_integrity(string data, string trailer);

void start_listener();
void receive_data();
void echo(string result);
void close_connection();
void start_echo();
void save_to_file(string data);
static int binary_to_dec(string sequenceNumber);
bitset<16> generate_checksum(string data, int length);

bool hasEnding (string const &data, string const &regex);
string unstuff_bits(string data);

int main()
{
	cout << endl << "************************************************************" << endl << endl;
	cout << "Server started..." << endl;
 	while(!isEnd){receive_data();}
  save_to_file(message);
  close_connection();
  return 0;
}

string get_allocated_ip(struct sockaddr_in addr)
{
	char address[INET_ADDRSTRLEN];
  return inet_ntop(AF_INET, &(addr.sin_addr.s_addr), address, INET_ADDRSTRLEN);
}

void close_connection()
{
	close(server_socket);
}

void start_listener()
{
  close(server_socket);
	server_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	server_addr.sin_addr.s_addr = inet_addr(IP_HOST);
	
	result = bind(server_socket, (struct sockaddr*) &server_addr, sizeof(struct sockaddr));
	
	if(result == -1)
	{
	  cout << "Bind error with status " << result << "..." << endl;
	}
	else
	{
	  cout << "Bind successful, listening on " << get_allocated_ip(server_addr) << ":" << SERVER_PORT << "..." << endl;
	}
}

void start_echo()
{
	close(server_socket);
	server_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	client_addr.sin_family = AF_INET;
	client_addr.sin_port = htons(CLIENT_PORT);
	client_addr.sin_addr.s_addr = inet_addr(IP_CLIENT);
}

static int binary_to_dec(string sequenceNumber)
{
  return stoi(sequenceNumber, nullptr, 2);
}

static string binary_to_ascii(string data)
{
  string output;
  stringstream sstream(data);
  while(sstream.good())
  {
    bitset<8> bits;
    sstream >> bits;
    char c = char(bits.to_ulong());
    output += c;
  }
  return output;
}

bitset<16> generate_checksum(string data, int length){
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
	
  return (remainder >> 4);
}

string generate_trailer(string data)
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
	
	return bitset<16>(generate_checksum(data_concat, data_concat.length())).to_string();
}

string analyse_integrity(string data, string trailer)
{
  if(generate_trailer(data) == trailer)
    return "GOOD";
  else return "BAD";
}

void echo(string result)
{
		//establish a new echo connection with the client from the receiving address	
		start_echo();
		socklen_t server_addr_len = sizeof(server_addr);
		
		//parse result of integrity check for client packet handling
		cout << "Echoing " << result << " to client..." << endl;
		const char* reply = result.c_str();
		strcpy(buffer, reply);
		
		usleep(1000);
		
		req_send = sendto(server_socket, &buffer, BUFFER_SIZE, 0, (struct sockaddr*) &client_addr, sizeof(client_addr));
		
		if(req_send == -1)
		{
			cout << "Send error!" << endl;		
		}
		else
		{
			cout << "Echo successful to client" << endl;
		}
		close_connection();
}

void receive_data()
{
	memset(buffer, 0, sizeof(buffer));
	start_listener();
	req_recv = recvfrom(server_socket, &buffer, BUFFER_SIZE, 0, (struct sockaddr*) &client_addr, (socklen_t*) &client_addr);
	 
	cout << endl << "************************************************************" << endl << endl;
	cout << "Retrieved data: " << endl << buffer << endl;
	string header, data, trailer, received(buffer);
	received = unstuff_bits(received);
	header = received.substr(0, 8);
	data = received.substr(8, 32);
	trailer = received.substr(40, 16);
	
	if(binary_to_dec(header) > 0)
	{
		cout << "Reinitialising server listener..." << endl;
	}

	if(binary_to_dec(header) == MAX_PACKETS)
	{
		isEnd = true;
	}
	
	string decompiled_data = binary_to_ascii(data);
		
	cout << "Packet " << binary_to_dec(header) << "/" << MAX_PACKETS << endl;
	cout << "Data contained in payload: \"" << decompiled_data << "\"" << endl;
	cout << "Integrity: " << analyse_integrity(binary_to_ascii(data), trailer) << endl;
		
	echo(analyse_integrity(binary_to_ascii(data), trailer));
	
	if(analyse_integrity(binary_to_ascii(data), trailer) == "GOOD")
	{
		message += decompiled_data;
	}
	//usleep(1000);
}

void save_to_file(string data)
{
	cout << endl << "************************************************************" << endl << endl;
	cout << "Transmission finished!" << endl;
	ofstream out;
	out.open("received.txt");
	
	cout << message << endl << endl;
	
	if(out.fail()){
		cout << "Error in opening file" << endl;
	} else {
		out << data;
		out.close();
		cout << "File created successfully" << endl << endl;
	}
}

bool hasEnding(string const &data, string const &regex) {
    return data.size() >= regex.size() && 
            data.compare(data.size() - regex.size(), regex.size(), regex) == 0;
}

string unstuff_bits(string data)
{
  //01111101... -> 0111111...
  cout << "Unstuffing bits..." << endl;
  
  char stuffedArray[data.length()];
  data.copy(stuffedArray, data.length());
  
  string currSequence = "", unstuffedSequence = "";
  
  for(int i = 0; i < data.length(); i++)
  {
    currSequence += stuffedArray[i];
    unstuffedSequence += stuffedArray[i];
    
    if(hasEnding(currSequence, regex))
    {
      currSequence = "";
      i++;
    }
  }
  
  return unstuffedSequence;
}
