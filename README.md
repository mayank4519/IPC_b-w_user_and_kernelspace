# IPC_b-w_user_and_kernelspace
1.This repo gives an understanding of user and kernel space.
2.How to creare/remove/insert linux  kernel modules in kernel.
3.Understanding of netlink sockets.

Basics:
1. Device drivers interact with hardware layer(CPU, in-built memory,devices) and operating system(which resides in kernel space).
These device drivers are linux kernel modules which after compilation are inserted into kernel. But all LKMs are not 
For example, there is a seperate code for mouse hardware and other hardwares.

NETLINK sockets are used for communicating b/w user and kernel space.
Other std IPCs are used for communication b/w multiple applications resides in user space.

Application in user space communicates with h/w devices using device drivers.

-------NETLINK SOCKETS----------
Other techniques can also be user for US<-->KS communication, but they are no invented for this purposes.
Eg : IOCTL, SYSTEM CALLS, DEVICE FILES
To use SYSTEM CALLS, one need to modify the kernel code, compile and re-built.
It's generally not a good idea to modify kernel code just to support one requirement of an application because system calls are general purpose calls 
used by all other process/application runnung on a linux platform.
Device files are invented specifically to write device drivers.
IOCTLs are also used for communication b/w userspace and kernel space.

Use case of Unified sockets:
2 application in the same system interact with eac other using unix domain sockets.
2 application in the different system interact with each other using network sockets.
Application can interact with various kernel subsystem like Routing table, ARP table, IP table, etc. using netlink sockets.

Project to develop:
Communication b/w user space application and Routing table mgr kernel subsystem.

USERSPACE APPLICATION ACTION:
Perform CRUD operations.

KERNELSPACE ACTION:
1. Write LKM which behave as a routing table manager residing in a kernel space.
2. Process CRUD orders/msgs sent from USA.

Same NL communication semantics used for all other kernel module subsystem.

----------------------------------------------------

Netlink msg format:
|NL MSG HDR | PADDING | PAYLOAD
  16B
  
NL MSG HDR:
struct nlmsghdr {
  u32 nlmsg_len; // Total len = NL MSG HDR + PADDING + PAYLOAD
  u32 nlmsg_type;
  u32 nlmsg_flag;
  u32 nlmsg_seq;
  u32 nlmsg_pid;
};

Both parties can exchange multiple NL msgs cascaded one after the other like below:
|NL MSG HDR | PADDING | PAYLOAD|NL MSG HDR | PADDING | PAYLOAD

Design:
In user space, we will have a multi threaded environment where one thread sends the netlink msg to kernel while the other thread recieves the NL msg from kernelspace.

Steps involved while creating kinux kernel module:
1. Register init and cleanup functions.
2. Intialize struct netlink_kernel_cfg
3. netlink socket creation
4. netlink socket destruction
5. recieve user space msg
6. process user space msg
7. reply to user space msg

Netlink protocol number:
Each kernel subsytem has a unique ID called ntelink protocol number.
User has to assign the protoocol nnumber corresponds to linux kernel subsystem while sending the msg.

NL protocol numbers can be verified in linux/netlink.h
We will be assigning a new protocl number 31 for our project.
