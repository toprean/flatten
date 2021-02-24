# What is "flatten"?
It is a C++ program written by Teodor Oprean in May/June 2020.

It can be used to flatten geometric distortion in image files and video files given the right coefficients for a specific lens.

You can find me on Linkedin at this address: http://www.linkedin.com/in/teodoroprean

# License
You are free to use, copy and modify the code under the MIT License, which is the least restrictive open source licence I am aware of.

I ask that you mention my name and this repository if you update/modify this code for your own projects. Also, please let me know through Linkedin if you use this code for anything. I am genuinely interested to learn if this code has helped you solve a real world problem.

# What does this program do?
It flattens complex geometric distortion in a list of still images or in a video file.

It does not alter the input files.

It creates new files with similar naming, so you can compare "before" against "after".

It uses OpenCV and an XML file containing lens-specific flattening coefficients to achieve this objective.

The first lens for which I derived the flattening coefficients is the 90 degree rectilinear wide angle lens of the Firefly 8SE action camera for consumer 4K resolution (3840x2160). I have included the XML file for that specific camera and resolution in this repository.

The second lens for which I derived the flattening coefficients is the 170 degree fisheye lens of the Firefly 8SE action camera for consumer 4K resolution (3840x2160). I have included the XML file for that specific camera and resolution in this repository.

You can see a demo of the results on my youtube channel:
https://www.youtube.com/channel/UCLrEgKOm4E9Sdd2X2Gno2-Q

- video 01: https://youtu.be/v6kNbqU19aI "Flattening the geometric distortion of the Firefly 8SE (90 degree lens)"
- video 02: https://youtu.be/Gx71k3igUtg "Still photographs flattened from the Firefly 8SE action camera with 90 degree lens"
- video 03: https://youtu.be/4lJwlOWkviM "Time lapse made with the Firefly 8SE"
- video 04: https://youtu.be/Oj5HPg8uYI4 "Time lapse made with the Firefly 8SE (deflickered)"

Note: Make sure to expand the "Description" section in YouTube below video 01 to see additional details.

# Motivation behind this project
The Firefly 8SE action camera is available with one of two possible lenses:
- a fisheye lens
- a 90 degree lens

The reason for choosing the 90 degree lens is that it is advertised as a rectilinear wide angle lens, but after using it for a while, I began to notice that it is not fully rectilinear. There is a small amount of mustache distortion in the footage, which can be distracting for certain images.

Reference: https://en.wikipedia.org/wiki/Distortion_(optics)

It is trivial to fix simple barrel distortion or simple pincushion distortion in post processing with most video editing tools, but complex distortion (which combines barrel distortion with pincushion distortion at the same time) takes more effort.

Some video editing tools offer a filter in which you can fool around with sliders until the footage looks reasonably corrected, but instead of guessing the values and eyballing the results with an increasing sense of irritation, I wanted to obtain the best possible flattening coefficients in a truly scientific manner. In the end, OpenCV provided the best solution for my goal.

# Doesn't the camera correct for lens distortion already?
If your camera does that, count yourself lucky. Most cameras on the market offer spotty or inconsistent support for flattening geometric distortion -- if they have that functionality at all. Even expensive, high end cameras that cost thousands of dollars will frustrate you with their selective ability to flatten geometric distortion for some lenses, but not all lenses you might want to use, and only in some modes but not necessarily in the mode you want to use (stills mode versus video mode).

In the Firefly 8SE specifically, the firmware shows a "distortion correction" menu item, but that feature does not actually work. It merely replaces the extreme barrel distortion of the fisheye lens with extreme pincushion distortion.

The Firefly 8SE firmware is completely unaware which lens is actually mounted to the camera. It always assumes that a fisheye lens is mounted to it even when in fact a 90 degree lens is mounted to it. The firmware, being lens-agnostic, does not write accurate information about the lens in the EXIF data.

# Can't you use "Gimp lens fun"?
Reference: https://seebk.github.io/GIMP-Lensfun/

- "Gimp lens fun" works for image files but not for video files. OpenCV can process both. I wanted to come up with a tool or process for flattening distortion in video files primarily.
- "Gimp lens fun" depends completely on the EXIF data of the image file being correct and accurate, so the Firefly 8SE EXIF data throws a monkey wrench into the works.
- "Gimp lens fun" was not aware of the Firefly 8SE action cameras at the time I checked (May 2020). Maybe that has changed in the meantime. If so, please let me know.

# Weren't the coefficients public domain already?
Nobody had published the coefficients for the Firefly 8SE anywhere on the internet when I looked into this topic in May 2020, so I decided to derive those coefficients myself from scratch.

I used OpenCV for deriving the coefficients because OpenCV was specifically designed for this purpose.

OpenCV meets my needs exactly, so I have settled on that tool for flattening distortion in post processing.

Note: Your sample of the same camera model might differ slightly due to lens alignment differences, so calibrating each camera individually might be necessary for optimal results. You can use the coefficients I derived as a starting point.

# Prerequisite: C++ compiler
I used GCC on Linux Mint to compile everything. (64-bit Linux Mint 19.3, with CPU being Intel Core i5 x86-64)

The options I defined in `CMakeLists.txt` are specific to GCC, with an x86-64 CPU in mind.

If you are using a different C++ compiler, you will need to update the options in that file to work for your preferred C++ compiler.

I used the version of GCC that is distributed by the ubuntu package repository at the time I worked on this (May/June 2020). The exact version of the C++ compiler should not matter too much for this code.

# Prerequisite: OpenCV
Following the official tutorial for installing OpenCV.
Source: https://docs.opencv.org/master/d7/d9f/tutorial_linux_install.html

