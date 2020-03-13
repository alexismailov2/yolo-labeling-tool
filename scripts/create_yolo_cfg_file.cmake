if (NOT YOLO_BATCH)
   set(YOLO_BATCH 64)
endif()

if (NOT YOLO_SUBDIVISIONS)
   set(YOLO_SUBDIVISIONS 16) #8
endif()

if (NOT YOLO_MODEL_NAME)
   message(WARNING Needed to be set YOLO_MODEL_NAME)
   return()
endif()

# calculate classes count
file(STRINGS ${YOLO_DATASET_FOLDER_PATH}/_.names YOLO_CLASSES)
list(LENGTH YOLO_CLASSES YOLO_CLASSES_COUNT)
message(STATUS YOLO_CLASSES_COUNT=${YOLO_CLASSES_COUNT})

math(EXPR YOLO_FILTER_COUNT "(${YOLO_CLASSES_COUNT} + 5) * 3")
message(STATUS YOLO_FILTER_COUNT=${YOLO_FILTER_COUNT})

math(EXPR YOLO_MAX_BATCHES "4000 + (2000 * ${YOLO_CLASSES_COUNT})")
message(STATUS YOLO_MAX_BATCHES=${YOLO_MAX_BATCHES})

math(EXPR YOLO_STEPS_80P "(${YOLO_MAX_BATCHES} * 8)/10")
message(STATUS YOLO_STEPS_80P=${YOLO_STEPS_80P})

math(EXPR YOLO_STEPS_90P "(${YOLO_MAX_BATCHES} * 9)/10")
message(STATUS YOLO_STEPS_90P=${YOLO_STEPS_90P})

file(READ ${CMAKE_CURRENT_LIST_DIR}/anchors.txt ANCHORS_LIST)
message(STATUS ANCHORS_LIST=${ANCHORS_LIST})

file(GENERATE OUTPUT "${CMAKE_CURRENT_LIST_DIR}/${YOLO_MODEL_NAME}.cfg" CONTENT
"[net]
# Testing
#batch=1
#subdivisions=1
# Training
batch=${YOLO_BATCH}
subdivisions=${YOLO_SUBDIVISIONS}
width=416
height=416
channels=3
momentum=0.9
decay=0.0005
angle=0
saturation = 1.5
exposure = 1.5
hue=.1

learning_rate=0.001
burn_in=1000
max_batches = ${YOLO_MAX_BATCHES}
policy=steps
steps=${YOLO_STEPS_80P},${YOLO_STEPS_90P}
scales=.1,.1

[convolutional]
batch_normalize=1
filters=16
size=3
stride=1
pad=1
activation=leaky

[maxpool]
size=2
stride=2

[convolutional]
batch_normalize=1
filters=32
size=3
stride=1
pad=1
activation=leaky

[maxpool]
size=2
stride=2

[convolutional]
batch_normalize=1
filters=64
size=3
stride=1
pad=1
activation=leaky

[maxpool]
size=2
stride=2

[convolutional]
batch_normalize=1
filters=128
size=3
stride=1
pad=1
activation=leaky

[maxpool]
size=2
stride=2

[convolutional]
batch_normalize=1
filters=256
size=3
stride=1
pad=1
activation=leaky

[maxpool]
size=2
stride=2

[convolutional]
batch_normalize=1
filters=512
size=3
stride=1
pad=1
activation=leaky

[maxpool]
size=2
stride=1

[convolutional]
batch_normalize=1
filters=1024
size=3
stride=1
pad=1
activation=leaky

###########

[convolutional]
batch_normalize=1
filters=256
size=1
stride=1
pad=1
activation=leaky

[convolutional]
batch_normalize=1
filters=512
size=3
stride=1
pad=1
activation=leaky

[convolutional]
size=1
stride=1
pad=1
filters=${YOLO_FILTER_COUNT}
activation=linear



[yolo]
mask = 3,4,5
anchors = ${ANCHORS_LIST}
classes=${YOLO_CLASSES_COUNT}
num=6
jitter=.3
ignore_thresh = .7
truth_thresh = 1
random=1

[route]
layers = -4

[convolutional]
batch_normalize=1
filters=128
size=1
stride=1
pad=1
activation=leaky

[upsample]
stride=2

[route]
layers = -1, 8

[convolutional]
batch_normalize=1
filters=256
size=3
stride=1
pad=1
activation=leaky

[convolutional]
size=1
stride=1
pad=1
filters=${YOLO_FILTER_COUNT}
activation=linear

[yolo]
mask = 0,1,2
anchors = ${ANCHORS_LIST}
classes=${YOLO_CLASSES_COUNT}
num=6
jitter=.3
ignore_thresh = .7
truth_thresh = 1
random=1")