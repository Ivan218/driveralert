/** Usage:
  * Used to train a HOG SVM classifier with training images in the directories specified below.
  * Images must all be cropped down to whatever is needed (ie. face-only or top-half of face).
  * Since HOG needs to be trained on all images of the same size, images differing from the set size
  * are resized to the set size in the best way to preserve information. 
  *
  * You MUST examine the HOGDescriptor object below and configure it to your needs, then specify the
  * setSize you want all images to be (which is dependent on HOGDescriptor fields). Assertions and research will help.
  * 
  * Automatic labelling is performed by labelling data dependent on the directory the data is all stored. All data in one
  * directory MUST be of the same classification as the rest within the directory.
  *
  * For SVM parameters, please autoTrain every now and then to get the best parameters for the SVM for use in future trainings.
  * Be sure to set the SVM parameters accordingly below once they are printed.
  * 
  * Helper functions below also print the absolute path for the test images which were wrongly predicted after testing, however
  * it currently uses the wrong index to find the images. This must be adjusted before use.
  *
  * Net accuracy is reported for the classifier during testing.
  *
  * Total image count in each directory is printed at the end.
  *
  * Resulting svm is saved in the SVMtrain function. It will OVERWRITE existing files with the same name.
  */
#include <iostream>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/ml.hpp>

#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>

using namespace cv;
using namespace cv::ml;
using namespace std;

void printPaths( vector<int> &wrongs );
void printCounts(void);

string prefix = "/PATH/TO/DATASET/";
string pathname[] = { prefix + "forward/", prefix + "eyesclosed/", prefix + "right/", prefix + "centerstack/", prefix + "rearviewmirror/" };
int counter[sizeof(pathname)];
const int setSize = 100;
const double testRatio = .1; // proportion of the data in each dir will be used for testing right after training
const cv::String WINDOW_NAME("Camera video");

/* LOAD TRAINING AND TESTING DATA INTO VECTORS AND LABEL THEM BY DIRECTORY */
void loadTrainTestLabel(vector<Mat> &trainCells, vector<Mat> &testCells, vector<int> &trainLabels, vector<int> &testLabels) {
    int pathsize = sizeof(pathname) / sizeof(pathname[0]);
    for (int i = 0; i < pathsize; i++) {
        // Determine the label based on INDEX since we can see dirnames above
        /*switch(i) {
            case 0: eyes closed
            case 1: forward
            case 2: right
            case 3: centerstack
            case 4: rearviewmirrow
            case 5: left
            case 6: other
        }*/
        const char * path = pathname[i].c_str();
        DIR *dp; 
        int total = 0, curr = 0;
        struct dirent *ep;
        fprintf(stderr, "path: %s\n", path);
        dp = opendir(path);
        if (dp != NULL) {
            while ((ep = readdir(dp)))
                total++;
            total--; total--; // atomically don't count . and ..
            (void) closedir(dp);
            counter[i] = total;
            int test = int(total / (total * testRatio));
            fprintf(stderr, "%d files in %s # tests == %d\n", total, path, int(total / test));

            if ((dp = opendir(path))) {
                readdir(dp); readdir(dp); // SKIP . and ..

                while ((ep = readdir(dp))) {
                    char *name = new char[strlen(path) + sizeof(ep->d_name)];
                    strcpy(name, path);
                    strcat(name, ep->d_name);
                    Mat tmp = imread(name);
                    cvtColor(tmp, tmp, COLOR_BGR2GRAY);
                    if (tmp.rows < 1 || tmp.cols < 1) {
                        fprintf(stderr, "ERROR READING IMAGE: %s\n", name);
                        fprintf(stderr, "\tERROR: tmp %d rows: %d cols: %d\n", curr, tmp.rows, tmp.cols);
                        abort();
                    }
                    else if (tmp.rows < setSize || tmp.cols < setSize) {
                        fprintf(stdout, "%s #rows=%d, cols=%d enlarging\n", name, tmp.rows, tmp.cols);
                        resize(tmp, tmp, Size(setSize, setSize), 0, 0, INTER_CUBIC);
                    }
                    else if (tmp.rows > setSize || tmp.cols > setSize) {
                        fprintf(stdout, "%s #rows=%d, cols=%d shrinking\n", name, tmp.rows, tmp.cols);
                        resize(tmp, tmp, Size(setSize, setSize), 0, 0, INTER_AREA);
                    }

                    if (curr++ % test) { // TRAIN
                        trainCells.push_back(tmp);
                        trainLabels.push_back(i);
                    }
                    else { // TEST
                        testCells.push_back(tmp);
                        testLabels.push_back(i);
                        // if (tester == 0 || tester == 250 || tester == 95) {
                        //     fprintf(stderr, "open %s; #tester: %d\n", name, tester);
                        // }
                        // tester++;
                    }
                    // fprintf(stderr, "loadTrainTestLabel curr: %d, rows: %d, cols: %d\n", curr - 1, tmp.rows, tmp.cols);
                    delete[] (name);
                }
                (void) closedir(dp);
            }
            else 
                perror("Unknown error for opendir(path)");
        }
        else
            perror("Couldn't open the directory");
    }
    // fprintf(stderr, "max img size: %d, min img size: %d\n", max, min);
}

