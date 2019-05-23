# IP-checksums-C-v-Python
Calculating the IP checksum as defined in RFC1071 

The "Internet checksum" is a checksum algorithm used in TCP, IP, and UDP
to ensure reliable data transfer (no bit errors!). It is defined as the
"16-bit one's complement of the one's complement sum of all 16-bit words"
over either the header (IP) or both the header and payload of a packet (TCP & UDP), 
with a pseudoheader of "0x0000" for the checksum header. For the purpose of this
comparison, the checksum will be calculated on an input file of sufficient size
to reveal effects on runtime.

The following strategies for computation are implemented:
0. Default implementation
1. Deferred Carries
2. Unwinding Loops
3. Combine with Data Copying
4. Incremental Update

The strategies used are recommended in RFC 1071(https://tools.ietf.org/html/rfc1071).

**Example usage:**

To list checksum strategies:
        `./IPChecksum -l`
        
To print help info: 
        `./IPChecksum -h`
        
To calculate checksum with the default strategy:
        `./IPChecksum -s 0 input/file_small.txt`
        

