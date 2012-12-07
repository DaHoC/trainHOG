/**
 * @file:   main.cpp
 * @author: Jan Hendriks (dahoc3150 [at] yahoo.com)
 * @date:   Created on 2. Dezember 2012, 13:32
 * @brief:  Example on how to train your custom HOG detecting vector
 * for use with openCV <code>hog.setSVMDetector(_descriptor)</code>;
 * 
 * 1. Read training sample files from specified directories 
 * 2. Calculate features and assign them to classes (pos, neg)
 * 3. Pass the features and their classes to a machine learning algorithm, e.g. SVMlight (@see http://svmlight.joachims.org/)
 * 
 * Compile by issuing:
 * g++ -o openCVHogTrainer main.cpp `pkg-config --cflags --libs opencv`
 */

#include <stdio.h>
#include <dirent.h>
#include <ios>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/ml/ml.hpp>

using namespace std;
using namespace cv;

/* Parameter definitions */

static const String posSamplesDir = "./pos";
static const String negSamplesDir = "./neg";
static const Size trainingPadding = Size(32, 32);
static const Size winStride = Size(8, 8);

/* Helper functions */

static string toLowerCase(const string& in) {
    string t;
    for (string::const_iterator i = in.begin(); i != in.end(); ++i) {
        t += tolower(*i);
    }
    return t;
}

static void storeCursor(void) {
    printf("\033[s");
}

static void resetCursor(void) {
    printf("\033[u");
}

/**
 * Saves the given descriptor vector to a file
 * @param descriptorVector the descriptor vector to save
 * @param _vectorIndices contains indices for the corresponding vector values (e.g. descriptorVector(0)=3.5f may have index 1)
 * @param fileName
 * @TODO Use _vectorIndices to write correct indices
 */
static void saveDescriptorVectorToFile(vector<float>& descriptorVector, vector<unsigned int>& _vectorIndices, string fileName) {
    printf("Saving descriptor vector to file '%s'\n", fileName.c_str());
    /// @WARNING: This is really important, ROS seems to set the system locale which takes decimal commata instead of points which causes the file input parsing to fail
    setlocale(LC_ALL, "C"); // Do not use the system locale
    string separator = " "; // Use blank as default separator between single features
    fstream File;
    float percent;
    File.open(fileName.c_str(), ios::out);
    if (File.is_open()) {
        //        File << "# This file contains the trained descriptor vector" << endl;
        printf("Saving descriptor vector features:\t");
//            printf("\n#features %d", descriptorVector->size());
        storeCursor();
        for (int feature = 0; feature < descriptorVector.size(); ++feature) {
            if ((feature % 10 == 0) || (feature == (descriptorVector.size()-1)) ) {
                percent = ((1 + feature) * 100 / descriptorVector.size());
                printf("%4u (%3.0f%%)", feature, percent);
                fflush(stdout);
                resetCursor();
            }
            //                File << _vectorIndices->at(feature) << ":";
            File << descriptorVector.at(feature) << separator;
        }
        printf("\n");
        File << endl;
        File.flush();
        File.close();
    }
}

/**
 * For unixoid systems only: Lists all files in a given directory and returns a vector of path+name in string format
 * @param dirName
 * @param fileNames found file names in specified directory
 * @param validExtensions containing the valid file extensions for collection in lower case
 * @return 
 */
static void getFilesInDirectory(const string& dirName, vector<string>& fileNames, const vector<string>& validExtensions) {
    printf("Opening directory %s\n", dirName.c_str());
    struct dirent* ep;
    size_t extensionLocation;
    DIR* dp = opendir(dirName.c_str());
    if (dp != NULL) {
        while ((ep = readdir(dp))) {
            // Ignore (sub-)directories like . , .. , .svn, etc.
            if (ep->d_type & DT_DIR) {
                continue;
            }
            extensionLocation = string(ep->d_name).find_last_of("."); // Assume the last point marks beginning of extension like file.ext
            // Check if extension is matching the wanted ones
            string tempExt = toLowerCase(string(ep->d_name).substr(extensionLocation + 1));
            if (std::find(validExtensions.begin(), validExtensions.end(), tempExt) != validExtensions.end()) {
                printf("Found file does not match required file type: '%s'\n", ep->d_name);
            } else {
                printf("Found matching data file '%s'\n", ep->d_name);
                fileNames.push_back((string) dirName + ep->d_name);
            }
        }
        (void) closedir(dp);
    } else {
        printf("Error opening directory '%s'!\n", dirName.c_str());
    }
}

static void getTrainedDectectingVectorFromSupportVectors(const vector< vector<float> >& supportVectors, vector<float>& resultingDetectingVector) {

}

/**
 * This is the actual calculation from the (input) image data to the HOG descriptor/feature vector using the hog.compute() functio
 * @param imageFilename
 * @param descriptorVector
 * @param hog
 */
