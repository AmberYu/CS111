#!/bin/bash
# test1.sh

touch t1_input.txt
echo 1 >> t1_input.txt
echo 3 >> t1_input.txt
echo 2 >> t1_input.txt
echo 6 >> t1_input.txt
# touch t1_output.txt
# touch t1_error.txt

./simpsh \
  --rdonly t1_input.txt \
  --creat --trunc --wronly t1_output.txt \
  --creat --append --wronly t1_error.txt \
  --command 0 1 2 sort