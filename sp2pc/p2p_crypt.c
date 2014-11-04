#include <openssl/rsa.h>
#include <openssl/bn.h>
#include <gcrypt.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int p2p_convert_private_key(RSA *key, gcry_sexp_t *r_key) {
    size_t err;
    char *buf;
    
    gcry_mpi_t n_mpi, e_mpi, d_mpi, p_mpi, q_mpi, u_mpi;
    
    buf = BN_bn2hex(key->n);
    gcry_mpi_scan(&n_mpi, GCRYMPI_FMT_HEX, buf, 0, &err);
    OPENSSL_free(buf);
    
    buf = BN_bn2hex(key->e);
    gcry_mpi_scan(&e_mpi, GCRYMPI_FMT_HEX, buf, 0, &err);
    OPENSSL_free(buf);
    
    buf = BN_bn2hex(key->d);
    gcry_mpi_scan(&d_mpi, GCRYMPI_FMT_HEX, buf, 0, &err);
    OPENSSL_free(buf);
    
    buf = BN_bn2hex(key->p);
    gcry_mpi_scan(&p_mpi, GCRYMPI_FMT_HEX, buf, 0, &err);
    OPENSSL_free(buf);
    
    buf = BN_bn2hex(key->q);
    gcry_mpi_scan(&q_mpi, GCRYMPI_FMT_HEX, buf, 0, &err);
    OPENSSL_free(buf);
    
    buf = BN_bn2hex(key->iqmp);
    gcry_mpi_scan(&u_mpi, GCRYMPI_FMT_HEX, buf, 0, &err);
    OPENSSL_free(buf);
    
    if(gcry_mpi_cmp(p_mpi, q_mpi) > 0) {
        gcry_mpi_swap(p_mpi, q_mpi);
        gcry_mpi_invm(u_mpi, p_mpi, q_mpi);
    }
    
    gcry_sexp_build(r_key, &err, "(private-key (rsa (n %m) (e %m) (d %m) (p %m) (q %m) (u %m)))", n_mpi, e_mpi, d_mpi, p_mpi, q_mpi, u_mpi);
    
    gcry_mpi_release(n_mpi);
    gcry_mpi_release(e_mpi);
    gcry_mpi_release(d_mpi);
    gcry_mpi_release(p_mpi);
    gcry_mpi_release(q_mpi);
    gcry_mpi_release(u_mpi);
    
    gcry_error_t sane = gcry_pk_testkey(*r_key);
    if(sane) {
        return -1;
    }
    
    return 0;
}

int p2p_convert_public_key(RSA *key, gcry_sexp_t *r_key) {
    size_t err;
    char *buf;
    
    gcry_mpi_t n_mpi, e_mpi;
    
    buf = BN_bn2hex(key->n);
    gcry_mpi_scan(&n_mpi, GCRYMPI_FMT_HEX, buf, 0, &err);
    OPENSSL_free(buf);
    
    buf = BN_bn2hex(key->e);
    gcry_mpi_scan(&e_mpi, GCRYMPI_FMT_HEX, buf, 0, &err);
    OPENSSL_free(buf);
    
    gcry_sexp_build(r_key, &err, "(public-key (rsa (n %m) (e %m)))", n_mpi, e_mpi);
    
    gcry_mpi_release(n_mpi);
    gcry_mpi_release(e_mpi);
    
    return 0;
}

int p2p_encrypt(unsigned char *msg, size_t msglen, unsigned char *buf, size_t buflen, gcry_sexp_t r_key) {
    gcry_sexp_t ciph, data;
    size_t err;
    gcry_sexp_build(&data, &err, "(data (flags pkcs1) (value %b))", msglen, msg);
    
    gcry_pk_encrypt(&ciph, data, r_key);
    gcry_sexp_release(data);
    
    gcry_sexp_t a = gcry_sexp_find_token(ciph, "a", 0);
    gcry_sexp_release(ciph);
    gcry_sexp_t sexp_mpi = gcry_sexp_cdr(a);
    gcry_sexp_release(a);
    
    gcry_mpi_t data_mpi = gcry_sexp_nth_mpi(sexp_mpi, 0, GCRYMPI_FMT_USG);
    gcry_mpi_print(GCRYMPI_FMT_PGP, buf, buflen, &err, data_mpi);
    
    return err;
}

int p2p_decrypt(unsigned char *buf, size_t buflen, char *msg, size_t msglen, gcry_sexp_t d_key) {    
    size_t err;
    gcry_sexp_t ciph;
    gcry_mpi_t data_mpi;
    gcry_mpi_scan(&data_mpi, GCRYMPI_FMT_PGP, buf, buflen, &err);
    gcry_sexp_build(&ciph, &err, "(enc-val (flags pkcs1) (rsa (a %M)))", data_mpi);
    gcry_mpi_release(data_mpi);
    
    gcry_sexp_t data_sexp;
    gcry_pk_decrypt(&data_sexp, ciph, d_key);
    gcry_sexp_release(ciph);
    gcry_sexp_t data = gcry_sexp_cdr(data_sexp);
    gcry_sexp_release(data_sexp);
    
    size_t len;
    const char *msg_data = gcry_sexp_nth_data(data, 0, &len);
    len = msglen > len ? len : msglen;
    memcpy(msg, msg_data, len);
    gcry_sexp_release(data);
    
    return len;
}

int p2p_sign(unsigned char *msg, size_t msglen, unsigned char *buf, size_t buflen, gcry_sexp_t d_key) {
    gcry_sexp_t data;
    size_t err;
    
    gcry_sexp_build(&data, &err, "(data (flags pkcs1) (hash sha256 %b))", msglen, msg);
    gcry_sexp_dump(data);
    return 0;
}

int main(int argc, char **argv) {
    if(argc != 3) {
        exit(1);
    }
    FILE *input;
    RSA *key;
    if(strcmp(argv[1], "-s") == 0) {
        input = fopen(argv[2], "r");
        
        key = RSA_new();
        PEM_read_RSAPrivateKey(input, &key, NULL, NULL);
        gcry_sexp_t d_key;
        p2p_convert_private_key(key, &d_key);
        RSA_free(key);
        
        fclose(input);
        
        struct sockaddr_in serv_addr;
        serv_addr.sin_port = htons(4711);
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        bind(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
        
        listen(sock, 5);
        
        struct sockaddr_in client_addr;
        socklen_t client_addrlen = sizeof(client_addr);
        int conn = accept(sock, (struct sockaddr *) &client_addr, &client_addrlen);
        
        unsigned char buf[1000];
        int datalen = recv(conn, buf, 1000, 0);
        
        char msg[50];
        
        int len = p2p_decrypt(buf, datalen, msg, 50, d_key);
        msg[len] = 0;
        printf("%s\n", msg);
        
    } else {
        input = fopen(argv[2], "r");
        
        key = RSA_new();
        PEM_read_RSA_PUBKEY(input, &key, NULL, NULL);
        gcry_sexp_t r_key;
        p2p_convert_public_key(key, &r_key);
        RSA_free(key);
        
        fclose(input);
        
        struct sockaddr_in serv_addr;
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(4711);
        serv_addr.sin_addr.s_addr = inet_addr("192.168.1.108");
        
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        int err = connect(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
        
        if(err < 0) {
            perror("connect");
            exit(1);
        }
        
        unsigned char buf[2000];
        char msg[50];
        printf("> ");
        fgets(msg, 50, stdin);
        size_t len = p2p_encrypt(msg, strlen(msg), buf, 2000, r_key);
        send(sock, buf, len, 0);
    }
    
    return 0;
}
