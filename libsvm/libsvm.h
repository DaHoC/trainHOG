/* 
 * @file:   libsvm.h
 * @author: Jan Hendriks (dahoc3150 [at] gmail.com)
 * @date:   Created on 6. Mai 2011, 15:00
 * @brief:  Wrapper interface for libSVM, 
 * @see http://www.csie.ntu.edu.tw/~cjlin/libsvm/ for libSVM details and terms of use
 * 
 */

#ifndef LIBSVM_H
#define	LIBSVM_H

#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <vector>
#include <locale.h>
#include <stdlib.h>

// Precision to use (float / double)
typedef float prec;

// namespace required for avoiding collisions of declarations (e.g. LINEAR being declared in flann, svmlight and libsvm)
namespace libsvm {
    #include "svm.h"
}
#define Malloc(type,n) (type *)malloc((n)*sizeof(type))

using namespace libsvm;

class libSVM {
private:
    struct svm_problem prob; // set by read_problem
    struct svm_model* model;
    struct svm_node* x_space;

    bool trainingDataStructsUsed;
    bool predictionDataStructsUsed;
    
    // Prediction-specific
    int max_nr_attr;
    int predict_probability;

    char* line;
    int max_line_len;

    void exit_input_error(int line_num) {
        fprintf(stderr, "Wrong input format at line %d\n", line_num);
        exit(1);
    }

    char* readline(FILE *input) {
        int len;

        if (fgets(line, max_line_len, input) == NULL)
            return NULL;

        while (strrchr(line, '\n') == NULL) {
            max_line_len *= 2;
            line = (char *) realloc(line, max_line_len);
            len = (int) strlen(line);
            if (fgets(line + len, max_line_len - len, input) == NULL)
                break;
        }
//        printf("%s\n", line);
        return line;
    }

    libSVM() : trainingDataStructsUsed(false), predictionDataStructsUsed(false) {
        line = NULL;
        max_nr_attr = 64;
        predict_probability = 1; // 0 or 1
        // Init some parameters
        param.cache_size = 512; // in MB
        param.coef0 = 0.0; // for poly/sigmoid kernel
        param.degree = 3; // for poly kernel
        /// @WARNING eps_criteria = 0.001, eps = 0.1 which one to use?
        param.eps = 1e-3; // 0.001 stopping criteria
        param.gamma = 0; // for poly/rbf/sigmoid
        param.kernel_type = 0; // libsvm::LINEAR; // type of kernel to use
        param.nr_weight = 0; // for C_SVC
        param.nu = 0.5; // for NU_SVC, ONE_CLASS, and NU_SVR
        param.p = 0.1; // for EPSILON_SVR, epsilon in loss function?
        param.probability = predict_probability; // do probability estimates
        param.C = 0.01; // From paper, soft classifier
        param.shrinking = 0; // use the shrinking heuristics, equals flag -h
        param.svm_type = EPSILON_SVR; // C_SVC; // EPSILON_SVR; // may be also NU_SVR; // do regression task
        param.weight_label = NULL; // for C_SVC
        param.weight = NULL; // for C_SVC
//        x = Malloc(svm_node, 1);
//        x = NULL;
//        x_space = NULL;
        model = NULL;
    }

    /**
     * Free the used pointers
     */
    virtual ~libSVM() {
//        printf("Freeing used SVM memory\n");
//        if (this->predictionDataStructsUsed) {
//            free(x);
//        }
        this->freeMem();
    }

public:

    struct svm_parameter param; // set by parse_command_line

    static libSVM* getInstance();

    const char* getSVMName() const {
        return "libSVM";
    }

    void freeMem() {
        // Try to avoid a double-free being thrown because the objects may not be allocated, because the functions that allocate them are not being called
        if (this->trainingDataStructsUsed) {
            free(prob.y);
            free(prob.x);
            free(x_space);
            free(line);            
        }
        svm_destroy_param(&this->param);
        svm_free_and_destroy_model(&model); // The model is used in training and prediction specific steps
    }

