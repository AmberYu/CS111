#!/bin/bash
# test5.sh

touch a.txt
echo aa >> a.txt
echo bb >> a.txt


./simpsh \
  --rdonly a.txt \
  --pipe \
  --pipe \
  --creat --trunc --wronly c.txt \
  --creat --append --wronly d.txt \
  --ignore 11 \
  --default 11 \
  --catch 11 \
  --abort \
  --command 3 5 6 tr A-Z a-z \
  --command 0 2 6 sort \
  --command 1 4 6 cat b - \
  --wait 
  



