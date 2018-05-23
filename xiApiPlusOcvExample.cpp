// xiApiPlusOcvExample.cpp : program opens first camera, captures and displays 40 images

#include <stdio.h>
#include "xiApiPlusOcv.hpp"

#include <string>

#include "WinsockMatTransmissionClient.h"

using namespace cv;
using namespace std;

bool imageToStreamFile(cv::Mat image, string filenamechar)
{
    FILE *fpw = fopen(filenamechar.c_str(), "wb");//如果没有则创建，如果存在则从头开始写
    if(fpw == NULL) {
        fclose(fpw);
        return false;
    }
    int channl = image.channels();//第一个字节  通道
    int rows = image.rows;     //四个字节存 行数
    int cols = image.cols;   //四个字节存 列数

    fwrite(&channl, sizeof(char), 1, fpw);
    fwrite(&rows, sizeof(char), 4, fpw);
    fwrite(&cols, sizeof(char), 4, fpw);
    char* dp = (char*)image.data;
    if(channl == 3) {
        for(int i = 0; i < rows*cols; i++) {
            fwrite(&dp[i * 3], sizeof(char), 1, fpw);
            fwrite(&dp[i * 3 + 1], sizeof(char), 1, fpw);
            fwrite(&dp[i * 3 + 2], sizeof(char), 1, fpw);
        }
    } else if(channl == 1) {
        for(int i = 0; i < rows*cols; i++) {
            fwrite(&dp[i], sizeof(char), 1, fpw);
        }
    }
    fclose(fpw);
    return true;
}

int main(int argc, char* argv[])
{
    WinsockMatTransmissionClient socketMat;
    if(socketMat.socketConnect("192.168.1.100", 6666) < 0) {
        return 0;
    }

	try
	{
		// Sample for XIMEA OpenCV
		xiAPIplusCameraOcv cam;

		// Retrieving a handle to the camera device
		printf("Opening first camera...\n");
		cam.OpenFirst();


        cam.SetWidth(640); // This is the width for all regions
        cam.SetHeight(512);

        cam.SetExposureTime(10000);
		cam.StartAcquisition();
		
		printf("First pixel value \n");
		XI_IMG_FORMAT format = cam.GetImageDataFormat();
        printf("Format:%d\n",format);
		#define EXPECTED_IMAGES 1000

		//for (int images=0;images < EXPECTED_IMAGES;images++)
        while(1)
		{

			Mat cv_mat_image = cam.GetNextImageOcvMat();
			if (format == XI_RAW16 || format == XI_MONO16) 
				normalize(cv_mat_image, cv_mat_image, 0, 65536, NORM_MINMAX, -1, Mat()); // 0 - 65536, 16 bit unsigned integer range
			cv::imshow("Image from camera",cv_mat_image);           
            if(cv_mat_image.empty())
                return 0;

            socketMat.transmit(cv_mat_image);
            if(cvWaitKey(20) == 'q') {
                printf("type:%d\n", cv_mat_image.type());
                cvDestroyWindow("Image from camera");
                break;
            }
		}

		cam.StopAcquisition();
		cam.Close();
		printf("Done \n");
		
		cvWaitKey(50000);
	}
	catch(xiAPIplus_Exception& exp)
	{
		printf("Error:\n");
		exp.PrintError();
#ifdef WIN32
		Sleep(20000);
#endif
		cvWaitKey(20000);

		return -1;
	}


    socketMat.socketDisconnect();

    system("pause");
	return 0;
}

