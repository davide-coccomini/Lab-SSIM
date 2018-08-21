#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream> 
#include <math.h>
#include <string>
#include <tiffio.h>

using namespace cv;
using namespace std;

// Two variables to stabilize the division with weak denominator
#define C1 (float) (0.01 * 255 * 0.01  * 255)
#define C2 (float) (0.03 * 255 * 0.03 * 255)



bool tifToMat(Mat& image,string imageName){
		  // Read images and create the Mat
		  TIFF* tif = TIFFOpen(imageName.c_str(), "r");
		  
		  if (tif) {
		     do {
				unsigned int width, height;
				uint32* raster;

				// Get the size of the tiff
				TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
				TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);

				uint npixels = width*height; // Get the total number of pixels

				raster = (uint32*)_TIFFmalloc(npixels * sizeof(uint32)); // Allocate temp memory (bitmap)
				if (raster == NULL) // Check the raster's memory was allocated
				{
					TIFFClose(tif);
					cerr << "Raster allocate error" << endl;
					return false;
				}
						
				// Check the tif read to the raster correctly
				if (!TIFFReadRGBAImage(tif, width, height, raster, 0))
				{
					TIFFClose(tif);
					cerr << "Raster read error" << endl;
					return false;
				}

				// Create a new matrix of width x height with 8 bits per channel and 4 channels (RGBA)
				image = Mat(width, height, CV_8UC4); 
						
				// Itterate through all the pixels of the tif
				for (uint x = 0; x < width; x++)
					for (uint y = 0; y < height; y++)
					{
						uint32& TiffPixel = raster[y*width+x]; // Read the current pixel of the TIF
						Vec4b& pixel = image.at<Vec4b>(Point(y, x)); // Read the current pixel of the matrix
						pixel[0] = TIFFGetB(TiffPixel); // Set the pixel values as BGRA
						pixel[1] = TIFFGetG(TiffPixel);
						pixel[2] = TIFFGetR(TiffPixel);
						pixel[3] = TIFFGetA(TiffPixel);
					}
				
				// Free temp memory
				_TIFFfree(raster); 

			
		    } while (TIFFReadDirectory(tif)); // Get the next tif to go into the channels

		   TIFFClose(tif); // Close the tif file
		  }else return false;
  return true;
}

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

 if(img_src.cols != img_compressed.cols || img_src.rows != img_compressed.rows){
   cout<<"The images got different size"<<endl;
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

			double avg_o 	= mean(img_src(Range(k, k + block_size), Range(l, l + block_size)))[0];
			double avg_c 	= mean(img_compressed(Range(k, k + block_size), Range(l, l + block_size)))[0];
			double sigma_o 	= variance(img_src, m, n, block_size);
			double sigma_c 	= variance(img_compressed, m, n, block_size);
			double sigma_co	= covariance(img_src, img_compressed, m, n, block_size);

			// SSIM: [(2*  μx   *  μy   + C1) * (2 * σxy      + C2)]/[(((μx)^2)        + ((μy)^2)      + C1) * (     ((σx)^2)     +     ((σy)^2)      + C2)]
			ssim += ((2 * avg_o * avg_c + C1) * (2 * sigma_co + C2)) / ((avg_o * avg_o + avg_c * avg_c + C1) * (sigma_o * sigma_o + sigma_c * sigma_c + C2));
			
		}
		// Progress %
		if (show_progress)
			cout << "\r>>SSIM [" << (int) ((( (double)k) / nbBlockPerHeight) * 100) << "%]";
	}
        ssim /= nbBlockPerHeight * nbBlockPerWidth;

	if (show_progress)
	{
		cout << "\r>>SSIM [100%]" << endl;
		cout << "SSIM : " << ssim << endl;
	}
 return ssim;
}

int main(){
  Mat originalImage;
  Mat compressedImage;
  string originalImagePath("images/0.tif");
  string compressedImagePath("images/1.tif");
  bool done = tifToMat(originalImage,originalImagePath);
  if(!done){
	cout << "Error converting original image tiff to matrix"<<endl;
	return -1;
  }

  done = tifToMat(compressedImage,compressedImagePath);
  if(!done){
	cout << "Error converting compressed image tiff to matrix"<<endl;
	return -1;
  }

  originalImage.convertTo(originalImage, CV_64F);
  compressedImage.convertTo(compressedImage, CV_64F);

  //imshow("Original image", originalImage); // Show the original image
  //imshow("Compressed image", compresedImage); // Show the compressed image
  //waitKey(0); // Wait for key before displaying next

  double SSIM = getSSIM(originalImage, compressedImage,1);
  return 0;
}
