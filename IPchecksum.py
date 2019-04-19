#!/usr/bin/env python3
import sys

'''
    Iterates through the paylod of a packet by 16-bit words and calculates 
    the "internet checksum" as specified in RFC 1071. 

    @param  message: payload of packet as a "bytes" object
    @return 16-bit checksum as an integer
'''
def calcChecksum( message ) :

    # Zeroes out the checksum to "0x0000".
    checksum = 0 

    # Number of bytes in message.
    msgLen = len( message )

    # This is 1 (true) if the message has an odd number of bytes.
    isOddLength = msgLen % 2
        
    # Skips last 1 or 2 bytes depending on if msgLen is odd or even.
    twoLess = msgLen - 2

    # Iterates through all 16-bit words, adding each to the current sum.
    for i in range( 0, twoLess , 2) :
        intValueOfWord = ( message[ i ] << 8 ) + message[ i + 1 ]
        result = checksum + intValueOfWord
        checksum = ( result & 0xFFFF ) + ( result >> 16 )

    # If the message has an odd number of bytes, the last byte is padded
    # with 8 zeroes and added as the resultant 16-bit word to the checksum.
    if ( isOddLength ) :
        intValueOfWord = message[ msgLen - 1] << 8
        result = checksum + intValueOfWord
        checksum = ( result & 0xFFFF ) + ( result >> 16 )
        
    # If the message has an even number of bytes, the last two bytes are added normally.
    else :
        intValueOfWord = ( message[ msgLen - 2] << 8 ) + message[ msgLen - 1]
        result = checksum + intValueOfWord
        checksum = ( result & 0xFFFF ) + ( result >> 16 )

    return ~checksum & 0xFFFF

inFile = open( sys.argv[ 1 ],"rb")
print( "Checksum: %X" %calcChecksum( inFile.read() ) )
