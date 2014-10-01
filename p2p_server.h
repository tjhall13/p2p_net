#ifndef _P2P_SERVER_H_
#define _P2P_SERVER_H_

#define BCAST_PORT 4723

typedef int socket_t;

struct server_cntxt {
    socket_t udp_bcast;
    socket_t stun_req;
    socket_t icmp_listen;
};

extern struct server_cntxt s_ctx;

int init_p2p_server();
int close_p2p_server();

int p2p_server_loop();

#endif
