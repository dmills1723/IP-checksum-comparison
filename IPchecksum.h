#include <stdio.h>

/*          FUNCTION DECLARATIONS           */
int    processArgs();
int    defaultSum();
void   openFile();
void   usage();

/*            STATIC CONSTANTS              */

// Available strategies that can be specified for the checksum calculation.
const char * STRATEGIES[] = { "FREAD_EAC", "DEFERRED_CARRY", "UNWINDING_LOOPS", "MINIMAL_COPYING", "INCREMENTAL_UPDATE" } ; 
typedef enum { FREAD_EAC, DEFERRED_CARRY, UNWINDING_LOOPS, MINIMAL_COPYING, INCREMENTAL_UPDATE } Strategy ;
// Terse details about command-line options.
const char * USAGE = "USAGE: \"./IPchecksum [-h] [-l] [-s checksum strategy] INPUT_FILE\"";
// Verbose details about command-line options.
const char * HELP  = "\t-h: help\n"
                     "\t-l: list checksum strategies\n"
                     "\t-s: specify checksum strategy\n";
