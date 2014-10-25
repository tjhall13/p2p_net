#ifndef P2P_CLIENT_H_
#define P2P_CLIENT_H_

typedef unsigned int socket_t;

struct client_cntxt {
    socket_t icmp_resp;
};

extern struct client_cntxt c_ctx;

int init_p2p_client();
int close_p2p_client();

int p2p_client_loop();

#endif
