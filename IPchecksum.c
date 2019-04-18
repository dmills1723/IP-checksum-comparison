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

// Number of currently implemented checksumming strategies. See IPchecksum.h.
int NUM_STRATEGIES = sizeof( STRATEGIES ) / sizeof( STRATEGIES[ 0 ] );

/**
    Main method. Reads command-line arguments. If the arguments specify an
    input file and a calculation strategy to use, the checksum is calculated
    with the result and elapsed time printed.

    @param  argc: Number of command-line arguments
    @param  argv: String array of command-line arguments
    @return program's exit status
*/
int main( int argc, char * argv[] ) {
    // Pointer to input file.
    FILE * inFile; 

    // Pointer to the size of the input file, in bytes.
    size_t fileSize; 

    // Checks that CL args are valid. Returns index of specified checksum strategy.
    int stratIdx = processArgs( argc, argv );

    // Attemps to open a file from one of the command-line arguments.
    openFile( argc, argv, &inFile, &fileSize );

    // Page size, in bytes for the host. Used to read input in multiples of the page size.
    size_t pageSize = sysconf( _SC_PAGESIZE );

    // Calls checksum function associated with strategy. Prints calculated checksum. 
    // TODO refactor this to use an array of function pointers.
    int checksum = -1;
    switch ( stratIdx ) {
        case 1 :
            checksum = defaultSum( inFile, fileSize, pageSize );
            break;
        case 2 : 
            checksum = linuxSum( inFile, (int) fileSize );
            break;
        default :
            printf( "\nInvalid strategy specified.\n\n" );
            exit( 0 );
            break;
    }
    printf( "Checksum\n\tDec: %5d\n\tHex: %5X\n", checksum, checksum );
}

/**
    Attemps to open a specified input file from the command-line arguments.
    On a successful open, a FILE pointer is returned. 
    On failure, the program exits.

    @param argc: Number of command-line arguments.
    @param argv: String array of command-line arguments
    TODO
    @return Pointer to opened input file 
*/
void openFile( int argc, char * argv[], FILE ** inFile, size_t * fileSize ) {
    // Sets index of where in argv the pathname might be.
    int idx; 
    if ( argc == 2 ) {
        idx = 1;
    } else if ( argc == 4 ) {
        idx = 3;
    } else {
        usage();
    } 

    // Attempts to open file.
    *inFile = fopen( argv[ idx ], "rb" );
    if ( *inFile == NULL ) 
        usage();

    // Calculates size of file.
    fseek( *inFile, 0L, SEEK_END );
    *fileSize = ftell( *inFile );
    rewind( *inFile );
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

    // Bitmap specifying that only 2 or 4 are valid "argc" values.
    char validArgCounts = 0x14;
    char mask = 0x01;
    
    // Prints USAGE string and exits if bad number of arguments provided.
    if ( ( ( mask << argc ) & validArgCounts ) == 0 )
        usage();

    // Switches on the specified command-line arguments and proceeds accordingly.
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

            // Lists strategies to compute the checksum.
            case 'l' :
                printf( "\nAvailable checksum strategies:\n" );
                for ( int i = 0; i < NUM_STRATEGIES; i++ ) 
                    printf( "\t( %d ) %s\n", i + 1, STRATEGIES[ i ] );
                printf( "\n" );
                if ( argc != 2 ) 
                    usage();
                exit( 0 );

            // Display's options. Falls through to default.
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

    @param inFile:   Pointer to the input file.
    @param fileSize: Size of the input file, in bytes
    TODO
    @return The calculated checksum
*/
int defaultSum( FILE * inFile, size_t fileSize, size_t pageSize ) {

    // Reads entire file into a buffer.
    unsigned char * buffer = calloc( fileSize, sizeof( char ) );
    fread( buffer, sizeof( char ), fileSize, inFile );


    // TODO Determine whether checksum needs to be 32 bits.
    register int checksum = 0x0000;

    // If the file has an odd number of bytes, a zero byte is appended.
    if ( fileSize & 1 )
        buffer[ fileSize ] = 0x00;

    // Calculates the checksum
    unsigned int result;
    for ( int offset = 0; offset <= fileSize ; offset += 2) {
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

    @param inFile:   Pointer to the input file.
    @param fileSize: Size of the input file, in bytes
    @return The calculated checksum
*/
int linuxSum( FILE * inFile, int fileSize ) {
    // Reads entire file into a buffer.
    unsigned char * buff = calloc( fileSize, sizeof( char ) );
    fread( buff, sizeof( char ), fileSize, inFile );

    unsigned int result = 0;

    if (fileSize >= 4) {
        const unsigned char *end = buff + ((unsigned) fileSize & ~3);
        unsigned int carry = 0;
        do {
            unsigned int w = *(unsigned int *) buff;
            buff += 4;
            result += carry;
            result += w; 
            carry = (w > result);
        } while (buff < end);
        result += carry; 
        result = (result & 0xffff) + (result >> 16);
    }
    if (fileSize & 2) {
        result += *(unsigned short *) buff;
        buff += 2;
    }
    if (fileSize & 1)
        result += *buff;

    result = from32to16(result);

    result = ((result >> 8) & 0xff) | ((result & 0xff) << 8);

    return ~result & 0xFFFF;
}
