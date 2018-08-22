all:
	g++ ssim.cpp -o ssim -ltiff -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_imgcodecs
