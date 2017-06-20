#include <sstream>
#include <string>
#include <iostream>
#include <highgui.h>
#include <cv.h>

using namespace cv;

int H_MIN = 0;
int H_MAX = 256;
int S_MIN = 0;
int S_MAX = 256;
int V_MIN = 0;
int V_MAX = 256;

const int FRAME_WIDTH = 640;
const int FRAME_HEIGHT = 480;

const int MAX_NUM_OBJECTS=50;

const int MIN_OBJECT_AREA = 20*20;
const int MAX_OBJECT_AREA = FRAME_HEIGHT*FRAME_WIDTH/1.5;

const string windowName = "Original Image";
const string windowName1 = "HSV Image";
const string windowName2 = "Thresholded Image";
const string windowName3 = "After Morphological Operations";
const string trackbarWindowName = "Trackbars";

void on_trackbar( int, void* )
{

}

string intToString(int number){

	std::stringstream ss;
	ss << number;
	return ss.str();
	
}

void createTrackbars(){

    namedWindow(trackbarWindowName,0);

    createTrackbar( "H_MIN", trackbarWindowName, &H_MIN, H_MAX, on_trackbar );
    createTrackbar( "H_MAX", trackbarWindowName, &H_MAX, H_MAX, on_trackbar );
    createTrackbar( "S_MIN", trackbarWindowName, &S_MIN, S_MAX, on_trackbar );
    createTrackbar( "S_MAX", trackbarWindowName, &S_MAX, S_MAX, on_trackbar );
    createTrackbar( "V_MIN", trackbarWindowName, &V_MIN, V_MAX, on_trackbar );
    createTrackbar( "V_MAX", trackbarWindowName, &V_MAX, V_MAX, on_trackbar );


}
void drawObject(int x, int y,Mat &frame){

	//uzycie funkcji opencv do rysowania celownika

	circle(frame,Point(x,y),20,Scalar(0,255,0),2);
    if(y-25>0)
    line(frame,Point(x,y),Point(x,y-25),Scalar(0,255,0),2);
    else line(frame,Point(x,y),Point(x,0),Scalar(0,255,0),2);
    if(y+25<FRAME_HEIGHT)
    line(frame,Point(x,y),Point(x,y+25),Scalar(0,255,0),2);
    else line(frame,Point(x,y),Point(x,FRAME_HEIGHT),Scalar(0,255,0),2);
    if(x-25>0)
    line(frame,Point(x,y),Point(x-25,y),Scalar(0,255,0),2);
    else line(frame,Point(x,y),Point(0,y),Scalar(0,255,0),2);
    if(x+25<FRAME_WIDTH)
    line(frame,Point(x,y),Point(x+25,y),Scalar(0,255,0),2);
    else line(frame,Point(x,y),Point(FRAME_WIDTH,y),Scalar(0,255,0),2);

	putText(frame,intToString(x)+","+intToString(y),Point(x,y+30),1,1,Scalar(0,255,0),2);

}
void morphOps(Mat &thresh){



	Mat erodeElement = getStructuringElement( MORPH_RECT,Size(3,3));
	Mat dilateElement = getStructuringElement( MORPH_RECT,Size(8,8));

	erode(thresh,thresh,erodeElement);
	erode(thresh,thresh,erodeElement);


	dilate(thresh,thresh,dilateElement);
	dilate(thresh,thresh,dilateElement);
	


}
void trackFilteredObject(int &x, int &y, Mat threshold, Mat &cameraFeed){

	Mat temp;
	threshold.copyTo(temp);
	//dwa wektory potrzebne do wyjscia findcountours
	vector< vector<Point> > contours;
	vector<Vec4i> hierarchy;
	//znajdz kontury
	findContours(temp,contours,hierarchy,CV_RETR_CCOMP,CV_CHAIN_APPROX_SIMPLE );
	double refArea = 0;
	bool objectFound = false;
	if (hierarchy.size() > 0) {
		int numObjects = hierarchy.size();
        //jesli wieksze to szum
        if(numObjects<MAX_NUM_OBJECTS){
			for (int index = 0; index >= 0; index = hierarchy[index][0]) {

				Moments moment = moments((cv::Mat)contours[index]);
				double area = moment.m00;

				
                if(area>MIN_OBJECT_AREA && area<MAX_OBJECT_AREA && area>refArea){
					x = moment.m10/area;
					y = moment.m01/area;
					objectFound = true;
					refArea = area;

					
					VideoCapture cap(0); 


					int iLastX = -1; 
 					int iLastY = -1;

					
 					Mat imgTmp;
 					cap.read(imgTmp); 

  					
 					Mat imgLines = Mat::zeros( imgTmp.size(), CV_8UC3 );;

					if (iLastX >= 0 && iLastY >= 0 && x >= 0 && y >= 0)
   					{
    				line(imgLines, Point(x, y), Point(iLastX, iLastY), Scalar(0,0,255), 2);
   					}

    				iLastX = x;
   					iLastY = y;
  				}
				else objectFound = false;


			}
			//jak znajdzie to rysuj
			if(objectFound ==true){
				putText(cameraFeed,"Tracking Object",Point(0,50),2,1,Scalar(0,255,0),2);
				
				drawObject(x,y,cameraFeed);}

		}else putText(cameraFeed,"TOO MUCH NOISE! ADJUST FILTER",Point(0,50),1,2,Scalar(0,0,255),2);
	}
}
int main(int argc, char* argv[])
{

    bool trackObjects = true;
    bool useMorphOps = true;
	
	Mat cameraFeed;
	
	Mat HSV;
	
	Mat threshold;
	
	int x=0, y=0;
	
	createTrackbars();
	
	VideoCapture capture;
	
	capture.open(0);
	
	capture.set(CV_CAP_PROP_FRAME_WIDTH,FRAME_WIDTH);
	capture.set(CV_CAP_PROP_FRAME_HEIGHT,FRAME_HEIGHT);
	
	while(1){
		
		capture.read(cameraFeed);
		
		cvtColor(cameraFeed,HSV,COLOR_BGR2HSV);
		
		inRange(HSV,Scalar(H_MIN,S_MIN,V_MIN),Scalar(H_MAX,S_MAX,V_MAX),threshold);

		if(useMorphOps)
		morphOps(threshold);

		if(trackObjects)
			trackFilteredObject(x,y,threshold,cameraFeed);

		 
		imshow(windowName2,threshold);
		
		imshow(windowName,cameraFeed);
		
		

		waitKey(30);
	}






	return 0;
}

