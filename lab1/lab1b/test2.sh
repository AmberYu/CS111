#!/bin/bash
# test2.sh

touch t2_input.txt
echo 1 >> t2_input.txt
echo 3 >> t2_input.txt
echo 2 >> t2_input.txt
echo 6 >> t2_input.txt
touch t2_output.txt
touch t2_error.txt

./simpsh \
  --verbose \
  --rdonly t2_input.txt \
  --wronly t2_output.txt \
  --wronly t2_error.txt \
  --command 0 1 2 sort

