/** Usage:
  * Runs through a specified video file, showing the user frame-by-frame images.
  * The user can select which directory to store the image in if the user deems it
  * useful for training/test data. 
  *
  * The user can enter numbers between 0 and the length of the classes array -1 to save images
  * to intermediate directories for processing of images (running detectors through the directories).
  *
  * If the image appears to be of no use to adding to the dataset, 'n' can be used to skip the image. 
  * This can be used to 'sift' through video files and make labelling very simple. Very useful for expiditing
  * the labelling process. 
  * 
  * 
  */

#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <dirent.h>

using namespace std;
using namespace cv;

const std::string person = "dana";
const cv::String WINDOW_NAME("File video");
// const std::string VIDPATH("/PATH/TO/DATASET/" + person + ".eyesclosed.2.mp4"); //example video filenames
// const std::string filepath("/PATH/TO/DATASET/" + person + ".left.detector.mp4");
const std::string DIRECTORY("/Users/ivanjrivera/Dropbox/dataset/%stmp/");
const std::string SAVE_PATH(DIRECTORY + person + ".%s.m.%d%c.jpg"); // save path for useful images.
// 									0			1			2			3				4
const std::string classes[] = { "forward\0", "eyesclosed\0", "right\0", "centerstack\0", "rearviewmirror\0" };
int saved[] = { 0, 0, 0, 0, 0 };

void test(void);
// void resizeAll(int size);
std::vector<string> getFiles(void);
// bool isdigits(const std::string &str);

/* Saves frame to the directory specified by classy with unique, sorted naming for ease of viewing in gridview (up to 260) */
void save(int classy, Mat &frame) {
	static char letlab[] = { 'a', 'a', 'a', 'a', 'a' };
	static int num[] = { 0, 0, 0, 0, 0 };

	char path[90];
	sprintf(path, SAVE_PATH.c_str(), classes[classy].c_str(), classes[classy].c_str(), num[classy], letlab[classy]);
	if (letlab[classy] == 'z') { num[classy]++; letlab[classy] = 'a'; }
	else letlab[classy]++;
	fprintf(stdout, "\tSave #%d. saving: %s\n", saved[classy], path);
	imwrite(path, frame);
}

int main(int argc, char ** argv) {
	for (int i = 0; i < argc; i++)
		fprintf(stderr, "argc %d: %s\n", i, argv[i]);
	if (argc > 1) {
		if (argv[1][1] == 't') {
			test();
		}/*
		else if (argv[1][1] == 'r') { // option for resize
			if (argc > 2) {
				string num(argv[2]);
				if (isdigits(num)) {
					fprintf(stderr, "stoi(num) = %d\n", stoi(num));
					resizeAll(stoi(num));
				}
			}
			fprintf(stderr, "Usage: ./sift -r <new_size>");
			exit(-1);
		}*/
	}
	
	cv::VideoCapture camera(0);
	// cv::VideoCapture camera(VIDPATH);
	if (!camera.isOpened()) {
		fprintf(stderr, "Camera could not be opened!\n");
		exit(1);
	}
	
	namedWindow(WINDOW_NAME, cv::WINDOW_KEEPRATIO);

	Mat frame;
	int total = -1;
	while (true) {	
		camera >> frame;
		if (frame.empty()) {
			fprintf(stderr, "\tEmpty frame!");
			break;
		}
		fprintf(stderr, "Frame col: %d, row: %d\n", frame.cols, frame.rows);
		total++;
		imshow(WINDOW_NAME, frame);
		char c;

		do {
			c = waitKey(400);
		} while ((c < '0' || c > '4') && (c != 'n' && c != 'q'));

		int num = -1;
		switch(c) {
			case '0': num = 0; break;
			case '1': num = 1; break;
			case '2': num = 2; break;
			case '3': num = 3; break;
			case '4': num = 4;
		}
		if (num > -1) {
			save(num, frame);
			saved[num]++;
			num = -1;
		}
		if (c == 'q') break;
	}
	fprintf(stdout, "parsed through %d frames\n", total);
	for (int i = 0; i < 5; i++)
		fprintf(stdout, "%s: %d\n", classes[i].c_str(), saved[i]);
}

/*  */
void test(void) {
	std::vector<string> files = getFiles();
	for (int i = 0; i < files.size(); i++) {
		Mat frame = imread(files[i]);
	    imshow(WINDOW_NAME, frame);
	    char c;
	    do {
	    	c = waitKey(400);
	    } while(c != 'n');
	}
	exit(0);
}
/* FILENAMES IN A DIRECTORY TO VECTOR<STRING> AND RETURNS VECTOR */
std::vector<string> getFiles(void) {
	std::vector<std::string> files;
    struct dirent *ep; 
    DIR *dp = opendir(DIRECTORY.c_str());
    if (dp != NULL) {
        readdir(dp); readdir(dp); // SKIP . and ..
        while ((ep = readdir(dp))) {
            char *name = new char[strlen(DIRECTORY.c_str()) + sizeof(ep->d_name)];
            strcpy(name, DIRECTORY.c_str());
            strcat(name, ep->d_name);
            files.push_back(name);
            delete[] name;
        }
        (void) closedir(dp);
    }
    else
        perror("Couldn't open the directory");
    std::sort(files.begin(), files.end());
    fprintf(stderr, "%lu total items in directory\n", files.size());
    return files;
}
/* used to resize files. Was functional prior to a recent edit. */
/*
void resizeAll(int size) {
	DIR *dp = opendir(DIRECTORY.c_str());
	struct dirent *ep;
	if (dp != NULL) {
		readdir(dp); readdir(dp); // SKIP . and ..
        while ((ep = readdir(dp))) {
        	if (!strcmp(ep->d_name, ".DS_Store")) continue; // skip macOS's .DS_Store file
            char *name = new char[strlen(DIRECTORY.c_str()) + sizeof(ep->d_name) + 1];
            strcpy(name, DIRECTORY.c_str());
            strcat(name, "_");
            strcat(name, ep->d_name);
            Mat tmp = imread(DIRECTORY + ep->d_name);
            fprintf(stderr, "name: %s", name + 45);
            fprintf(stderr, " cols: %d, rows: %d\n", tmp.cols, tmp.rows);
            cv::resize(tmp, tmp, Size(size, size), (size < tmp.cols) ? cv::INTER_AREA : cv::INTER_CUBIC);
            assert(tmp.cols == size && tmp.rows == size);
            
            imwrite(name, tmp);
            delete[] name;
        }
        (void) closedir(dp);
	}
	else perror("Coudln't open the directory");
	exit(0);
}*/

// bool isdigits(const std::string &str) { return str.find_first_not_of("0123456789") == std::string::npos; }