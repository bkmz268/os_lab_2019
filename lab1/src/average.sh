#!/bin/bash
sum = 0
for varaible in `cat numbers.txt`
do
let "sum += varaible/150"
done
echo "$sum"
