mkdir -p ./new

for file in `ls *.jpg`; do
  ffmpeg -i $file -vf scale=iw*.25:-1 ./new/$file
done
