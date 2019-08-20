package main

import (
	"fmt"
)

const MAX_DECODED_PACKET_LEN int = 512

type packet_markers int

/* below are not packet markersrather they are values which
can are values escaped , for optmization or instruction
I would like to compare the unescaped values
*/

const (
	start_transaction              packet_markers = 0x02
	end_transaction                packet_markers = 0x03
	escape_sequnce_begning         packet_markers = 0x10
	unescape_value                 packet_markers = 0x20
	escape_value_start_transaction packet_markers = 0x22
	escape_value_end_transaction   packet_markers = 0x23
	escape_the_escapecharacter     packet_markers = 0x30
)

type context struct{}

type state int

const (
	start_state        state = 1
	data_ready_state   state = 2
	escape_start_state state = 3
	end_state          state = 4
	invalid_data_state state = 5
)

type pkt_read_fn_t func(ctx *context, len int, data [MAX_DECODED_PACKET_LEN]byte)
type pkt_decoder struct {
	decoded_packet      [MAX_DECODED_PACKET_LEN]byte
	decode_packet_index int
	printf_function     pkt_read_fn_t
	decoder_state       state
	callback_context    context
}

func pkt_decoder_create(callback pkt_read_fn_t, callback_ctx context) *pkt_decoder {
	var decoder pkt_decoder
	decoder.printf_function = callback
	decoder.callback_context = callback_ctx
	decoder.decoder_state = start_state
	decoder.decode_packet_index = 0
	return &decoder
}

// Destructor for a pkt_decoder
func pkt_decoder_destroy(decoder *pkt_decoder) {
	//golang manages memory automatically
	//free(decoder);
}

func get_current_packet_markers(decoder *pkt_decoder, data byte) packet_markers {
	data_byte_int := int(data)
	var data_byte packet_markers = packet_markers(data_byte_int)
	return data_byte
}

func handel_start_state(decoder *pkt_decoder, data []byte) {

	var data_byte packet_markers = get_current_packet_markers(decoder, data[0])

	switch data_byte {
	//STX
	case start_transaction:
		decoder.decoder_state = data_ready_state
		decoder.decode_packet_index = 0

		//ETX
	case end_transaction:
		decoder.decoder_state = end_state
		decoder.printf_function(&decoder.callback_context, decoder.decode_packet_index, decoder.decoded_packet)
		decoder.decode_packet_index = 0

	case escape_sequnce_begning:
		decoder.decoder_state = escape_start_state

	default:

		decoder.decoded_packet[decoder.decode_packet_index] = data[0]
		decoder.decode_packet_index = decoder.decode_packet_index + 1
	}
}

func handel_escape_bytes(decoder *pkt_decoder, data []byte) {
	data_byte := get_current_packet_markers(decoder, data[0])
	if (data_byte == escape_value_start_transaction) ||
		(data_byte == escape_value_end_transaction) ||
		(data_byte == escape_the_escapecharacter) {
		data_byte = data_byte ^ unescape_value
		decoder.decoded_packet[decoder.decode_packet_index] = byte(data_byte)
		decoder.decode_packet_index = decoder.decode_packet_index + 1
		decoder.decoder_state = data_ready_state
	} else {
		decoder.decoder_state = invalid_data_state
	}
}

/*reset state to start state if current state is invalid packet or end sequnce is receved */
func reset_state_if_invalid_or_end_transaction(decoder *pkt_decoder) {
	if decoder.decoder_state == invalid_data_state || decoder.decoder_state == end_state {
		decoder.decoder_state = start_state
		decoder.decode_packet_index = 0
	}
}

// Called on incoming, undecoded bytes to be translated into packets
func pkt_decoder_write_bytes(decoder *pkt_decoder, len int, data []byte) {
	var stx_position int = 0
	var etx_position int = len
	var frame_index int

	for frame_index = stx_position; frame_index < etx_position; frame_index++ {
		data_byte := data[frame_index:cap(data)]

		if decoder.decode_packet_index == MAX_DECODED_PACKET_LEN {
			decoder.decoder_state = start_state
			decoder.decode_packet_index = 0
		}

		reset_state_if_invalid_or_end_transaction(decoder)

		//in a state of ready and copying bytes to decoder->decoded_packet buffer
		if decoder.decoder_state == data_ready_state {
			handel_start_state(decoder, data_byte)
			continue
		}

		//handel escape sequence
		if decoder.decoder_state == escape_start_state {
			handel_escape_bytes(decoder, data_byte)
			continue
		}

		//Detect the STX
		if decoder.decoder_state == start_state {

			packet_market := get_current_packet_markers(decoder, data[frame_index])
			if packet_market == start_transaction {
				decoder.decoder_state = data_ready_state
			}
			continue
		}

	}

}

func print(ctx *context, len int, data [MAX_DECODED_PACKET_LEN]byte) {

	printdata:=data[0:len]
	fmt.Printf(" %x",printdata)
}
func main() {

	var ctx context
	var packet []byte = []byte{0x02, 0x45,0x01,0x44, 0x03}
	decoder := pkt_decoder_create(print, ctx)
	pkt_decoder_write_bytes(decoder, len(packet), packet)
}
