#include <asm/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <net/if.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>


#include "net_envent.h"

char gArray_RTM_Type [RTM_MAX][128];
char gArray_IFA_Type [__IFA_MAX][128];


void net_event_show()
{
   ip_add();
   ip_show();

   ip_del();
   ip_show();

   gateway_add();
   ip_show();

   gateway_del();
   ip_show();

   gateway_show();
   ip_show();


}
//******************************************************
//*
//******************************************************
int main (int argc, char *argv[])
{
   int vRetCode = 0;

   net_event_show();

   //vRetCode = LoopEvent(argc,argv);

   return vRetCode ;

}
int LoopEvent (int argc, char *argv[])
{
   fd_set rfds, wfds;
   struct timeval tv;
   int retval;
   struct sockaddr_nl addr;

   SetArray();

   int nl_socket = socket (AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
   if (nl_socket < 0)
   {
      printf ("Socket Open Error!");
      exit (1);
   }

   memset ((void *) &addr, 0, sizeof (addr));

   addr.nl_family = AF_NETLINK;
   addr.nl_pid = getpid ();
   addr.nl_groups = RTMGRP_LINK | RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR;
   //  addr.nl_groups = RTMGRP_LINK;

   if (bind (nl_socket, (struct sockaddr *) &addr, sizeof (addr)) < 0)
   {
      printf ("Socket bind failed!");
      exit (1);
   }

   while (1)
   {
      FD_ZERO (&rfds);
      FD_CLR (nl_socket, &rfds);
      FD_SET (nl_socket, &rfds);

      tv.tv_sec = 10;
      tv.tv_usec = 0;

      retval = select (FD_SETSIZE, &rfds, NULL, NULL, &tv);
      if (retval == -1)
         printf ("Error select() \n");
      else if (retval)
      {
         //printf ("Event recieved >> \n");
         read_event (nl_socket);
      }
      //else          printf ("## Select TimedOut ## \n");
   }
   return 0;
}
//******************************************************
//*
//******************************************************
void SetArray()
{

   strcpy(gArray_RTM_Type[RTM_BASE],"RTM_BASE");
   strcpy(gArray_RTM_Type[RTM_NEWLINK],"RTM_NEWLINK");

   strcpy(gArray_RTM_Type[RTM_DELLINK],"RTM_DELLINK");
   strcpy(gArray_RTM_Type[RTM_GETLINK],"RTM_GETLINK");

   strcpy(gArray_RTM_Type[RTM_SETLINK],"RTM_SETLINK");

   strcpy(gArray_RTM_Type[RTM_NEWADDR],"RTM_NEWADDR");
   strcpy(gArray_RTM_Type[RTM_DELADDR],"RTM_DELADDR");
   strcpy(gArray_RTM_Type[RTM_GETADDR],"RTM_GETADDR");

   strcpy(gArray_RTM_Type[RTM_NEWROUTE],"RTM_GETADDR");
   strcpy(gArray_RTM_Type[RTM_DELROUTE],"RTM_DELROUTE");
   strcpy(gArray_RTM_Type[RTM_GETROUTE],"RTM_GETROUTE");

   strcpy(gArray_RTM_Type[RTM_NEWNEIGH],"RTM_NEWNEIGH");
   strcpy(gArray_RTM_Type[RTM_DELNEIGH],"RTM_DELNEIGH");
   strcpy(gArray_RTM_Type[RTM_GETNEIGH],"RTM_GETNEIGH");

   strcpy(gArray_RTM_Type[RTM_NEWPREFIX],"RTM_NEWPREFIX");

   strcpy(gArray_RTM_Type[RTM_NEWNETCONF],"RTM_NEWNETCONF");
   strcpy(gArray_RTM_Type[RTM_GETNETCONF],"RTM_GETNETCONF");



strcpy(gArray_IFA_Type[IFA_UNSPEC],    "IFA_UNSPEC");
strcpy(gArray_IFA_Type[IFA_ADDRESS],   "IFA_ADDRESS");
strcpy(gArray_IFA_Type[IFA_LOCAL],     "IFA_LOCAL");
strcpy(gArray_IFA_Type[IFA_LABEL],     "IFA_LABEL");
strcpy(gArray_IFA_Type[IFA_BROADCAST], "IFA_BROADCAST");
strcpy(gArray_IFA_Type[IFA_ANYCAST],   "IFA_ANYCAST");
strcpy(gArray_IFA_Type[IFA_CACHEINFO], "IFA_CACHEINFO");
strcpy(gArray_IFA_Type[IFA_MULTICAST], "IFA_MULTICAST");
strcpy(gArray_IFA_Type[IFA_FLAGS],     "IFA_FLAGS");


}
//****************************************
//*
//****************************************
char* getRTM_Type(int a_Type)
{
   int    ii         = 0;
   char* pRetvalue   = 0;

   if( ii < RTM_MAX )
   {
      pRetvalue = gArray_RTM_Type[ii];
   }

   return pRetvalue;

}
//****************************************
//*
//****************************************
char* getIFA_Type(int a_Type)
{
   char* pRetvalue = 0;


   if( a_Type < __IFA_MAX )
   {
      pRetvalue = gArray_IFA_Type[a_Type];
   }

   return pRetvalue;
}

//******************************************************
//*
//******************************************************
int read_event (int sockint)
{
   int                  status = 0;
   int                  ret = 0;
   char                 vBuf[4096];
   struct iovec         iov = { vBuf, sizeof vBuf };
   struct sockaddr_nl   snl;
   struct msghdr        msg = { (void *) &snl, sizeof snl, &iov, 1, NULL, 0, 0 };
   struct nlmsghdr *    pNlmsghdr;
   struct ifinfomsg *   pIfinfomsg;
   char                 vStrMsg[1024];

/*
   printf ("\nread_event: sizeof(struct nlmsghdr)== %d\n",
           (int)sizeof(struct nlmsghdr));

   printf ("read_event: sizeof(struct ifinfomsg)== %d\n",
           (int)sizeof(struct ifinfomsg));
   printf ("read_event: sizeof(struct rtattr )== %d\n",
           (int)sizeof(struct rtattr ));
*/
   status = recvmsg (sockint, &msg, 0);

   printf ("read_event   >>>>>>>>>>>>>>>>>>>>>>\n");
   printf ("read_event: recvmsg() == %d\n", status);

   if (status < 0)
   {
      /* Socket non-blocking so bail out once we have read everything */
      if (errno == EWOULDBLOCK || errno == EAGAIN)
         return ret;

      /* Anything else is an error */
      printf ("read_event: Error recvmsg: %d\n", status);
      perror ("read_event: Error: ");
      return status;
   }

   if (status == 0)
   {
      printf ("read_event: EOF\n");
   }

   pNlmsghdr = (struct nlmsghdr *) vBuf;

   int vPayLoad = 0;
   vPayLoad= NLMSG_PAYLOAD(pNlmsghdr,0);

   printf ("read_event : nlmsg_len = %d nlmsg_flags=%#x nlmsg_pid=%d nlmsg_seq=%d nlmsg_type= %d-%s NLMSG_PAYLOAD=%d\n \n",
           (int)pNlmsghdr->nlmsg_len,
           (int)pNlmsghdr->nlmsg_flags,
           (int)pNlmsghdr->nlmsg_pid,
           (int)pNlmsghdr->nlmsg_seq,
           (int)pNlmsghdr->nlmsg_type , getRTM_Type(pNlmsghdr->nlmsg_type),
           vPayLoad);

   struct rtattr *pRta = 0;
   char* pChar=0;


   // We need to handle more than one message per 'recvmsg'
   for (    pNlmsghdr = (struct nlmsghdr *) vBuf;
            NLMSG_OK (pNlmsghdr,(unsigned int) status);
            pNlmsghdr = NLMSG_NEXT (pNlmsghdr, status))
   {
      printf ("read_event   >>>>>>>>>>>>>>>>>>>>>>\n");

      //Finish reading
      if (pNlmsghdr->nlmsg_type == NLMSG_DONE)
      {
         return ret;
      }
      // Message is some kind of error
      else if (pNlmsghdr->nlmsg_type == NLMSG_ERROR)
      {
         printf ("read_event: Message is an error - decode TBD\n");
         return -1;        // Error
      }
      else if (pNlmsghdr->nlmsg_type == RTM_NEWADDR )
      {
         RTM_NEWADDR_Check(pNlmsghdr);
      }
      else if (pNlmsghdr->nlmsg_type == RTM_DELADDR )
      {
         RTM_DELADDR_Check(pNlmsghdr);
      }
      else if (pNlmsghdr->nlmsg_type == RTM_GETADDR )
      {
         RTM_GETADDR_Check(pNlmsghdr);
      }
      else if (pNlmsghdr->nlmsg_type == RTM_NEWLINK )
      {
         RTM_NEWLINK_Check(pNlmsghdr);
      }
      else if (pNlmsghdr->nlmsg_type == RTM_DELLINK )
      {
         RTM_DELLINK_Check(pNlmsghdr);
      }
      else if (pNlmsghdr->nlmsg_type == RTM_GETLINK )
      {
         RTM_GETLINK_Check(pNlmsghdr);
      }
      else if (pNlmsghdr->nlmsg_type == RTM_SETLINK )
      {
         RTM_SETLINK_Check(pNlmsghdr);
      }
      else
      {
         printf("\n read_event: else  %d,%s",pNlmsghdr->nlmsg_type,getRTM_Type(pNlmsghdr->nlmsg_type));
      }


      /*
      //PrintMessageType(pNlmsghdr);
      PrintMessageFlag(pNlmsghdr);
      PrintIfinfomsg(pNlmsghdr);


      for( pRta = NLMSG_DATA(pNlmsghdr);
            ((char*)pRta) < vBuf + 4096;
            )
      {
//         printf("pRta->rta_len = %d pRta->rta_type=%d \n",
//                (int)pRta->rta_len,(int)pRta->rta_type);

         if(      ( ( pRta->rta_len == 0) && (pRta->rta_type > 0 ))
               || ( ( pRta->rta_len > 0 ) && ( pRta->rta_len < 20 ) )
           )
         {
            //snprintf(vStrMsg,pRta->rta_len,"%s",                     (char*)(pRta + sizeof(struct rtattr)));

            memcpy(  vStrMsg,
                     (char*)(pRta + sizeof(struct rtattr)),
                     pRta->rta_len);

              vStrMsg[pRta->rta_len] = 0;

              printf("pRta->rta_len = %d pRta->rta_type=%d \n=>%s.\n",
                     (int)pRta->rta_len,(int)pRta->rta_type,vStrMsg);

              printf("%s\n",vStrMsg);
         }
         pChar = (char*)pRta;
         pRta =   (struct rtattr *) ( pChar + sizeof(struct rtattr)+pRta->rta_len);
      }
*/

      printf ("read_event   <<<<<<<<<<<<<<<<<<<<<<<\n");
   }

   fflush(0);

   printf ("read_event   <<<<<<<<<<<<<<<<<<<<\n");

   return ret;
}
//******************************************************
//*
//******************************************************
void PrintMessageType(struct nlmsghdr * a_pNlmsghdr)
{

   if(   (a_pNlmsghdr->nlmsg_type == RTM_NEWLINK)
      || (a_pNlmsghdr->nlmsg_type == RTM_DELLINK)

      || (a_pNlmsghdr->nlmsg_type == RTM_BASE)

      || (a_pNlmsghdr->nlmsg_type == RTM_GETLINK)
      || (a_pNlmsghdr->nlmsg_type == RTM_SETLINK)

      || (a_pNlmsghdr->nlmsg_type == RTM_NEWADDR)
      || (a_pNlmsghdr->nlmsg_type == RTM_DELADDR)
      || (a_pNlmsghdr->nlmsg_type == RTM_GETADDR)

      || (a_pNlmsghdr->nlmsg_type == RTM_NEWROUTE)
      || (a_pNlmsghdr->nlmsg_type == RTM_DELROUTE)
      || (a_pNlmsghdr->nlmsg_type == RTM_GETROUTE)

      || (a_pNlmsghdr->nlmsg_type == RTM_NEWNEIGH)
      || (a_pNlmsghdr->nlmsg_type == RTM_DELNEIGH)
      || (a_pNlmsghdr->nlmsg_type == RTM_GETNEIGH)

      || (a_pNlmsghdr->nlmsg_type == RTM_NEWPREFIX)

      || (a_pNlmsghdr->nlmsg_type == RTM_NEWNETCONF)
      || (a_pNlmsghdr->nlmsg_type == RTM_GETNETCONF)
            )
   {
      printf ("\nnlmsg_type %s = %d\n",
              getRTM_Type(a_pNlmsghdr->nlmsg_type),
              a_pNlmsghdr->nlmsg_type);
   }

}
//******************************************************
//*
//******************************************************
void PrintMessageFlag(struct nlmsghdr * a_pNlmsghdr)
{

   struct ifinfomsg *  pIfinfomsg = NLMSG_DATA (a_pNlmsghdr);

//   printf ("\n\n");
   if(pIfinfomsg->ifi_flags & IFF_UP)            printf ("%-20s -  %d \n",    "IFF_UP",           (int)(pIfinfomsg->ifi_flags & IFF_UP));
   if(pIfinfomsg->ifi_flags & IFF_RUNNING)       printf ("%-20s -  %d \n",    "IFF_RUNNING",      (int)(pIfinfomsg->ifi_flags & IFF_RUNNING));
   if(pIfinfomsg->ifi_flags & IFF_BROADCAST)     printf ("%-20s -  %d \n",    "IFF_BROADCAST",    (int)(pIfinfomsg->ifi_flags & IFF_BROADCAST));
   if(pIfinfomsg->ifi_flags & IFF_DEBUG)         printf ("%-20s -  %d \n",    "IFF_DEBUG",        (int)(pIfinfomsg->ifi_flags & IFF_DEBUG));
   if(pIfinfomsg->ifi_flags & IFF_LOOPBACK)      printf ("%-20s -  %d \n",    "IFF_LOOPBACK",     (int)(pIfinfomsg->ifi_flags & IFF_LOOPBACK));
   if(pIfinfomsg->ifi_flags & IFF_POINTOPOINT)   printf ("%-20s -  %d \n",    "IFF_POINTOPOINT",  (int)(pIfinfomsg->ifi_flags & IFF_POINTOPOINT));
   if(pIfinfomsg->ifi_flags & IFF_NOTRAILERS)    printf ("%-20s -  %d \n",    "IFF_NOTRAILERS",   (int)(pIfinfomsg->ifi_flags & IFF_NOTRAILERS));
   if(pIfinfomsg->ifi_flags & IFF_NOARP)         printf ("%-20s -  %d \n",    "IFF_NOARP",        (int)(pIfinfomsg->ifi_flags & IFF_NOARP));
   if(pIfinfomsg->ifi_flags & IFF_PROMISC)       printf ("%-20s -  %d \n",    "IFF_PROMISC",      (int)(pIfinfomsg->ifi_flags & IFF_PROMISC));
   if(pIfinfomsg->ifi_flags & IFF_ALLMULTI)      printf ("%-20s -  %d \n",    "IFF_ALLMULTI",     (int)(pIfinfomsg->ifi_flags & IFF_ALLMULTI));
   if(pIfinfomsg->ifi_flags & IFF_MASTER)        printf ("%-20s -  %d \n",    "IFF_MASTER",       (int)(pIfinfomsg->ifi_flags & IFF_MASTER));
   if(pIfinfomsg->ifi_flags & IFF_SLAVE)         printf ("%-20s -  %d \n",    "IFF_SLAVE",        (int)(pIfinfomsg->ifi_flags & IFF_SLAVE));
   if(pIfinfomsg->ifi_flags & IFF_MULTICAST)     printf ("%-20s -  %d \n",    "IFF_MULTICAST",    (int)(pIfinfomsg->ifi_flags & IFF_MULTICAST));
   if(pIfinfomsg->ifi_flags & IFF_PORTSEL)       printf ("%-20s -  %d \n",    "IFF_PORTSEL",      (int)(pIfinfomsg->ifi_flags & IFF_PORTSEL));
   if(pIfinfomsg->ifi_flags & IFF_AUTOMEDIA)     printf ("%-20s -  %d \n",    "IFF_AUTOMEDIA",    (int)(pIfinfomsg->ifi_flags & IFF_AUTOMEDIA));
   if(pIfinfomsg->ifi_flags & IFF_DYNAMIC)       printf ("%-20s -  %d \n",    "IFF_DYNAMIC",      (int)(pIfinfomsg->ifi_flags & IFF_DYNAMIC));
}
//******************************************************
//*
//******************************************************
void PrintIfinfomsg(struct nlmsghdr * a_pNlmsghdr)
{
   struct ifinfomsg *  pIfinfomsg = NLMSG_DATA (a_pNlmsghdr);

   printf("\nifi_change = %12d \nifi_family = %12d \nifi_index  = %12d \nifi_type   = %12d \n\n",
      (int)pIfinfomsg->ifi_change,
      (int)pIfinfomsg->ifi_family,
      (int)pIfinfomsg->ifi_index,
      (int)pIfinfomsg->ifi_type);

}
//******************************************************
//*
//******************************************************
void RTM_NEWADDR_Check(struct nlmsghdr * a_pNlmsghdr)
{
   int                        is_del         = 0;
   char                       address[48];
   char                       ifname[IFNAMSIZ];
   struct   ifaddrmsg*        ifa;
   struct   rtattr*           rta;
   int                        ifa_len;
   struct   lan_addr_s *      lan_addr;

   address[0] = '\0';
   ifname[0] = '\0';

   printf("RTM_NEWADDR_Check\n");

   ifa = (struct ifaddrmsg *)NLMSG_DATA(a_pNlmsghdr);
   rta = (struct rtattr *)IFA_RTA(ifa);
   ifa_len = IFA_PAYLOAD(a_pNlmsghdr);
   printf( "RTM_NEWADDR_Check : %s %s index=%d fam=%d prefixlen=%d flags=%d scope=%d\n",
          "ProcessInterfaceWatchNotify", is_del ? "RTM_DELADDR" : "RTM_NEWADDR",
          ifa->ifa_index, ifa->ifa_family, ifa->ifa_prefixlen,
          ifa->ifa_flags, ifa->ifa_scope);


   for(;RTA_OK(rta, ifa_len); rta = RTA_NEXT(rta, ifa_len))
   {
      /*RTA_DATA(rta)*/
      /*rta_type : IFA_ADDRESS, IFA_LOCAL, etc. */
      char tmp[128];
      memset(tmp, 0, sizeof(tmp));
      switch(rta->rta_type) {
      case IFA_ADDRESS:
      case IFA_LOCAL:
      case IFA_BROADCAST:
      case IFA_ANYCAST:
         inet_ntop(ifa->ifa_family, RTA_DATA(rta), tmp, sizeof(tmp));
         //if(rta->rta_type == IFA_ADDRESS)
            strncpy(address, tmp, sizeof(address));
         break;
      case IFA_LABEL:
         strncpy(tmp, RTA_DATA(rta), sizeof(tmp));
         strncpy(ifname, tmp, sizeof(ifname));
         break;
      case IFA_CACHEINFO:
         {
            struct ifa_cacheinfo *cache_info;
            cache_info = RTA_DATA(rta);
            snprintf(tmp, sizeof(tmp), "valid=%u prefered=%u",
                     cache_info->ifa_valid, cache_info->ifa_prefered);
         }
         break;
      default:
         strncpy(tmp, "*unknown*", sizeof(tmp));
      }
      printf("RTM_NEWADDR_Check :  rta_len=%d rta_type=%d-%s '%s'\n",
             rta->rta_len,
             rta->rta_type,
             getIFA_Type(rta->rta_type),
             tmp);
   }
   printf("RTM_NEWADDR_Check : %s: %s/%d %s\n",
          is_del ? "RTM_DELADDR" : "RTM_NEWADDR",
          address, ifa->ifa_prefixlen, ifname);



}
//******************************************************
//*
//******************************************************
void RTM_DELADDR_Check(struct nlmsghdr * a_pNlmsghdr)
{
   printf("RTM_DELADDR_Check\n");
}
//******************************************************
//*
//******************************************************
void RTM_GETADDR_Check(struct nlmsghdr * a_pNlmsghdr)
{
   printf("RTM_GETADDR_Check\n");

}
//******************************************************
//*
//******************************************************
void RTM_NEWLINK_Check(struct nlmsghdr * a_pNlmsghdr)
{
   printf("RTM_NEWLINK_Check\n");
}
//******************************************************
//*
//******************************************************
void RTM_DELLINK_Check(struct nlmsghdr * a_pNlmsghdr)
{
   printf("RTM_DELLINK_Check\n");
}
//******************************************************
//*
//******************************************************
void RTM_GETLINK_Check(struct nlmsghdr * a_pNlmsghdr)
{
   printf("RTM_GETLINK_Check\n");
}
//******************************************************
//*
//******************************************************
void RTM_SETLINK_Check(struct nlmsghdr * a_pNlmsghdr)
{
   printf("RTM_SETLINK_Check\n");
}
