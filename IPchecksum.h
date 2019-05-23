#include <stdio.h>

/*          FUNCTION DECLARATIONS           */
int    processArgs();
int    defaultSum();
int    linuxSum();
FILE * openFile();
void   usage();

/*            STATIC CONSTANTS              */

// Available strategies that can be specified for the checksum calculation.
const char * STRATEGIES[] = { 
    "End around carries.", 
    "Linux kernel's \"lib/checksum.c\" implementation.", 
    "Deferred carries" 
} ; 
// Terse details about command-line options.
const char * USAGE = "USAGE: \"./IPchecksum [-h] [-l] [-s checksum strategy] INPUT_FILE\"";
// Verbose details about command-line options.
const char * HELP  = "\t-h: help\n"
                     "\t-l: list checksum strategies\n"
                     "\t-s: specify checksum strategy\n";
