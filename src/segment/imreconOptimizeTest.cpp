/*
 * test.cpp
 *
 *  Created on: Jun 28, 2011
 *      Author: tcpan
 */
#include "cv.h"
#include "highgui.h"
#include <iostream>
#include <dirent.h>
#include <vector>
#include <string>
#include <errno.h>
#include <time.h>
#include "MorphologicOperations.h"
#include "utils.h"
#include <stdio.h>

#include "opencv2/gpu/gpu.hpp"



using namespace cv;
using namespace cv::gpu;

void runTest(const char* markerName, const char* maskName, bool binary) {
	Mat marker, mask, recon;
	if (binary) marker = imread(markerName, 0) > 0;
	else marker = imread(markerName, 0);
	if (binary) mask = imread(maskName, 0) > 0;
	else mask = imread(maskName, 0);

	std::cout << "testing with " << markerName << " + " << maskName << " " << (binary ? "binary" : "grayscale") << std::endl;
	
	Stream stream;
	GpuMat g_marker, g_mask, g_recon;

	uint64_t t1, t2;

	
	t1 = cciutils::ClockGetTime();
	if (binary) recon = nscale::imreconstructBinary<uchar>(marker, mask, 4);
	else recon = nscale::imreconstruct<uchar>(marker, mask, 4);
	t2 = cciutils::ClockGetTime();
	std::cout << "\tcpu recon 4-con took " << t2-t1 << "ms" << std::endl;

	t1 = cciutils::ClockGetTime();
	if (binary) recon = nscale::imreconstructBinary<uchar>(marker, mask, 8);
	else recon = nscale::imreconstruct<uchar>(marker, mask, 8);
	t2 = cciutils::ClockGetTime();
	std::cout << "\tcpu recon 8-con took " << t2-t1 << "ms" << std::endl;
	
	t1 = cciutils::ClockGetTime();
	recon = nscale::imreconstructUChar(marker, mask, 4);
	t2 = cciutils::ClockGetTime();
	std::cout << "\tcpu reconUChar 4-con took " << t2-t1 << "ms" << std::endl;

	t1 = cciutils::ClockGetTime();
	recon = nscale::imreconstructUChar(marker, mask, 8);
	t2 = cciutils::ClockGetTime();
	std::cout << "\tcpu reconUChar 8-con took " << t2-t1 << "ms" << std::endl;

	
	
	
	
	stream.enqueueUpload(marker, g_marker);
	stream.enqueueUpload(mask, g_mask);
	stream.waitForCompletion();
	std::cout << "\tfinished uploading to GPU" << std::endl;

//	t1 = cciutils::ClockGetTime();
//	g_recon = nscale::gpu::imreconstruct2<uchar>(g_marker, g_mask, 4, stream);
//	stream.waitForCompletion();
//	t2 = cciutils::ClockGetTime();
//	std::cout << "\tgpu recon2 4-con took " << t2-t1 << "ms" << std::endl;
//	g_recon.release();
//
//	t1 = cciutils::ClockGetTime();
//	g_recon = nscale::gpu::imreconstruct2<uchar>(g_marker, g_mask, 8, stream);
//	stream.waitForCompletion();
//	t2 = cciutils::ClockGetTime();
//	std::cout << "\tgpu recon2 8-con took " << t2-t1 << "ms" << std::endl;
//	g_recon.release();

	
	t1 = cciutils::ClockGetTime();
	if (binary) g_recon = nscale::gpu::imreconstructBinary<uchar>(g_marker, g_mask, 4, stream);
	else g_recon = nscale::gpu::imreconstruct<uchar>(g_marker, g_mask, 4, stream);
	stream.waitForCompletion();
	t2 = cciutils::ClockGetTime();
	std::cout << "\tgpu recon 4-con took " << t2-t1 << "ms" << std::endl;
	g_recon.release();

	t1 = cciutils::ClockGetTime();
	if (binary) g_recon = nscale::gpu::imreconstructBinary<uchar>(g_marker, g_mask, 8, stream);
	else g_recon = nscale::gpu::imreconstruct<uchar>(g_marker, g_mask, 8, stream);
	stream.waitForCompletion();
	t2 = cciutils::ClockGetTime();
	std::cout << "\tgpu recon 8-con took " << t2-t1 << "ms" << std::endl;
	g_recon.release();

	g_marker.release();
	g_mask.release();


	
}


int main (int argc, char **argv){

	runTest("test/in-bwselect-marker.pgm", "test/in-bwselect-mask.pgm", true);
	runTest("test/in-bwselect-marker.pgm", "test/in-bwselect-mask.pgm", false);
	runTest("test/in-fillholes-bin-marker.pgm", "test/in-fillholes-bin-mask.pgm", true);
	runTest("test/in-fillholes-bin-marker.pgm", "test/in-fillholes-bin-mask.pgm", false);
	runTest("test/in-imrecon-gray-marker.pgm", "test/in-imrecon-gray-mask.pgm", true);
	runTest("test/in-imrecon-gray-marker.pgm", "test/in-imrecon-gray-mask.pgm", false);
		
	return 0;
}

