#ifndef _P2P_STUN_H_
#define _P2P_STUN_H_

struct server_cntxt;
struct sockaddr_in;

int init_p2p_stun();

int stun_retrieve(int stun_sock, struct sockaddr_in *server_addr);
int bcast_udp_pkt(int icmp_sock);

#endif