HOGDescriptor hog(
        Size(100, 100), //winSize
        Size(20, 20),   //blocksize
        Size(10, 10),   //blockStride,
        Size(10, 10),   //cellSize,
                  9,    //nbins,
                  1,    //derivAper,
                 -1,    //winSigma,
                  0,    //histogramNormType,
                0.2,    //L2HysThresh,
                  0,    //gammal correction,
                 64,    //nlevels=64
                  1);   //use signed gradients

/* COMPUTE THE HOG FOR EACH MAT IN TRAINING AND TESTING SETS, ADDING THE RESULTING VECTORS TO ONE VECTOR EACH SET */
void CreateTrainTestHOG(vector<vector<float>> &trainHOG, vector<vector<float>> &testHOG, vector<Mat> &trainCells, vector<Mat> &testCells) {
    int trainsize = trainCells.size(), testsize = testCells.size();
    fprintf(stderr, "CreateTrainTestHOG: trainCells.size() == %d\n", trainsize);
    fprintf(stderr, "CreateTrainTestHOG: testCells.size() == %d\n", testsize);
    for(int y = 0; y < trainsize; y++) {
        vector<float> descriptors;
        hog.compute(trainCells[y], descriptors);
        trainHOG.push_back(descriptors);
    }
   
    for(int y = 0; y < testsize; y++) {
        vector<float> descriptors;
        hog.compute(testCells[y], descriptors);
        testHOG.push_back(descriptors);
    } 
}
void ConvertVectortoMatrix(vector<vector<float> > &trainHOG, vector<vector<float> > &testHOG, Mat &trainMat, Mat &testMat) {
    int descriptor_size = trainHOG[0].size();
    for(int i = 0; i < trainHOG.size(); i++) 
        for(int j = 0; j < trainHOG[i].size(); j++) 
            trainMat.at<float>(i, j) = trainHOG[i][j];
    
    for(int i = 0; i < testHOG.size(); i++)
        for(int j = 0; j < descriptor_size; j++)
            testMat.at<float>(i, j) = testHOG[i][j];
}

void getSVMParams(SVM *svm) {
    cout << "Kernel type     : " << svm->getKernelType() << endl;
    cout << "Type            : " << svm->getType() << endl;
    cout << "C               : " << svm->getC() << endl;
    cout << "Degree          : " << svm->getDegree() << endl;
    cout << "Nu              : " << svm->getNu() << endl;
    cout << "Gamma           : " << svm->getGamma() << endl;
}

void SVMtrain(Mat &trainMat, vector<int> &trainLabels, Mat &testResponse, Mat &testMat) {
    Ptr<SVM> svm = SVM::create();
    svm->setGamma(0.00225);
    svm->setC(2.5);
    svm->setKernel(SVM::RBF);
    svm->setType(SVM::C_SVC);
    Ptr<TrainData> td = TrainData::create(trainMat, ROW_SAMPLE, trainLabels);
    svm->train(td);
    // svm->trainAuto(td);
    svm->save("model5.yml");
    svm->predict(testMat, testResponse);
    getSVMParams(svm);
}

