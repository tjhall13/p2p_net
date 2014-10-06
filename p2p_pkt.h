#ifndef _P2P_PKT_H_
#define _P2P_PKT_H_

int init_p2p_server_pkt();
int init_p2p_client_pkt();

int bcast_udp_pkt(int udp_sock);

#endif
