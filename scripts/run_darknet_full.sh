mkdir -p ./detect_result

i=0
for file in `ls $1/*.png`; do
  ./darknet detector test ./yolov3.data ./fullyolov3/yolov3.cfg ./fullyolov3/yolov3.weights $file -i 0 -tresh 0.25 -dont_show -save_labels
  item=$(printf "%06d" $i)
  ((i+=1))
  mv ./predictions.jpg ./detect_result/$item.jpg
done

