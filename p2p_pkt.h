#ifndef _P2P_PKT_H_
#define _P2P_PKT_H_

#define IP_BCAST_SZ 32

extern unsigned char bcast_pkt[IP_BCAST_SZ];

int init_p2p_server_pkt();
int init_p2p_client_pkt();

#endif