static void calculateFeaturesFromInput(const string& imageFilename, vector<float>& descriptorVector, HOGDescriptor& hog) {
    printf("Reading image file '%s'\n", imageFilename.c_str());
    /** for imread flags from openCV documentation, 
     * @see http://docs.opencv.org/modules/highgui/doc/reading_and_writing_images_and_video.html?highlight=imread#Mat imread(const string& filename, int flags)
     * @note If you get a compile-time error complaining about following line (esp. imread),
     * you either do not have a current openCV version (>2.0) 
     * or the linking order is incorrect, try g++ -o openCVHogTrainer main.cpp `pkg-config --cflags --libs opencv`
     */
    Mat imageData = imread(imageFilename, 0);
    if (imageData.empty()) {
        descriptorVector.clear();
        printf("Error: HOG image is empty, features calculation skipped!\n");
        return;
    }
    // Check for mismatching dimensions
    if (imageData.cols != hog.winSize.width || imageData.rows != hog.winSize.height) {
        descriptorVector.clear();
        printf("Error: Image dimensions (%u x %u) do not match HOG window size (%u x %u)!\n", imageData.cols, imageData.rows, hog.winSize.width, hog.winSize.height);
        return;
    }
    vector<cv::Point> locations;
    hog.compute(imageData, descriptorVector, winStride, trainingPadding, locations);
    imageData.release(); // Release the image again after features are extracted
}

/**
 * Main program entry point
 * @param argc
 * @param argv
 * @return EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int main(int argc, char** argv) {

    HOGDescriptor hog; // Use standard parameters here
    // Get the files to train from somewhere
    vector<string> positiveTrainingImages;
    vector<string> negativeTrainingImages;
    vector<string> validExtensions;
    validExtensions.push_back("jpg");
    validExtensions.push_back("png");
    getFilesInDirectory(posSamplesDir, positiveTrainingImages, validExtensions);
    getFilesInDirectory(negSamplesDir, negativeTrainingImages, validExtensions);
    /// Retrieve the descriptor vectors from the samples
    vector<vector<float> > positiveSampleFeatures;
    vector<vector<float> > negativeSampleFeatures;
    // positive samples
    for (vector<string>::const_iterator samplesIt = positiveTrainingImages.begin(); samplesIt != positiveTrainingImages.end(); ++samplesIt) {
        vector<float> featureVector;
        calculateFeaturesFromInput(*samplesIt, featureVector, hog);
        if (!featureVector.empty()) {
            positiveSampleFeatures.push_back(featureVector);
        }
    }
    // negative samples
    for (vector<string>::const_iterator samplesIt = negativeTrainingImages.begin(); samplesIt != negativeTrainingImages.end(); ++samplesIt) {
        vector<float> featureVector;
        calculateFeaturesFromInput(*samplesIt, featureVector, hog);
        if (!featureVector.empty()) {
            negativeSampleFeatures.push_back(featureVector);
        }
    }

    /// Train the gathered feature vectors with e.g. SVMlight, @see http://svmlight.joachims.org/
/*
//                printf("\nSaving feature vectors of samples to file '%s' for %s training\n", this->featuresFile.c_str(), learner->getSVMName());
//                trainerHelper::saveFeatureVectorsInSVMLightCompatibleFormat(&trainingSamplesClasses, trainingSamplesClasses, this->featuresFile);
                printf("Passing feature vectors to %s (Warning: This can take quite some while!)\n", SVMlight::getInstance()->getSVMName());
                SVMlight::getInstance()->read_problem(const_cast<char*> (this->featuresFile.c_str()));
                SVMlight::getInstance()->train(); // Call the core libsvm training procedure
                printf("Training done, saving model file!\n");
                if (!this->featProt->getSVMModelFile().empty())
                    SVMlight::getInstance()->saveModelToFile(this->featProt->getSVMModelFile());
*/

/*
                printf("Generating representative single HOG feature vector using svmlight!\n");
                vector<float> descriptorVector;
                vector<unsigned int> descriptorVectorIndices;
                SVMlight::getInstance()->retrieveSingleDetectingVector(descriptorVector, descriptorVectorIndices);
                trainerHelper::saveDescriptorVectorToFile(descriptorVector, descriptorVectorIndices, this->featProt->getDetectorFile());
                if (!this->featProt->getSVMModelFile().empty()) {
                    SVMlight::getInstance()->saveModelToFile(this->featProt->getSVMModelFile());
*/

/*
            libSVM::getInstance()->freeMem();
        trainingSamplesFeatures.release();
        trainingSamplesClasses.release();

        trainingSamplesFeaturesMat.release();
        classCorrespondences.release();
        regionCorrespondences.release();
*/

    return EXIT_SUCCESS;
}