    /**
     * Reads in a file in svmlight format
     * @param filename
     */
    void read_problem(char *filename) {
        /// @WARNING: This is really important, ROS seems to set the system locale which takes decimal commata instead of points which causes the file input parsing to fail
        // Do not use the system locale setlocale(LC_ALL, "C");
        setlocale(LC_NUMERIC,"C");
        setlocale(LC_ALL, "POSIX");
//        cout.getloc().decimal_point ();
        int elements, max_index, inst_max_index, i, j;
        FILE *fp = fopen(filename, "r");
        char *endptr;
        char *idx, *val, *label;

        if (fp == NULL) {
            fprintf(stderr, "can't open input file %s\n", filename);
            exit(1);
        }

        prob.l = 0;
        elements = 0;

        max_line_len = 1024;
        line = Malloc(char, max_line_len);
        while (readline(fp) != NULL) {
            char *p = strtok(line, " \t"); // label

            // features
            while (1) {
                p = strtok(NULL, " \t");
                if (p == NULL || *p == '\n') // check '\n' as ' ' may be after the last feature
                    break;
                ++elements;
            }
            ++elements;
            ++prob.l;
        }
        rewind(fp);

        prob.y = Malloc(double, prob.l);
        prob.x = Malloc(struct svm_node *, prob.l);
        x_space = Malloc(struct svm_node, elements);

        max_index = 0;
        j = 0;
        for (i = 0; i < prob.l; i++) {
            inst_max_index = -1; // strtol gives 0 if wrong format, and precomputed kernel has <index> start from 0
            readline(fp);
            prob.x[i] = &x_space[j];
            label = strtok(line, " \t\n");
            if (label == NULL) { // empty line
                printf("Empty line encountered!\n");
                exit_input_error(i + 1);
            }

            prob.y[i] = strtod(label, &endptr);
            if (endptr == label || *endptr != '\0') {
                printf("Wrong line ending encountered!\n");
                exit_input_error(i + 1);
            }

            while (1) {
                idx = strtok(NULL, ":");
                val = strtok(NULL, " \t");

                if (val == NULL)
                    break;

                errno = 0;
                x_space[j].index = (int) strtol(idx, &endptr, 10);
                if (endptr == idx || errno != 0 || *endptr != '\0' || x_space[j].index <= inst_max_index) {
                    printf("File input error at feature index encountered: %i, %i, %i, %i!\n", endptr == idx, errno != 0, *endptr != '\0', x_space[j].index <= inst_max_index);
                    exit_input_error(i + 1);                    
                } else {
                    inst_max_index = x_space[j].index;
//                    printf("Index %i ok\n", inst_max_index);
                }

//                printf("Raw val: '%s'\n", val);
                // Use the following to convert . to , if some weird locale setting is set - see above for locale setting
//                char* pos = strchr(val, '.');
//                if (pos != NULL && pos[0] != '\0' && pos[0] != '\n') pos[0] = ',';
                
                errno = 0;
                x_space[j].value = std::strtod(val, &endptr);
//                printf("Value: '%f'\n", x_space[j].value);
                if (endptr == val || errno != 0 || (*endptr != '\0' && !isspace(*endptr))) {
                    printf("File input error at feature value encountered: %i, %i, %i: '%s'\n",endptr == val, errno != 0, (*endptr != '\0' && !isspace(*endptr)), endptr);
                    exit_input_error(i + 1);
                }
                ++j;
            }

            if (inst_max_index > max_index)
                max_index = inst_max_index;
            x_space[j++].index = -1;
        }

        if (param.gamma == 0 && max_index > 0) {
            param.gamma = 1.0 / max_index;
        }
        if (param.kernel_type == PRECOMPUTED) {
            for (i = 0; i < prob.l; i++) {
                if (prob.x[i][0].index != 0) {
                    fprintf(stderr, "Wrong input format: first column must be 0:sample_serial_number\n");
                    exit(1);
                }
                if ((int) prob.x[i][0].value <= 0 || (int) prob.x[i][0].value > max_index) {
                    fprintf(stderr, "Wrong input format: sample_serial_number out of range\n");
                    exit(1);
                }
            }
        }
        fclose(fp);
        this->trainingDataStructsUsed = true;
    }

    /**
     * Modelfile is saved to filesystem and contains the svm parameters used for training, which need to be retrieved for classification
     * Only makes sense after training was done
     * @param _modelFileName file name to save the model to
     */
    void saveModelToFile(const std::string _modelFileName) {
        if (svm_save_model(_modelFileName.c_str(), getInstance()->model)) {
            fprintf(stderr, "Error: Could not save model to file %s\n", _modelFileName.c_str());
            exit(EXIT_FAILURE);
        }
    }

