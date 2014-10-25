#include <p2p_client.h>

#include <netinet/in.h>
#include <string.h>
#include <stdio.h>

#define FAILED(err) err < 0

void var_dump(void *data, size_t len) {
    int i;
    for(i = 0; i < len - 1; i++) {
        printf("%02X ", ((uint8_t *) data)[i]);
    }
    if(len - 1 > 0) {
        printf("%02X\n", ((uint8_t *) data)[len - 1]);
    }
}

int main() {
    
    struct sockaddr_in addr;
    
    uint16_t inet_addr[4];
    uint8_t s_addr[4];
    uint16_t port;
    scanf("%hu.%hu.%hu.%hu:%hu", &inet_addr[3], &inet_addr[2], &inet_addr[1], &inet_addr[0], &port);
    
    s_addr[0] = inet_addr[0];
    s_addr[1] = inet_addr[1];
    s_addr[2] = inet_addr[2];
    s_addr[3] = inet_addr[3];
    
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(*((uint32_t *) s_addr));
    addr.sin_port = htons(port);
    
    init_p2p_client(&addr);
    
    p2p_client_loop();
    
    close_p2p_client();
    
    printf("exiting\n");
    return 0;
}
