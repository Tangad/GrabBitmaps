# GrabBitmaps

The main purpose of this project is to grab bitmap images from .AVI file, actually converting video frames into bitmap images. This was part of a larger project where ultrasound video files are converted into .AVI files, bitmap images are extracted from the AVI files, and finally the bitmap images could be input into 3D images. You can play the file, replay, pause, stop and reverse play, set the starting stream and ending stream to grab the images in the video.

Requirements
- Visual Studio 2008 preferred, Visual studio 2010 should work. Other versions untested.
- Windows XP or newer.
- Video file must be .AVI (example file ruby.avi included)
- .AVI must be in the same directory of the project

The project consists of
- Source Files
- Header Files
- Resource Files

Header File includes
- header files of the libraries

Source Files includes
- main file and classes (class constructor and destructor)

Resource Files includes
-images
-Icons
-buttons

How to Run
- Compile project
- Run .EXE file
- Open file (Ruby.avi)
- Set the start and ending of the streams you want to grab bitmaps
- Click Grab Image Buttons
- Images will be in the Folder is named "ruby" in the same directory of the project.
