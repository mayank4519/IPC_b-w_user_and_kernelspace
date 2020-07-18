#include <linux/module.h>
#include <linux/netlink.h>
#include <linux/string.h>
#include <net/sock.h>
#include "kernelUtils.h"

static struct sock *nl_sock = NULL;

static void netlink_recv_msg_fn(struct sk_buff *skb_in) {

	printk(KERN_INFO "%s() fn invoked", __FUNCTION__);
	struct nlmsghdr *nlh_recv, *nlh_reply;
	int us_process_port_id;
	char *user_space_data,  kernel_reply[256];
	struct sk_buff *skb_out;
	int res;

	nlh_recv = (struct nlmsghdr*)(skb_in->data);

	nlmsg_dump(nlh_recv);

	us_process_port_id = nlh_recv->nlmsg_pid;

	printk(KERN_INFO "%s(%d): port id of the sending process: %u",
		       	__FUNCTION__, __LINE__, us_process_port_id);

	user_space_data = (char*)nlmsg_data(nlh_recv);

	user_space_data_len = skb_in->len;

	printk(KERN_INFO "%s(%d): msg recvd from user space : %s 
		skb_in->len : %u nlh_recv->len : %u",__FUNCTION__, __LINE__, 
		user_space_data, user_space_data_len, nlh_recv->nlmsg_len);

	if(nlh_recv->nlmsg_flag & NLM_F_ACK) {
		//Sending reply back to user space.
		memset(kernel_reply, 0, sizeof(kernel_reply));
		snprintf(kernel_reply, sizeof(kernel_reply),
		  "Message from process %u is processed by Kernel", us_process_port_id);

		//Allocate memory to sk_buf using std linux functions.
		skb_out = nlmsg_new(sizeof(kernel_reply), 0);

		//Populate the nlmsg_reply using std linux functions.
		nlh_reply = nlmsg_put(skb_out,
				0,/*Sender is kernel. Hence port ID = 0*/
				nlh_recv->nlmsg_seq,/*Same seq number*/
				NLMSG_DONE,/*nl hdr msg type*/
				sizof(kernel_reply),
				0);

		strncpy(nlmsg_data(nlh_reply), kernel_reply, sizeof(kernel_reply));
		res = nlmsg_unicast(nl_sock, skb_out, us_process_port_id);
		if(res < 0) {
			printk(KERN_INFO "Error sending a reply to userspace\n");
			kfree_skb(skb_out);
		}
	}
}

static struct netlink_kernel_cfg cfg = {

	.input = netlink_recv_msg_fn,
};

static int __init NetlinkGreetings_init(void) {

  printk(KERN_INFO "Hello Kernel, I'm kernel module NetlinkGreetingsLKM.ko\n");
  nl_sock = netlink_kernel_create(&inet_net, NETLINK_TEST_PROTOCOL, &cfg);
  if(!nl_sock) {
	  printk(KERN_INFO "Kernel netlink socket for NL protocol %u failed.\n", NETLINK_TEST_PROTOCOL);
	  return -ENOMEM;
  }
  printk(KERN_INFO "Netlink socket created successfully\n");
  return 0;
}

static void __exit NetlinkGreetings_exit(void) {
	
  printk(KERN_INFO "Hello Kernel, Exiting kernel module NetlinkGreetingsLKM.ko\n");
  netlink_kernel_release(nl_sock);
  nl_sock = NULL;
}

static void nlmsg_dump(struct nlmsghdr* nlh) {

  printk(KERN_INFO "Netlink msg header details\n");  
  printk(KERN_INFO "nl msg type: %s\n", netlink_get_msg_type(nlh->nlmsg_type));  
  printk(KERN_INFO "nl msg len : %u\n", nlh->nlmsg_len);  
  printk(KERN_INFO "nl msg flag: %u\n", nlh->nlmsg_flag);  
  printk(KERN_INFO "nl msg seq : %u\n", nlh->nlmsg_seq);  
  printk(KERN_INFO "nl msg pid : %u\n", nlh->nlmsg_pid);  
}

module_init(NetlinkGreetings_init);
module_exit(NetlinkGreetings_exit);

MODULE_LICENSE("GPL");
