# SSIM
The structural similarity (SSIM) index is a method for predicting the perceived quality of digital television and cinematic pictures, as well as other kinds of digital images and videos.


# Lab Based SSIM Value
The normal SSIM is good to compare gray-scale images but fail when you try to compare colored images. This is a specific implementation to compare CIELAB 8bit images considering the color too.

In this sperimental Lab Based SSIM, the luminance is obtained by calculating the mean of luminance measure of a-channel and b-channel, the contrast and structural measure is calculated considering only the L-channel. 

# HOW TO RUN (Windows)
- Install Visual Studio 2017 (https://visualstudio.microsoft.com/it/downloads/)
- Download Opencv Win Pack (https://opencv.org/releases.html) and extract it into the C folder 
- Create a Visual Studio project and edit the project properties (all configurations x64) as follow:
    -) C/C++ -> General -> Include Directories = C:\opencv\build\include
    -) Linker -> General -> Library Directories = C:\opencv\build\x64\vc14\lib
    -) Linker -> Input -> Edit -> New -> opencv_world343.lib
- Create an "images" folder, into your project directory, containing the two images you want to compare
- Add the C++ code into the project and run it in Release x64 mode.


# HOW TO RUN (Linux)
- Install openCV (and libtiff) following: https://docs.opencv.org/master/d7/d9f/tutorial_linux_install.html
- Create an "images" folder containing the two images you want to compare into your project folder
- Compile with the command: make 
- Launch the command ./ssim to run the application


