#!/bin/bash
#test5_bash.sh

(sort < test5_input | cat b - | tr A-Z a-z > test5_output.txt) 2>> test5_error.txt
wait
wait
wait
