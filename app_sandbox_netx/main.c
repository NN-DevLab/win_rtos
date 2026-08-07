/* win_rtos_sandbox_netx - minimal NetX Duo API sandbox.

   Just ThreadX + NetX Duo, nothing else (no FileX, no GUIX). Two
   loopback IP instances exchange UDP packets so the basic NetX Duo
   API flow is visible on its own: packet pool create -> IP create ->
   ARP/UDP enable -> socket create/bind -> allocate/send/receive/
   release.
*/

#include "tx_api.h"
#include "nx_api.h"
#include <stdio.h>

#define DEMO_STACK_SIZE 4096
#define PACKET_SIZE 1536
#define POOL_SIZE   ((sizeof(NX_PACKET) + PACKET_SIZE) * 16)

TX_THREAD thread_tx;
TX_THREAD thread_rx;

NX_PACKET_POOL pool_0;
NX_IP          ip_0;
NX_IP          ip_1;
NX_UDP_SOCKET  socket_0;
NX_UDP_SOCKET  socket_1;
UCHAR          pool_buffer[POOL_SIZE];

void _nx_ram_network_driver(struct NX_IP_DRIVER_STRUCT *driver_req);

void thread_tx_entry(ULONG thread_input)
{
UINT        status;
NX_PACKET  *packet;
NXD_ADDRESS dest;
ULONG       sent = 0;

    (void)thread_input;
    tx_thread_sleep(NX_IP_PERIODIC_RATE);

    dest.nxd_ip_version = NX_IP_VERSION_V4;
    dest.nxd_ip_address.v4 = IP_ADDRESS(1, 2, 3, 5);

    status = nx_udp_socket_create(&ip_0, &socket_0, "sandbox socket 0", NX_IP_NORMAL, NX_FRAGMENT_OKAY, 0x80, 5);
    printf("nx_udp_socket_create(tx) -> 0x%x\n", status);

    status = nx_udp_socket_bind(&socket_0, 0x88, TX_WAIT_FOREVER);
    printf("nx_udp_socket_bind(tx) -> 0x%x\n", status);

    nx_udp_socket_checksum_disable(&socket_0);
    nx_arp_dynamic_entry_set(&ip_0, IP_ADDRESS(1, 2, 3, 5), 0, 0);

    while (1)
    {
        status = nx_packet_allocate(&pool_0, &packet, NX_UDP_PACKET, TX_WAIT_FOREVER);
        if (status != NX_SUCCESS)
        {
            printf("nx_packet_allocate failed: 0x%x\n", status);
            break;
        }

        nx_packet_data_append(packet, "hello from netx sandbox", 24, &pool_0, TX_WAIT_FOREVER);

        status = nxd_udp_socket_send(&socket_0, packet, &dest, 0x89);
        if (status != NX_SUCCESS)
        {
            printf("nxd_udp_socket_send failed: 0x%x\n", status);
            break;
        }

        sent++;
        printf("[tx] sent packet #%lu\n", sent);
        tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND);
    }
}

void thread_rx_entry(ULONG thread_input)
{
UINT       status;
NX_PACKET *packet;
ULONG      received = 0;
CHAR       buffer[32];
ULONG      bytes_copied;

    (void)thread_input;
    tx_thread_sleep(NX_IP_PERIODIC_RATE);

    status = nx_udp_socket_create(&ip_1, &socket_1, "sandbox socket 1", NX_IP_NORMAL, NX_FRAGMENT_OKAY, 0x80, 5);
    printf("nx_udp_socket_create(rx) -> 0x%x\n", status);

    status = nx_udp_socket_bind(&socket_1, 0x89, TX_WAIT_FOREVER);
    printf("nx_udp_socket_bind(rx) -> 0x%x\n", status);

    while (1)
    {
        status = nx_udp_socket_receive(&socket_1, &packet, TX_WAIT_FOREVER);
        if (status != NX_SUCCESS)
        {
            printf("nx_udp_socket_receive failed: 0x%x\n", status);
            break;
        }

        nx_packet_data_extract_offset(packet, 0, buffer, sizeof(buffer) - 1, &bytes_copied);
        buffer[bytes_copied] = 0;
        nx_packet_release(packet);

        received++;
        printf("[rx] received packet #%lu: \"%s\"\n", received, buffer);
    }
}

int main(void)
{
    printf("=== win_rtos_sandbox_netx ===\n");
    printf("NetX Duo only: two loopback IP instances exchange UDP packets.\n\n");

    tx_kernel_enter();
    return 0;
}

void tx_application_define(void *first_unused_memory)
{
CHAR *pointer = (CHAR *)first_unused_memory;
UINT  status;

    tx_thread_create(&thread_tx, "tx", thread_tx_entry, 0,
                      pointer, DEMO_STACK_SIZE, 4, 4, TX_NO_TIME_SLICE, TX_AUTO_START);
    pointer += DEMO_STACK_SIZE;

    tx_thread_create(&thread_rx, "rx", thread_rx_entry, 0,
                      pointer, DEMO_STACK_SIZE, 3, 3, TX_NO_TIME_SLICE, TX_AUTO_START);
    pointer += DEMO_STACK_SIZE;

    nx_system_initialize();

    status = nx_packet_pool_create(&pool_0, "sandbox packet pool", PACKET_SIZE, pool_buffer, POOL_SIZE);
    printf("nx_packet_pool_create -> 0x%x\n", status);

    status = nx_ip_create(&ip_0, "sandbox IP 0", IP_ADDRESS(1, 2, 3, 4), 0xFFFFF000UL,
                           &pool_0, _nx_ram_network_driver, pointer, 2048, 1);
    pointer += 2048;
    printf("nx_ip_create(ip_0) -> 0x%x\n", status);

    status = nx_ip_create(&ip_1, "sandbox IP 1", IP_ADDRESS(1, 2, 3, 5), 0xFFFFF000UL,
                           &pool_0, _nx_ram_network_driver, pointer, 2048, 1);
    pointer += 2048;
    printf("nx_ip_create(ip_1) -> 0x%x\n", status);

    status = nx_arp_enable(&ip_0, (void *)pointer, 1024);
    pointer += 1024;
    printf("nx_arp_enable(ip_0) -> 0x%x\n", status);

    status = nx_arp_enable(&ip_1, (void *)pointer, 1024);
    pointer += 1024;
    printf("nx_arp_enable(ip_1) -> 0x%x\n", status);

    status = nx_udp_enable(&ip_0);
    printf("nx_udp_enable(ip_0) -> 0x%x\n", status);

    status = nx_udp_enable(&ip_1);
    printf("nx_udp_enable(ip_1) -> 0x%x\n", status);
}
