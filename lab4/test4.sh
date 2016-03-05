#!/bin/bash
# test4.sh

./simpsh \
  --rdonly t4_input \
  --wronly t4_output.txt \
  --wronly t4_error.txt \
  --command 0 1 2 sort \
  

