#include "pkt_decoder.h"
#include <stdlib.h>

enum packet_markers
{
    start_transaction                  = 0x02,
    end_transaction                    = 0x03,
    escape_sequnce_begning             = 0x10,
    /* below are not packet markersrather they are values which
       can are values escaped , for optmization or instruction 
       I would like to compare the unescaped values
    */  
    unescape_value                     = 0x20,
    escape_value_start_transaction     = 0x22,
    escape_value_end_transaction       = 0x23,
    escape_the_escapecharacter         = 0x30
};

enum state
{
    start_state               = 1,
    data_ready_state          = 2,
    escape_start_state        = 3,
    end_state                 = 4,
    invalid_data_state        = 5
};


struct pkt_decoder
{
    uint8_t decoded_packet[MAX_DECODED_PACKET_LEN];
    size_t decode_packet_index;
    pkt_read_fn_t printf_function;
    state decoder_state;
    void * callback_context;
};


pkt_decoder_t* pkt_decoder_create(pkt_read_fn_t callback, void *callback_ctx)
{
    pkt_decoder_t *decoder = (pkt_decoder_t *) calloc(1, sizeof(pkt_decoder_t) ) ;
    decoder->printf_function = callback;
    decoder->callback_context = callback_ctx;
    decoder->decoder_state = start_state;
    decoder->decode_packet_index = 0;
    return decoder;
}

// Destructor for a pkt_decoder
void pkt_decoder_destroy(pkt_decoder_t *decoder)
{
    free(decoder);
}

void handel_start_state( pkt_decoder_t *decoder,uint8_t data_byte)
{
    switch(data_byte)
    {
            //STX
        case start_transaction:
            decoder->decoder_state = data_ready_state;
            decoder->decode_packet_index = 0;
            break;
            
            //ETX
        case end_transaction:
            decoder->decoder_state = end_state;
            decoder->printf_function(decoder->callback_context,decoder->decode_packet_index,decoder->decoded_packet);
            decoder->decode_packet_index = 0 ;
            break;
            
        case escape_sequnce_begning:
            decoder->decoder_state = escape_start_state;
            break;
            
        default:
            decoder->decoded_packet[decoder->decode_packet_index++] = data_byte;
            break;
    }
}


void handel_escape_bytes( pkt_decoder_t *decoder,uint8_t data_byte)
{
    if((data_byte == escape_value_start_transaction) ||
       (data_byte == escape_value_end_transaction) ||
       (data_byte == escape_the_escapecharacter) )
    {
        data_byte = data_byte ^ unescape_value;
        decoder->decoded_packet[decoder->decode_packet_index++] = data_byte;
        decoder->decoder_state = data_ready_state ;
    }
    else
    {
        decoder->decoder_state = invalid_data_state;
    }
}

/*reset state to start state if current state is invalid packet or end sequnce is receved */
void reset_state_if_invalid_or_end_transaction(pkt_decoder_t *decoder)
{
    if(decoder->decoder_state == invalid_data_state || decoder->decoder_state == end_state)
    {
        decoder->decoder_state = start_state;
        decoder->decode_packet_index = 0 ;
    }
}

// Called on incoming, undecoded bytes to be translated into packets
void pkt_decoder_write_bytes(pkt_decoder_t *decoder, size_t len, const uint8_t *data)
{
    size_t stx_position = 0 ;
    size_t etx_position = len;
    
    
    for(size_t frame_index = stx_position ; frame_index < etx_position ; frame_index++)
    {
        uint8_t data_byte = data[frame_index];
        
        //discard if big packets
        if(decoder->decode_packet_index == MAX_DECODED_PACKET_LEN)
        {
            decoder->decode_packet_index = 0 ;
        }
        
        
        reset_state_if_invalid_or_end_transaction(decoder);
        
        //in a state of ready and copying bytes to decoder->decoded_packet buffer
        if(decoder->decoder_state == data_ready_state)
        {
            handel_start_state( decoder,data_byte);
            continue;
        }
        
        //handel escape sequence
        if( decoder->decoder_state == escape_start_state)
        {
            handel_escape_bytes(decoder,data_byte);
            continue;
        }
        
        //Detect the STX
        if(decoder->decoder_state == start_state)
        {
            decoder->decoder_state = (data[frame_index] == start_transaction) ? data_ready_state : decoder->decoder_state;
            continue;
        }
        
    }
    
}
