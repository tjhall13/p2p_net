#include <netinet/in.h>
#include <stdio.h>

#include <signal.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/select.h>

#include <p2p_pkt.h>
#include <p2p_stun.h>
#include <p2p_server.h>

#define FAILED(err) err < 0

struct server_cntxt s_ctx;

static struct sockaddr_in _local_addr = { 0 };

static int serv_running;

static void serv_sig_handler(int signo) {
    if(signo == SIGINT) {
        serv_running = 0;
    }
}

static int init_server_cntxt(struct server_cntxt *ctx) {
    int optval = 1;
    int err;
    
    ctx->udp_bcast = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    
    err = setsockopt(ctx->udp_bcast, IPPROTO_IP, IP_HDRINCL, &optval, sizeof(optval));
    if(FAILED(err)) {
        return -1;
    }
    
    err = bind(ctx->udp_bcast, (struct sockaddr *) &_local_addr, sizeof(_local_addr));
    if(FAILED(err)) {
        return -1;
    }
    
    ctx->stun_req = socket(AF_INET, SOCK_DGRAM, 0);
    
    err = bind(ctx->stun_req, (struct sockaddr *) &_local_addr, sizeof(_local_addr));
    if(FAILED(err)) {
        return -1;
    }
    
    ctx->icmp_listen = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    
    return 0;
}

static int close_server_cntxt(struct server_cntxt *ctx) {
    close(ctx->udp_bcast);
    close(ctx->stun_req);
    close(ctx->icmp_listen);
    
    return 0;
}

static int _init;

int init_p2p_server() {
    int err = 0;
    
    err = init_p2p_server_pkt();
    if(FAILED(err)) {
        return -1;
    }
    
    err = init_p2p_stun();
    if(FAILED(err)) {
        return -1;
    }
    
    _local_addr.sin_family = AF_INET;
    _local_addr.sin_addr.s_addr = INADDR_ANY;
    _local_addr.sin_port = htons(BCAST_PORT);
    
    err = init_server_cntxt(&s_ctx);
    if(FAILED(err)) {
        return -1;
    }
    
    _init = 1;
    
    serv_running = 0;
    if(signal(SIGINT, serv_sig_handler) == SIG_ERR) {
        return -1;
    }
    
    return 0;
}

int close_p2p_server() {
    close_server_cntxt(&s_ctx);
    
    _init = 0;
    return 0;
}

static int handle_fd(struct server_cntxt *ctx, fd_set *fds) {
    if(FD_ISSET(ctx->icmp_listen, fds)) {
        printf("icmp responded");
        
        return 0;
    } else {
        return -1;
    }
}

int p2p_server_loop() {
    struct sockaddr_in server_addr = { 0 };
    
    struct timeval timeout;
    fd_set s_fds;
    
    int fd = 0;
    
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
    
    int nfds = s_ctx.icmp_listen + 1;
    
    serv_running = 1;
    while(serv_running) {
        if(FAILED(fd)) {
            return -1;
        }
        if(fd == 0) {
            timeout.tv_sec = 10;
            timeout.tv_usec = 0;
            
            bcast_udp_pkt(s_ctx.udp_bcast);
            stun_retrieve(s_ctx.stun_req, &server_addr);
            
            printf("%d\n", ntohs(server_addr.sin_port));
        } else {
            handle_fd(&s_ctx, &s_fds);
        }
        
        FD_ZERO(&s_fds);
        FD_SET(s_ctx.icmp_listen, &s_fds);
        
        fd = select(nfds, &s_fds, NULL, NULL, &timeout);
    }
    return 0;
}
