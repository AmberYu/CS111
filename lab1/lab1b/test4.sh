#!/bin/bash
# test4.sh


touch t4_input.txt
echo 1 >> t4_input.txt
echo 3 >> t4_input.txt
echo 2 >> t4_input.txt
echo 6 >> t4_input.txt
# touch t1_output.txt
# touch t1_error.txt

./simpsh \
  --rdonly t4_input.txt \
  --pipe \
  --creat --trunc --wronly t4_output.txt \
  --creat --append --wronly t4_error.txt \
  --command 0 2 4 sort \
  --command 1 3 4 tr 1 a \
  --pause \
  --wait
