#ifndef __UDPTASK_H__
#define __UDPTASK_H__

#include <stdio.h>
#include "ikcp.h"
#include "common.h"
#include "udpsocket.h"

class udptask
{
public:
	udptask()
	{
		conv = 0;
		kcp = NULL;
		nexttime = 0;
		memset(buffer, 0, sizeof(buffer));
	}

	~udptask()
	{
		if (kcp != NULL)
		{
			ikcp_flush(kcp);
			ikcp_release(kcp);
		}
		printf("关闭连接 %d\n", conv);
	}

	bool init(IUINT32 conv, udpsocket *udpsock,  int mode = 2)
	{
		if (udpsock == NULL)
		{
			return false;
		}
		this->conv = conv;
		this->nexttime = 0;

		kcp = ikcp_create(conv, (void*)udpsock);

		kcp->output = &udptask::udp_output;
		//kcp->logmask = 0xffff;
		//kcp->writelog = &udptask::writelog;

		ikcp_wndsize(kcp, 128, 128);

		switch (mode)
		{
		case 0:
			// 默认模式
			ikcp_nodelay(kcp, 0, 10, 0, 0);
			break;
		case 1:
			// 普通模式，关闭流控等
			ikcp_nodelay(kcp, 0, 10, 0, 1);
			break;
		case 2:
			// 启动快速模式
			// 第二个参数 nodelay-启用以后若干常规加速将启动
			// 第三个参数 interval为内部处理时钟，默认设置为 10ms
			// 第四个参数 resend为快速重传指标，设置为2
			// 第五个参数 为是否禁用常规流控，这里禁止
			//ikcp_nodelay(kcp, 0, 10, 0, 0);
			//ikcp_nodelay(kcp, 0, 10, 0, 1);
			//ikcp_nodelay(kcp, 1, 10, 2, 1);
			ikcp_nodelay(kcp, 1, 5, 1, 1); // 设置成1次ACK跨越直接重传, 这样反应速度会更快. 内部时钟5毫秒.

			kcp->rx_minrto = 10;
			kcp->fastresend = 1;
			break;
		default:
			printf("%d,%d 模式错误!\n", conv,mode);
		}

		printf("新建连接 %d\n", conv);
		return true;
	}

	int recv(const char  * buf, int len)
	{
		int nret = ikcp_input(kcp, buf, len);
		if (nret == 0)
		{
			nexttime = iclock();
		}
		return nret;
	}

	int send(const char  * buf, int len)
	{
		int nret = ikcp_send(kcp, buf, len);
		if (nret == 0)
		{
			nexttime = iclock();
		}
		printf("发送数据 %d,%s\n", conv, buf);
		return nret;
	}

	void timerloop()
	{
		IUINT32 current = iclock();

		if (nexttime > current)
		{
			return;
		}

		nexttime = ikcp_check(kcp, current);
		if (nexttime != current)
		{
			return;
		}

		ikcp_update(kcp, current);

		while (true) {
			int nrecv = ikcp_recv(kcp, buffer, sizeof(buffer));
			if (nrecv < 0)
			{
				if (nrecv == -3)
				{
					printf("buffer太小 %d,%d\n", conv, sizeof(buffer));
				}
				break;
			}
			parsemsg(buffer, nrecv);
		}
	}

public:
	virtual int parsemsg(const char *buf, int len) = 0;

private:
	static int udp_output(const char *buf, int len, ikcpcb *kcp, void *user)
	{
		return ((udpsocket*)user)->sendto(buf, len);
	}

	static void writelog(const char *log, struct IKCPCB *kcp, void *user)
	{
		printf("%s\n", log);
	}

private:
	IUINT32 conv;
	ikcpcb *kcp;
	IUINT32 nexttime;
	char buffer[10240];
};

#endif