#include <stdlib.h>     // exit()
#include <string.h>     // strcmp()
#include <getopt.h>     // getopt()
#include <inttypes.h>   // uint16_t
#include <unistd.h>     // sysconf()
#include <sys/stat.h>   // stat(), struct stat
#include <time.h>       // clock()
#include "IPchecksum.h" // sanity checking

/**
    @file   IPchecksum.c
    @author Daniel Mills 

    See README.md for project info.
    
    This file defines several implementations of the IP checksum for comparison.
*/

// Array of function pointers. Stores the implemented checksum strategies.
int ( * strategies[ 4 ] ) ( unsigned char * buffer, int fileSize ) = 
    { &defaultSum, &linuxSum, &deferredCarries, &loopUnwinding };

// Number of currently implemented checksumming strategies. 
int NUM_STRATEGIES = sizeof( strategies ) / sizeof( strategies[ 0 ] );

/**
    Main method. Reads command-line arguments. If the arguments specify an
    input file and a calculation strategy to use, the checksum is calculated
    with the result and elapsed time printed.
*/
int main( int argc, char * argv[] ) {
    // Valid argcounts are 2-4.
    if ( argc <= 1 || argc >= 5 )   
        usage();

    // Checks that command-line args are valid. 
    // Returns index of specified checksum strategy.
    int stratIdx = processArgs( argc, argv );


    // Pointer to the size of the input file, in bytes.
    size_t fileSize; 

    // Attemps to open a file from one of the command-line arguments.
    // Stores size of file in bytes in "fileSize".
    FILE * inFile = openFile( argc, argv, &fileSize );

    // If the file has odd number of bytes, "fileSize" is incremented so the
    // buffer is allocated an even number of bytes. 
    if ( fileSize & 1 )
        fileSize++;

    // Reads entire file into "buffer". The buffer is initialized with all 0s.
    // If "fileSize" was originally even, the file will completely occupy the 
    // buffer. If "fileSize" was originally odd (and then incremented 1), the
    // file will occupy all but the least byte, which will have a value of 0.
    // The appending of a zero-byte to odd-sized checksum payloads is part of 
    // the calculation defined in RFC 1071.
    unsigned char * buffer = calloc( fileSize, sizeof( char ) );
    fread( buffer, sizeof( char ), fileSize, inFile );

    if ( stratIdx == TEST_ALL_IDX ) {
        testAll( buffer, fileSize );
    }

    // If an invalid index is given for a strategy, exits with message.
    if ( stratIdx >= NUM_STRATEGIES ) {
        printf( "\nInvalid strategy specified.\n\n" );
        exit( 1 );
    }


    // Calls checksum function for specified strategy with timer.
    clock_t startTime = clock();

    int checksum = ( *strategies[ stratIdx ] )( buffer, fileSize );

    clock_t endTime = clock();

    double elapsedTime = ( endTime - startTime ) / ( double ) CLOCKS_PER_SEC;

    // Prints calculated checksum. 
    printf( "Dec:\t%-5d\n", checksum );
    printf( "Hex:\t%-5X\n", checksum );
    printf( "Time:\t%.6f\n", elapsedTime );


    free( buffer );
}

/**
    Attemps to open a specified input file from the command-line arguments.
    On a successful open, a FILE pointer is returned and "fileSize" is 
    determined. On failure, the program exits.

    @return Pointer to opened input file 
*/
FILE * openFile( int argc, char * argv[], size_t * fileSize ) {

    // Attempts to open file.
    FILE * inFile = fopen( argv[ argc - 1 ], "rb" );
    if ( inFile == NULL ) 
        usage();

    // Calculates size of file.
    fseek( inFile, 0L, SEEK_END );
    *fileSize = ftell( inFile );
    rewind( inFile );

    return inFile;
}

/**
    Prints the USAGE and HELP messages defined in IPChecksum.h, then exits.
*/
void usage() {
    printf( "\n%s\n%s\n", USAGE, HELP );
    exit( 0 );
}

/**
    Processes the command-line arguments. If a strategy and input file were 
    specified, the index of the associated strategy in STRATEGIES[] is returned.

    @return Index in "strategies" of desired checksum strategy
*/
int processArgs( int argc, char * argv[] ) {

    // Bitmask specifying that only 2-4 are valid "argc" values 
    char validArgCounts = 0b00011100;
    char mask = 1;
    
    // Prints USAGE string and exits if bad number of arguments provided.
    if ( ( ( mask << argc ) & validArgCounts ) == 0 )
        usage();

    // Switches on the command-line arguments and proceeds accordingly.
    int option;
    while (( option = getopt( argc, argv, "ahls:" )) != -1 ) {
        switch ( option ) {

            // Selects a strategy from those implemented.
            case 's' :
                if ( argc != 4 )
                    usage();

                int strategy = atoi( argv[ 2 ] );
                if ( strategy > NUM_STRATEGIES )
                    usage();

                return strategy;

            // Lists strategies. 
            case 'l' :
                printf( "\nAvailable checksum strategies:\n" );
                for ( int i = 0; i < NUM_STRATEGIES; i++ ) 
                    printf( "\t( %d ) %s\n", i, STRATEGIES[ i ] );
                printf( "\n" );
                if ( argc != 2 ) 
                    usage();
                exit( 0 );

            case 'a' :
                return TEST_ALL_IDX;

            // Help message. Falls through to default.
            case 'h' :
            default :
                if ( argc != 2 )
                    usage();
                break;

        } // switch
    } // while

    // Default option for no supplied arguments other than an input file.
    return 0;
}


