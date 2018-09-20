// SSIM.cpp: definisce il punto di ingresso dell'applicazione console.
//



#include "pch.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream> 
#include <math.h>
#include <string>
#include <stdlib.h>  

using namespace cv;
using namespace std;

// Two variables to stabilize the division with weak denominator
#define C1 (float) (0.01 * 255 * 0.01  * 255)
#define C2 (float) (0.03 * 255 * 0.03 * 255)

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


double getSSIM(Mat & img_src, Mat & img_compressed, int block_size, bool show_progress = true)
{
	double ssim = 0;

	if (img_src.cols != img_compressed.cols || img_src.rows != img_compressed.rows) {
		cout << "The images got different size" << endl;
		return -1;
	}
	int nbBlockPerHeight = img_src.rows / block_size;
	int nbBlockPerWidth = img_src.cols / block_size;

	// Foreach block in the images
	for (int k = 0; k < nbBlockPerHeight; k++)
	{
		for (int l = 0; l < nbBlockPerWidth; l++)
		{
			int m = k * block_size;
			int n = l * block_size;

			double avg_o = mean(img_src(Range(k, k + block_size), Range(l, l + block_size)))[0];
			double avg_c = mean(img_compressed(Range(k, k + block_size), Range(l, l + block_size)))[0];
			double sigma_o = variance(img_src, m, n, block_size);
			double sigma_c = variance(img_compressed, m, n, block_size);
			double sigma_co = covariance(img_src, img_compressed, m, n, block_size);

			// SSIM: [(2*  ?x   *  ?y   + C1) * (2 * ?xy      + C2)]/[(((?x)^2)        + ((?y)^2)      + C1) * (     ((?x)^2)     +     ((?y)^2)      + C2)]
			ssim += ((2 * avg_o * avg_c + C1) * (2 * sigma_co + C2)) / ((avg_o * avg_o + avg_c * avg_c + C1) * (sigma_o * sigma_o + sigma_c * sigma_c + C2));

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

int main() {
	Mat originalImage, compressedImage;
	string defaultSettings;
	string originalImagePath("images/0.tif");
	string compressedImagePath("images/1.tif");

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

/*	imshow("Original image", originalImage); // Show the original image
	imshow("Compressed image", compressedImage); // Show the compressed image
	waitKey(0); // Wait for key before displaying next
	*/
	originalImage.convertTo(originalImage, CV_64FC3);
	compressedImage.convertTo(compressedImage, CV_64FC3);


	double SSIM = getSSIM(originalImage, compressedImage, 1);
	system("pause");
	return 0;
}
