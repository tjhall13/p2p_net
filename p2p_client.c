#include <netinet/in.h>
#include <stdio.h>

#include <signal.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/select.h>

#include <p2p_pkt.h>
#include <p2p_client.h>

struct client_cntxt c_ctx;

static int init_client_cntxt(struct client_cntxt *ctx) {
    ctx->icmp_resp = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    
    return 0;
}

static int close_client_cntxt(struct client_cntxt *ctx) {
    close(ctx->icmp_resp);
    
    return 0;
}

static int client_running = 0;

static void client_sig_handler(int signo) {
    switch(signo) {
    case SIGINT:
        client_running = 0;
        break;
    default:
        break;
    }
}

int init_p2p_client(struct sockaddr_in *addr) {
    init_p2p_client_pkt(addr);
    
    init_client_cntxt(&c_ctx);
    
    signal(SIGINT, client_sig_handler);
    
    client_running = 1;
    return 0;
}

int p2p_client_loop() {
    while(client_running) {
        bcast_icmp_resp_pkt(c_ctx.icmp_resp);
        printf("sending ttl...\n");
        sleep(30);
    }
    
    return 0;
}

int close_p2p_client() {
    close_client_cntxt(&c_ctx);
    return 0;
}
