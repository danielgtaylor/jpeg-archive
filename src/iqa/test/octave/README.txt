The correct unit test values are derived by recreating the image quality
algorithms in Octave (the open-source equivalent of Matlab). To run the scripts
yourself, make sure (1) to set the current directory in Octave to where the
scripts are located, and (2) make sure the path to the test bitmaps in the 
'resources' directory is in Octave's IMAGE_PATH (I do this by running Octave
with the --image-path=<path> argument).
.
