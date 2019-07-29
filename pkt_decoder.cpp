#include "pkt_decoder.h"
#include <stdlib.h>

enum packet_markers
{
    start_transaction                  = 0x02,
    end_transaction                    = 0x03,
    escape_sequnce_begning             = 0x10,
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
    size_t decode_packet_index = 0 ;
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
    return decoder;
}

// Destructor for a pkt_decoder
void pkt_decoder_destroy(pkt_decoder_t *decoder)
{
    free(decoder);
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
        
        
        if(decoder->decoder_state == invalid_data_state || decoder->decoder_state == end_state)
        {
            decoder->decoder_state = start_state;
            decoder->decode_packet_index = 0 ;
        }
        
        //in a state of ready and copying bytes to decoder->decoded_packet buffer
        if(decoder->decoder_state == data_ready_state)
        {
            //STX
            if(data_byte == start_transaction)
            {
                decoder->decoder_state = data_ready_state;
                decoder->decode_packet_index = 0;
                continue ;
            }
            
            //ETX
            if(data_byte == end_transaction)
            {
                decoder->decoder_state = end_state;
                decoder->printf_function(decoder->callback_context,decoder->decode_packet_index,decoder->decoded_packet);
                decoder->decode_packet_index = 0 ;
                continue;
            }
            
            if(data_byte == escape_sequnce_begning)
            {
                decoder->decoder_state = escape_start_state;
                continue;
            }
            
            decoder->decoded_packet[decoder->decode_packet_index++] = data_byte;
            continue;
        }
        
        
        //handel escape sequence
        if( decoder->decoder_state == escape_start_state)
        {
            if((data_byte == escape_value_start_transaction) ||
               (data_byte == escape_value_end_transaction) ||
               (data_byte == escape_the_escapecharacter) )
            {
                data_byte = data_byte ^ unescape_value;
                decoder->decoded_packet[decoder->decode_packet_index++] = data_byte;
                decoder->decoder_state = data_ready_state ;
                continue;
            }
            else
            {
                decoder->decoder_state = invalid_data_state;
                continue;
            }
        }
        
        
        //Detect the STX
        if(decoder->decoder_state == start_state)
        {
            decoder->decoder_state = (data[frame_index] == start_transaction) ? data_ready_state : decoder->decoder_state;
            continue;
        }
        
    }
    
}
