
#include "opencv2/opencv.hpp"
#include <iostream>
#ifdef _MSC_VER
#include "direntWin.h"
#else
#include <dirent.h>
#endif
#include <vector>
#include <errno.h>
#include <time.h>
#include "MorphologicOperations.h"
#include "PixelOperations.h"
#include "NeighborOperations.h"

#include "Logger.h"
#include <stdio.h>


#if defined (WITH_CUDA)
#include "opencv2/gpu/gpu.hpp"
#include "opencv2/gpu/stream_accessor.hpp"
#endif

using namespace cv;
using namespace cv::gpu;


// http://stackoverflow.com/questions/10167534/how-to-find-out-what-type-of-a-mat-object-is-with-mattype-in-opencv 
string type2str(int type) {
	string r;

	uchar depth = type & CV_MAT_DEPTH_MASK;
	uchar chans = 1 + (type >> CV_CN_SHIFT);

	switch (depth) {
	case CV_8U:  r = "8U"; break;
	case CV_8S:  r = "8S"; break;
	case CV_16U: r = "16U"; break;
	case CV_16S: r = "16S"; break;
	case CV_32S: r = "32S"; break;
	case CV_32F: r = "32F"; break;
	case CV_64F: r = "64F"; break;
	default:     r = "User"; break;
	}

	r += "C";
	r += (chans + '0');

	return r;
}

int main (int argc, char **argv){

	if(argc != 3){
		printf("Usage: ./imreconTest <inputImage> <connectivity=4|8>");
	}
	Mat input = imread(argv[1], -1);
	int connectivity = atoi(argv[2]);
	std::cout << "Cols: " << input.cols << " Rows: "<< input.rows<< std::endl; 
	if(input.channels() == 3){
		input = nscale::PixelOperations::bgr2gray(input);

		imwrite("in-cpu-watershed-gray.png", input);
	}
	Mat auxInput;
		input.convertTo(auxInput, CV_16U);
		input = auxInput;


	uint64_t t1, t2;

	std::cout << "Cols: " << input.cols << " Rows: "<< input.rows<< std::endl; 
	t1 = cci::common::event::timestampInUS();

	imwrite("in-cpu-watershed.png", input);
	Mat waterResult;
	waterResult = nscale::watershed(input, connectivity);

	t2 = cci::common::event::timestampInUS();
	//watershed seems to return int32_t matrix
	//check http://stackoverflow.com/questions/10167534/how-to-find-out-what-type-of-a-mat-object-is-with-mattype-in-opencv to interpret type
	std::cout << "cpu watershed loop took " << (t2-t1)/1000 << "ms. Output type is "<<type2str( waterResult.type() )<< ". Depth = "<<waterResult.depth()<<". Channels = "<<waterResult.channels()<< std::endl;

	imwrite("out-watershed.ppm", waterResult);
	return 0;
}
