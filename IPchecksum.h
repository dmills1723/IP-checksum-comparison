#include <stdio.h>

int    processArgs();
int    defaultSum();
int    linuxSum();
int    deferredCarries();
int    loopUnwinding();
FILE * openFile();
void   usage();
void   testAll();

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

// Value indicating to run a comparison of all strategies. Returned from
// processArgs(), it indicates that testAll() should be run.
#define TEST_ALL_IDX 500

#define NUM_TEST_RUNS 100
