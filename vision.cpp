#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <cmath>
#include <stdio.h>
#include "include/networktables/NetworkTable.h"
//#include <opencv2/opencv.hpp>




#define real true

//This is here for later cmake -DCUDA_USE_STATIC_CUDA_RUNTIME=OFF .
using namespace cv;


cv::Mat  matOriginal,matThresh, matGray, matThreshCopy, input, matFlipped;
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


void NetworkTables(double azi,double dist) {

NetworkTable::SetClientMode();
NetworkTable::SetTeam(668);
auto table = NetworkTable::GetTable("SmartDashboard");
table->PutNumber("Azimuth",azi);
table->PutNumber("Distance",dist);


}

void processImage(){

	vector<vector<Point> > contours;

	while(true){

		contours.clear();

		cap.read(input);

//		Point2f center(input.cols/2.0, input.rows/2.0);
//                Mat rot_mat = getRotationMatrix2D(center, 180, 1.0);
//		warpAffine(input, matOriginal, rot_mat, input.size());
	
		cv::flip(input, matFlipped, -1);
 		GaussianBlur(matFlipped, matOriginal, Size(5,5), 0, 0); 	
		cvtColor(matOriginal, matGray, COLOR_BGR2GRAY);

	 

//		inRange(matBlurred, LOWER_BOUNDS, UPPER_BOUNDS, matGray);

		threshold(matGray, matThresh, 16.0, 255, THRESH_BINARY);
//		Mat x = matThresh.clone();
		threshold(matGray, matThreshCopy, 16.0, 255, THRESH_BINARY);

		findContours(matThreshCopy, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);



		int xVals [2];
                int yVals [2];
		int side1 [2];
                int side2 [2];

                int index = 0;
		double topOfBox = 0;
		double bottomOfBox = 0;
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

					  double area = (double)rec.height * (double)rec.width;
					int R = (int)matOriginal.at<Vec3b>((int)((rec.tl().y + rec.br().y)/2), (int)((rec.br().x + rec.tl().x)/2))[2];
				if(area > 100){ 
					 if(G>150&&R<20){
					std::cout <<" B:"<< (int)matOriginal.at<Vec3b>((int)((rec.tl().y + rec.br().y)/2), (int)((rec.br().x + rec.tl().x)/2))[0];
                                        std::cout <<" G:"<< (int)matOriginal.at<Vec3b>((int)((rec.tl().y + rec.br().y)/2), (int)((rec.br().x + rec.tl().x)/2))[1];
                                        std::cout <<" R:"<< (int)matOriginal.at<Vec3b>((int)((rec.tl().y + rec.br().y)/2), (int)((rec.br().x + rec.tl().x)/2))[2]<<std::endl;
					//This code prints out the BGR values for the center pixel that we reference for 
						
					//std::cout <<" Ratio: "<< ratio<<std::endl;
//					double area = (double)rec.height * (double)rec.width;
					std::cout <<" Area : "<< area<<std::endl;
				
					rectangle(matOriginal, rec.br(), rec.tl(), RED);
					circle(matOriginal, Point((int)((rec.br().x + rec.tl().x)/2), (int)((rec.tl().y + rec.br().y)/2)), 10, RED, 2, 8, 0);
					
					
 
					if (index == 0){                                       
 				        xVals[index] = rec.br().x;
                                                yVals[index] = rec.br().y;
						side1[0] = rec.br().y;
						side1[1] = rec.tl().y;
						
                                        }else if (index == 1){
						
						 xVals[index] = rec.tl().x;
						
					                                                yVals[index] = rec.tl().y;
					
						
						side2[0] = rec.br().y;
						
						
						side2[1] = rec.tl().y;
						
					}
                                       		}
				} // if g>100

                                        index++;
				} // aspect ratio?

			} // poly size?
		} // for contours.com
	
		//Drawing Center Point
		double avgX = (xVals[0] + xVals[1])/2;

                double avgY = (yVals[0] + yVals[1])/2; 


		



		circle(matOriginal, Point(avgX, avgY), 10, RED, 2, 8, 0);

		//Square ratio comparison
		double s1 = side1[0]-side1[1];	
		double s2 = side2[0]-side2[1];

		double SquareRatio = s1/s2;

		
//		std::cout <<" Ratio: "<<SquareRatio<<std::endl;
//		int angle = (-176.2673*SquareRatio)+177.7535;
//		std::cout <<" Angle: "<<angle<<std::endl;
		




		//Declaration of constants for calculations

		double CAMERA_ANGLE = 7.5;
 		double HORIZONTAL_FOV = 82.15;
		double VERTICAL_FOV = 82.15;
		double TOP_TARGET_HEIGHT = 15.75;
		double TOP_CAMERA_HEIGHT = 4.75;
		double PI = 3.14159;

		//This is calculating the Azimuth
		double targetX = (2* (avgX /matOriginal.cols))-1;
  		double azi = normalize360((targetX)* HORIZONTAL_FOV / 2.0);


		//This is calculating the distance from the target
	
 
		double targetY = -((2 * (avgY/ matOriginal.rows)) - 1);
		
		double hyp = (TOP_TARGET_HEIGHT - TOP_CAMERA_HEIGHT)/ tan((targetY * VERTICAL_FOV / 2.0 + CAMERA_ANGLE) *PI /180);

		double  distance = sqrt((hyp*hyp) - (162.5625)); 

		//This prints the values to the picture
		char AzimuthCharArr [80];
		sprintf(AzimuthCharArr, "Azimuth: %f", azi); // this is old-school c. good to know gary's good for something.
//		std::cout <<"Azimuth"<<azi<<std::endl;
		putText(matOriginal, AzimuthCharArr, cvPoint(30,30), FONT_HERSHEY_COMPLEX_SMALL, 0.8, RED, 1, CV_AA);


		char DistanceCharArr [80];
		sprintf(DistanceCharArr, "Distance: %f", distance);
//		std::cout <<"Distance"<<distance<<std::endl;
		putText(matOriginal, DistanceCharArr, cvPoint(30,60), FONT_HERSHEY_COMPLEX_SMALL, 0.8, RED, 1, CV_AA);


		NetworkTables(azi,distance);
//		imshow("Thresh", contours);
//		imshow("Center", matTest);
//		imshow("Original", matOriginal);
//		VideoWriter::write(matOriginal);
		if (waitKey(66) >= 0)
			break; 

	}

}


int main(){

//This initalizes the exposure and contrast for the camera 
system("xterm -hold -e ' v4l2-ctl -c exposure_auto=1 && v4l2-ctl --set-ctrl exposure_absolute=20 && v4l2-ctl --set-ctrl contrast=255  && kill| less' &");

	cap.open(0);

	processImage();

	return 0;
}
