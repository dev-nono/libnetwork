/*
 * net_envent.h
 *
 *  Created on: Jun 15, 2017
 *      Author: hico
 */

#ifndef NET_ENVENT_H_
#define NET_ENVENT_H_


#include <netinet/in.h>
#include <net/if.h>
#include <sys/queue.h>

/* structure and list for storing lan addresses
 * with ascii representation and mask */
struct lan_addr_s {
   char ifname[IFNAMSIZ];  /* example: eth0 */
   unsigned int index;     /* use if_nametoindex() */
   char str[16];  /* example: 192.168.0.1 */
   struct in_addr addr, mask; /* ip/mask */
#ifdef MULTIPLE_EXTERNAL_IP
   char ext_ip_str[16];
   struct in_addr ext_ip_addr;
#endif
   //LIST_ENTRY(lan_addr_s) list;
};
LIST_HEAD(lan_addr_list, lan_addr_s);


int LoopEvent(int argc, char *argv[]);


void  SetArray();
char* getRTM_Type(int a_Type);
char* getIFA_Type(int a_Type);

int   read_event (int sockint);

void PrintMessageFlag(struct nlmsghdr * a_pNlmsghdr);
void PrintMessageType(struct nlmsghdr * a_a_pNlmsghdr);
void PrintIfinfomsg(struct nlmsghdr * a_pNlmsghdr);

void RTM_NEWADDR_Check(struct nlmsghdr * a_pNlmsghdr);
void RTM_DELADDR_Check(struct nlmsghdr * a_pNlmsghdr);
void RTM_GETADDR_Check(struct nlmsghdr * a_pNlmsghdr);

void RTM_NEWLINK_Check(struct nlmsghdr * a_pNlmsghdr);
void RTM_DELLINK_Check(struct nlmsghdr * a_pNlmsghdr);
void RTM_GETLINK_Check(struct nlmsghdr * a_pNlmsghdr);
void RTM_SETLINK_Check(struct nlmsghdr * a_pNlmsghdr);

int ip_add();
int ip_del();
int ip_show();
int gateway_add();
int gateway_del();
int gateway_show();


#endif /* NET_ENVENT_H_ */
