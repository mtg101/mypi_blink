// need to make #include "pico/cyw43_arch.h" stop being red underlined...
#ifndef _LWIPOPTS_H
#define _LWIPOPTS_H

#define NO_SYS                      1
#define LWIP_SOCKET                 0
#define LWIP_NETCONN                0
#define LWIP_NETIF_HOSTNAME         1
#define LWIP_DHCP                   1
#define LWIP_STATS                  0
#define TCP_MSS                     1460
#define TCP_WND                     (8 * TCP_MSS)

#endif