For my machine specifically (64-bit Linux Mint 19.3), I needed to follow these steps:
```
sudo apt-get install build-essential
sudo apt-get install cmake git libgtk2.0-dev pkg-config libavcodec-dev libavformat-dev libswscale-dev
sudo apt-get install python-dev python-numpy libtbb2 libtbb-dev libjpeg-dev libpng-dev libtiff-dev libdc1394-22-dev
```
For the `libjasper-dev` package I needed to follow the work-around published at https://stackoverflow.com/questions/43484357/opencv-in-ubuntu-17-04/48326450#48326450
```
sudo add-apt-repository "deb http://security.ubuntu.com/ubuntu xenial-security main"
sudo apt update
sudo apt install libjasper1 libjasper-dev
```
I downloaded
https://github.com/opencv/opencv/archive/4.3.0.zip
to a local folder.

I extracted the zip file to:
`~/opencv-4.3.0`

I created this directory:
`~/opencv-4.3.0/build`

Then I opened a terminal window at that location:
`~/opencv-4.3.0/build`

I executed these commands in the terminal:
```
sudo apt install cmake-qt-gui
cmake-gui
```
In the Cmake GUI window:

I clicked "Configure".
Result: A list of red options.

- `CMAKE_BUILD_TYPE=Release` <--- I defined this value manually.
- `CMAKE_INSTALL_PREFIX=/usr/local` <--- This was already defined.
- `BUILD_DOCS=yes` <--- I defined this value manually.
- `BUILD_EXAMPLES=yes` <--- I defined this value manually.

I clicked "Generate".

After it completed the work, I closed the Cmake GUI window.

In the terminal window, I ran the "make" utility:
```
~/opencv-4.3.0/build$ make -j7
```
Next I ran the "make install" command:
```
~/opencv-4.3.0/build$ sudo make install
```

These are some of the resulting files. You can confirm this as a sanity check on your machine.

- `/usr/local/lib/libopencv_calib3d.so`
- `/usr/local/include/opencv4/opencv2/calib3d.hpp`

I improved the camera calibration tutorial code to save the intermediate images to the file system for later inspection.

You can obtain these two files from this repository to view the differences in a file diff tool. Place them in your local file system at these locations:
- `~/opencv-4.3.0/samples/cpp/tutorial_code/calib3d/camera_calibration/camera_calibration.cpp` <--- This is my modified file.
- `~/opencv-4.3.0/samples/cpp/tutorial_code/calib3d/camera_calibration/camera_calibration_original.cpp.original` <--- This is the original.

After replacing the calibration tutorial code with my updated version, just run the make command again to recompile the camera calibration executable.

On my machine, the resulting executable is located at `~/opencv-4.3.0/build/bin/example_tutorial_camera_calibration`

# To compile and run flatten:

I created this directory: `~/flatten-prog`

In that directory I placed these files:
- `~/flatten-prog/CMakeLists.txt`
- `~/flatten-prog/flatten.cpp`

In a terminal window, run these two commands in sequence:
```
~/flatten-prog$ cmake .
~/flatten-prog$ make
```

The resulting binary executable should be: `~/flatten-prog/flatten`

# Sample usage: list of frames

Let's say that you have a list of frames. They can be any image file type supported by OpenCV, for example: PNG, TIFF, JPG.

- `~/mydir1/frame-000001.JPG`
- `~/mydir1/frame-000002.JPG`
- `~/mydir1/frame-000003.JPG`
- `~/mydir1/frame-000004.JPG`

Make a copy of `flatten_image_list.xml` from this repository to `~/mydir1`. Modify the content of that XML file to enumerate the frames you want to flatten. I recommend using the absolute path to each file. For this purpose I use the `ls` command in a terminal and redirect the output to a temporary text file, then copy the content into the XML file.

Make a copy of `flatten-settings.xml` from this repository to `~/mydir1`. Modify the content of that XML file to reference the image list XML. Modify the desired dimensions as needed.

With the Firefly 8SE, I have so far tried out consumer 4K resolution (3840x2160).

I found by trial and error how to define appropriately sized intermediate and final resolutions for two aspect ratios: 1.85 and 16:9.

In a terminal window, run this command:
```
~/flatten-prog/flatten ~/mydir1/flatten-settings.xml
```

The resulting output will be new files in the same directory:
- `~/mydir1/frame-000001-b.JPG`
- `~/mydir1/frame-000002-b.JPG`
- `~/mydir1/frame-000003-b.JPG`
- `~/mydir1/frame-000004-b.JPG`

You can migrate these files to a separate directory for later processing if you want. Example:

```
~/mydir1$ mkdir mytemp
~/mydir1$ mv *-b.JPG mytemp
```

Afterwards you can use ffmpeg to join the frames into a video file. Example:
```
ffmpeg -hide_banner -framerate 30000/1001 -i ~/mydir1/frame-%06d-b.JPG -pix_fmt yuv420p -c:v libx264 -preset fast -crf 21 -g 15 -bf 2 -movflags +faststart ~/mydir1/output.MP4
```

# Sample usage: single video file

Let's say that you have an MP4 file: `~/mydir2/test.MP4`

Make a copy of `flatten-settings.xml` from this repository to `~/mydir2`. Modify the content of that XML file to reference the MP4 file directly in the `<input>` tag. Update the dimensions to match the MP4 file. The same principles apply as before when it comes to the intermediate and final dimensions. This is the only XML file neded in this scenario.

In a terminal window, run this command:
```
~/flatten-prog/flatten ~/mydir2/flatten-settings.xml
```

The resulting output will be one new file in the same directory: `~/mydir2/test-b.avi`

Afterwards you can use any video editing tool you want to work on the video file further.

For video, OpenCV intentionally uses AVI for the output and no other file types in order to keep things as simple as possible.
