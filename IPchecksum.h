#include <stdio.h>

/*          FUNCTION DECLARATIONS           */
int    processArgs();
int    defaultSum();
int    linuxSum();
int    deferredCarries();
int    loopUnwinding();
FILE * openFile();
void   usage();
void   testAll();

/*            STATIC CONSTANTS              */

// Available strategies that can be specified for the checksum calculation.
const char * STRATEGIES[] = { 
    "End-around carries", 
    "Linux kernel implementation", 
    "Deferred carries",
    "Loop Unwinding"
} ; 
// Terse details about command-line options.
const char * USAGE = "USAGE: \"./IPchecksum [-h] [-l] [-s checksum_strategy] INPUT_FILE\"";
// Verbose details about command-line options.
const char * HELP  = "\t-a: test all strategies\n"
                     "\t-h: help\n"
                     "\t-l: list checksum strategies\n"
                     "\t-s: specify checksum strategy\n";

int TEST_ALL_IDX = 500;

// Value indicating to run a comparison of all strategies. Returned from
// processArgs(), it indicates that testAll() should be run.
int NUM_TEST_RUNS = 100;
