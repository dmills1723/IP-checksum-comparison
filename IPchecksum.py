
'''
    @file IPChecksum.py

'''

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
    msg_len = len( message )

    # This is 1 (true) if the message has an odd number of bytes.
    is_odd_length = msg_len % 2
        
    # Skips last 1 or 2 bytes depending on if msg_len is odd or even.
    two_less = msg_len - 2

    # Iterates through all 16-bit words, adding each to the current sum.
    for i in range( 0, two_less , 2) :
        intValueOfWord = ( message[ i ] << 8 ) + message[ i + 1 ]
        result = checksum + intValueOfWord
        checksum = ( result & 0xFFFF ) + ( result >> 16 )

    # If the message has an odd number of bytes, the last byte is padded
    # with 8 zeroes and added as the resultant 16-bit word to the checksum.
    if ( is_odd_length ) :
        intValueOfWord = message[ msg_len - 1]
        result = checksum + intValueOfWord
        checksum = ( result & 0xFFFF ) + ( result >> 16 )
        
    # If the message has an even number of bytes, the last two bytes are added normally.
    else :
        intValueOfWord = ( message[ msg_len - 2] << 8 ) + message[ msg_len - 1]
        result = checksum + intValueOfWord
        checksum = ( result & 0xFFFF ) + ( result >> 16 )

    return ~checksum & 0xFFFF

inFile = open("file_huge.txt","rb")
print( calcChecksum( inFile.read() ) )
