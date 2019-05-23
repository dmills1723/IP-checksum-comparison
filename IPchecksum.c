#include <stdlib.h>     // exit()
#include <string.h>     // strcmp()
#include <getopt.h>     // getopt()
#include <inttypes.h>   // uint16_t
#include <unistd.h>     // sysconf()
#include <sys/stat.h>   // stat(), struct stat
#include "IPchecksum.h" // sanity checking

/**
    @file   IPchecksum.c
    @author Daniel Mills 

    See README.md for project info.
    
    This file defines several implementations of the IP checksum for comparison.
*/

// Array of function pointers. Stores the implemented checksum strategies.
int ( * strategies[ 2 ] ) ( unsigned char * buffer, int fileSize ) = 
    { defaultSum, linuxSum };

// Number of currently implemented checksumming strategies. 
int NUM_STRATEGIES = sizeof( strategies ) / sizeof( strategies[ 0 ] );

/**
    Main method. Reads command-line arguments. If the arguments specify an
    input file and a calculation strategy to use, the checksum is calculated
    with the result and elapsed time printed.

    @param  argc: Number of command-line arguments
    @param  argv: String array of command-line arguments
    @return program's exit status
*/
int main( int argc, char * argv[] ) {

    // Pointer to the size of the input file, in bytes.
    size_t fileSize; 

    // Checks that CL args are valid. Returns index of specified checksum strategy.
    int stratIdx = processArgs( argc, argv );

    // Attemps to open a file from one of the command-line arguments.
    // Stores size of file in bytes in "fileSize".
    FILE * inFile = openFile( argc, argv, &fileSize );

    // Reads entire file into a buffer.
    unsigned char * buffer = calloc( fileSize, sizeof( char ) );
    fread( buffer, sizeof( char ), fileSize, inFile );

    // If an invalid index is given for a strategy, exits with message.
    if ( stratIdx >= NUM_STRATEGIES ) {
        printf( "\nInvalid strategy specified.\n\n" );
        exit( 1 );
    }

    // Calls checksum function for specified strategy. 
    int checksum = ( *strategies[ stratIdx ] )( buffer, fileSize );

    // Prints calculated checksum. 
    printf( "Checksum\n\tDec: %5d\n\tHex: %5X\n", checksum, checksum );
}

/**
    Attemps to open a specified input file from the command-line arguments.
    On a successful open, a FILE pointer is returned and "fileSize" is 
    determined. On failure, the program exits.

    @param argc: Number of command-line arguments.
    @param argv: String array of command-line arguments
    @return Pointer to opened input file 
*/
FILE * openFile( int argc, char * argv[], size_t * fileSize ) {
    // Sets index of where in argv the pathname might be.
    int idx; 
    // If no strategy is specified, filename will be here.
    if ( argc == 2 ) {
        idx = 1;
    // If a strategy is specified, filename will be in 3rd position.
    } else if ( argc == 4 ) {
        idx = 3;
    } else {
        usage();
    } 

    // Attempts to open file.
    FILE * inFile = fopen( argv[ idx ], "rb" );
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
    Processes the command-line arguments. If a strategy and input file were specified,
    the index of the associated strategy in STRATEGIES[] is returned.

    @param argc: Number of command-line arguments
    @param argv: String array of command-line arguments
*/
int processArgs( int argc, char * argv[] ) {

    // Bitmask specifying that only 2 or 4 are valid "argc" values 
    // Note that "0x14" is "0001 0100".
    char validArgCounts = 0x14;
    //char validArgCounts = 0b00010100;
    char mask = 1;
    
    // Prints USAGE string and exits if bad number of arguments provided.
    if ( ( ( mask << argc ) & validArgCounts ) == 0 )
        usage();

    // Switches on the command-line arguments and proceeds accordingly.
    int option;
    while (( option = getopt( argc, argv, "f:hls:" )) != -1 ) {
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

            // Help message. Falls through to default.
            case 'h' :
            default :
                if ( argc != 2 )
                    usage();
                break;

        } // switch
    } // while
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
    register int checksum = 0x0000;

    // If the file has an odd number of bytes, a zero byte is appended.
    if ( bufferSize & 1 )
        buffer[ bufferSize ] = 0x00;

    // Calculates the checksum
    unsigned int result;
    for ( int offset = 0; offset <= bufferSize ; offset += 2) {
        uint16_t currentWord = ( (uint16_t) (buffer[ offset ] << 8) | 
                                 (uint16_t) (buffer[ offset + 1 ]) );
        result = currentWord + checksum;
        checksum = ( result & 0xFFFF ) + (result >> 16);
    }

    free( buffer );
    
    return ~checksum & 0xFFFF;
}

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
    Additionally, 32-bit words are summed per iteration.
    ( see section 1C, "Parallel Summation" in RFC 1071 )

    @param  buffer     Data to compute checksum of.
    @param  bufferSize Number of bytes in "buffer"
    @return The calculated checksum
*/
int linuxSum( unsigned char * buffer, int bufferSize ) {
    unsigned int result = 0;

    if (bufferSize >= 4) {
        const unsigned char *end = buffer + ((unsigned) bufferSize & ~3);
        unsigned int carry = 0;
        do {
            unsigned int w = *(unsigned int *) buffer;
            buffer += 4;
            result += carry;
            result += w; 
            carry = (w > result);
        } while (buffer < end);
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

//int deferredCarries( FILE *inFile, int fileSize )