/**
    A basic implementation of the checksum algorithm using fread() to
    buffer chunks of data and end-arround-carry on every addition.

    @param  buffer     Data to compute checksum of.
    @param  bufferSize Number of bytes in "buffer"
    @return The calculated checksum
*/
int defaultSum( unsigned char * buffer, int bufferSize ) {
    // TODO Determine whether checksum needs to be 32 bits.
    register int checksum = 0;

    // Calculates the checksum
    unsigned int result;
    for ( int offset = 0; offset <= bufferSize ; offset += 2) {
        uint16_t currentWord = ( (uint16_t) (buffer[ offset ] << 8) | 
                                 (uint16_t) (buffer[ offset + 1 ]) );
        result = currentWord + checksum;
        checksum = ( result & 0xFFFF ) + (result >> 16);
    }

    return ~checksum & 0xFFFF;
}

/**
    Helper method used in linuxSum(). In the Linux kernel implementation, 
    16-bit words are added two at a time (two per 32-bit int). This function
    sums the left and right 16-bit words and then the sum's resultant carry.

    @param  x 32-bit intermediate checksum
    @return Final 16-bit checksum
*/
static inline unsigned short from32to16(unsigned int x)
{
    /* add up 16-bit and 16-bit for 16+c bit */
    x = (x & 0xffff) + (x >> 16);

    /* add up carry.. */
    x = (x & 0xffff) + (x >> 16);
    return x;
}

/**
    A reference implementation, borrowed from lib/checksum.c in the Linux 
    kernel source code (version 5.0.8). Some modifications have been made.

    Their implementation adds words in reverse order and then swaps the bytes.
    ( see section 1B, "Byte Order Independence" in RFC 1071 )
    Additionally, 32-bit words are summed per iteration. i.e. Two 16-bit words,
    side-by-side in a 32-bit int, are added to the sum.
    ( see section 1C, "Parallel Summation" in RFC 1071 )

    @param  buffer     Data to compute checksum of.
    @param  bufferSize Number of bytes in "buffer"
    @return The calculated checksum
*/
int linuxSum( unsigned char * buffer, int bufferSize ) {
    unsigned int result = 0;

    if (bufferSize >= 4) {
        // Marks the offset into the buffer of the last 4 bytes. 
        const unsigned char *end = buffer + ((unsigned) bufferSize & ~3);
        unsigned int carry = 0;

        // Adds 32-bits at a time, storing overflows in "carry".
        do {
            unsigned int w = *(unsigned int *) buffer;
            buffer += 4;
            result += carry;
            result += w; 
            carry = (w > result);
        } while (buffer < end);

        // Adds the accumulated carrys back into the sum.
        result += carry; 
        result = (result & 0xffff) + (result >> 16);
    }

    if (bufferSize & 2) {
        result += *(unsigned short *) buffer;
        buffer += 2;
    }

    if (bufferSize & 1)
        result += *buffer;

    result = from32to16(result);
    result = ((result >> 8) & 0xff) | ((result & 0xff) << 8);

    return ~result & 0xFFFF;
}

/**
    Computes the checksum but defers the end-around carries until all words
    have been summed. The addition overflows accumulate in the high-order 
    bits of "sum" and are added at the end. This halves the number of additions 
    per iteration, but doubles the total number of iterations, compared to the 
    Parallel Summation strategy described in section 1C of RFC 1071, used 
    in linuxSum().

    @param  buffer     Data to compute checksum of.
    @param  bufferSize Number of bytes in "buffer"
    @return The calculated checksum
*/
int deferredCarries( unsigned char * buffer, int bufferSize ) {
    register int sum = 0;

    // Iterates over buffer by 16-bit words, adding to checksum.
    for ( int offset = 0; offset < bufferSize; offset += 2 ) {
        sum += ( (uint16_t) (buffer[ offset ] << 8) | 
                 (uint16_t) (buffer[ offset + 1 ]) );
    }

    // Adds accumulated carries from high-order bits.
    while ( sum >> 16 ) {
        sum = ( sum & 0xFFFF ) + ( sum >> 16 );
    }

    return ~sum & 0xFFFF;
}

