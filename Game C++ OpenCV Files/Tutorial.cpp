#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>
#include <chrono>
#include <thread>
#include <iostream>

using namespace cv;
using namespace std;
using namespace std::chrono;

int faceX = 0;
int faceY = 0;
bool isEyeOpen = false;

VideoCapture webcam(0);
String windowName = "WebCam Window"; //Name of the window

Mat frame;
Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3));
CascadeClassifier faceCascade;
CascadeClassifier eyeCascade;

vector<Rect> faces;
vector<Rect> eyes;
Point track_Eyeball;
vector<Point>centers;

void Detect();
void detectEye(Mat _faceROI, int _i);

Vec3f eyeBallDetection(Mat& eye, vector<Vec3f>& circles);
Rect detectLeftEye(vector<Rect>& eyes);
Point makeStable(vector<Point>& points, int iteration);

//extern "C" int __declspec(dllexport) __stdcall  Load()

extern "C" int __declspec(dllexport) __stdcall  Load()
{
	if (!faceCascade.load("haarcascade_frontalface_default.xml")) {
		return -2;
	}
	if (!eyeCascade.load("haarcascade_eye.xml")) {
		return -3;
	}

	webcam.open(0); //open stream

	if (!webcam.isOpened()) {
		return -1;
	}

	return 0;
}

extern "C" void __declspec(dllexport) __stdcall  Quit() {
	webcam.release();
}

void Detect()
{
	resize(frame, frame, Size(300, 200));

	faceCascade.detectMultiScale(frame, faces, 1.1, 10);

	int horizontal = 0; int vertical = 0;

	for (int i = 0; i < faces.size(); i++) { //Initiating for loop to detect the largest face//

		Point center(faces[i].x + faces[i].width * 0.5, faces[i].y + faces[i].height * 0.5);//getting the center of the face//
		horizontal = (faces[i].x + faces[i].width * 0.5);//Getting the horizontal value of coordinate//
		vertical = (faces[i].y + faces[i].width * 0.5);//Getting the vertical value of coordinate//

		faceX = horizontal;
		faceY = vertical;

		rectangle(frame, faces[i].tl(), faces[i].br(), Scalar(255, 0, 255), 3); //drawing rectangle for faces

		Mat faceROI = frame(faces[i]);//Taking area of the face as Region of Interest for eyes//

		detectEye(faceROI, i);
	}
}

void detectEye(Mat _faceROI, int _i)
{
	eyeCascade.detectMultiScale(_faceROI, eyes, 1.1, 2, 0 | CASCADE_SCALE_IMAGE, Size(10, 10));//detect eyes in every face//
	for (size_t j = 0; j < eyes.size(); j++) { //for locating eyes//
		Point center(faces[_i].x + eyes[j].x + eyes[j].width * 0.5, faces[_i].y + eyes[j].y + eyes[j].height * 0.5);//getting the centers of both eyes//
		int radius = cvRound((eyes[j].width + eyes[j].height) * 0.25);//declaring radius of the eye enclosing circles//

		Rect eyeRect = detectLeftEye(eyes);
		Mat eyeROI = _faceROI(eyeRect);

		//pre proccess
		cvtColor(eyeROI, eyeROI, COLOR_BGR2GRAY);
		equalizeHist(eyeROI, eyeROI);
		GaussianBlur(eyeROI, eyeROI, Size(3, 3), 3, 0);


		//Applying Hough Circles to detect the circles in eye region//
		Mat hough_Circle_Input = eyeROI;
		vector<Vec3f>circles;
		int method = 3;
		int detect_Pixel = 1;
		int minimum_Distance = eyeROI.cols / 8;
		int threshold = 250;
		int minimum_Area = 15;
		int minimum_Radius = eyeROI.rows / 8;
		int maximum_Radius = eyeROI.rows / 3;
		HoughCircles(hough_Circle_Input, circles,
			HOUGH_GRADIENT, detect_Pixel, minimum_Distance, threshold, minimum_Area, minimum_Radius, maximum_Radius);
		//Detecting the drawing circle that encloses the eyeball//
		if (circles.size() > 0) {
			Vec3f eyeball = eyeBallDetection(eyeROI, circles);
			Point center(eyeball[0], eyeball[1]);
			centers.push_back(center);
			center = makeStable(centers, 5);
			track_Eyeball = center;
			int radius = (int)eyeball[2];
			if (!isEyeOpen) { isEyeOpen = true; }
		}
		else {
			if (isEyeOpen) { isEyeOpen = false; }
		}
		imshow("Eye", eyeROI);
	}
}

Vec3f eyeBallDetection(Mat& eye, vector<Vec3f>& circles)
{
	vector<int>sums(circles.size(), 0);
	for (int y = 0; y < eye.rows; y++) {
		uchar* data = eye.ptr<uchar>(y);
		for (int x = 0; x < eye.cols; x++) {
			int pixel_value = static_cast<int>(*data);
			for (int i = 0; i < circles.size(); i++) {
				Point center((int)round(circles[i][0]), (int)round(circles[i][1]));
				int radius = (int)round(circles[i][2]);
				if (pow(x - center.x, 2) + pow(y - center.y, 2) < pow(radius, 2)) {
					sums[i] = sums[i] + pixel_value;
				}
			}
			++data;
		}
	}
	int smallestSum = 9999999;
	int smallestSumIndex = -1;
	for (int i = 0; i < circles.size(); i++) {
		if (sums[i] < smallestSum) {
			smallestSum = sums[i];
			smallestSumIndex = i;
		}
	}
	return circles[smallestSumIndex];
}

Rect detectLeftEye(vector<Rect>& eyes) //this works
{
	int leftEye = 99999999;
	int index = 0;
	for (int i = 0; i < eyes.size(); i++) {
		if (eyes[i].tl().x < leftEye) {
			leftEye = eyes[i].tl().x;
			index = i;
		}
	}
	return eyes[index];
}

Point makeStable(vector<Point>& points, int iteration)
{
	float sum_of_X = 0;
	float sum_of_Y = 0;
	int count = 0;
	int j = max(0, (int)(points.size() - iteration));
	int number_of_points = points.size();
	for (j; j < number_of_points; j++) {
		sum_of_X = sum_of_X + points[j].x;
		sum_of_Y = sum_of_Y + points[j].y;
		++count;
	}
	if (count > 0) {
		sum_of_X /= count;
		sum_of_Y /= count;
	}
	return Point(sum_of_X, sum_of_Y);
}