    /**
     * Function was unit tested, can be assumed libSVM model is correctly loaded from file
     * @param _modelFileName
     */
    void loadModelFromFile(const std::string _modelFileName) {
        this->freeMem();
        /// @WARNING: This is really important, ROS seems to set the system locale which takes decimal commata instead of points which causes the file input parsing to fail
        // Do not use the system locale setlocale(LC_ALL, "C");
        setlocale(LC_NUMERIC,"C");
        setlocale(LC_ALL, "POSIX");
        /// @TODO Test if this works as intended
        if (this->predictionDataStructsUsed && model != NULL) {
//            svm_free_and_destroy_model(&model);
            delete model; // clear the previous model and deallocate/free the occupied memory
            model = NULL; // set the pointer to NULL
        }
        printf("Loading model from file '%s'\n", _modelFileName.c_str());
        model = svm_load_model(_modelFileName.c_str());
        if (model == NULL) {
            printf("Loading of SVM model failed!\n");
            exit(EXIT_FAILURE);
        }

        int classNr = svm_get_nr_class(model);
        int labels[classNr];
        for (int i = 0; i < classNr; ++i) {
                labels[i] = 0;
        }
        svm_get_labels(model, labels);
        double b = -(model->rho[0]);
        double probA = (model->probA[0]);
        double probB = (model->probB[0]);
        
        int kernelType = model->param.kernel_type;
        printf("Loaded model: SVM type %d, Kernel type %d, %d classes: labels %d, %d, #SVs %d, bias b %3.5f, probA %3.5f, probB %3.5f\n", svm_get_svm_type(model), kernelType, classNr, labels[0], labels[1], model->l, b, probA, probB);
        this->predictionDataStructsUsed = true;
//        exit(EXIT_SUCCESS);
    }

    svm_problem getProblem() {
        return this->prob;
    }

    /**
     * Converts a vector of floats into an array of structs used by libsvm
     * @WARNING: Implicitly this changes the memory size because it casts from float to double
     * @WARNING: Untested as of now
     * @TODO This conversion is slow when called in every detection step, how to avoid?
     * May be it's faster using something like memcpy( destination, &myVector[0], sizeof( int ) * myVector.size() );
     * @param _sample
     * @param y
     */
/*
    inline svm_node* convertFromVectorToSVMNodeArray(std::vector<float>* _sample) {
        int max_nr_attrlocal = (_sample->size() + 1); // +1 because of the last index
//        svm_node* y = (svm_node *) realloc(y, max_nr_attrlocal * sizeof (svm_node));
//        y = (svm_node*) malloc (max_nr_attrlocal * sizeof (struct svm_node));
        svm_node* y = (svm_node*) calloc (max_nr_attrlocal, sizeof (svm_node));
        
        for (unsigned int component = 0; component < _sample->size(); ++component) {
            // Populate struct with vector values
            y[component].index = component;
            y[component].value = (double)_sample->at(component);
        }
        y[_sample->size()].index = -1; // UINT_MAX; // -1; // Indicate the end of the array for libsvm
        y[_sample->size()].value = 0.; // Be polite, do not leave some arbitrary memory chunk in here
        return y;
    }
*/
    /**
     * Predicts a label/class from specified _sample and given model
     * @param _sample input sample to be classified
     * @return label of class the SVM predicted for given sample
     */
/*
    float predictLabel(std::vector<float>* _sample, double* prob_estimates) {
        if (model == NULL) { /// @WARNING This is unelegant, valgrind complains because of check against uninitialized variable
            printf("Error: A model must first be loaded or trained!\n");
            exit(EXIT_FAILURE);
        }
        /// @TODO This conversion has to be avoided because we need speed in the detection step
        struct svm_node* x = this->convertFromVectorToSVMNodeArray(_sample);
//        printf("SVM type: %u, # classes: %u\n", svm_get_svm_type(model), svm_get_nr_class(model));
//        this->predictLabel(x);
//        float predict_label = svm_predict(model, x);
        float predict_label = svm_predict_probability(model, x, prob_estimates);
//        printf("Vorhergesagtes label: %+1.2f, Probability estimate: %+1.2f\n", predict_label, *prob_estimates);
        free(x);
        this->predictionDataStructsUsed = true;
        return predict_label;
    }
*/