void SVMevaluate(Mat &testResponse, float &accuracy, vector<int> &testLabels){
    int count = 0;
    for (int i = 0; i < testResponse.rows; i++)
        // cout << testResponse.at<float>(i,0) << " " << testLabels[i] << endl;
        if (testResponse.at<float>(i, 0) == testLabels[i]) 
            count++;
    accuracy = (count / testResponse.rows) * 100;
}

/* SAVE THE WRONGLY CLASSIFIED TEST IMAGES TO RECORD THE PATHS LATER USING THEIR INDEXES */
void pickImages(vector<Mat> &testCells, vector<int> &testLabels, Mat &testResponse, vector<int> &wrongs) {
    for (int i = 0; i < testResponse.rows; i++)
        if (testResponse.at<float>(i, 0) != testLabels[i])
            wrongs.push_back(i);
}

int main(){
    vector<Mat> trainCells;
    vector<Mat> testCells;
    vector<int> trainLabels;
    vector<int> testLabels;
    
    loadTrainTestLabel(trainCells, testCells, trainLabels, testLabels);

    fprintf(stderr, "SIZE:\ttrainCells\ttestCells\n\t  %lu\t\t%lu\n", trainCells.size(), testCells.size());
    fprintf(stderr, "train size: %lu\n", trainCells.size());
    
    // namedWindow(WINDOW_NAME, WINDOW_KEEPRATIO | WINDOW_AUTOSIZE);
    // imshow(WINDOW_NAME, trainCells[10]);
    // cvWaitKey(0);
    // cv::destroyWindow(WINDOW_NAME);

    vector<vector<float>> trainHOG;
    vector<vector<float>> testHOG;
    CreateTrainTestHOG(trainHOG, testHOG, trainCells, testCells);
    
    int descriptor_size = trainHOG[0].size();
    printf("Descriptor Size : %d\n", descriptor_size);
    
    Mat trainMat (trainHOG.size(), descriptor_size, CV_32FC1);
    Mat testMat (testHOG.size(), descriptor_size, CV_32FC1);
    
    ConvertVectortoMatrix(trainHOG, testHOG, trainMat, testMat);
    
    printf("Training...\n");

    Mat testResponse;
    SVMtrain(trainMat, trainLabels, testResponse, testMat);
    
    float accuracy = 0;
    SVMevaluate(testResponse, accuracy, testLabels);

    // vector<int> wrongs;
    // pickImages(testCells, testLabels, testResponse, wrongs);
    // printPaths( wrongs );
    
    printf("Accuracy        : %f%%\n", accuracy);
    printCounts();

    return 0;
}

// TODO: FIX AS THIS PRINTS THE IMAGES BASED ON THE TOTAL # OF IMAGES IN THE DIRECTORIES, NOT BY INDEX IN TESTING SET
/* PRINTS THE PATHNAMES OF TEST IMAGES WHICH WERE WRONGLY CLASSIFIED */
void printPaths( vector<int> &wrongs ) {
    int pathsize = sizeof(pathname) / sizeof(pathname[0]);
    int idx = 0;
    for (int i = 0; i < pathsize; i++) {
        const char * path = pathname[i].c_str();
        DIR *dp; 
        struct dirent *ep;
        dp = opendir(path);
        if (dp != NULL) {
            if ((dp = opendir(path))) {
                readdir(dp); readdir(dp); // SKIP . and ..
                int curr = 0;
                while ((ep = readdir(dp))) {
                    if (idx >= sizeof(wrongs)) break;
                    
                    char *name = new char[strlen(path) + sizeof(ep->d_name)];
                    strcpy(name, path);
                    strcat(name, ep->d_name);
                    Mat tmp = imread(name);
                    
                    if (curr == wrongs[idx]) {
                        fprintf(stderr, "open %s; #tester: %d", name, idx);
                        if (tmp.rows != 100 || tmp.cols != 100)
                            fprintf(stderr, "#resize");
                        fprintf(stderr, "\n");
                        idx++;
                    }
                    curr++;
                    delete[] (name);
                }
                (void) closedir(dp);
            }
            else 
                perror("Unknown error for opendir(path)");
        }
        else
            perror("Couldn't open the directory");
    }
}
void printCounts() {
    int pathsize = sizeof(pathname) / sizeof(pathname[0]);
    for (int i = 0; i < pathsize; i++) {
        fprintf(stderr, "\t%d files for %s\n", counter[i], pathname[i].c_str());
    }
}