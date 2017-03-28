#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <cmath>
#include <stdio.h>
#include "include/networktables/NetworkTable.h"
#define real true

//This is here for later cmake -DCUDA_USE_STATIC_CUDA_RUNTIME=OFF .
using namespace cv;



VideoCapture cap;
Mat input, matBlur,matGray, matThresh;

const Scalar RED(0, 0, 255);

double blue_scale = .80;
double red_scale = .95;


int topLeftX [2];
int topLeftY [2];
int bottomRightX [2];
int bottomRightY [2];

double CAMERA_ANGLE = 11.5;
double HORIZONTAL_FOV = 82.15;
double VERTICAL_FOV = 82.15;
double TOP_TARGET_HEIGHT = 15.75;
double TOP_CAMERA_HEIGHT = 2.55;
double PI = 3.14159;

double normalize360(double angle) {

		while (angle >= 360.0) {
			angle -= 360.0;
		}
		while (angle < 0.0) {
			angle += 360.0;
		}
		return angle;
}


void processImage(){

	vector<vector<Point> > contours;

	while(true){
	
		
		cap.read(input);
		GaussianBlur(input, matBlur, Size(5,5), 0, 0); 
		cvtColor(matBlur, matGray, COLOR_BGR2GRAY);
		
		

// This converts to grayscale using the formula gray = green - (blue*blue_scale + red*red_scale)

		for(int x = 0; x<input.rows;x++){ //Loops through every pixel of the image
			for(int y = 0; y<input.cols;y++){
			//Find the BGR values at every point
			int B = (int)input.at<Vec3b>(x,y)[0];
			int G = (int)input.at<Vec3b>(x,y)[1];
			int R = (int)input.at<Vec3b>(x,y)[2];
			//gray = green - (blue*blue_scale + red*red_scale)
			int Gray = G - (B*blue_scale+R*red_scale);
			//If gray is less negative it is equal to 0
			if(Gray<0){
			Gray = 0;
			}
			//Sets the point to the gray value
			matGray.at<uchar>(x,y) = Gray;
			}
		}

		//We convert to thresh and then run contour detection
		threshold(matGray, matThresh, 16.0, 255, THRESH_BINARY);
		findContours(matThresh, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
		int index = 0;
		for(int i = 0; i < contours.size(); i++){
			
			vector<Point> matOfPoint = contours[i];	
			vector<Point> approxPoly;
			approxPolyDP(matOfPoint, approxPoly, arcLength(matOfPoint, true) * .02, true);
			

			if(approxPoly.size() == 4||approxPoly.size() == 5 ){
	
			Rect rec = boundingRect(contours[i]);

			double ratio = (double)rec.height/(double)rec.width;
			double area = (double)rec.height * (double)rec.width;

				if(ratio<3&&ratio>1.7){
					if(area > 100){
				
					rectangle(input, rec.br(), rec.tl(), RED);
					 

						
						topLeftX[index] = rec.tl().x;
						topLeftY[index] = rec.tl().y;                                      
 				        	bottomRightX[index] = rec.br().x;
                                                bottomRightY[index] = rec.br().y;
						index++;
						
			
			}
		}

	}
		

}

		double avgX = (topLeftX[0] + bottomRightX[1])/2;
                double avgY = (topLeftY[0] + bottomRightY[1])/2; 

		circle(input, Point(avgX, avgY), 10, RED, 2, 8, 0);

		
		double targetX = (2* (avgX /input.cols))-1;
  		double azi = normalize360((targetX)* HORIZONTAL_FOV / 2.0);		

		double targetY = -((2 * (topLeftY[0]/ input.rows)) - 1);
		double hyp = (TOP_TARGET_HEIGHT - TOP_CAMERA_HEIGHT)/ tan((targetY * VERTICAL_FOV / 2.0 + CAMERA_ANGLE) *PI /180);
		double  distance = sqrt((hyp*hyp) - (162.5625)); 



		//This prints the values to the picture
		char AzimuthCharArr [80];
		sprintf(AzimuthCharArr, "Azimuth: %f", azi); // this is old-school c. good to know gary's good for something.
		putText(input, AzimuthCharArr, cvPoint(30,30), FONT_HERSHEY_COMPLEX_SMALL, 0.8, RED, 1, CV_AA);



		char DistanceCharArr [80];
		sprintf(DistanceCharArr, "Distance: %f", distance);
		putText(input, DistanceCharArr, cvPoint(30,60), FONT_HERSHEY_COMPLEX_SMALL, 0.8, RED, 1, CV_AA);


		std::cout <<"Distance: "<< distance<<std::endl;


		imshow("gray", matGray);
		imshow("input", input);

		
		
		if (waitKey(66) >= 0)
			break; 

	}

}


int main(){

//This initalizes the exposure and contrast for the camera 
system("xterm -hold -e ' v4l2-ctl -c exposure_auto=1 && v4l2-ctl --set-ctrl exposure_absolute=40 && v4l2-ctl --set-ctrl contrast=255  && kill| less' &");

	cap.open(0);

	processImage();

	return 0;
}
