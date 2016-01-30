#!/bin/bash
# test5.sh

./simpsh \
  --rdonly test5_input \
  --pipe \
  --pipe \
  --creat --trunc --wronly test5_output.txt \
  --creat --append --wronly test5_error.txt \
  --command 3 5 6 tr A-Z a-z \
  --command 0 2 6 sort \
  --command 1 4 6 cat b - \
  --wait 
  



