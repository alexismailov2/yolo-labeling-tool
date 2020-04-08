ln -s /content/gdrive/My\ Drive/Yolo/scripts_for_training scripts_for_training
cd scripts_for_training
rm -rf generate
cmake . -Bgenerate -DYOLO_DATASET_FOLDER_PATH=./1