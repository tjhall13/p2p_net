#include <linux/ip.h>
#include <linux/udp.h>

#include <netinet/in.h>
#include <string.h>

#include <p2p_pkt.h>
#include <p2p_server.h>

#define UDP_BCAST_SZ 12

uint8_t bcast_pkt[IP_BCAST_SZ];

static struct sockaddr_in bcast_addr = { 0 };

static uint16_t checksum(void *vdata, size_t len) {
    uint8_t *data = (uint8_t *) vdata;
    uint64_t acc = 0xffff;
    uint16_t word;
    size_t i;

    for( i = 0; i + 1 < len; i += 2 ) {
        memcpy( &word, data + i, 2 );
        acc += ntohs( word );
        if( acc > 0xffff ) {
            acc -= 0xffff;
        }
    }
    
    if( len & 1 ) {
        memcpy( &word, data + len - 1, 1 );
        acc += ntohs( word );
        if( acc > 0xffff ) {
            acc -= 0xffff;
        }
    }
    
    return htons( ~acc );
}

static int _init_p2p_pkt() {
    
    struct iphdr  *ip   = (struct iphdr *) bcast_pkt;
    struct udphdr *udp  = (struct udphdr *) ((void *) ip + sizeof(*ip));
    int           *data = (int *) ((uint8_t *) udp + sizeof(*udp));
    
    ip->version  = 4;
    ip->ihl      = 5;
    ip->tos      = 0;
    ip->tot_len  = htons(IP_BCAST_SZ);
    ip->id       = htons(0x7011);
    ip->frag_off = htons(0x4000);
    ip->ttl      = 0;       // will be different for client/server
    ip->protocol = IPPROTO_UDP;
    ip->check    = 0;       // will be different for client/server
    ip->saddr    = 0;       // will be differnet for client/server
    ip->daddr    = inet_addr("1.1.1.1");
    
    udp->source  = 0;       // will be differnet for client/server
    udp->dest    = htons(0x1111);
    udp->len     = htons(UDP_BCAST_SZ);
    udp->check   = 0;       // will be differnet for client/server
    
    *data = 0x7018A5D4;
    
    udp->check = checksum(udp, UDP_BCAST_SZ);
    
    bcast_addr.sin_family      = AF_INET;
    bcast_addr.sin_port        = udp->dest;
    bcast_addr.sin_addr.s_addr = ip->daddr;
    
    return 0;
}

int init_p2p_client_pkt(struct sockaddr_in *addr) {

    int val = _init_p2p_pkt();
    
    struct iphdr *ip = (struct iphdr *) bcast_pkt;
    ip->saddr = addr->sin_addr.s_addr;
    ip->ttl   = 0x01;
    ip->check = checksum(ip, sizeof(*ip));
    
    struct udphdr *udp = (struct udphdr *) ((void *) ip + sizeof(*ip));
    udp->source = htons(addr->sin_port);
    udp->check  = checksum(udp, UDP_BCAST_SZ);
    
    return val;
}

int init_p2p_server_pkt() {
    
    int val = _init_p2p_pkt();
    
    struct iphdr *ip = (struct iphdr *) bcast_pkt;
    ip->ttl   = 0x40;
    
    struct udphdr *udp = (struct udphdr *) ((void *) ip + sizeof(*ip));
    udp->source = htons(BCAST_PORT);
    udp->check  = checksum(udp, UDP_BCAST_SZ);
    
    return val;
}

int bcast_udp_pkt(int udp_sock) {
    return sendto(udp_sock, bcast_pkt, sizeof(bcast_pkt), 0, (struct sockaddr *) &bcast_addr, sizeof(bcast_addr));
}
