#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <openssl/sha.h>

#include <stun.h>
#include <p2p_server.h>

#define STUN_PORT 3478

#define FAILED(err) err < 0

static int _init = 0;
static int crypt_seed = 0;
static struct sockaddr_in stun_addr = { 0 };

int init_p2p_stun() {
    int seed = time(NULL);
    srand(seed);
    crypt_seed = rand();
    
    struct addrinfo *ptr, *res = NULL;
    
    int err = getaddrinfo("stun.ekiga.net", NULL, NULL, &res);
    
    if(FAILED(err)) {
        return -1;
    }
    
    struct sockaddr_in *addr = NULL;
    
    for(ptr = res; ptr != NULL; ptr = ptr->ai_next) {
        if(ptr->ai_family == AF_INET) {
            addr = (struct sockaddr_in *) ptr->ai_addr;
            addr->sin_port = htons(STUN_PORT);
            break;
        }
    }
    if(addr == NULL) {
        return -1;
    }
    memcpy(&stun_addr, addr, sizeof(struct sockaddr_in));
    
    _init = 1;
    return 0;
}

static void stun_id_gen(uint32_t arr[3]) {
    uint8_t hash[20];
    
    SHA_CTX ctx;
    SHA1_Init(&ctx);
    SHA1_Update(&ctx, &crypt_seed, sizeof(crypt_seed));
    SHA1_Final(hash, &ctx);
    
    crypt_seed++;
    
    memcpy(arr, hash, sizeof(arr));
}

int stun_retrieve(int stun_sock, struct sockaddr_in *server_addr) {
    if(!_init) {
        return -1;
    }
    int err;
    
    struct stunhdr hdr;
    hdr.zero     = 0;
    hdr.msg_type = htons(0x0001);
    hdr.msg_len  = htons(0x0000);
    hdr.magic    = htonl(0x2112A442);
    
    stun_id_gen(hdr.trans_id);
    
    err = sendto(stun_sock, &hdr, sizeof(hdr), 0, (struct sockaddr *) &stun_addr, sizeof(struct sockaddr_in));
    if(FAILED(err)) {
        return -1;
    }
    
    struct sockaddr_in dest;
    socklen_t dest_len = sizeof(dest);
    
    uint8_t buffer[256];
    int len = sizeof(buffer);
    
    len = recvfrom(stun_sock, buffer, len, 0, (struct sockaddr *) &dest, &dest_len);
    
    struct stunhdr *recv_hdr = (struct stunhdr *) buffer;
    void *stun_attrs = (void *) buffer + sizeof(struct stunhdr);
    
    struct stunattr *attr = (struct stunattr *) stun_attrs;
    
    len = ntohs(recv_hdr->msg_len);
    int attr_len = 0;
    for(len = ntohs(recv_hdr->msg_len); len > 0; len -= attr_len) {
        attr_len = ntohs(attr->attr_len) + 4;
        switch(ntohs(attr->attr_type)) {
            case MAPPED_ADDRESS:
                server_addr->sin_family = AF_INET;
                server_addr->sin_port = attr->attr_value.stun_addr.port;
                server_addr->sin_addr.s_addr = attr->attr_value.stun_addr.addr;
                return 0;
                break;
            case ERROR_CODE:
                return -1;
                break;
            default:
                break;
        }
        
        attr = (struct stunattr *) ((void *) attr + attr_len);
    }
    
    return -1;
}
