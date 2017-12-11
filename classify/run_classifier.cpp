/** Usage:
  * Loads SVM classifier with specified filepath. Make sure the HOGDescriptor matcehs that which was used for training.
  * The classifier is used on a video file (builtin cameras can be used). 
  * Program must have the global variable modes set in the same order of training directories to match the labels.
  * There is also a built in accuracy measure for evaluation of a specific class, which must be set by matching the
  * global variable mode to the index to be used in the global variable modes.
  *
  * Write the original image to the screen with the calculation FPS of detection and classification, as well as with the
  * resulting classification for the specified image. Remember frame rate will be very lowed compared to the calculated FPS
  * due to writing to I/O.
  */

#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/ml.hpp>

#include "VideoFaceDetector.h"

using namespace cv;
using namespace cv::ml;
using namespace std;

void evaluate(int classy);
const Ptr<SVM> classifier = SVM::load("/Users/ivanjrivera/cse379/driveralert/model5.yml");
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
const cv::String WINDOW_NAME("Driver Alert");
const int mode = 0; // 			0			1			2			3				4
const string modes[] = { "forward", "eyes closed", "right", "centerstack", "rearviewmirror" };

static int total = 0, pos = 0;

int main() {
	const string DETECTOR_PATH = "/Users/ivanjrivera/cse379/driveralert/detector/haarcascade_frontalface_alt.xml";
	// Load detector // Setup detector with camera
	VideoCapture camera(0);
	if (!camera.isOpened()) {
		fprintf(stderr, "Camera cannot be opened!\n");
		exit(1);
	}
	VideoFaceDetector detector(DETECTOR_PATH, camera);

	// Set up evaluation (area of focus / point of interest)
	
	// fprintf(stderr, "while\n");
	Mat frame, gray, crop;
	Rect box;
	while(true) {
		auto start = cv::getCPUTickCount();
		detector >> frame;
		auto end = cv::getCPUTickCount();
		double time_per_frame = (end - start) / cv::getTickFrequency();
        double fps = (15 * fps + (1 / time_per_frame)) / 16;

		if (frame.empty()) {
			break;
		}
		if (detector.isFaceFound()) {
			if (frame.channels() > 1)
				cvtColor(frame, gray, COLOR_BGR2GRAY);
			else
				gray = frame;
			box = detector.face();
			// frame.convertTo(frame, CV_32F);
			// frame = frame.reshape(1, 1);
			cv::rectangle(gray, box, cv::Scalar(0, 255, 0));
			cv::Mat ROI(gray, box);
            ROI.copyTo(crop);

			resize(crop, crop, Size(100, 100), 0, 0, INTER_AREA);
			
			vector<float> descriptors;
			hog.compute(crop, descriptors);
			
			vector<float> response;
			classifier->predict(descriptors, response);
			
			auto endclassify = cv::getCPUTickCount();
			double time_per_frame_and_classify = (endclassify - start) / cv::getTickFrequency();
			double fps_classify = (15* fps + (1 / time_per_frame_and_classify)) / 16;
			// fprintf(stderr, "Time per frame: %3.3f\tFPS: %3.3f", time_per_frame, fps);
			// fprintf(stderr, "\tTime for get frame, detection, and classification: %3.3f\tFPS: %3.3f\n", time_per_frame_and_classify, fps_classify);
			evaluate(int(response[0]));

			putText(frame, modes[int(response[0])], Point(30,30), 
    			FONT_HERSHEY_COMPLEX_SMALL, 1.5, Scalar(0, 0, 0), 1, CV_AA);
			ostringstream strs;
			strs << fps_classify;
			string fpsstr = strs.str();
			putText(frame, fpsstr, Point(1000, 30),
				FONT_HERSHEY_COMPLEX_SMALL, 1.5, Scalar(0, 0, 0), 1, CV_AA);
			imshow(WINDOW_NAME, frame);
			waitKey(1);
		}
		if (waitKey(300) == 27) break; // ESC
	}
	camera.release();
	fprintf(stdout, "MODE: %s total: %d positive: %d accuracy %.3f%%\n", modes[mode].c_str(), total, pos, (pos == 0) ? 0.0 : float(pos)/float(total) * 100);
}

void evaluate(int classy) { 
	total++;
	if (classy == mode) {
		pos++;
	}
	fprintf(stderr, "%s\n", modes[classy].c_str());
}