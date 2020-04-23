#!/bin/bash
> numbers.txt
for (( count=1; count<151; count++ ))
do
echo $RANDOM >> numbers.txt
done
