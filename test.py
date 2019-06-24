import subprocess
import re
from os import listdir 

#########################################################################
#
# Author: Daniel Mills (@dmills1723)
# 
# This script tests all strategies implemented in IPchecksum.c
# against multiple input files. 
#
# Run as "python test.py" with Python 3.7 or higher (for f-strings).
#
# For now, manually increment "STRATEGIES_IMPLEMENTED" after updating
# IPchecksum.c.
#
#########################################################################


# Update this after making an update to IPchecksum.c 
# TODO find a better way to do this
STRATEGIES_IMPLEMENTED = 4

INPUT_DIR         = "./input"
EXPECTED_OUT_PATH = "expected_out.txt"
INITIAL_ARGS      = [ "./IPchecksum", "-s" ]

# Codes for colored terminal text. Used for pass/fail messages.
RED_TXT_PREFIX   = '\033[91m'
GREEN_TXT_PREFIX = '\033[32m'
TXT_SUFFIX       = '\033[0m'

current_args  = []
current_args += INITIAL_ARGS
input_files   = listdir( "input" )

# Opens a file containing expected checksum values for a list of input files.
with open( EXPECTED_OUT_PATH , "r" ) as expout_file :

    # Iterates through lines of "expected_out.txt".
    # Each line is formatted as "<filename>:<expected_sum>".
    for line in expout_file.readlines() :

        # Stores a file path and its expected checksum for one test.
        curr_input_file = line.split( ':' )[ 0 ]
        expected_sum    = line.split( ':' )[ 1 ][:-1]

        # For each strategy implemented, runs a test against "curr_input_file".
        for i in range( STRATEGIES_IMPLEMENTED ) :

            # Builds cmdline argument list for a single test.
            current_args.clear()
            current_args.extend( INITIAL_ARGS )
            current_args.append( str( i ) )
            current_args.append( f"{INPUT_DIR}/{curr_input_file}" )

            test_run = subprocess.run( current_args, capture_output=True, text=True)
            
            # Searches for the checksum in hex then stores it in "actual_sum".
            output = re.search( r'(Hex:\s+)([A-F0-9]+)(\s)', test_run.stdout )
            actual_sum = output.group( 2 )

            # Prints expected and actual output of test.
            print( '________________________________________________________\n' ) 
            print( f"args = {current_args}\n" )

            if ( expected_sum == actual_sum ) :
                # Prints green-colored message on successful test.
                print( f"{GREEN_TXT_PREFIX}\t\t\tTEST PASSED! {TXT_SUFFIX}\n" )
            else :
                # Prints red-colored message on failed test.
                print( f"{RED_TXT_PREFIX}\t\t\tTEST FAILED! {TXT_SUFFIX}\n" )

            print( f"EXPECTED: {expected_sum}" )
            print( f"ACTUAL:   {actual_sum}  " )