    float predictLabel(svm_node* _sample, double* _probEstimate) {
        
//        double* prob_estimates;
//printf("Predict function before pred 2nd svm_node value: %3.6f\n", _sample[1].value);
        float predict_label = svm_predict_probability(model, _sample, _probEstimate);
//printf("Predict function after pred 2nd svm_node value: %3.6f\n", _sample[1].value);
//        printf("Vorhergesagtes label: %+1.2f\n", predict_label);
//        printf("Probability estimates: %+1.2f, %1.2f\n", _probEstimate[0], _probEstimate[1]);
//        printf("Model kernel type %d\n", model->param.kernel_type);
        return predict_label;
//        return svm_predict(model, _sample);
    }

    /**
     * After read in the training samples from a file, set parameters for training and call training procedure
     */
    void train() {
        model = svm_train(&prob, &param);
//        model = svm_train(&prob, &param);
        trainingDataStructsUsed = true;
    }

    /**
     * Generates a single detecting feature vector (vec1) from the trained support vectors, for use e.g. with the HOG algorithm
     * vec1 = sum_1_n (alpha_y*x_i). (vec1 is a 1 x n column vector. n = feature vector length )
     * @param singleDetectorVector resulting single detector vector for use in openCV HOG
     * @param singleDetectorVectorIndices vector containing indices of features inside singleDetectorVector
     */
    void getSingleDetectingVector(std::vector<prec>& singleDetectorVector, std::vector<unsigned int>& singleDetectorVectorIndices) {
        // Now we use the trained svm to retrieve the single detector vector
        printf("Calculating single detecting feature vector out of support vectors (may take some time)\n");
        singleDetectorVector.clear();
        singleDetectorVectorIndices.clear();
        printf("Total number of support vectors: %d \n", model->l);
        //        printf("Number of SVs for each class: %d \n", _model->nr_class);
        // Walk over every support vector and build a single vector
        for (unsigned long ssv = 0; ssv < model->l; ++ssv) { // Walks over available classes (e.g. +1, -1 representing positive and negative training samples)
            // Retrieve the current support vector from the training set
            svm_node* singleSupportVector = model->SV[ssv]; // Get next support vector ssv==class, 2nd index is the component of the SV
            //            _prob->x[singleSupportVector->index];
            // sv_coef[i] = alpha[i]*sign(label[i]) = alpha[i] * y[i], where i is the training instance, y[i] in [+1,-1]
            double alpha = model->sv_coef[0][ssv];
            int singleVectorComponent = 0;
//            while (singleSupportVector[singleVectorComponent].index != UINT_MAX) { // index=UINT_MAX indicates the end of the array
            while (singleSupportVector[singleVectorComponent].index != -1) { // index=-1 indicates the end of the array
                //    if (singleVectorComponent > 3777)
                //        printf("\n-->%d", singleVectorComponent);
                //                printf("Support Vector index: %u, %+3.5f \n", singleSupportVector[singleVectorComponent].index, singleSupportVector[singleVectorComponent].value);
                if (ssv == 0) { // During first loop run determine the length of the support vectors and adjust the required vector size
                    singleDetectorVector.push_back(singleSupportVector[singleVectorComponent].value * alpha);
                    //                    printf("-%d", singleVectorComponent);
                    singleDetectorVectorIndices.push_back(singleSupportVector[singleVectorComponent].index); // Holds the indices for the corresponding values in singleDetectorVector, mapping from singleVectorComponent to singleSupportVector[singleVectorComponent].index!
                } else {
                    if (singleVectorComponent > singleDetectorVector.size()) { // Catch oversized vectors (maybe from differently sized images?)
                        printf("Warning: Component %d out of range, should have the same size as other/first vector\n", singleVectorComponent);
                    } else
                        singleDetectorVector.at(singleVectorComponent) += (singleSupportVector[singleVectorComponent].value * alpha);
                }
                singleVectorComponent++;
            }
        }

        // This is a threshold value which is also recorded in the lear code in lib/windetect.cpp at line 1297 as linearbias and in the original paper as constant epsilon, but no comment on how it is generated
//        singleDetectorVector.push_back(b); // Add threshold
//        singleDetectorVectorIndices.push_back(-1); // Add maximum unsigned int as index indicating the end of the vector
//        singleDetectorVectorIndices->push_back(UINT_MAX); // Add maximum unsigned int as index indicating the end of the vector
    }

    /**
     * Return model detection threshold / bias
     * @return detection threshold / bias
     */
    double getThreshold() const {
        return model->rho[0];
    }

};

/// Singleton, @see http://oette.wordpress.com/2009/09/11/singletons-richtig-verwenden/
libSVM* libSVM::getInstance() {
    static libSVM theInstance;
    return &theInstance;
}

#endif	/* LIBSVM_H */
