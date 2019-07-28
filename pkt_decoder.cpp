#include "pkt_decoder.h"

pkt_decoder_t *decoder;
pkt_read_fn_t printf_function;
void *callback_context;
size_t length_packet;
const uint8_t *packet_data;

pkt_decoder_t* pkt_decoder_create(pkt_read_fn_t callback, void *callback_ctx)
{
    printf_function = callback;
    callback_context = callback_ctx;
    return decoder;
}

// Destructor for a pkt_decoder
void pkt_decoder_destroy(pkt_decoder_t *decoder)
{
    printf_function(callback_context,length_packet,packet_data);
}

// Called on incoming, undecoded bytes to be translated into packets
void pkt_decoder_write_bytes(pkt_decoder_t *decoder, size_t len, const uint8_t *data)
{
    length_packet = len;
    packet_data = data;
}
