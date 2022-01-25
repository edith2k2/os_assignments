#! /bin/bash
num=$1
i=2
while [ $i -le $num ];do
while [ $(($num % i)) -eq 0 ];do
echo -n "$i "
num=$((num / i));done;
i=$(( $i + 1 ));done;
