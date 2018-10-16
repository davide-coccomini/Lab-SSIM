// SSIM.cpp: definisce il punto di ingresso dell'applicazione console.
//

// ************** SSIM ORIGINALE **************

#include "pch.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream> 
#include <math.h>
#include <string>
#include <stdlib.h> 
#include <math.h>  

using namespace cv;
using namespace std;

// Two variables to stabilize the division with weak denominator
#define C1 (float) (0.01 * 255 * 0.01  * 255)
#define C2 (float) (0.03 * 255 * 0.03 * 255)
#define C3 (float) C2/2

// Variance
double variance(Mat & m, int i, int j, int block_size)
{
	double var = 0;

	Mat m_tmp = m(Range(i, i + block_size), Range(j, j + block_size)); // Create temporary matrix (Range is used to generate the rows)
	Mat m_squared(block_size, block_size, CV_64F); // Create the matrix to scan

	multiply(m_tmp, m_tmp, m_squared);

	double avg = mean(m_tmp)[0]; 	// E(x) (mean calculate medium point)
	double avg_2 = mean(m_squared)[0]; 	// E(x²) 
	var = sqrt(avg_2 - avg * avg);
	return var;
}

// Covariance
double covariance(Mat & m1, Mat & m2, int i, int j, int block_size)
{
	Mat m3 = Mat::zeros(block_size, block_size, m1.depth()); // Create 0 filled matrix 
	Mat m1_tmp = m1(Range(i, i + block_size), Range(j, j + block_size)); // Create temporary matrix (Range is used to generate the rows)
	Mat m2_tmp = m2(Range(i, i + block_size), Range(j, j + block_size));

	multiply(m1_tmp, m2_tmp, m3);

	double avg_co = mean(m3)[0]; // E(XY) medium point
	double avg_c = mean(m1_tmp)[0]; // E(X)
	double avg_o = mean(m2_tmp)[0]; // E(Y)

	double cov = avg_co - avg_o * avg_c; // E(XY) - E(X)E(Y)

	return cov;
}
// Luminance
double luminance(double avg_o, double avg_c) {
	return (2 * avg_o * avg_c + C1) / (pow(avg_o, 2) + pow(avg_c, 2)  + C1);
}

// Contrast
double contrast(double sigma_o, double sigma_c) {
	return ((2 * sigma_o*sigma_c) + C2) / ((pow(sigma_o, 2) + pow(sigma_c, 2) + C2));
}

// Structure
double structure(double sigma_c, double sigma_o, double sigma_co) {
	return ((sigma_co + C3) / (sigma_o*sigma_c + C3));
}

double getSSIM(Mat img_src[3], Mat img_compressed[3], int block_size, bool show_progress = true)
{
	double ssim = 0;

	if (img_src[0].cols != img_compressed[0].cols || img_src[0].rows != img_compressed[0].rows) {
		cout << "The images got different size" << endl;
		return -1;
	}
	int nbBlockPerHeight = img_src[0].rows / block_size;
	int nbBlockPerWidth = img_src[0].cols / block_size;

	// Foreach block in the images
	for (int k = 0; k < nbBlockPerHeight; k++)
	{
		for (int l = 0; l < nbBlockPerWidth; l++)
		{
			int m = k * block_size;
			int n = l * block_size;

			// Avg values for a-channel
			double avg_o = mean(img_src[1](Range(k, k + block_size), Range(l, l + block_size)))[0];
			double avg_c = mean(img_compressed[1](Range(k, k + block_size), Range(l, l + block_size)))[0];
			double luminance_a = luminance(avg_o, avg_c);

			// Avg values for b-channel
			avg_o = mean(img_src[2](Range(k, k + block_size), Range(l, l + block_size)))[0];
			avg_c = mean(img_compressed[2](Range(k, k + block_size), Range(l, l + block_size)))[0];
			double luminance_b = luminance(avg_o, avg_c);

			// Mean of luminance of a and b channels
			double luminance_ab = (luminance_a+luminance_b)/2;

			// Sigma values for L-channel
			double sigma_o = variance(img_src[0], m, n, block_size);
			double sigma_c = variance(img_compressed[0], m, n, block_size);
			double sigma_co = covariance(img_src[0], img_compressed[0], m, n, block_size);

			// Contrast and structure for the L channel
			double contrast_L = contrast(sigma_o, sigma_c);
			double structure_L = structure(sigma_o, sigma_c, sigma_co);

			
			ssim += (luminance_ab * contrast_L * structure_L);
	

		}
		// Progress %
		if (show_progress)
			cout << "\r>>SSIM [" << (int)((((double)k) / nbBlockPerHeight) * 100) << "%]";
	}
	ssim /= nbBlockPerHeight * nbBlockPerWidth;

	if (show_progress)
	{
		cout << "\r>>SSIM [100%]" << endl;
		cout << "SSIM : " << ssim << endl;
	}
	return ssim;
}

