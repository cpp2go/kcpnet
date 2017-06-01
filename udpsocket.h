#ifndef __UDPSOCKET_H__
#define __UDPSOCKET_H__

#include <winsock2.h> 
#pragma comment(lib,"ws2_32.lib")

class udpsocket
{
public:
	udpsocket()
	{
		this->udpsock = 0;
	}
	udpsocket(SOCKET udpsock, struct sockaddr_in *paddr)
	{
		this->udpsock = udpsock;
		this->sockaddr = *paddr;
	}

	~udpsocket()
	{

	}

	bool bind(const char *addr, unsigned int short port)
	{
		udpsock = socket(AF_INET, SOCK_DGRAM, 0);
		if (udpsock <= 0)
		{
			return false;
		}

		unsigned int yes = 1;
		setsockopt(udpsock, SOL_SOCKET, SO_REUSEADDR, (char*)&yes, sizeof(yes));

		memset(&sockaddr, 0, sizeof(sockaddr));
		sockaddr.sin_family = AF_INET;
		sockaddr.sin_addr.S_un.S_addr = inet_addr(addr);
		sockaddr.sin_port = htons(port);
		if (::bind(udpsock, (struct sockaddr*)&sockaddr, sizeof(struct sockaddr)) < 0)
		{
			return false;
		}

		return true;
	}

	bool connect(const char *addr, unsigned short int port)
	{
		if (!this->bind("0.0.0.0", 0))
		{
			return false;
		}

		memset(&sockaddr, 0, sizeof(sockaddr));
		sockaddr.sin_family = AF_INET;
		sockaddr.sin_addr.S_un.S_addr = inet_addr(addr);
		sockaddr.sin_port = htons(port);

		return true;
	}

	void close()
	{
		closesocket(udpsock);
	}

	int recvfrom(char * buf, int len, struct sockaddr * from, int * fromlen)
	{
		return ::recvfrom(udpsock, buf, len, 0, from, fromlen);
	}

	int sendto(const char  * buf, int len)
	{
		return ::sendto(udpsock, buf, len, 0, (const struct sockaddr  *)&sockaddr, sizeof(sockaddr));
	}

	SOCKET getsocket()
	{
		return udpsock;
	}

private:
	SOCKET udpsock;
	struct sockaddr_in sockaddr;
};


#endif