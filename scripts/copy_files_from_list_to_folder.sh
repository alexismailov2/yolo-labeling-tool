#!/bin/bash
FILE=$1
count=$3
out_dir=$2
mkdir $out_dir
while read LINE; do
     item=$(printf "%06d" $count)
     cp $LINE $2/$item.jpg
     substring=".jpg"
     replacement=".txt"
     annotation="${LINE//.jpg/.txt}"
     echo $annotation
     cp $annotation $2/$item.txt
     ((count--))
     if [ $count -eq 0 ]
     then
       break
     fi
done < $FILE