/**
    Unwinds the main checksum loop such that (BYTES_PER_LOOP / 2) additions 
    are made for each iteration.

    First the buffer is extended to a multiple of BYTES_PER_LOOP.
    The buffer is then extended using realloc() to the new buffer size.
    The new bytes in the buffer are all initialized to 0.

    NOTES:
    - Setting BYTES_PER_LOOP to 2 is equivalent to the default behavior.
    - realloc() will attempt to extend a block of memory to the specified size.
      If this isn't possible, it will allocate a new block and copy over the
      contents of the old block. If this happens, a message is printed. The 
      tests should be rerun if you see this message.

    @param  buffer     Data to compute checksum of.
    @param  bufferSize Number of bytes in "buffer"
    @return The calculated checksum
*/
int loopUnwinding( unsigned char * buffer, int bufferSize ) {
    // Increase this and add additions in the loop below for improved efficiency.
    // The number of words added per loop is BYTES_PER_LOOP / 2.
    short BYTES_PER_LOOP = 16;

    // Increase the buffer size to be a multiple of BYTES_PER_LOOP.
    int bytesToMultiple = 0;
    if ( bufferSize % BYTES_PER_LOOP ) {
        // Bytes needed to add to round bufferSize up to a multiple 
        // of BYTES_PER_LOOP.
        int bytesToMultiple = BYTES_PER_LOOP - ( bufferSize % BYTES_PER_LOOP );
    
        bufferSize += bytesToMultiple;
    }

    // Stores the current pointer of the buffer for comparison after realloc().
    // If it changes, then realloc() has allocated a new block (costly!).
    unsigned char * origLocation = buffer;

    // Extend the buffer to new size.
    buffer = ( unsigned char * ) realloc( buffer, bufferSize );
    memset( buffer + bufferSize - bytesToMultiple, 0, bytesToMultiple );

    // If the realloc() call allocated a new block instead of extending 
    // the old one, we print to the user, but continue as normal. 
    //if ( origLocation != buffer )
    if( origLocation != buffer )
        printf( "\nrealloc() moved buffer! This test run should be ignored!\n"
                "See IPchecksum.loopUnwinding() for more information.\n\n");
    
    register int sum = 0;

    for ( int offset = 0; offset < bufferSize; offset += BYTES_PER_LOOP ) {
        // One's complement addition: 1st word
        sum += ( (uint16_t) (buffer[ offset ] << 8) | 
                 (uint16_t) (buffer[ offset + 1 ]) );

        // One's complement addition: 2nd word
        sum += ( (uint16_t) (buffer[ offset + 2 ] << 8) | 
                 (uint16_t) (buffer[ offset + 3 ]) );

        // One's complement addition: 3rd word
        sum += ( (uint16_t) (buffer[ offset + 4 ] << 8) | 
                 (uint16_t) (buffer[ offset + 5 ]) );

        // One's complement addition: 4th word
        sum += ( (uint16_t) (buffer[ offset + 6 ] << 8) | 
                 (uint16_t) (buffer[ offset + 7 ]) );

        // One's complement addition: 5th word
        sum += ( (uint16_t) (buffer[ offset + 8 ] << 8) | 
                 (uint16_t) (buffer[ offset + 9 ]) );

        // One's complement addition: 6th word
        sum += ( (uint16_t) (buffer[ offset + 10 ] << 8) | 
                 (uint16_t) (buffer[ offset + 11 ]) );

        // One's complement addition: 7th word
        sum += ( (uint16_t) (buffer[ offset + 12 ] << 8) | 
                 (uint16_t) (buffer[ offset + 13 ]) );

        // One's complement addition: 8th word
        sum += ( (uint16_t) (buffer[ offset + 14 ] << 8) | 
                 (uint16_t) (buffer[ offset + 15 ]) );

        sum = ( sum & 0xFFFF ) + ( sum >> 16 );
    }

    return ~sum & 0xFFFF;


}


/**
    Tests all implemented strategies and prints a table for comparison.
    After printing the table, the program exits.

    @param  buffer     Data to compute checksum of.
    @param  bufferSize Number of bytes in "buffer"
*/
void testAll( unsigned char * buffer, int bufferSize ) {
    // Prints table header.
    printf( "     STRATEGY                  ELAPSED TIME (s)\n");
    printf( "-----------------------------------------------\n");

    // Calculates checksum for each strategy NUM_TEST_RUNS times.
    for ( int stratIdx = 0; stratIdx < NUM_STRATEGIES; stratIdx++ ) {
        double totalElapsedTime = 0;

        for ( int run = 0; run < NUM_TEST_RUNS; run++ ) {
            clock_t startTime = clock();
            
            // Calls checksum function for specified strategy. 
            ( *strategies[ stratIdx ] )( buffer, bufferSize );

            clock_t endTime = clock();
            double elapsedTime = ( endTime - startTime ) / ( double ) CLOCKS_PER_SEC;
            totalElapsedTime += elapsedTime;
        }

        // Prints current strategy's results.
        printf( "%-35s%.6f\n", STRATEGIES[ stratIdx ], totalElapsedTime / NUM_TEST_RUNS );
    }

    exit( 0 );
}
