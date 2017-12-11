 /** USAGE:
   * This program is for running the specified detector on saved images in the directories listed above.
   * Any images without a detected face will trigger a print to stdout of format "rm <filename>"
   * Any images that cannot be opened will yield an error message to stdout.
   * All other prints are to stderr as they are used only for debugging.
   * Result is all images with a face detected are cropped down to the bounding box of the detected face.
   */

 /** BUG INFO:
   * There is a semi-heisenbug in here... It will sometimes throw an exception on some images.
   * Upon re-running the program the program may or may not crash on the same image.
   * Due to time constraints, re-running the program repeatedly was the best solution for
   * getting results. Sometimes the file is overwritten with a null byte, so the file no longer
   * exists after some runs of this program.
   */

#include "opencv2/opencv.hpp"
#include <opencv2/core.hpp>

#include <sys/types.h>
#include <dirent.h>
 
using namespace cv;
using namespace std;

Rect* detectFace( Mat &frame );
Rect* biggestFace(vector<Rect> &faces);

 
CascadeClassifier *cascade;
// tmp dir used to isolate potential training data being prepped from training data that's already good
const std::string DIRECTORY("/PATH/TO/DATASET/%stmp/"); 
// specify the subdirectory prefixes to be used with tmp
const std::string classes[] = { "forward\0", "eyesclosed\0", "right\0", "centerstack\0", "rearviewmirror\0" };
 
int main() 
{ 
	const string filename = "haarcascade_frontalface_alt.xml"; 
	 cascade = new CascadeClassifier(filename);
	if (cascade == NULL) {
		fprintf(stderr, "frontalface cascade could not be loaded.");
		exit(-1);
	}
	else fprintf(stderr, "classifier loaded\n");
 	
	assert( cascade );
	
	// namedWindow( "video", 1 ); 

	int numdirs = sizeof(classes) / sizeof(classes[0]);
	for (int i = 0; i < numdirs; i++) {
		char path[90];
		sprintf(path, DIRECTORY.c_str(), classes[i].c_str());
		fprintf(stderr, "Opening dir '%s'\n", path);
		// while (waitKey(300) != 'n');
		DIR *dp = opendir(path);
		Mat frame, crop;
		if (dp != NULL) {
			struct dirent *ep;
			readdir(dp); readdir(dp); // skip . and ..
			// fprintf(stderr, "while\n");
			while((ep = readdir(dp))) { 
				char *name = new char[sizeof(path) + sizeof(ep->d_name)];
	            strcpy(name, path);
	            strcat(name, ep->d_name);

	            frame = imread(name);
				fprintf(stderr, "\t%s#cols: %d rows: %d\n", name, frame.cols, frame.rows);
				if(frame.empty()) 
				{ 
					fprintf( stdout, "Problem reading frame: %s\n", name ); 
					continue;
				} 
				if (frame.rows < 500)
					continue;
				// fprintf(stderr, "\tdetection\n");
				Rect* face = detectFace( frame ); 
				if (face == NULL) {
					fprintf(stdout, "rm %s\n", name); // print the rm command to remove images that don't have detected faces
					continue;
				}
				Mat ROI(frame, *face);
	            ROI.copyTo(crop);
		        // imshow("video", crop);
		        imwrite(name, crop);
		        fprintf(stderr, "\t\twritten\n");
		        free(name);
		        if(cvWaitKey( 200 ) == 27) break;
			}
		}
		else
			perror("Couldn't open directory");
	}
	// cvDestroyWindow( "video" );
	return 0; 
} 
 
Rect* detectFace( Mat &frame ) 
{
	vector<Rect> faces;
	cascade->detectMultiScale(frame, faces, 1.15, 3, 0, 
		Size(frame.rows / 5, frame.rows / 5), Size(frame.rows * 2 / 3, frame.rows * 2 / 3)); 

    if (faces.empty()) return NULL;
    // alternative: filter out any positive images that aren't the driver's face
    // Rect *biggest = biggestFace(faces);
    // return biggest->width > 200 ? biggest : NULL;
    return biggestFace(faces);
}

Rect* biggestFace(vector<Rect> &faces)
{
    assert(!faces.empty());

    Rect *biggest = &faces[0];
    for (auto &face : faces) {
        if (face.area() > biggest->area())
            biggest = &face;
    }
    return biggest;
}