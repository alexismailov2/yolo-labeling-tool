mkdir -p ./new

i=$1
for file in `ls *.txt`; do
  item=$(printf "%06d" $i)
  ((i+=1))
  mv ./${file} ./new/$item.txt
done

i=$1
for file in `ls *.png`; do
  item=$(printf "%06d" $i)
  ((i+=1))
  mv ./${file} ./new/$item.png
done

