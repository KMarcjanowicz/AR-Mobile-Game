#include "opencv2/objdetect.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <iostream>
#include <stdio.h>

using namespace std;
using namespace cv;

// Declare structure to be used to pass data from C++ to Mono.
struct Circle
{
	Circle(int x, int y, int radius) : X(x), Y(y), Radius(radius) {}
	int X, Y, Radius;
};

CascadeClassifier _faceCascade;
CascadeClassifier _eyeCascade;
String _windowName = "Unity OpenCV Interop Sample";
VideoCapture _capture;
int _scale = 1;

Point makeStable(vector<Point>& points, int iteration);
Vec3f eyeBallDetection(Mat& eye, vector<Vec3f>& circles);
Rect detectLeftEye(vector<Rect>& eyes);


extern "C" int __declspec(dllexport) __stdcall  Init(int& outCameraWidth, int& outCameraHeight)
{
	// Load haar face cascade.
	if (!_faceCascade.load("haarcascade_frontalface_default.xml"))
		return -1;

	// Load haar eye cascade.
	if (!_eyeCascade.load("haarcascade_eye.xml"))
		return -1;

	// Open the stream.
	_capture.open(0);
	if (!_capture.isOpened())
		return -2;

	outCameraWidth = _capture.get(CAP_PROP_FRAME_WIDTH);
	outCameraHeight = _capture.get(CAP_PROP_FRAME_HEIGHT);

	return 0;
}

extern "C" void __declspec(dllexport) __stdcall  Close()
{
	_capture.release();
}

extern "C" void __declspec(dllexport) __stdcall SetScale(int scale)
{
	_scale = scale;
}

extern "C" void __declspec(dllexport) __stdcall Detect(Circle * outFaces, int maxOutFacesCount, int& outDetectedFacesCount)
{
	Mat frame;
	_capture >> frame;
	if (frame.empty())
		return;

	std::vector<Rect> faces;
	// Convert the frame to grayscale for cascade detection.
	Mat grayscaleFrame;
	cvtColor(frame, grayscaleFrame, COLOR_BGR2GRAY);
	Mat resizedGray;
	// Scale down for better performance.
	resize(grayscaleFrame, resizedGray, Size(frame.cols / _scale, frame.rows / _scale));
	equalizeHist(resizedGray, resizedGray);

	// Detect faces.
	_faceCascade.detectMultiScale(resizedGray, faces);

	// Draw faces.
	for (size_t i = 0; i < faces.size(); i++)
	{
		Point center(_scale * (faces[i].x + faces[i].width / 2), _scale * (faces[i].y + faces[i].height / 2));
		ellipse(frame, center, Size(_scale * faces[i].width / 2, _scale * faces[i].height / 2), 0, 0, 360, Scalar(0, 0, 255), 4, 8, 0);

		// Send to application.
		outFaces[i] = Circle(faces[i].x, faces[i].y, faces[i].width / 2);
		outDetectedFacesCount++;

		
		Mat face = frame(faces[i]);
		std::vector<Rect> eyes;

		_eyeCascade.detectMultiScale(face, eyes, 1.1, 2, 0 | CASCADE_SCALE_IMAGE, Size(10, 10));//detect eyes in every face//

		if (eyes.size() == 2) {
			//Drawing the rectangle around the eyes//
			for (Rect& eye : eyes) {
				rectangle(frame, faces[i].tl() + eye.tl(), faces[i].tl() + eye.br(), Scalar(0, 255, 0), 2); // draw rectangle around eyes

				Rect eyeRect = detectLeftEye(eyes);

				Mat eye = face(eyeRect);

				//pre proccess
				cvtColor(eye, eye, COLOR_BGR2GRAY);
				equalizeHist(eye, eye);
				GaussianBlur(eye, eye, Size(3, 3), 3, 0);

				Point track_Eyeball;
				vector<Point>centers;

				//Applying Hough Circles to detect the circles in eye region//
				Mat hough_Circle_Input = eye;
				vector<Vec3f>circles;
				int method = 3;
				int detect_Pixel = 1;
				int minimum_Distance = eye.cols / 8;
				int threshold = 250;
				int minimum_Area = 15;
				int minimum_Radius = eye.rows / 8;
				int maximum_Radius = eye.rows / 3;
				HoughCircles(hough_Circle_Input, circles,
					HOUGH_GRADIENT, detect_Pixel, minimum_Distance, threshold, minimum_Area, minimum_Radius, maximum_Radius);
				//Detecting the drawing circle that encloses the eyeball//
				if (circles.size() > 0) {
					Vec3f eyeball = eyeBallDetection(eye, circles);
					Point center(eyeball[0], eyeball[1]);
					centers.push_back(center);
					center = makeStable(centers, 5);
					track_Eyeball = center;
					int radius = (int)eyeball[2];
					circle(frame, faces[i].tl() + eyeRect.tl() + center, radius, Scalar(0, 0, 255), 2);

				}
				imshow("Eye", eye);
			}
		}
		

		if (outDetectedFacesCount == maxOutFacesCount)
			break;
	}

	// Display debug output.
	imshow(_windowName, frame);
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