# PylonRawFileConverter:
   Converts a Pylon Viewer .raw image file to a different format like .png, .tiff, .jpg, .bmp.
   Run "PylonRawFileConverter.exe --help" to display these instructions.
	 
 Usage Options:
   1. Manual: Simply run program and follow the menus.
   2. Console: Run PylonRawFileConverter with these options:
       --file (name of the raw file to convert)
       --width (the Width of the raw image)
       --height (the Height of the raw image)
       --pixeltype (the Pixel Type of the raw image. See list below...)
       --fileformat (the file format to convert to. See list below...)
       --batch (convert all raw images in current folder. All must have same Width, Height, Pixel Type, and format.)
       --parse (parse a raw image's file name to determine properties. File name must follow the style below...)
       --parseprefix (specify your own filename prefix for parsing. Default: "parseme")
       --silent (suppress all console output except error messages)
   3. Drag-n-Drop: On Windows, simply drag and drop a parseable raw image with default prefix file onto the icon.
	 
 Examples:
   1. Convert a single file:
       PylonRawFileConverter.exe --file myimage.raw --width 640 --height 480 --pixeltype 1 --fileformat 2
   2. Convert a batch of files:
       PylonRawFileConverter.exe --batch --width 640 --height 480 --pixeltype 1 --fileformat 2
   3. Parse and convert a single file: 
       (filename MUST be in this style: <parseprefix>_<width>_<height>_<pixeltype>_<fileformat>_<anything>.raw)
       (Default parseprefix is "parseme")
       Drag-n-Drop the file parseme_640_480_1_2_blahblah.raw onto the .exe icon.
       PylonRawFileConverter.exe --parse --file parseme_640_480_1_2_blahblah.raw
       PylonRawFileConverter.exe --parse --parseprefix myPrefix --file myPrefix_640_480_1_2_blahblah.raw
   4. Parse and convert all raw files in current directory: (filenames MUST be in the above style.)
       PylonRawFileConverter.exe --batch --parse
       PylonRawFileConverter.exe --batch --parse --parseprefix myPrefix
		 
Pixel Type List:  
   1 : PixelType_Mono8
   2 : PixelType_Mono10
   3 : PixelType_Mono12 
   4 : PixelType_BayerBG8
   5 : PixelType_BayerBG12
   6 : PixelType_BayerGB8
   7 : PixelType_BayerGB12
   8 : PixelType_BayerGR8
   9 : PixelType_BayerGR12
   10: PixelType_BayerRG8
   11: PixelType_BayerRG12
   12: PixelType_RGB8packed
   13: PixelType_BGR8packed
   14: PixelType_YUV422_YUYV_Packed
   15: PixelFormat_YCbCr422_8
	 
File Format List: 
   1 : TIFF
   2 : PNG
   3 : BMP
   4 : JPG