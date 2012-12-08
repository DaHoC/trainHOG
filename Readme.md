# openCV HOG Trainer

Example program on how to train your custom HOG detecting vector for use with openCV '''hog.setSVMDetector(_descriptor)'''

## Information

For the paper regarding Histograms of Oriented Gradients (HOG), see http://lear.inrialpes.fr/pubs/2005/DT05/
You can populate the positive samples dir with files from the INRIA person detection dataset, see http://pascal.inrialpes.fr/data/human/

What this program basically does:
* Read positive and negative training sample image files from specified directories
* Calculate their HOG features and keep track of their classes (pos, neg)
* Pass the features and their classes to a machine learning algorithm, e.g. SVMlight (see http://svmlight.joachims.org/)
* Use the calculated support vectors and SVM model to calculate a single detecting descriptor vector

## Usage

Build by issuing:
    g++ `pkg-config --cflags opencv` -c -g -MMD -MP -MF main.o.d -o main.o main.cpp
    gcc -c -g `pkg-config --cflags opencv` -MMD -MP -MF svmlight/svm_learn.o.d -o svmlight/svm_learn.o svmlight/svm_learn.c
    gcc -c -g `pkg-config --cflags opencv` -MMD -MP -MF svmlight/svm_hideo.o.d -o svmlight/svm_hideo.o svmlight/svm_hideo.c
    gcc -c -g `pkg-config --cflags opencv` -MMD -MP -MF svmlight/svm_common.o.d -o svmlight/svm_common.o svmlight/svm_common.c
    g++ `pkg-config --cflags opencv` -o opencvhogtrainer main.o svmlight/svm_learn.o svmlight/svm_hideo.o svmlight/svm_common.o `pkg-config --libs opencv`

## Contact
Jan Hendriks (dahoc3150 [at] yahoo.com)

## Terms of use

This program is to be used as an example and is provided on an "as-is" basis without any warranties of any kind, either express or implied.
Use at your own risk.
For used third-party software, refer to their respective terms of use and licensing