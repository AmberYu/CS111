#!/bin/bash
# test3.sh

touch t3_input.txt
echo A > t3_input.txt
touch t3_output.txt
touch t3_error.txt

./simpsh \
  --rdonly t3_input.txt \
  --wronly t3_output.txt \
  --wronly t3_error.txt \
  --command 0 1 2 tr A a
