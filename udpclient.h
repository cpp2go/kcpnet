#ifndef __UDPCLIENT_H__
#define __UDPCLIENT_H__

#include <winsock2.h> 
#include <stdio.h>
#include <thread>
#include <mutex>
#include "udptask.h"

template<typename T>
class udpclient 
{
public:
	udpclient()	{}
	bool connect(const char *addr, unsigned short int port)
	{
		if (!udpsock.connect(addr, port))
		{
			return false;
		}

		utask = new T;
		if (!utask->init(123456, &udpsock))
		{
			return false;
		}

		isstop = false;

		_thread = std::thread(std::bind(&udpclient::run, this));
		_threadtm = std::thread(std::bind(&udpclient::loop, this));
		return true;
	}

	int send(const char *buf, int size)
	{
		_mutex.lock();
		int ret = utask->send(buf, size);
		_mutex.unlock();
		return ret;
	}

	void close()
	{
		isstop = true;
		udpsock.close();
		_thread.join();
		_threadtm.join();
		delete utask;
	}

	void loop()
	{
		for (; !isstop;)
		{
			_mutex.lock();
			utask->timerloop();
			_mutex.unlock();
			std::chrono::milliseconds dura(1);
			std::this_thread::sleep_for(dura);
		}
	}

	void run()
	{
		char buff[1024] = { 0 };
		struct sockaddr_in seraddr;
		for (; !isstop;)
		{
			int len = sizeof(struct sockaddr_in);
			int size = udpsock.recvfrom(buff, sizeof(buff), (struct sockaddr*)&seraddr, &len);
			if (size == 0)
			{
				continue;
			}
			if (size < 0)
			{
				printf("接收失败 %d \n", udpsock);
				break;
			}
			_mutex.lock();
			utask->recv(buff, size);
			_mutex.unlock();
		}
	}

	virtual int parsemsg(const char *buf, int len)
	{
		printf("收到数据 %s,%d\n", buf, len);
		return 0;
	}
private:
	udpsocket udpsock;
	udptask* utask;
	std::thread _thread;
	std::thread _threadtm;
	std::mutex _mutex;
	volatile bool isstop;
};

#endif