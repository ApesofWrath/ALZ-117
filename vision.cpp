#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <cmath>
#include <stdio.h>
#include "include/networktables/NetworkTable.h"





#define real true

//This is here for later cmake -DCUDA_USE_STATIC_CUDA_RUNTIME=OFF .
using namespace cv;


cv::Mat matOriginal, matThresh, matGray, matThreshCopy;
cv::Mat matColor, matBlurred, matOrigCopy, matTest;


VideoCapture cap;


const Scalar YELLOW(0, 255, 255),
	RED(0, 0, 255),
	GREEN(33, 255, 0),
	MIDDLEGROUND(255, 0, 0),
	LOWER_BOUNDS(0, 40, 0),
	UPPER_BOUNDS(40, 255, 40);

 double normalize360(double angle) {
		while (angle >= 360.0) {
			angle -= 360.0;
		}
		while (angle < 0.0) {
			angle += 360.0;
		}
		return angle;
}


void NetworkTables(double azi) {
NetworkTable::SetClientMode();
NetworkTable::SetTeam(668);
auto table = NetworkTable::GetTable("SmartDashboard");
table->PutNumber("Azimuth",azi);


}

void processImage(){

	vector<vector<Point> > contours;

	while(true){

		contours.clear();

		cap.read(matOriginal);

		//matOriginal = matTest;
		cvtColor(matOriginal, matGray, COLOR_BGR2GRAY);

//		GaussianBlur(matOriginal, matBlurred, Size(7,7), 0, 0); 

//		inRange(matBlurred, LOWER_BOUNDS, UPPER_BOUNDS, matGray);

		threshold(matGray, matThresh, 16.0, 255, THRESH_BINARY);

		threshold(matGray, matThreshCopy, 16.0, 255, THRESH_BINARY);

		findContours(matThreshCopy, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

#if real

		int xVals [2];
                int yVals [2];

                int index = 0;

		for(int i = 0; i < contours.size(); i++){

			vector<Point> matOfPoint = contours[i];

			vector<Point> approxPoly;

			approxPolyDP(matOfPoint, approxPoly, arcLength(matOfPoint, true) * .02, true);

			if(approxPoly.size() == 4 ){

				Rect rec = boundingRect(contours[i]);

				double ratio = (double)rec.height/(double)rec.width;
					if(ratio<3&&ratio>1.7){
					
				
					int G = (int)matOriginal.at<Vec3b>((int)((rec.tl().y + rec.br().y)/2), (int)((rec.br().x + rec.tl().x)/2))[1];
					int B = (int)matOriginal.at<Vec3b>((int)((rec.tl().y + rec.br().y)/2), (int)((rec.br().x + rec.tl().x)/2))[0];
					int R = (int)matOriginal.at<Vec3b>((int)((rec.tl().y + rec.br().y)/2), (int)((rec.br().x + rec.tl().x)/2))[2];
				NetworkTable::SetTeam(668);	

					//This code prints out the BGR values for the center pixel that we reference for 
					
					if(G>200&&R<10){

					std::cout <<" B:"<< (int)matOriginal.at<Vec3b>((int)((rec.tl().y + rec.br().y)/2), (int)((rec.br().x + rec.tl().x)/2))[0];
					std::cout <<" G:"<< (int)matOriginal.at<Vec3b>((int)((rec.tl().y + rec.br().y)/2), (int)((rec.br().x + rec.tl().x)/2))[1];
					std::cout <<" R:"<< (int)matOriginal.at<Vec3b>((int)((rec.tl().y + rec.br().y)/2), (int)((rec.br().x + rec.tl().x)/2))[2];
					std::cout <<" Ratio: "<< ratio<<std::endl;
					
					
					rectangle(matOriginal, rec.br(), rec.tl(), RED);
					circle(matOriginal, Point((int)((rec.br().x + rec.tl().x)/2), (int)((rec.tl().y + rec.br().y)/2)), 10, RED, 2, 8, 0);
					
					
 
					if (index == 0){
                                                xVals[index] = rec.br().x;
                                                yVals[index] = rec.br().y;
                                        }else if (index == 1){

                                                xVals[index] = rec.tl().x;
                                                yVals[index] = rec.tl().y;

                                       		}
					} // if g>100

                                        index++;
				} // aspect ratio?

			} // poly size?
		} // for contours.com
	
		
		double avgX = (xVals[0] + xVals[1])/2;
                double avgY = (yVals[0] + yVals[1])/2; 

		circle(matOriginal, Point(avgX, avgY), 10, RED, 2, 8, 0);

		double CAMERA_ANGLE = 21.5;
 		double HORIZONTAL_FOV = 82.15;
		double VERTICAL_FOV = 82.15;
		double TOP_TARGET_HEIGHT = 15.75;
		double TOP_CAMERA_HEIGHT = .5;
		double PI = 3.14159;

		//This is calculating the Azimuth
		double targetX = (2* (avgX /matOriginal.cols))-1;
  		double azi = normalize360((targetX)* HORIZONTAL_FOV / 2.0);


		//This is calculating the distance from the target
		double targetY = -((2 * (avgY/ matOriginal.rows)) - 1);

		double distance = (TOP_TARGET_HEIGHT - TOP_CAMERA_HEIGHT)/ tan((targetY * VERTICAL_FOV / 2.0 + CAMERA_ANGLE) *PI /180);

		NetworkTables(azi);
		

/*
		double CameraAngleTan = tan ( CameraAngle * PI / 180.0 );
		double DistanceOfPeg = 13.6875;
		
 		double distance =  avgY/CameraAngleTan;
		std::cout <<distance<<std::endl; 
*/

		

		
	


		//This prints the values to the picture
		char AzimuthCharArr [80];
		sprintf(AzimuthCharArr, "Azimuth: %f", azi); // this is old-school c. good to know gary's good for something.
		
		putText(matOriginal, AzimuthCharArr, cvPoint(30,30), FONT_HERSHEY_COMPLEX_SMALL, 0.8, RED, 1, CV_AA);


		char DistanceCharArr [80];
		sprintf(DistanceCharArr, "Distance: %f", distance);
		putText(matOriginal, DistanceCharArr, cvPoint(30,60), FONT_HERSHEY_COMPLEX_SMALL, 0.8, RED, 1, CV_AA);

#else
		int xVals [2];
		int yVals [2];

		int index = 0;

		for(int i = 0; i < contours.size(); i++){

			vector<Point> matOfPoint = contours[i];

                        vector<Point> approxPoly;

                        approxPolyDP(matOfPoint, approxPoly, arcLength(matOfPoint, true) * 0.02, true);

                        if(approxPoly.size() == 4){

				Rect rec = boundingRect(contours[i]);

				if ((abs((rec.height/rec.width) - 2) < 0.3)){// &&  (rec.height > 20 && rec.width > 10)){ //spacial requirements

					rectangle(matOriginal, rec.br(), rec.tl(), RED);

					if (index == 0){
						xVals[index] = rec.br().x;
						yVals[index] = rec.br().y;
					}else if (index == 1){

						xVals[index] = rec.tl().x;
						yVals[index] = rec.tl().y;

					}

					index++;

                        	}else{

					contours.erase(contours.begin() + i); //doesnt work, keep in code

				}
			}


			 
		}//end of for loop

		int avgX = (xVals[0] + xVals[1])/2;
		int avgY = (yVals[0] + yVals[1])/2;

		circle(matOriginal, Point(avgX, avgY), 10, YELLOW, 2, 8, 0);
		
#endif


		imshow("Thresh", matThresh);
		//imshow("Center", matTest);
		imshow("Original", matOriginal);

		if (waitKey(66) >= 0)
			break; 

	}

}


int main(){

system("xterm -hold -e ' v4l2-ctl -c exposure_auto=1&&v4l2-ctl --set-ctrl exposure_absolute=18 && v4l2-ctl --set-ctrl contrast=255 && kill| less' &");

	cap.open(0);

	processImage();

	return 0;
}