Mat* normalizeLabValues(Mat image[]) {
	int rows = image[0].rows;
	int cols = image[0].cols;

	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			// L
			Scalar intensity = image[0].at<uchar>(i, j);
			Scalar normalized_intensity = intensity.val[0] * 100 / 256;
			image[0].at<uchar>(i,j) = normalized_intensity.val[0];
			
			// a
			intensity = image[1].at<uchar>(i, j);
			normalized_intensity = intensity.val[0] - 126;
			image[1].at<uchar>(i, j) = normalized_intensity.val[0];

			// b
			intensity = image[2].at<uchar>(i, j);
			normalized_intensity = intensity.val[0] - 126;
			image[2].at<uchar>(i, j) = normalized_intensity.val[0];
		}
	}


	image[0].convertTo(image[0], CV_64FC3);
	image[1].convertTo(image[1], CV_64FC3);
	image[2].convertTo(image[2], CV_64FC3);

	return image;
}

int main() {
	Mat originalImage, compressedImage;
	string defaultSettings;
	string originalImagePath("images/10.tif");
	string compressedImagePath("images/12.tif");

	cout << "+++ Welcome to SSIM calculator +++" << endl << "You can use this program to calculate how much similar are two TIFF images with the same subject" << endl;
	cout << "Do you want to use the default settings? Y/N" << endl;
	cin >> defaultSettings;
	if (defaultSettings.compare("N") == 0) {
		string folderName, originalImageName, compressedImageName;
		cout << "Insert the folder name of the images" << endl;
		cin >> folderName;
		cout << "Insert the name of the original image (without format)" << endl;
		cin >> originalImageName;
		cout << "Insert the name of the compressed image (without format)" << endl;
		cin >> compressedImageName;
		originalImagePath = folderName + "/" + originalImageName + ".tif";
		compressedImagePath = folderName + "/" + compressedImageName + ".tif";
		cout << "Settings updated successfully" << endl;
	}
	cout << "Starting computation ..." << endl;


	originalImage = imread(originalImagePath, CV_LOAD_IMAGE_UNCHANGED);
	compressedImage = imread(compressedImagePath, CV_LOAD_IMAGE_UNCHANGED);
	Mat originalImageLab, compressedImageLab;
	cvtColor(originalImage, originalImageLab, COLOR_RGB2Lab);
	cvtColor(compressedImage, compressedImageLab, COLOR_RGB2Lab);

	Mat originalImageSplitted[3], compressedImageSplitted[3]; 
	split(originalImageLab, originalImageSplitted);
	split(compressedImageLab, compressedImageSplitted);
	

	originalImage.convertTo(originalImage, CV_64FC3);
	compressedImage.convertTo(compressedImage, CV_64FC3);

	Mat* originalImageNormalized = normalizeLabValues(originalImageSplitted);
	Mat* compressedImageNormalized = normalizeLabValues(compressedImageSplitted);

	double SSIM = getSSIM(originalImageNormalized, compressedImageNormalized, 20);
	system("pause");
	return 0;
}
