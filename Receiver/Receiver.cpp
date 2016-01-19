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

static const int PORT = 2016;
static const int BUFFER_SIZE = 1024;
char buffer[BUFFER_SIZE];

int server_socket, result, count;
struct sockaddr_in server_addr, client_addr;

void start_listener()
{
  close(server_socket);
	cout << "Server started..." << endl;
	server_socket = socket(AF_INET, SOCK_DGRAM, 0);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	
	result = bind(server_socket, (struct sockaddr*) &server_addr, sizeof(struct sockaddr));
	char address[INET_ADDRSTRLEN];
	
	if(result == -1)
	{
	  cout << "Bind error with status " << result << "..." << endl;
	}
	else
	{
	  cout << "Bind successful, listening on " << inet_ntop(AF_INET, &(server_addr.sin_addr.s_addr), address, INET_ADDRSTRLEN) << "..." << endl;
	}
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
    
  return "BAD";
}

void receive_data()
{
  count = recvfrom(server_socket, &buffer, BUFFER_SIZE, 0, (struct sockaddr*) &client_addr, (socklen_t*) &client_addr);
  if(count == -1)
  {
    cout << "recv() error" << endl;
  }
  else
  {
    cout << endl << "************************************************************" << endl << endl;
    cout << "Retrieved data: " << buffer << endl;
    string header, data, trailer, received(buffer);
    header = received.substr(8, 8);
    data = received.substr(16, 32);
    trailer = received.substr(48, 16);
    
    cout << "HEADER: " << binary_to_dec(header) << endl;
    cout << "DATA: " << binary_to_ascii(data) << endl;
    cout << "CHECKSUM IS " << analyse_integrity(binary_to_ascii(data), trailer) << endl;
    
    sleep(1);
  }
}
void save_to_file(){}
void echo(){}
void close_connection()
{
  close(server_socket);
}

int main()
{
  start_listener();
  receive_data();
  close_connection();
  return 0;
}
