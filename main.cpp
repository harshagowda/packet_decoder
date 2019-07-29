//
//  main.cpp
//  packet_decoder
//
//  Created by Harsha on 7/28/19.
//  Copyright © 2019 Harsha. All rights reserved.
//

#include "pkt_decoder.h"
#include <iostream>
#include <stdio.h>

static void pkt_printer(void *ctx, size_t len, const uint8_t *data)
{
    (void)ctx;
    
    printf("pkt (%zd bytes) -", len);
    for(size_t i = 0; i < len; i++)
    {
        printf(" %02x", data[i]);
    }
    
    printf("\n");
    
}

int main()
{
    pkt_decoder_t* decoder = pkt_decoder_create(pkt_printer, nullptr);
    

     const uint8_t pkt1[] = {0x02, 0xFF, 0x10, 0x22, 0x45, 0x03};
    
    pkt_decoder_write_bytes(decoder, sizeof(pkt1), pkt1);
    
    const uint8_t pkt2[] = {0x02, 0xFF, 0x03, 0x02, 0x45, 0x03};
    
    pkt_decoder_write_bytes(decoder, sizeof(pkt2), pkt2);
    
    const uint8_t pkt3[] = {0x02, 0xFF, 0x02, 0xFE, 0x03};
    
    pkt_decoder_write_bytes(decoder, sizeof(pkt3), pkt3);
    
    const uint8_t pkt4[] = {0x02, 0x10, 0x22, 0x03};
    
    pkt_decoder_write_bytes(decoder, sizeof(pkt4), pkt4);
    
    
    const uint8_t pkt5a[] = {0x02, 0xFF, 0x10};
    const uint8_t pkt5b[] = {0x22, 0x03};
    
    pkt_decoder_write_bytes(decoder, sizeof(pkt5a), pkt5a);
    pkt_decoder_write_bytes(decoder, sizeof(pkt5b), pkt5b);
    pkt_decoder_destroy(decoder);
    
    return 0;
}

