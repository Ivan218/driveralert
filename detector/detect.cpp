/** Usage:
  * This program runs the detector through the specified video file. 0 for laptop camera.
  * Use pathname for a video file.
  * Currently the saving component is commented out and may need work to suit the user's liking.
  * Program is meant to save images that have a face detected.
  * Configurable components in the globals and comments. Example here uses haarcascade_upperbody.xml
  * as the detector to use when sifting through images.
  */

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/opencv.hpp"

#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>

using namespace std;
using namespace cv;

void detectAndWrite(Mat &frame);

const cv::String    WINDOW_NAME("Camera video");

CascadeClassifier upper_body_cascade;

string prefix = "/Users/ivanjrivera/Dropbox/dataset/";
string MODEL_PATH = "haarcascade_upperbody.xml";
string person = "ivan";
int main() {
    // load classifier
    if (!upper_body_cascade.load(MODEL_PATH)) 
        { printf("--(!)Error loading classifier\n"); return -1; }
    
    // open the video
    // cv::VideoCapture camera(0);
    cv::VideoCapture camera(prefix + person + ".left.mp4");
        // capture frame
        // save frame temporarily
        // pass image path to face detection object file
    if (!camera.isOpened()) {
        fprintf(stderr, "Camera could not be opened!");
        exit(1);
    }

    cv::namedWindow(WINDOW_NAME, cv::WINDOW_KEEPRATIO | cv::WINDOW_AUTOSIZE);
    Mat frame;
    Rect box;
    int i = 1;
    while (true) {
        auto start = cv::getCPUTickCount();
        camera >> frame;
        auto end = cv::getCPUTickCount();
        
        double time_per_frame = (end - start) / cv::getTickFrequency();
        double fps = (15 * fps + (1 / time_per_frame)) / 16;

        fprintf(stderr, "Time per frame for image: #%d,  %3.3f\tFPS: %3.3f\n", i++, time_per_frame, fps);

        if (!frame.empty()) {
            detectAndWrite(frame);
        }
        else {
            fprintf(stderr, " --(!) No captured frame -- Break!"); break;
        }

        // char *name = (char *) malloc(64);
        // sprintf(name, "/Users/ivanjrivera/Dropbox/dataset/lefttmpd/dana.left.%d.jpg", i++);
        // cv::imshow(WINDOW_NAME, frame);
        // cv::imwrite(name, frame);
        
        if (cv::waitKey(25) == 'c') break;
    }
    cv::destroyWindow(WINDOW_NAME);
}

/** @function detectAndWrite */
void detectAndWrite(Mat &frame)
{
    std::vector<Rect> bodies;
    Mat frame_gray;

    cvtColor(frame, frame_gray, CV_BGR2GRAY);
    equalizeHist(frame_gray, frame_gray);

    //-- Detect faces
    upper_body_cascade.detectMultiScale(frame_gray, bodies, 1.1, 1, CV_HAAR_SCALE_IMAGE, Size(700, 700), Size(1100, 1500));

    for (size_t i = 0; i < bodies.size(); i++)
    {
        fprintf(stderr, "DETECTION: %lu\n", bodies.size());
        rectangle(frame, bodies[i], Scalar(255, 0, 255));
    }
    //-- Show what you got
    imshow(WINDOW_NAME, frame);
}