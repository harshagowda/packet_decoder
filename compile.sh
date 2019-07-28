gcc -O -c pkt_decoder.cpp 
ar rcs pkt_decoder.a pkt_decoder.o
gcc main.cpp pkt_decoder.a 
