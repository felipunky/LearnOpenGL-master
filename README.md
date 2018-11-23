# Navier-Stokes
This is the Windows application's repository, if you want to go to the Mac version go to:
https://github.com/felipunky/Navier-Stokeish-Mac

![navier-stokeishone](https://user-images.githubusercontent.com/21000020/48667011-bbb71080-ea9a-11e8-975a-302d2d594885.gif)
# To run
Download the .zip file, extract. In the debug folder there is an executable file: LearnOpenGL.exe, double click and you are in.
# Description
The algorithm is mostly done through a ping-pong between shaders using C++ for the CPU and OpenGL's GLSL for the GPU. It is loosely based on the GPUGems article by NVidia at:

http://developer.download.nvidia.com/books/HTML/gpugems/gpugems_ch38.html

I added a python script to initialize the simulation through a .png, it relies heavily on gdal and matplot lib, to install the dependencies to be able to run:

https://github.com/felipunky/Navier-Stokeish-Mac/blob/master/ImageFromGrib.py

Go to:

https://github.com/felipunky/GISPython

# Edit
I implemented a GUI to modify the simulation and the possibility to specify a file path to access different images, in the repository you can find some of the images that I extracted through the python script, they are mostly from the GFS Forecasts dataset that you can get at:

http://nomads.ncep.noaa.gov

But there are some from the GFS Ensemble Forecasts and the CMC Ensemble Forecasts
.
![simulation](https://user-images.githubusercontent.com/21000020/48935165-abf15f00-eed4-11e8-9718-6bf1e08e0929.JPG)
