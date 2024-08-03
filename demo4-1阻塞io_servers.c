#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

int count;
// tcp通信，将客户端的地址数据和套接字封装
typedef struct client_info{
	struct sockaddr_in client_addr; // 客户端地址数据结构体
	int con_fd; // 给客户端分配的新套接字
}cli_info;

// 定义结构体数组，保存连接的客户端的数据
cli_info SaveClientData[3];

// 获取监听的套接字
int get_listen_fd(char const *port)
{
	// 1.创建套接字
	int tcp_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(tcp_fd == -1)
	{
		perror("socket error\n");
		exit(0);
	}
	// 2.准备自身的地址结构体
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET; // 地址族
	server_addr.sin_port = htons(atoi(port)); // 端口号
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // ip
	// 3.绑定
	bind(tcp_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
	// 4.监听
	listen(tcp_fd, 1);
	// 5.返回套接字
	return tcp_fd;
}

// 客户端退出
void cli_exit(cli_info *p)
{
	printf("<%s>,[%hu]:退出了\n", inet_ntoa(p->client_addr.sin_addr),
									ntohs(p->client_addr.sin_port));
			// 如果客户端退出，那么需要将客户端的信息从结构体数组中剔除
			// 遍历结构体数组，通过con_fd，找到count
			for(int i=0; i<count; i++)
			{
				if(p->con_fd == SaveClientData[i].con_fd)
				{
					// 覆盖
					for(int j=i; j<count; j++)
					{
						SaveClientData[j] = SaveClientData[j+1];
					}
					break;
				}
			}
			count--;
}

// 线程任务函数
void *func(void *arg)
{
	// 获取参数
	cli_info *p = (cli_info *)arg;
	// 获取信息
	char buf[100] = {0};
	int ret;
	while(1)
	{
		memset(buf, 0, 100);
		// 读取数据
		ret = recv(p->con_fd, buf, sizeof(buf), 0);
		if(ret == 0 || strcmp(buf, "bye") == 0)
		{
			cli_exit(p);
			break;
		}
		// 打印  ip   端口  内容
		printf("<%s>,[%hu]:%s\n", inet_ntoa(p->client_addr.sin_addr),
									ntohs(p->client_addr.sin_port),
									buf);
	}
}

int main(int argc, char const *argv[])
{
	if(argc!=2)
	{
		printf("usage：./demo potr\n");
	}
	// 定义变量，保存客户端的地址数据
	struct sockaddr_in client_addr;
	socklen_t len = sizeof(client_addr);
	int con_fd;//客户端的套接字

	// 获取监听的套接字
	int tcp_fd = get_listen_fd(argv[1]);
	printf("进入等待\n");
	// 循环检测客户端的连接
	while(1)
	{
		// 获取连接服务器的套接字
		con_fd = accept(tcp_fd, (struct sockaddr*)&client_addr, &len);
		if(con_fd == -1)
		{
			perror("连接失败\n");
			return -1;
		}
		if(count == 2)
		{
			send(con_fd, "服务器已满,请稍后重试", 31, 0);
			continue;
		}
		// 连接成功，保存客户端的数据，并开启线程
		SaveClientData[count].client_addr = client_addr; // 存获取的客户端的地址
		SaveClientData[count].con_fd = con_fd; // 存获取的客户端的地址
		// 创建线程
		pthread_t tid;
		pthread_create(&tid, NULL, func, (void *)&SaveClientData[count]);
		count++; // 连接了新用户
	}

	return 0;
}
