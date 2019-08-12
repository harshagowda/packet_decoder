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
#include <stdlib.h>
#include <string.h>
//global packet for test
typedef struct expected_result
{
    uint8_t expect_value_buffer[MAX_DECODED_PACKET_LEN];
    size_t length;
    
}expected_result;

expected_result expect_result;

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

static void validate_decoder_test(void *ctx, size_t len, const uint8_t *data)
{
    expected_result *expected_data = (expected_result *)ctx;
    
    printf("TEST = ");
    
    for(size_t i = 0; i < len; i++)
    {
        if(data[i] != expected_data->expect_value_buffer[i])
            {
                printf("FAILED\n");
                return ;
            }
    }
    
    printf("PASSED\n");
    
}



void run_test_case(pkt_decoder_t* decoder, const uint8_t *packet ,size_t packet_len, const uint8_t * expected_value,size_t expected_length)
{
    memset(expect_result.expect_value_buffer,0,sizeof(expect_result.expect_value_buffer));
    memcpy(expect_result.expect_value_buffer,expected_value,expected_length);
    expect_result.length = expected_length;
    pkt_decoder_write_bytes(decoder, packet_len, packet);
}



int main()
{
   
    
    pkt_decoder_t* decoder = pkt_decoder_create(validate_decoder_test, (void *)&expect_result);
    

    //PASS
    const uint8_t packet1[] = {0x02, 0xFF, 0x10, 0x22, 0x00, 0x03};
    const uint8_t packet1_expected[] = {0xFF,0x02,0x00} ;
    run_test_case(decoder, packet1,sizeof(packet1), packet1_expected ,sizeof(packet1_expected));
    
    //PASS
    const uint8_t packet2[] = {0x02, 0x10, 0x22, 0x03};
    const uint8_t packet2_expected[] = {0x02};
    run_test_case(decoder, packet2,sizeof(packet2), packet2_expected ,sizeof(packet2_expected));
    
    //PASS
    const uint8_t packet3[] = {0x02, 0xFF, 0x02, 0xFE, 0x03};
    const uint8_t packet3_expected[] = {0xFE};
    run_test_case(decoder, packet3,sizeof(packet3), packet3_expected ,sizeof(packet3_expected));

    //PASS
    //fragmented packet
    const uint8_t packet5a[] = {0x02, 0xFF, 0x10};
    const uint8_t packet5b[] = {0x22, 0x03};
    const uint8_t packet5_expected[] = {0xFF, 0x02};
    
    run_test_case(decoder, packet5a,sizeof(packet5a), packet5_expected ,sizeof(packet5_expected));
    run_test_case(decoder, packet5b,sizeof(packet5b), packet5_expected ,sizeof(packet5_expected));


    //FAIL = NEGATIVE TEST CASE
    const uint8_t packet6[] = {0x02, 0xFF, 0x02, 0xFE, 0x03};
    const uint8_t packet6_expected[] = {0x34};
    run_test_case(decoder, packet6,sizeof(packet6), packet6_expected ,sizeof(packet6_expected));
    
    pkt_decoder_destroy(decoder);
    
    return 0;
}

