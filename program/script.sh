#!/bin/bash

if [ $# -ne 3 ]
then
  echo "usage $0: key value1 value 2"
  exit
fi

key=$1
value=$2
other=$3

while true; do
  ./test set $key $value
  returned=$(./test get $key)
  if [ "$returned" == "$value" ] || [ "$returned" == "$other" ]
  then
    echo "OK"
  else
    echo "FAIL"
    exit
  fi
done
