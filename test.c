#include <p2p_pkt.h>
#include <p2p_server.h>
#include <p2p_stun.h>

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
    init_p2p_server();
    
    p2p_server_loop();
    
    close_p2p_server();
    
    printf("exiting\n");
    return 0;
}
