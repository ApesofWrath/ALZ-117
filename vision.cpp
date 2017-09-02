#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <iostream>
#include <string>

using namespace cv;
using namespace std;

int hUpperLimit = 62;
int hLowerLimit = 96;
int sUpperLimit = 76;
int sLowerLimit = 248;
int vUpperLimit = 27;
int vLowerLimit = 255;

int avgR;
int avgG;
int avgB;

VideoCapture cap;
Mat rgbImg;
Mat grayImg;
Mat binaryImg;
Mat drawing;

bool setExposure= true;

//sean is reallyy bad





void threshImageRGB(){


	cvtColor(rgbImg, binaryImg, CV_RGB2GRAY );
	cvtColor(rgbImg, grayImg, CV_RGB2GRAY );

	for(int x = 0; x<rgbImg.rows;x++){ //Loops through every pixel of the image
		for(int y = 0; y<rgbImg.cols;y++){

        		int B = (int)rgbImg.at<Vec3b>(x,y)[0];
        		int G = (int)rgbImg.at<Vec3b>(x,y)[1];
        		int R = (int)rgbImg.at<Vec3b>(x,y)[2];

			//gray = green - (blue*blue_scale + red*red_scale)
        		int Gray = G - (B*.80+R*.5);

			//If gray is less negative it is equal to 0
			if(Gray<0){
                		Gray = 0;
        		}

			//Sets the point to the gray value
        		grayImg.at<uchar>(x,y) = Gray;

		}
	}

	threshold(grayImg, binaryImg, 16.0, 255, THRESH_BINARY);

}

void threshImageHSV(){

	Mat hsvImg;

	cvtColor(rgbImg, binaryImg, CV_RGB2GRAY );
	cvtColor(rgbImg, grayImg, CV_RGB2GRAY );
	cvtColor(rgbImg, hsvImg, CV_BGR2HSV);

	for(int i = 0;i < rgbImg.rows;i++){
		for(int j = 0;j < rgbImg.cols;j++){

			int Gray = 0;
			Vec3b hsv=hsvImg.at<Vec3b>(i,j);
			int h=hsv.val[0]; //hue
			int s=hsv.val[1]; //saturation
			int v=hsv.val[2]; //value

			if(h < hUpperLimit && h > hUpperLimit){
				 if(s < sUpperLimit && h > sUpperLimit){
					 if(h < hUpperLimit && h > hUpperLimit){

						Gray = 0;

					}
				}
			}

			grayImg.at<uchar>(i,j) = 0;


		}
	}

	threshold(grayImg, binaryImg, 16.0, 255, THRESH_BINARY);

}

double normalize360(double angle) {

                while (angle >= 360.0) {
                        angle -= 360.0;
                }
                while (angle < 0.0) {
                        angle += 360.0;
                }
                return angle;
}

void findAzimuth(int x){
        double HORIZONTAL_FOV = 82.15;
        double targetX = (2* ((double)x /rgbImg.cols))-1;
        double azi = normalize360((targetX)* HORIZONTAL_FOV / 2.0);
        cout<<x<<endl;
}



void findTargets(){

		int numOfTargets = 2;
		int currentTarget = 0;
		int centerPoints[numOfTargets][numOfTargets];
		const Scalar RED(0, 0, 255);

		vector <vector<Point>> contours;
    		vector<Vec4i> hierarchy;

		findContours(binaryImg, contours, hierarchy,RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

		for( int i = 0; i< contours.size(); i++ ){

			//this loops  through each contour and determine with some simple test if is the correct target
     			vector<Point> matOfPoint = contours[i];
			vector<Point> approxPoly;
			approxPolyDP(matOfPoint, approxPoly, arcLength(matOfPoint, true) * .02, true);

			Rect rect = boundingRect(contours[i]);
			int height = rect.br().y - rect.tl().y;
			int width = rect.br().x - rect. tl().x;

			int centerY = rect.tl().y - (height/2);
			int centerX = rect.tl().y - (width/2);
			//the first filter just aproximates things with 4 sides
			if(approxPoly.size() > 2 && approxPoly.size() < 10 ){

			//Second filter is the area of the box. We can assume it is bigger than 100 pixels. That number is complelty made up and should be tuned
				if((height*width)>100){

					rectangle(rgbImg, rect.br(), rect.tl(), RED);
					centerPoints[currentTarget][0] = centerX;
					centerPoints[currentTarget][1] = centerY;

					if(currentTarget >= numOfTargets){
						currentTarget = 0;
					}

				}
			}
		}

		int aziX = (centerPoints[0][0]+centerPoints[0][1])/2;
		if(aziX < 0){
			aziX=aziX*(-1);
		}
		findAzimuth(aziX);

}





int main(){

	cap.open(0);
	while(true){

		Mat bgrImg;
		cap.read(rgbImg);

		//convert the BGR Image to RGB off the bat to avoid  confusion
		//cvtColor(bgrImg, rgbImg, CV_BGR2RGB);
		GaussianBlur(rgbImg, rgbImg, Size(5,5), 0, 0); 
		threshImageRGB();
		findTargets();

		imshow("Input", rgbImg);
		imshow("Output", binaryImg);
		if(setExposure){
			//Tune the exposure value before for every event
			system("v4l2-ctl -c exposure_auto=1 && v4l2-ctl --set-ctrl exposure_absolute=50 && v4l&& v4l2-ctl --set-ctrl contrast=255");
			setExposure = false;

		}

		if (waitKey(66) >= 0){

			break;

		}

	}


	return 0;

}
