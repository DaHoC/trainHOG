# openCV HOG Trainer

Example linux program on how to train your custom HOG detecting vector for use with openCV hog.setSVMDetector(_descriptor)

## Information

For the paper regarding Histograms of Oriented Gradients (HOG), see http://lear.inrialpes.fr/pubs/2005/DT05/.
You can populate the positive samples dir with files from the INRIA person detection dataset, see http://pascal.inrialpes.fr/data/human/.
This program uses SVMlight as machine learning algorithm (see http://svmlight.joachims.org/), but is not restricted to it.
Tested in Ubuntu Linux 64bit 12.04 "Precise Pangolin" with openCV 2.3.1, SVMlight 6.02, g++ 4.6.3 and standard HOG settings, training images of size 64x128px.

What this program basically does:
* Read positive and negative training sample image files from specified directories
* Calculate their HOG features and keep track of their classes (pos, neg)
* Save the feature map (vector of vectors/matrix) to file system
* Read in and pass the features and their classes to a machine learning algorithm, e.g. SVMlight
* Train the machine learning algorithm using the specified parameters
* Use the calculated support vectors and SVM model to calculate a single detecting descriptor vector
* Dry-run the newly trained custom HOG descriptor against training set and against camera images, if available

## Usage

See the tutorial at https://github.com/DaHoC/trainHOG/wiki/trainHOG-Tutorial (previously http://opencv.willowgarage.com/wiki/trainHOG) for instructions on how to use.
The steps in the preparations part are necessary for this program to be able to compile and run.

Build by issuing:
    g++ `pkg-config --cflags opencv` -c -g -MMD -MP -MF main.o.d -o main.o main.cpp
   
    gcc -c -g `pkg-config --cflags opencv` -MMD -MP -MF svmlight/svm_learn.o.d -o svmlight/svm_learn.o svmlight/svm_learn.c
   
    gcc -c -g `pkg-config --cflags opencv` -MMD -MP -MF svmlight/svm_hideo.o.d -o svmlight/svm_hideo.o svmlight/svm_hideo.c
   
    gcc -c -g `pkg-config --cflags opencv` -MMD -MP -MF svmlight/svm_common.o.d -o svmlight/svm_common.o svmlight/svm_common.c
   
    g++ `pkg-config --cflags opencv` -o opencvhogtrainer main.o svmlight/svm_learn.o svmlight/svm_hideo.o svmlight/svm_common.o `pkg-config --libs opencv`
   

## Warning words

* At least one of the functions (opendir) doing file system operations is unix/linux-only, for using the program in other operating systems a alternative API functions have to be used.
* Be aware that the program may consume a considerable amount of main memory, hard disk memory and time, dependent on the amount of training samples.
* Also be aware that (esp. for 32bit systems), there are limitations for the maximum file size which may take effect when writing the features file.

## Issues

See https://github.com/DaHoC/trainHOG/issues

## Contact
Jan Hendriks (dahoc3150 [at] gmail.com)

## Terms of use

This program is to be used as an example and is provided on an "as-is" basis without any warranties of any kind, either express or implied.
Use at your own risk.
For used third-party software, refer to their respective terms of use and licensing

## License

This software is licensed under the Apache License 2.0 (http://www.apache.org/licenses/LICENSE-2.0.html)
Please find the Apache license 2.0 statement in the respective file alongside this software.

Copyright 2015 Jan Hendriks

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
