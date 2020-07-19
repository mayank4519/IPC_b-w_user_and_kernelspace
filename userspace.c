#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>
#include <memory.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include "kernelUtils.h"

typedef struct thread_arg_s{

	int sock_fd;
}thread_arg_t;

int
send_netlink_msg_to_kernel(int sock_fd,
			char *msg,
			uint32_t msg_size,
			int nlmsg_type,
			uint16_t flags);

int
send_netlink_msg_to_kernel(int sock_fd,
			char *msg,
			uint32_t msg_size,
			int nlmsg_type,
			uint16_t flags) {

	struct sockaddr_nl dest_addr;
	memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.nl_family = AF_NETLINK;
	dest_addr.nl_pid = 0;

	struct nlmsghdr *nlh = (struct nlmsghdr *)calloc(1,
			NLMSG_HDRLEN + NLMSG_SPACE(MAX_PAYLOAD));

	nlh->nlmsg_len   = NLMSG_HDRLEN + NLMSG_SPACE(MAX_PAYLOAD);
	nlh->nlmsg_pid   = getpid();
	nlh->nlmsg_flags = flags;
	nlh->nlmsg_type  = nlmsg_type;
	nlh->nlmsg_seq   = 0;

	strncpy(NLMSG_DATA(nlh), msg, msg_size);

	struct iovec iov;
	iov.iov_base = (void*)nlh;
	iov.iov_len = nlh->nlmsg_len;

	static struct msghdr outermsghdr;
	memset(&outermsghdr, 0, sizeof(struct msghdr));
	outermsghdr.msg_name = (void *)&dest_addr;
	outermsghdr.msg_namelen = sizeof(dest_addr);
	outermsghdr.msg_iov = &iov;
	outermsghdr.msg_iovlen = 1;

	int rc = sendmsg(sock_fd, &outermsghdr, 0);
	if (rc < 0) {
		printf("Msg sending failed, errno : %d\n", errno);
	}
	return rc;
}

static void*
_kernel_data_reciever_thread(void *arg) {

	thread_arg_t *thread_arg = (thread_arg_t *)arg;
	int rc, sock_fd;
	static struct msghdr outermsghdr;
	struct iovec iov;
	struct nlmsghdr *nlh_recv = NULL;
	sock_fd = thread_arg->sock_fd;

	nlh_recv = (struct nlmsghdr *)calloc(1,
			NLMSG_HDRLEN + NLMSG_SPACE(MAX_PAYLOAD));

	do {	
		memset(&nlh_recv, 0, NLMSG_HDRLEN + NLMSG_SPACE(MAX_PAYLOAD));
		memset(&outermsghdr, 0, sizeof(struct msghdr));
	
		iov.iov_base = (void *)nlh_recv;
		iov.iov_len  =  NLMSG_HDRLEN + NLMSG_SPACE(MAX_PAYLOAD);

		outermsghdr.msg_name = NULL;
		outermsghdr.msg_namelen = 0;
		outermsghdr.msg_iov = &iov;
		outermsghdr.msg_iovlen = 1;

		rc = recvmsg(sock_fd, &outermsghdr, 0);

		if (rc < 0) {
			printf("No data recieved from kernel on socket =%u\n",
				sock_fd);
			continue;
		}

		nlh_recv = outermsghdr.msg_iov->iov_base;
		char *payload = NLMSG_DATA(nlh_recv);

		printf("Netlink msg recieved from kernel, bytes recieved =%d\n", rc);

		printf("msg recieved from kernel =%s\n", payload);
	}while(1);
}

void
kernel_data_reciever_thread(thread_arg_t *arg) {

	pthread_t rcv_pkt_tid;
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&rcv_pkt_tid, &attr, 
			_kernel_data_reciever_thread, (void *)arg);
		
}

static void
greet_kernel(int sock_fd, char* msg, uint32_t msg_len) {

    send_netlink_msg_to_kernel(sock_fd, msg, msg_len, NLMSG_GREET, NLM_F_ACK);
}

/*Create a netlink socket
 * bind the socket.
 * start kernel_data_reciever_thread which recieves msgs from kernel.
 * Provide user menu to send a msg or exit
 * If user press send, call send_netlink_msg_to_kernel() fn 
*/
int main(int argc, char** argv) {

	struct sockaddr_nl src_addr;
	int choice, sock_fd;

	sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_TEST_PROTOCOL);
	
	if(sock_fd == -1) {
		printf("Socket creation failed, errno: %d\n", errno);
	 	exit(EXIT_FAILURE);
 	}
	printf("Socket =%u created successfully\n", sock_fd); 
	memset(&src_addr, 0, sizeof(src_addr));
	src_addr.nl_family = AF_NETLINK;
	src_addr.nl_pid = getpid();

	if(bind(sock_fd, (struct sockaddr*)&src_addr,
		       	sizeof(src_addr)) < 0) {
		printf("Bind failed\n");
		exit(EXIT_FAILURE);
	}

	thread_arg_t thread_arg;
	thread_arg.sock_fd = sock_fd;

	kernel_data_reciever_thread(&thread_arg);	

	while(1) {

		printf("1. Greet kernel\n");
		printf("2. Exit\n");
		printf("Enter user input\n");
		scanf("%d\n", &choice);

		switch(choice) {
			case 1:
			{	
			    char user_msg[MAX_PAYLOAD] = {0};
			    if(fgets((char*)user_msg, MAX_PAYLOAD, stdin) == NULL) {
			        printf("Error reading from console\n");
				exit(EXIT_FAILURE);
			    }
			    greet_kernel(sock_fd, user_msg, strlen(user_msg));
			}
			break;
			case 2:
				close(sock_fd);
			break;
			default:
				;
		}
	}
	return 0;
}
