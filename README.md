# packet_decoder

To compiler :  
              Use compile.sh which will compile the packet decoder in to a static library and will link to the test/ main.cpp

assumptions made : 
              anything other than STX (0x02), ETX (0x03) & Escape character (0x10) cannot be escaped and treated a invalid packet.

