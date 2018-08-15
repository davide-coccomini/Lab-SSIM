# SSIM
The structural similarity (SSIM) index is a method for predicting the perceived quality of digital television and cinematic pictures, as well as other kinds of digital images and videos.


# HOW TO RUN (Linux)
- Install openCV (and libtiff) following: https://docs.opencv.org/master/d7/d9f/tutorial_linux_install.html
- Create an "images" folder containing the two images to compare (named 0.tiff and 1.tiff) into your project folder
- Compile with the command: g++ ssim.cpp -o ssim -ltiff -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_imgcodecs 
- Launch the command: ./ssim

