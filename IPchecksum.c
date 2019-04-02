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

// Number of currently implemented checksumming strategies.
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

    // Calls function associated with strategy.
    int checksum = -1;
    switch ( stratIdx ) {
        case 0 :
            checksum = defaultSum( inFile, fileSize, pageSize );
            printf( "Checksum\n\tDec: %X\n\tHex: %d\n", checksum, checksum );
            break;
        default :
            printf( "Invalid index." );
            break;
    }
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
    Prints the USAGE and HELP messages specified in IPChecksum.h, then exits.
*/
void usage() {
    printf( "%s\n%s\n", USAGE, HELP );
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
                if ( option != -1 ) 
                    usage();

                if ( argc != 4 ) 
                    usage();

                for ( int i = 0; i < NUM_STRATEGIES; i++ ) 
                    if( strcmp( STRATEGIES[ i ], argv[ 2 ] ) == 0 ) 
                        return i;

            // Lists strategies to compute the checksum.
            case 'l' :
                printf( "Available checksum strategies:\n" );
                for ( int i = 0; i < NUM_STRATEGIES; i++ ) 
                    printf( "\t%s\n", STRATEGIES[ i ] );
                if ( argc != 2 ) 
                    usage();

            // Display's options. Falls through to default.
            case 'h' :
            default :
                if ( argc == 2 ) {
                    return 0;
                } else {
                    usage();
                }
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

    // Determines buffer size used with fread().
    int    chunkSize = pageSize;
    char * buffer = malloc( chunkSize * sizeof( char ) );

    // True if the file has an odd number of bytes. Needed for handling the last byte.
    char   isOdd = fileSize % 2;

    register int checksum = 0x0000;
    size_t bytesRead;

    // Calculates the IP checksum with end-around-carries on each addition.
    while (( bytesRead = fread( buffer, sizeof( char ), chunkSize, inFile )) != 0 ) {

        // Appends a byte of 0s after the last byte if there's an odd number. 
        if ( ( bytesRead < chunkSize ) && isOdd )
            buffer[ bytesRead ] = 0x00;

        // Calculates the checksum
        for ( int offset = 0; offset < bytesRead; offset += 2) {
            unsigned short currentWord = ( (uint16_t) (buffer[ offset ] << 8) | 
                                           (uint16_t) (buffer[ offset + 1 ]) );
            int result = currentWord + checksum;
            checksum = ( result & 0xFFFF ) + (result >> 16);
        }
    }
    
    return ~checksum & 0xFFFF;
}
