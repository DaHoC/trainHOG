/**
 * @file:   main.cpp
 * @author: Jan Hendriks (dahoc3150 [at] yahoo.com)
 * @date:   Created on 2. Dezember 2012, 13:32
 * @brief:  Example on how to train your custom HOG detecting vector
 * for use with openCV <code>hog.setSVMDetector(_descriptor)</code>;
 * 
 * Compile by issuing:
 * g++ `pkg-config --cflags --libs opencv` -o openCVHogTrainer main.cpp
 */

#include <stdio.h>
#include <dirent.h>
#include <ios>
#include <opencv2/opencv.hpp>
#include <opencv2/ml/ml.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;

/* Parameter definitions */

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
     * @note If you get a compile-time error complaining about following line (esp. imread), you probably do not have a current openCV version (>2.0)!
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

    HOGDescriptor hog;
    // Get the files to train from somewhere
    vector<string> positiveTrainingImages;
    vector<string> negativeTrainingImages;
    vector<string> validExtensions;
    validExtensions.push_back("jpg");
    validExtensions.push_back("png");
    getFilesInDirectory("./pos/", positiveTrainingImages, validExtensions);
    getFilesInDirectory("./neg/", negativeTrainingImages, validExtensions);
    // Retrieve the descriptor vectors from the samples
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

//    printf("Generating representative single detecing HOG feature vector using svmlight\n");
//    vector<float> descriptorVector;
//    vector<unsigned int> descriptorVectorIndices;
//    SVMlight::getInstance()->retrieveSingleDetectingVector(descriptorVector, descriptorVectorIndices);


    return EXIT_SUCCESS;
}

