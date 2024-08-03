#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char const *argv[])
{
	if(argc!=2)
	{
		printf("Usage: ./demo port\n");
		return -1;
	}
	// 1.创建套接字      // ipv4   tcp协议   扩展协议
	int tcp_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(tcp_fd == -1)
	{
		perror("socket error\n");
		return -1;
	}
	printf("tcp_fd=%d\n", tcp_fd);
	// 2.准备自身的网络地址数据
	struct sockaddr_in server_addr; // 定义地址结构体
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET; // ipv4地址族
	server_addr.sin_port = htons(atoi(argv[1])); // 端口，小端转大端
	// 方法三：使用特殊的宏
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	// 地址可重用
	int on = 1; // 写一，表示启用
	setsockopt(tcp_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	// 3.将套接字和地址绑定            地址结构体      结构体大小
	bind(tcp_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
	
	// 4.设置监听
	listen(tcp_fd, 1);

	// 5.等待客户端连接
	printf("正在等待客户端的连接\n");
	int con_fd; // 该套接字为服务端给客户端分配的新套接字，用于同客户端通信
	// 定义结构体变量，用于存储客户端的地址数据
	struct sockaddr_in client_addr;
	socklen_t len = sizeof(client_addr);
	memset(&client_addr, 0, len);
	con_fd = accept(tcp_fd, (struct sockaddr *)&client_addr, &len);
	if(con_fd == -1)
	{
		perror("accept error\n");
		return -1;
	}
	printf("连接成功con_fd=%d！\n", con_fd);
	printf("当前的客户端ip=%s\n", inet_ntoa(client_addr.sin_addr));
	printf("当前的客户端port=%hu\n", ntohs(client_addr.sin_port));

	// 通信
	/* char buf[1024] = {0};
	int ret = 0;
	while(1)
	{
		memset(buf, 0, 1024);
		ret = read(con_fd, buf, 1024);
		if(ret == 0)
		{
			// 如果发送消息方，ctrl+c结束，则结果为0
			printf("客户端已经退出\n");
			break;
		}
		printf("当前获取的数据=%s\n", buf);
		if(strcmp(buf, "bye") == 0)
			break;
	} */
	char buf[1024] = {0};
	while(1)
	{
		memset(buf, 0, 1024);
		printf("请输入准备发送的数据\n");
		scanf("%s", buf);
		write(con_fd, buf, strlen(buf));
		if(strcmp(buf, "bye") == 0)
			break;
	}
	// 关闭套接字
	close(tcp_fd); // 关闭服务端本身的套接字
	close(con_fd); // 关闭给客户端分配的套接字

	return 0;
}
