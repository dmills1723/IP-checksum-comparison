#include <stdio.h>      // mmap()
#include <stdlib.h>     // exit()
#include <string.h>     // strcmp()
#include <getopt.h>     // getopt()
#include <inttypes.h>   // uint16_t
#include <unistd.h>     // sysconf()
#include <sys/stat.h>   // stat(), struct stat
#include <sys/mman.h>   // mmap()
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

    // Reads index in STRATEGIES of supplied argument. Exits within getStrategy() 
    // if command-line arguments don't specify a checksum strategy and filename.
    int stratIdx = processArgs( argc, argv, inFile);

    // Attemps to open a file from one of the command-line arguments.
    inFile = openFile( argc, argv );

    // Page size, in bytes for the host. Used to read input in multiples of the page size.
    pageSize = sysconf( _SC_PAGESIZE );

    // Calls function associated with strategy.
    switch ( stratIdx ) {
        case 0 :
            defaultSum( inFile, filesize );
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
    @return Pointer to opened input file 
*/
FILE * openFile( int argc, char * argv[] ) {
    FILE * temp;
    int idx; 
    if ( argc == 2 ) {
        idx = 1;
    } else if ( argc == 4 ) {
        idx = 3;
    } else {
        usage();
    } 

    // Attempts to open file.
    temp = fopen( argv[ idx ], "rb" );
    if ( temp == NULL ) {
        usage();
    }
    return temp;
}

/**
    Prints the USAGE and HELP messages specified in IPChecksum.h, then exits.
*/
void usage() {
    printf( "%s\n%s\n", USAGE, HELP );
    if ( inFile != NULL )
        fclose( inFile );
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
                if ( getopt( argc, argv, "f:hls:" ) != -1 ) 
                    usage();

                if ( argc == 4 ) {
                    filesize = getFilesize( argv[ 3 ]);
                } else {
                    usage();
                }
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
                printf( " made it here ") ;
                // No strategy was specified, but argv[1] might be a valid input
                // file, so the index of "DEFAULT" in STRATEGIES is returned.
                if ( argc == 2 ) {
                    printf( "%s\n", argv[ 1 ] );
                    inFile = fopen( argv[ 1 ], "rb" );
                    if ( inFile == NULL )
                        usage();
                    filesize = getFilesize( argv[ 1 ]);
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
    Give a path name, returns the file's size in bytes, as a size_t type.

    @param filename Name of file
    @return size of file, as a size_t
*/
size_t getFilesize( const char * filename ) {
    struct stat st;
    stat( filename, &st );
    return st.st_size;
}


/**
    A basic implementation of the checksum algorithm using fread() to
    buffer chunks of data and end-arround-carry on every addition.

    @param inFile:   Pointer to the input file.
    @param filesize: Size of the input file, in bytes
    @return The calculated checksum
*/
int defaultSum( FILE * inFile, size_t filesize ) {

    // Determines buffer size used with fread().
    int    chunkSize = pageSize * 4;
    char * buffer = malloc( chunkSize * sizeof( char ) );

    // True if the file has an odd number of bytes. Needed for handling the last byte.
    char   isOdd = filesize % 2;

    register int checksum = 0x0000;
    size_t bytesRead;
    if ( inFile == NULL ) {
        printf( "BAD POINTER!" );
        usage();

    }
    
    // Calculates the IP checksum with end-around-carries on each addition.
    while (( bytesRead = fread( buffer, sizeof( char ), chunkSize, inFile )) != 0 ) {
        if ( ( bytesRead < chunkSize ) && isOdd )
            buffer[ bytesRead ] = 0x00;

        for ( int offset = 0; offset < bytesRead; offset += 2) {
            unsigned short currentWord = ( (uint16_t) (buffer[ offset ] << 8) | 
                                           (uint16_t) (buffer[ offset + 1 ]) );
            //checksum += currentWord;
            int result = currentWord + checksum;
            checksum = ( result & 0xFFFF ) + (result >> 16);
        }
    }
    
    return ~checksum & 0xFFFF;
}
