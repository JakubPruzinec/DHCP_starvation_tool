#ifndef __IPK_DHCPSTARVE_API__
#define __IPK_DHCPSTARVE_API__

#include "defs.h"

#define MAC_ADDRESS_SIZE    6

#define DHCP_SERVER_PORT    67
#define DHCP_CLIENT_PORT    68

#define BOOTREQUEST         1

#define DHCP_HTYPE_ETH      1
#define DHCP_HLEN_ETH       MAC_ADDRESS_SIZE

/* https://tools.ietf.org/html/rfc2131#page-11 */
#define DHCP_BROADCAST_FLAG (1<<15)

#define DHCP_MAGIC_COOKIE       ("\x63""\x82""\x53""\x63")
#define DHCP_MAGIC_COOKIE_LEN   strlen(DHCP_MAGIC_COOKIE)

/* https://www.ietf.org/rfc/rfc2132.txt section 9.6 */
#define DHCP_MESSAGE_TYPE       53
#define DHCP_MESSAGE_TYPE_LEN   1
#define DHCPDISCOVER            1

/* https://www.ietf.org/rfc/rfc2132.txt 3.2 */
#define DHCP_END_OPTION         255

/* DOCUMENTATION -> Zdroje */
/* "A DHCP client must be prepared to receive DHCP
** messages with an 'options' field of at least
** length 312 octets." -> not really relevant to us,
** but what the heck :^)
*/
#define MAX_DHCP_CHADDR_LENGTH           16
#define MAX_DHCP_SNAME_LENGTH            64
#define MAX_DHCP_FILE_LENGTH             128
#define MAX_DHCP_OPTIONS_LENGTH          312

/* DHCP MESSAGE format */
/* https://tools.ietf.org/html/rfc2131#page-10 */
/* https://www.ietf.org/rfc/rfc2132.txt */
/* https://stackoverflow.com/questions/40642765/how-to-tell-gcc-to-disable-padding-inside-struct */
/* modified declaration of message defined in DOCUMENTATION -> Register - Zdroje */
struct dhcp_message_struct {
        u_int8_t  op;                   /* packet type */
        u_int8_t  htype;                /* type of hardware address for this machine (Ethernet, etc) */
        u_int8_t  hlen;                 /* length of hardware address (of this machine) */
        u_int8_t  hops;                 /* hops */
        u_int32_t xid;                  /* random transaction id number - chosen by this machine */
        u_int16_t secs;                 /* seconds used in timing */
        u_int16_t flags;                /* flags */
        struct in_addr ciaddr;          /* IP address of this machine (if we already have one) */
        struct in_addr yiaddr;          /* IP address of this machine (offered by the DHCP server) */
        struct in_addr siaddr;          /* IP address of DHCP server */
        struct in_addr giaddr;          /* IP address of DHCP relay */
        unsigned char chaddr [MAX_DHCP_CHADDR_LENGTH];      /* hardware address of this machine */
        char sname [MAX_DHCP_SNAME_LENGTH];                 /* name of DHCP server */
        char file [MAX_DHCP_FILE_LENGTH];                   /* boot file name (used for diskless booting?) */
        unsigned char options[MAX_DHCP_OPTIONS_LENGTH];     /* options */
} __attribute__((packed));

typedef struct dhcp_message_struct dhcp_message_t;

/* interface name passed via cml line argument */
extern char interface_cmd[IFNAMSIZ];
/* MAC address used as HARDWARE ADDRESS in DISCOVERY packet */
extern unsigned char MAC_address[MAC_ADDRESS_SIZE];
/* DHCP message used in DISCOVERY packet */
extern dhcp_message_t dhcp_message;

/*
** Parses the command line arguments and sets globals
** returns 0 on error
*/
int parse_cmd(int argc, char **argv);

/*
** Creates a DHCP socket
** returns -1 on error
*/
int create_DHCP_socket(void);

/*
** Broadcasts a DHCP message
** return -1 on failure
*/
int dhcp_message_broadcast(int sock);

/*
** Generates a random MAC adress
*/
void generate_MAC_address(void);

/*
** Generate a random transaction id
*/
uint32_t generate_transaction_id(void);

/*
** Initializes dhcp_message
*/
void dhcp_message_innit(void);

/*
** Sets dhcp_message to a discovery message
** MAC address is not set
*/
void dhcp_set_discovery_message(void);

/*
** Changes the MAC address in dhcp_message
*/
void dhcp_message_set_random_MAC(void);

/*
** Changes the transaction ID in dhcp_message
*/
void dhcp_message_set_random_transaction_id(void);

#endif