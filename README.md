# packet_decoder

To compiler :  
              Use compile.sh which will compile the packet decoder in to a static library and will link to the test/ main.cpp

assumptions made : 
              anything other than STX (0x02), ETX (0x03) & Escape character (0x10) cannot be escaped and treated a invalid packet.

Compiler : GCC , Please have a look at the GCC version info below, 
           had not got a chance to compile on any other platform or version of GCC to check if it works fine with.

gcc -v

Configured with: --prefix=/Applications/Xcode.app/Contents/Developer/usr --with-gxx-include-dir=/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.14.sdk/usr/include/c++/4.2.1
Apple LLVM version 10.0.1 (clang-1001.0.46.4)
Target: x86_64-apple-darwin18.5.0
Thread model: posix
InstalledDir: /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin
