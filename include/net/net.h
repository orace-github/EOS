/*
 * Copyright 2019 Orace KPAKPO </ orace.kpakpo@yahoo.fr >
 *
 * This file is part of EOS.
 *
 * EOS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * EOS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef __NET_H__
#define __NET_H__
#include <types.h>
#include <error.h>
typedef unsigned int ip_addr_t;
typedef struct{
    #define MAC_LEN 0x06
    u8_t mac[MAC_LEN];
}__attribute__((packed)) mac_t;

static inline u16_t flip_short(u16_t short_int) {
    u16_t first_byte = *((u8_t*)(&short_int));
    u16_t second_byte = *((u8_t*)(&short_int) + 1);
    return (first_byte << 8) | (second_byte);
}

static inline u32_t flip_long(u32_t long_int) {
    u32_t first_byte = *((u8_t*)(&long_int));
    u32_t second_byte = *((u8_t*)(&long_int) + 1);
    u32_t third_byte = *((u8_t*)(&long_int)  + 2);
    u32_t fourth_byte = *((u8_t*)(&long_int) + 3);
    return (first_byte << 24) | (second_byte << 16) | (third_byte << 8) | (fourth_byte);
}

static inline u8_t flip_byte(u8_t byte, int num_bits) {
    u8_t t = byte << (8 - num_bits);
    return t | (byte >> num_bits);
}

static inline u8_t htonb(u8_t byte, int num_bits) {
    return flip_byte(byte, num_bits);
}

static inline u8_t ntohb(u8_t byte, int num_bits) {
    return flip_byte(byte, 8 - num_bits);
}

static inline u16_t htons(u16_t hostshort) {
    return flip_short(hostshort);
}

static inline u32_t htonl(u32_t hostlong) {
    return flip_long(hostlong);
}

static inline u16_t ntohs(u16_t netshort) {
    return flip_short(netshort);
}

static inline u32_t ntohl(u32_t netlong) {
    return flip_long(netlong);
}

static unsigned short checksum(void* __mm, int __size){
    int __len = __size / 2;
    u16_t* __array = (u16_t*)__mm;
    u32_t __sum = 0;
    for(int i = 0; i < __len; i++){
        __sum += flip_short(__array[i]);
    }
    u32_t __carry = __sum >> 16;
    __sum &= 0x0000ffff;
    __sum += __carry;
    return (u16_t)(~__sum);
}

#endif //__NET_H__