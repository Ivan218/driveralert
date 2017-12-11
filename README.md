DriverAlert README
==================

The project has several components for assisting in training an SVM classifier using HOG features from grayscale images. The executable retrieved by compiling in the project root trains the classifier if the configurable parameters are set in train_faces.cpp.

The repository contains other programs of code that may be useful to assist in generating a dataset to train and test with, however they are all auxiliary. No executable processes arguments. Each file listed has comments within that further delineate file usage, and extra functionality.

train_faces.cpp
---------------
File for loading dataset and training on it. The executable takes no args.

detector
--------
There are two files in here that have separate use cases. 

detector/newdetect.cpp
----------------------
Runs the specified detector through a directory of images, cropping the images to the biggest face detected (overwriting the original image). Images which have no faces print rm <filename> statements.

detector/detect.cpp
-------------------
Runs the specified detector through a video file. Currently does not save anything new, can easily be modified to do so however.

imageselection/sift.cpp
-----------------------
Very useful for 'sifting' through video files (.mp4) and labelling each frame by saving it to a temporary subdirectory which serves as the label (ie. all 'forward' images are saved in the subdir 'forwardtmp'). This does not run any detectors through, and should be followed by using newdetect.cpp on the tmp directories.


classify/run_classifier.cpp
---------------------------
Can be used to validate / run the trained classifier on a video file. Currently has performance metrics only by setting a single correct label, accuracy measured by counting the number of classifications that match the set 'correct label.' If used in production, comment out I/O functions.


## Pipeline: ##
    1. Record data and convert the video to an .mp4. Images can also be recorded through sift.cpp from a connected camera. 
    2. If the video contains multiple classifications, use sift.cpp to process and label the video file. Otherwise, run modified detect.cpp through the file to extract crops of the face and save them to the appropriate label directory.
    3. Train the classifier using train_faces.cpp
    4. Validate / use the classifier by using run_classifier.cpp

