#!/bin/bash
#test6.sh

./simpsh \
  --rdonly test6_input \
  --pipe \
  --pipe \
  --creat --trunc --wronly test6_output.txt \
  --creat --append --wronly test6_error.txt \
  --command 0 2 6 cat \
  --command 1 4 6 grep CS111 \
  --command 3 5 6 sort \
  

