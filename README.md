# IP-checksum-comparison
Calculating the IP checksum as defined in [RFC 1071](https://tools.ietf.org/html/rfc1071)

The "Internet checksum" is a checksum algorithm used in TCP, IP, and UDP
to ensure reliable data transfer (no bit errors!). It is defined as the
"16-bit one's complement of the one's complement sum of all 16-bit words"
over either the header (IP) or both the header and payload of a packet (TCP & UDP), 
with a pseudoheader of "0x0000" for the checksum header. For the purpose of this
comparison, the checksum will be calculated on an input file of sufficient size
to reveal effects on runtime.

The following strategies for computation are implemented:
0. Default Implementation
1. Deferred Carries
2. Linux Reference (lib/checksum.c)
3. Unwinding Loops

The following strategies might be implemented in the future:
- Combine with Data Copying
- Incremental Update
- Map Reduce (multi-threaded - 2A)
- Multi-Word Parallel Addition (2C)

These strategies are recommended in [RFC 1071](https://tools.ietf.org/html/rfc1071).

**Example usage:**

To list checksum strategies:

```./IPchecksum -l```
        
To print help info: 

```./IPchecksum -h```
        
To calculate checksum with the default strategy:

```./IPchecksum -s 0 input/file_small.txt```
        
To run a test of all implemented strategies (if you're just passing through, this is what you should run):

```./IPchecksum -a input/file_medium.txt```
