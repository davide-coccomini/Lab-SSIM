
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

#define C1 (float) (0.01 * 255 * 0.01  * 255)
#define C2 (float) (0.03 * 255 * 0.03 * 255)



bool tifToMat(Mat& image,string imageName){

		  // Read images and create the Mat
		  TIFF* tif = TIFFOpen(imageName.c_str(), "r");
		  
		  if (tif) {
		     do {
			unsigned int width, height;
			uint32* raster;

			// get the size of the tiff
			TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
			TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);

			uint npixels = width*height; // get the total number of pixels

			raster = (uint32*)_TIFFmalloc(npixels * sizeof(uint32)); // allocate temp memory (must use the tiff library malloc)
			if (raster == NULL) // check the raster's memory was allocaed
			{
			   TIFFClose(tif);
			   cerr << "Could not allocate memory for raster of TIFF image" << endl;
			   return false;
			}
					
			// Check the tif read to the raster correctly
			if (!TIFFReadRGBAImage(tif, width, height, raster, 0))
			{
			   TIFFClose(tif);
			   cerr << "Could not read raster of TIFF image" << endl;
			   return false;
			}

			image = Mat(width, height, CV_8UC4); // create a new matrix of w x h with 8 bits per channel and 4 channels (RGBA)
					
			// itterate through all the pixels of the tif
			for (uint x = 0; x < width; x++)
			  for (uint y = 0; y < height; y++)
			  {
			    uint32& TiffPixel = raster[y*width+x]; // read the current pixel of the TIF
			    Vec4b& pixel = image.at<Vec4b>(Point(y, x)); // read the current pixel of the matrix
			    pixel[0] = TIFFGetB(TiffPixel); // Set the pixel values as BGRA
			    pixel[1] = TIFFGetG(TiffPixel);
			    pixel[2] = TIFFGetR(TiffPixel);
			    pixel[3] = TIFFGetA(TiffPixel);
			  }

			 _TIFFfree(raster); // release temp memory
			// Rotate the image 90 degrees couter clockwise
			 image = image.t();
			 flip(image, image, 0);
			
		    } while (TIFFReadDirectory(tif)); // get the next tif

		   TIFFClose(tif); // close the tif file
		  }
  return true;
}
double sigma(Mat & m, int i, int j, int block_size)
	{
		double sd = 0;
		
		Mat m_tmp = m(Range(i, i + block_size), Range(j, j + block_size));
		Mat m_squared(block_size, block_size, CV_64F);

		multiply(m_tmp, m_tmp, m_squared);

		// E(x)
		double avg = mean(m_tmp)[0];
		// E(x²)
		double avg_2 = mean(m_squared)[0];

	
		sd = sqrt(avg_2 - avg * avg);
		return sd;
}
// Covariance
	double cov(Mat & m1, Mat & m2, int i, int j, int block_size)
	{
		Mat m3 = Mat::zeros(block_size, block_size, m1.depth());
		Mat m1_tmp = m1(Range(i, i + block_size), Range(j, j + block_size));
		Mat m2_tmp = m2(Range(i, i + block_size), Range(j, j + block_size));


		multiply(m1_tmp, m2_tmp, m3);

		double avg_ro 	= mean(m3)[0]; // E(XY)
		double avg_r 	= mean(m1_tmp)[0]; // E(X)
		double avg_o 	= mean(m2_tmp)[0]; // E(Y)


		double sd_ro = avg_ro - avg_o * avg_r; // E(XY) - E(X)E(Y)

		return sd_ro;
	}


double getSSIM(Mat & img_src, Mat & img_compressed, int block_size, bool show_progress = true)
{
 double ssim = 0;
 cout<<"SSIM: "<<ssim<<endl;

 int nbBlockPerHeight = img_src.rows / block_size;
  cout<<"blockperheight: "<<nbBlockPerHeight<<endl;

 int nbBlockPerWidth = img_src.cols / block_size;
 cout<<"blockperwidth: "<<nbBlockPerWidth<<endl;

	for (int k = 0; k < nbBlockPerHeight; k++)
	{
		for (int l = 0; l < nbBlockPerWidth; l++)
		{
			int m = k * block_size;
			int n = l * block_size;

			double avg_o 	= mean(img_src(Range(k, k + block_size), Range(l, l + block_size)))[0];
			double avg_r 	= mean(img_compressed(Range(k, k + block_size), Range(l, l + block_size)))[0];
			double sigma_o 	= sigma(img_src, m, n, block_size);
			double sigma_r 	= sigma(img_compressed, m, n, block_size);
			double sigma_ro	= cov(img_src, img_compressed, m, n, block_size);
			/*cout<<"avg_o "<<avg_o<<endl;
			cout<<"avg_r "<<avg_r<<endl;
			cout<<"sigma_o "<<sigma_o<<endl;
 			cout<<"sigma_r "<<sigma_r<<endl;
			cout<<"sigma_ro "<<sigma_ro<<endl;*/
			
			ssim += ((2 * avg_o * avg_r + C1) * (2 * sigma_ro + C2)) / ((avg_o * avg_o + avg_r * avg_r + C1) * (sigma_o * sigma_o + sigma_r * sigma_r + C2));
			
		}
		// Progress
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
  if(!done) return -1;
  done = tifToMat(compressedImage,compressedImagePath);
  if(!done) return -1;

  originalImage.convertTo(originalImage, CV_64F);
  compressedImage.convertTo(compressedImage, CV_64F);

  //imshow("Original image", originalImage); // show the image
  //imshow("Compressed image", compresedImage); // show the image
  //waitKey(0); // wait for anykey before displaying next

  double SSIM = getSSIM(originalImage, compressedImage,1);
  return 0;

}
