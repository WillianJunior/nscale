/*
 * RedBloodCell.cpp
 *
 *  Created on: Jul 1, 2011
 *      Author: tcpan
 */

#include "HistologicalEntities.h"
#include <iostream>
#include "MorphologicOperations.h"
#include "PixelOperations.h"
#include "highgui.h"
#include "float.h"
#include "utils.h"

namespace nscale {

using namespace cv;


Mat HistologicalEntities::getRBC(const Mat& img) {
	CV_Assert(img.channels() == 3);

	std::vector<Mat> bgr;
	split(img, bgr);
	return getRBC(bgr);
}

Mat HistologicalEntities::getRBC(const std::vector<Mat>& bgr) {
	CV_Assert(bgr.size() == 3);
	/*
	%T1=2.5; T2=2;
    T1=5; T2=4;

	imR2G = double(r)./(double(g)+eps);
    bw1 = imR2G > T1;
    bw2 = imR2G > T2;
    ind = find(bw1);
    if ~isempty(ind)
        [rows, cols]=ind2sub(size(imR2G),ind);
        rbc = bwselect(bw2,cols,rows,8) & (double(r)./(double(b)+eps)>1);
    else
        rbc = zeros(size(imR2G));
    end
	 */
	std::cout.precision(5);
	double T1 = 5.0;
	double T2 = 4.0;
	Size s = bgr[0].size();
	Mat bd(s, CV_32FC1);
	Mat gd(s, bd.type());
	Mat rd(s, bd.type());

	bgr[0].convertTo(bd, bd.type(), 1.0, FLT_EPSILON);
	bgr[1].convertTo(gd, gd.type(), 1.0, FLT_EPSILON);
	bgr[2].convertTo(rd, rd.type(), 1.0, 0.0);

	Mat imR2G = rd / gd;
	Mat imR2B = (rd / bd) > 1.0;
	Mat bw1 = imR2G > T1;
	Mat bw2 = imR2G > T2;
	Mat rbc;
	if (countNonZero(bw1) > 0) {
//		imwrite("test/in-bwselect-marker.pgm", bw2);
//		imwrite("test/in-bwselect-mask.pgm", bw1);
		rbc = bwselect<uchar>(bw2, bw1, 8) & imR2B;
	} else {
		rbc = Mat::zeros(bw2.size(), bw2.type());
	}

	return rbc;
}


int HistologicalEntities::segmentNuclei(const Mat& img, Mat& output, cciutils::SimpleCSVLogger& logger, int stage) {
	// image in BGR format

//	Mat img2(Size(1024,1024), img.type());
//	resize(img, img2, Size(1024,1024));
//	namedWindow("orig image", CV_WINDOW_AUTOSIZE);
//	imshow("orig image", img2);

	/*
	* this part to decide if the tile is background or foreground
	THR = 0.9;
    grayI = rgb2gray(I);
    area_bg = length(find(I(:,:,1)>=220&I(:,:,2)>=220&I(:,:,3)>=220));
    ratio = area_bg/numel(grayI);
    if ratio >= THR
        return;
    end
	 */
	logger.logStart("start");
	if (stage == 0) {
			output = img;
			return 0;
		}

	std::vector<Mat> bgr;
	split(img, bgr);
	logger.logTimeElapsedSinceLastLog("toRGB");


	Mat background = (bgr[0] > 220) & (bgr[1] > 220) & (bgr[2] > 220);

	int bgArea = countNonZero(background);
	float ratio = (float)bgArea / (float)(img.size().area());
//TODO: TMEP	std::cout << " background size: " << bgArea << " ratio: " << ratio << std::endl;
	logger.log("backgroundRatio", ratio);

	if (ratio >= 0.9) {
		//TODO: TEMP std::cout << "background.  next." << std::endl;
		logger.logTimeElapsedSinceLastLog("background");
//		logger.endSession();
		return 0;
	}
	logger.logTimeElapsedSinceLastLog("background");

	if (stage == 1) {
		output = background;
		return 0;
	}

	uint64_t t1 = cciutils::ClockGetTime();
	Mat rbc = nscale::HistologicalEntities::getRBC(bgr);
	uint64_t t2 = cciutils::ClockGetTime();
	logger.logTimeElapsedSinceLastLog("RBC");
	int rbcPixelCount = countNonZero(rbc);
	logger.log("RBCPixCount", rbcPixelCount);
	//TODO: TEMP std::cout << "rbc took " << t2-t1 << "ms" << std::endl;

//	imwrite("test/out-rbc.pbm", rbc);
	if (stage == 2) {
		output = rbc;
		return 0;
	}

	/*
	rc = 255 - r;
    rc_open = imopen(rc, strel('disk',10));
    rc_recon = imreconstruct(rc_open,rc);
    diffIm = rc-rc_recon;
	 */

	Mat rc = nscale::PixelOperations::invert<uchar>(bgr[2]);
	logger.logTimeElapsedSinceLastLog("invert");

	Mat rc_open(rc.size(), rc.type());
	//Mat disk19 = getStructuringElement(MORPH_ELLIPSE, Size(19,19));
	// structuring element is not the same between matlab and opencv.  using the one from matlab explicitly....
	// (for 4, 6, and 8 connected, they are approximations).
	uchar disk19raw[361] = {
			0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
			0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0,
			0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
			0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
			0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
			0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0,
			0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0};
	std::vector<uchar> disk19vec(disk19raw, disk19raw+361);
	Mat disk19(disk19vec);
	disk19 = disk19.reshape(1, 19);
//	imwrite("test/out-rcopen-strel.pbm", disk19);
	morphologyEx(rc, rc_open, CV_MOP_OPEN, disk19, Point(-1, -1), 1);
//	imwrite("test/out-rcopen.ppm", rc_open);
	logger.logTimeElapsedSinceLastLog("open19");

	if (stage == 3) {
		output = rc_open;
		return 0;
	}


//	imwrite("test/in-imrecon-gray-marker.pgm", rc_open);
//	imwrite("test/in-imrecon-gray-mask.pgm", rc);
	Mat rc_recon = nscale::imreconstruct<uchar>(rc_open, rc, 8);
	if (stage == 4) {
		output = rc_recon;
		return 0;
	}

	Mat diffIm = rc - rc_recon;
//	imwrite("test/out-redchannelvalleys.ppm", diffIm);
	logger.logTimeElapsedSinceLastLog("reconToNuclei");
	int rc_openPixelCount = countNonZero(rc_open);
	logger.log("rc_openPixCount", rc_openPixelCount);
	if (stage == 5) {
		output = diffIm;
		return 0;
	}
/*
    G1=80; G2=45; % default settings
    %G1=80; G2=30;  % 2nd run

    bw1 = imfill(diffIm>G1,'holes');
 *
 */
	uchar G1 = 80;
	Mat diffIm2 = diffIm > G1;
	logger.logTimeElapsedSinceLastLog("threshold1");
	if (stage == 6) {
		output = diffIm2;
		return 0;
	}

	Mat bw1 = nscale::imfillHoles<uchar>(diffIm2, true, 4);
//	imwrite("test/out-rcvalleysfilledholes.ppm", bw1);
	logger.logTimeElapsedSinceLastLog("fillHoles1");
	if (stage == 7) {
		output = bw1;
		return 0;
	}


//	// TODO: change back
//	return 0;
/*
 *     %CHANGE
    [L] = bwlabel(bw1, 8);
    stats = regionprops(L, 'Area');
    areas = [stats.Area];

    %CHANGE
    ind = find(areas>10 & areas<1000);
    bw1 = ismember(L,ind);
    bw2 = diffIm>G2;
    ind = find(bw1);

    if isempty(ind)
        return;
    end
 *
 */
	bw1 = nscale::bwareaopen<uchar>(bw1, 11, 1000, 8);
	if (stage == 8) {
		output = bw1;
		return 0;
	}
	if (countNonZero(bw1) == 0) {
		logger.logTimeElapsedSinceLastLog("areaThreshold1");
//		logger.endSession();
		return 0;
	}
//	imwrite("test/out-nucleicandidatessized.ppm", bw1);
	logger.logTimeElapsedSinceLastLog("areaThreshold1");
	if (stage == 9) {
		output = bw1;
		return 0;
	}


	uchar G2 = 45;
	Mat bw2 = diffIm > G2;
	if (stage == 10) {
		output = bw2;
		return 0;
	}


	/*
	 *
    [rows,cols] = ind2sub(size(diffIm),ind);
    seg_norbc = bwselect(bw2,cols,rows,8) & ~rbc;
    seg_nohole = imfill(seg_norbc,'holes');
    seg_open = imopen(seg_nohole,strel('disk',1));
	 *
	 */

	Mat seg_norbc = nscale::bwselect<uchar>(bw2, bw1, 8);
	if (stage == 11) {
		output = seg_norbc;
		return 0;
	}

	seg_norbc = seg_norbc & (rbc == 0);
	if (stage == 12) {
		output = seg_norbc;
		return 0;
	}
//	imwrite("test/out-nucleicandidatesnorbc.ppm", seg_norbc);
	logger.logTimeElapsedSinceLastLog("blobsGt45");

	Mat seg_nohole = nscale::imfillHoles<uchar>(seg_norbc, true, 4);
	logger.logTimeElapsedSinceLastLog("fillHoles2");
	if (stage == 13) {
		output = seg_nohole;
		return 0;
	}

	Mat seg_open = Mat::zeros(seg_nohole.size(), seg_nohole.type());
	Mat disk3 = getStructuringElement(MORPH_ELLIPSE, Size(3,3));
	morphologyEx(seg_nohole, seg_open, CV_MOP_OPEN, disk3, Point(-1, -1), 1);
//	imwrite("test/out-nucleicandidatesopened.ppm", seg_open);
	logger.logTimeElapsedSinceLastLog("openBlobs");
	if (stage == 14) {
		output = seg_open;
		return 0;
	}

	/*
	 *
	seg_big = imdilate(bwareaopen(seg_open,30),strel('disk',1));
	 */
	// bwareaopen is done as a area threshold.
	Mat seg_big_t = nscale::bwareaopen<uchar>(seg_open, 30, std::numeric_limits<int>::max(), 8);
	logger.logTimeElapsedSinceLastLog("30To1000");
	if (stage == 15) {
		output = seg_big_t;
		return 0;
	}

	Mat seg_big = Mat::zeros(seg_big_t.size(), seg_big_t.type());
	dilate(seg_big_t, seg_big, disk3);
	if (stage == 16) {
		output = seg_big;
		return 0;
	}
//	imwrite("test/out-nucleicandidatesbig.ppm", seg_big);
	logger.logTimeElapsedSinceLastLog("dilate");

	/*
	 *
		distance = -bwdist(~seg_big);
		distance(~seg_big) = -Inf;
		distance2 = imhmin(distance, 1);
		 *
	 *
	 */
	// distance transform:  matlab code is doing this:
	// invert the image so nuclei candidates are holes
	// compute the distance (distance of nuclei pixels to background)
	// negate the distance.  so now background is still 0, but nuclei pixels have negative distances
	// set background to -inf

	// really just want the distance map.  CV computes distance to 0.
	// background is 0 in output.
	// then invert to create basins
	Mat dist(seg_big.size(), CV_32FC1);

	distanceTransform(seg_big, dist, CV_DIST_L2, CV_DIST_MASK_PRECISE);
	if (stage == 17) {
		output = dist;
		return 0;
	}

	double mmin, mmax;
	minMaxLoc(dist, &mmin, &mmax);
	dist = (mmax + 1.0) - dist;
	if (stage == 18) {
			output = dist;
			return 0;
		}

//	cciutils::cv::imwriteRaw("test/out-dist", dist);

	// then set the background to -inf and do imhmin
	Mat distance = Mat::zeros(dist.size(), dist.type());
	dist.copyTo(distance, seg_big);
//	cciutils::cv::imwriteRaw("test/out-distance", distance);
	// then do imhmin.
	//Mat distance2 = nscale::imhmin<float>(distance, 1.0f, 8);
	//cciutils::cv::imwriteRaw("test/out-distanceimhmin", distance2);
	logger.logTimeElapsedSinceLastLog("distTransform");
	if (stage == 19) {
			output = distance;
			return 0;
		}


	/*
	 *
		seg_big(watershed(distance2)==0) = 0;
		seg_nonoverlap = seg_big;
     *
	 */

	Mat nuclei = Mat::zeros(img.size(), img.type());
	img.copyTo(nuclei, seg_big);
	logger.logTimeElapsedSinceLastLog("nucleiCopy");
	if (stage == 20) {
			output = nuclei;
			return 0;
		}

	// watershed in openCV requires labels.  input foreground > 0, 0 is background
	Mat watermask = nscale::watershed2(nuclei, distance, 8);
//	cciutils::cv::imwriteRaw("test/out-watershed", watermask);
	if (stage == 21) {
			output = watermask;
			return 0;
		}


	Mat seg_nonoverlap = Mat::zeros(seg_big.size(), seg_big.type());
	seg_big.copyTo(seg_nonoverlap, (watermask > 0));
//	imwrite("test/out-seg_nonoverlap.ppm", seg_nonoverlap);
	logger.logTimeElapsedSinceLastLog("watershed");
	if (stage == 22) {
		output = seg_nonoverlap;
		return 0;
	}


	/*
     %CHANGE
    [L] = bwlabel(seg_nonoverlap, 4);
    stats = regionprops(L, 'Area');
    areas = [stats.Area];

    %CHANGE
    ind = find(areas>20 & areas<1000);

    if isempty(ind)
        return;
    end
    seg = ismember(L,ind);
	 *
	 */
	Mat seg = nscale::bwareaopen<uchar>(seg_nonoverlap, 21, 1000, 4);
	if (stage == 23) {
		output = seg;
		return 0;
	}
	if (countNonZero(seg) == 0) {
		logger.logTimeElapsedSinceLastLog("20To1000");
//		logger.endSession();
		return 0;
	}
//	imwrite("test/out-seg.ppm", seg);
	logger.logTimeElapsedSinceLastLog("20To1000");
	if (stage == 24) {
		output = seg;
		return 0;
	}


	/*
	 *     %CHANGE
    %[L, num] = bwlabel(seg,8);

    %CHANGE
    [L,num] = bwlabel(imfill(seg, 'holes'),4);
	 *
	 */
	// don't worry about bwlabel.
	output = nscale::imfillHoles<uchar>(seg, true, 8);
	imwrite("test/out-nuclei.ppm", seg);

//	logger.endSession();

	return 0;
}

}
