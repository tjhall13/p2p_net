#ifndef _STUN_H_
#define _STUN_H_

#include <asm/byteorder.h>
#include <stdint.h>

struct stunhdr {
#if defined(__LITTLE_ENDIAN_BITFIELD)
    uint16_t msg_type: 14;
    uint16_t zero:      2;
#elif defined(__BIG_ENDIAN_BITFIELD)
    uint16_t zero:      2;
    uint16_t msg_type: 14;
#else
#error "Unknown Endian"
#endif
    uint16_t msg_len;
    uint32_t magic;
    uint32_t trans_id[3];
};

#define MAPPED_ADDRESS     0x0001
#define RESPONSE_ADDRESS   0x0002
#define CHANGED_REQUEST    0x0003
#define SOURCE_ADDRESS     0x0004
#define CHANGED_ADDRESS    0x0005
#define USERNAME           0x0006
#define PASSWORD           0x0007
#define MESSAGE_INTEGRITY  0x0008
#define ERROR_CODE         0x0009
#define UNKNOWN_ATTRIBUTES 0x000A
#define REFLECTED_FROM     0x000B

struct stunattr {
    uint16_t attr_type;
    uint16_t attr_len;
    union {
        struct {
            uint8_t unspec;
            uint8_t family;
            uint16_t port;
            uint32_t addr;
        } stun_addr;
        
        struct {
#if defined(__LITTLE_ENDIAN_BITFIELD)
            uint32_t:          5;
            uint32_t chg_ip:   1;
            uint32_t chg_port: 1;
            uint32_t:          1;
            uint32_t:         24;
#elif defined(__BIG_ENDIAN_BITFIELD)
            uint32_t:         29;
            uint32_t chg_ip:   1;
            uint32_t chg_port: 1;
            uint32_t:          1;
#else
#error "Unknown Endian"
#endif
        } change_request;
        
        uint32_t data;
        
        struct {
            uint32_t msg_hmac[5];
        };
        
        struct {
            uint16_t zero;
#if defined(__LITTLE_ENDIAN_BITFIELD)
            uint8_t class: 3;
            uint8_t:       5;
#elif defined(__BIG_ENDIAN_BITFIELD)
            uint8_t:       5;
            uint8_t class: 3;
#else
#error "Unknown Endian"
#endif
            uint8_t  number;
            uint32_t reason;  // variable length
        } error_code;
        
        struct {
            uint16_t attr1;
            uint16_t attr2;
        };
    } attr_value;
};

#endif
