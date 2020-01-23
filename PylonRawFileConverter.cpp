// PylonRawFileConverter.cpp
// Loads a Pylon-Saved .raw file from disk and converts it to another file format.
//
// Copyright (c) 2019 Matthew Breit - matt.breit@baslerweb.com or matt.breit@gmail.com
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http ://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

// Include files to use the PYLON API.
#include <pylon/PylonIncludes.h>
#include <iostream>
#include <fstream>
#include <stdexcept>
#ifndef PYLON_WIN_BUILD
#include <dirent.h> // For Listing all files in a directory in Utility_ZipAllFiles().
#endif

using namespace Pylon;
using namespace std;

#define PARSE_PREFIX_DEFAULT "parseme"
#define PARSE_NUM_FIELDS 6

bool pauseBeforeExit = true;
bool silent = false;

Pylon::PixelType PixelTypeFromInt(int pixelTypeID)
{
	switch (pixelTypeID)
	{
		case 1:
			return Pylon::EPixelType::PixelType_Mono8;
		case 2:
			return Pylon::EPixelType::PixelType_Mono10;
		case 3:
			return Pylon::EPixelType::PixelType_Mono12;
		case 4:
			return Pylon::EPixelType::PixelType_BayerBG8;
		case 5:
			return Pylon::EPixelType::PixelType_BayerBG12;
		case 6:
			return Pylon::EPixelType::PixelType_BayerGB8;
		case 7:
			return Pylon::EPixelType::PixelType_BayerGB12;
		case 8:
			return Pylon::EPixelType::PixelType_BayerGR8;
		case 9:
			return Pylon::EPixelType::PixelType_BayerGR12;
		case 10:
			return Pylon::EPixelType::PixelType_BayerRG8;
		case 11:
			return Pylon::EPixelType::PixelType_BayerRG12;
		case 12:
			return Pylon::EPixelType::PixelType_RGB8packed;
		case 13:
			return Pylon::EPixelType::PixelType_BGR8packed;
		case 14:
			return Pylon::EPixelType::PixelType_YUV422_YUYV_Packed;
		case 15:
			return Pylon::EPixelType::PixelType_YUV422_YUYV_Packed;
		default:
			throw std::runtime_error("Invalid Pixel Type Selection");
	}
}

Pylon::EImageFileFormat FileFormatFromInt(int fileFormatID)
{
	switch (fileFormatID)
	{
		case 1:
			return Pylon::EImageFileFormat::ImageFileFormat_Tiff;
		case 2:
			return Pylon::EImageFileFormat::ImageFileFormat_Png;
#ifdef PYLON_WIN_BUILD
		case 3:
			return Pylon::EImageFileFormat::ImageFileFormat_Bmp;
		case 4:
			return Pylon::EImageFileFormat::ImageFileFormat_Jpeg;
#endif
		default:
			throw std::runtime_error("Invalid File Format Selection");
	}
}

void LoadRaw(const Pylon::String_t& fileName, Pylon::CPylonImage& image, uint32_t width, uint32_t height, Pylon::EPixelType pixelType)
{
	std::string errorMessage = "ERROR: ";
	errorMessage.append(__FUNCTION__);
	errorMessage.append("(): ");

	std::ifstream myFile;
	char *pBuffer = NULL;

	Pylon::PylonAutoInitTerm autoInitTerm;

	try
	{
		myFile.open(fileName.c_str(), std::ifstream::binary);

		if (myFile)
		{
			uint32_t fileSize = 0;
			uint32_t imageSize = 0;

			myFile.seekg(0, myFile.end);
			fileSize = myFile.tellg();
			myFile.seekg(0, myFile.beg);

			if (Pylon::BitPerPixel(pixelType) == 8)
				imageSize = (width * height);
			if (Pylon::BitPerPixel(pixelType) == 10)
				imageSize = (width * height) * 1.25;
			if (Pylon::BitPerPixel(pixelType) == 12)
				imageSize = (width * height) * 1.5;

			if ((double)fileSize != imageSize)
			{
				errorMessage.append("File size does not match image size!");
				errorMessage.append(" File: ");
				errorMessage.append(std::to_string(fileSize));
				errorMessage.append(" Image: ");
				errorMessage.append(std::to_string(imageSize));
				myFile.close();
				throw std::runtime_error(errorMessage.c_str());
			}

			pBuffer = new char[fileSize];
			myFile.read(pBuffer, fileSize);

			if (!myFile)
			{
				errorMessage.append("File could not be read entirely!");
				errorMessage.append(" Read: ");
				errorMessage.append(std::to_string(myFile.gcount()));
				errorMessage.append(" Size: ");
				errorMessage.append(std::to_string(fileSize));
				myFile.close();
				delete[] pBuffer;
				throw std::runtime_error(errorMessage.c_str());
			}

			Pylon::CPylonImage temp;
			temp.AttachUserBuffer(pBuffer, imageSize, pixelType, width, height, 0);
			image.CopyImage(temp);

			delete[] pBuffer;
			myFile.close();
		}
		else
		{
			errorMessage.append("File could not be opened!");
			errorMessage.append(" File Name: ");
			errorMessage.append(fileName);
			myFile.close();
			throw std::runtime_error(errorMessage.c_str());
		}
	}
	catch (GenICam::GenericException &e)
	{
		// Error handling.
		std::cerr << "An exception occurred: " << e.GetDescription() << std::endl;
		delete[] pBuffer;
		myFile.close();
	}
	catch (std::runtime_error &e)
	{
		// Error handling.
		std::cerr << "An exception occurred: " << e.what() << std::endl;
		delete[] pBuffer;
		myFile.close();
	}
	catch (...)
	{
		// Error handling.
		std::cerr << "An unknown exception occurred: " << std::endl;
		delete[] pBuffer;
		myFile.close();
	}
}

bool RawFileConverter(std::string fileName, uint32_t imageWidth, uint32_t imageHeight, Pylon::EPixelType imagePixelFormat, Pylon::EImageFileFormat destinationFileFormat)
{
	try
	{
		std::string extension = "";
		switch (destinationFileFormat)
		{
			case ImageFileFormat_Tiff:
				extension = ".tiff";
				break;
			case ImageFileFormat_Png:
				extension = ".png";
				break;
			case ImageFileFormat_Raw:
				extension = ".raw";
				break;
#ifdef PYLON_WIN_BUILD
			case ImageFileFormat_Bmp:
				extension = ".bmp";
				break;
			case ImageFileFormat_Jpeg:
				extension = ".jpg";
				break;
#endif
			default:
				extension = ".undefined";
		}

		if (silent == false)
		{
			std::cout << "File Name  : " << fileName << std::endl;
			std::cout << "Width      : " << imageWidth << std::endl;
			std::cout << "Height     : " << imageHeight << std::endl;
			std::cout << "PixelType  : " << Pylon::CPixelTypeMapper::GetNameByPixelType(imagePixelFormat) << std::endl;
			std::cout << "FileFormat : " << extension << std::endl;
		}

		if (fileName == "")
			throw std::runtime_error("No Filename Given");
		if (imageWidth == 0)
			throw std::runtime_error("Width must be greater than 0.");
		if (imageHeight == 0)
			throw std::runtime_error("Height must be greater than 0.");

		Pylon::CPylonImage tempImage;

		LoadRaw(fileName.c_str(), tempImage, imageWidth, imageHeight, imagePixelFormat);

		std::string newFileName = "";
		size_t lastdot = fileName.find_last_of(".");

		if (lastdot == std::string::npos)
			newFileName = fileName;
		else
			newFileName = fileName.substr(0, lastdot);

		newFileName.append(extension);

		if (silent == false)
			std::cout << "Converting and Saving Image..." << std::endl;
		
		Pylon::CImagePersistence::Save(destinationFileFormat, newFileName.c_str(), tempImage);
		
		if (silent == false)
			std::cout << "Image saved as: " << newFileName << std::endl;

		return true;
	}
	catch (GenICam::GenericException &e)
	{
		// Error handling.
		std::cerr << "An exception occurred: " << e.GetDescription() << std::endl;
		return false;
	}
	catch (std::runtime_error &e)
	{
		// Error handling.
		std::cerr << "An exception occurred: " << e.what() << std::endl;
		return false;
	}
	catch (...)
	{
		// Error handling.
		std::cerr << "An unknown exception occurred: " << std::endl;
		return false;
	}
}

void PrintHelpMenu()
{
	std::cout << std::endl;
	std::cout << "PylonRawFileConverter:" << std::endl;
	std::cout << " Converts a Pylon Viewer .raw image file to a different format like .png, .tiff, .jpg, .bmp." << std::endl;
	std::cout << " Run \"PylonRawFileConverter.exe --help\" to display these instructions." << std::endl;
	std::cout << std::endl;
	std::cout << "Usage Options:" << std::endl;
	std::cout << " 1. Manual: Simply run program and follow the menus." << std::endl;
	std::cout << " 2. Console: Run PylonRawFileConverter with these options:" << std::endl;
	std::cout << "      --file (name of the raw file to convert)" << std::endl;
	std::cout << "      --width (the Width of the raw image)" << std::endl;
	std::cout << "      --height (the Height of the raw image)" << std::endl;
	std::cout << "      --pixeltype (the Pixel Type of the raw image. See list below...)" << std::endl;
	std::cout << "      --fileformat (the file format to convert to. See list below...)" << std::endl;
	std::cout << "      --batch (convert all raw images in current folder. All must have same Width, Height, Pixel Type, and format.)" << std::endl;
	std::cout << "      --parse (parse a raw image's file name to determine properties. File name must follow the style below...)" << std::endl;
	std::cout << "      --parseprefix (specify your own filename prefix for parsing. Default: \"" << PARSE_PREFIX_DEFAULT << "\")" << std::endl;
	std::cout << "      --silent (suppress all console output except error messages)" << std::endl;
	std::cout << " 3. Drag-n-Drop: On Windows, simply drag and drop a parseable raw image with default prefix file onto the icon." << std::endl;
	std::cout << endl;
	std::cout << "Examples:" << std::endl;
	std::cout << " 1. Convert a single file:" << std::endl;
	std::cout << "     PylonRawFileConverter.exe --file myimage.raw --width 640 --height 480 --pixeltype 1 --fileformat 2" << std::endl;
	std::cout << " 2. Convert a batch of files:" << std::endl;
	std::cout << "     PylonRawFileConverter.exe --batch --width 640 --height 480 --pixeltype 1 --fileformat 2" << std::endl;
	std::cout << " 3. Parse and convert a single file: " << std::endl;
	std::cout << "     (filename MUST be in this style: <parseprefix>_<width>_<height>_<pixeltype>_<fileformat>_<anything>.raw)" << std::endl;
	std::cout << "     (Default parseprefix is \"" << PARSE_PREFIX_DEFAULT << "\")" << std::endl;
	std::cout << "     Drag-n-Drop the file " << PARSE_PREFIX_DEFAULT << "_640_480_1_2_blahblah.raw onto the .exe icon." << std::endl;
	std::cout << "     PylonRawFileConverter.exe --parse --file " << PARSE_PREFIX_DEFAULT << "_640_480_1_2_blahblah.raw" << std::endl;
	std::cout << "     PylonRawFileConverter.exe --parse --parseprefix myPrefix --file myPrefix_640_480_1_2_blahblah.raw" << std::endl;
	std::cout << " 4. Parse and convert all raw files in current directory: (filenames MUST be in the above style.)" << std::endl;
	std::cout << "     PylonRawFileConverter.exe --batch --parse" << std::endl;
	std::cout << "     PylonRawFileConverter.exe --batch --parse --parseprefix myPrefix" << std::endl;
	std::cout << std::endl;
	std::cout << "Pixel Type List: " << std::endl;
	std::cout << " 1 : PixelType_Mono8" << std::endl;
	std::cout << " 2 : PixelType_Mono10" << std::endl;
	std::cout << " 3 : PixelType_Mono12" << std::endl; 
	std::cout << " 4 : PixelType_BayerBG8" << std::endl;
	std::cout << " 5 : PixelType_BayerBG12" << std::endl;
	std::cout << " 6 : PixelType_BayerGB8" << std::endl;
	std::cout << " 7 : PixelType_BayerGB12" << std::endl;
	std::cout << " 8 : PixelType_BayerGR8" << std::endl;
	std::cout << " 9 : PixelType_BayerGR12" << std::endl;
	std::cout << " 10: PixelType_BayerRG8" << std::endl;
	std::cout << " 11: PixelType_BayerRG12" << std::endl;
	std::cout << " 12: PixelType_RGB8packed" << std::endl;
	std::cout << " 13: PixelType_BGR8packed" << std::endl;
	std::cout << " 14: PixelType_YUV422_YUYV_Packed" << std::endl;
	std::cout << " 15: PixelFormat_YCbCr422_8" << std::endl;
	std::cout << std::endl;
	std::cout << "File Format List: " << std::endl;
	std::cout << " 1: TIFF" << std::endl;
	std::cout << " 2: PNG" << std::endl;
	std::cout << " 3: BMP" << std::endl;
	std::cout << " 4: JPG" << std::endl;
	std::cout << std::endl;
	std::cout << "Copyright (c) 2019 Matthew Breit - matt.breit@baslerweb.com or matt.breit@gmail.com" << std::endl;
	std::cout << "Licensed under the Apache License, Version 2.0 (http ://www.apache.org/licenses/LICENSE-2.0)" << std::endl;
	std::cout << "Distributed on an \"AS IS\" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND." << std::endl;
}

bool ParseFileName(std::string &rawFileName, std::string prefix, int numFields, uint32_t &rawWidth, uint32_t &rawHeight, int &rawPixelType_int, int &newFileFormat_int)
{
	try
	{
		size_t prefixPos = rawFileName.find(prefix);
		if (prefixPos != std::string::npos)
			rawFileName.erase(0, prefixPos);

		if (rawFileName.substr(0, prefix.length()) == prefix)
		{
			std::string s = rawFileName;
			std::string delimiter = "_";
			std::vector<std::string> info;

			size_t pos = 0;
			do
			{
				pos = s.find(delimiter);
				std::string data = s.substr(0, pos);
				info.push_back(data);
				s.erase(0, pos + delimiter.length());
			} while (pos != std::string::npos);

			if (info.size() < numFields)
			{
				std::cout << rawFileName << std::endl;
				std::string errMsg = "File Name Invalid. Please check format matches eg: ";
				errMsg.append(prefix);
				errMsg.append("_640_480_1_2_blahblah.raw.");
				throw std::runtime_error(errMsg);
			}

			std::string::size_type sz;
			rawWidth = std::stoi(info[1], &sz, 10);
			rawHeight = std::stoi(info[2], &sz, 10);
			rawPixelType_int = std::stoi(info[3], &sz, 10);
			newFileFormat_int = std::stoi(info[4], &sz, 10);

			return true;
		}
		else
		{
			std::cout << rawFileName << std::endl;
			std::string errMsg = "File Name Invalid. Please check format matches eg: ";
			errMsg.append(prefix);
			errMsg.append("_640_480_1_2_blahblah.raw.");
			throw std::runtime_error(errMsg);
		}
	}
	catch (GenICam::GenericException &e)
	{
		// Error handling.
		std::cerr << "An exception occurred: " << e.GetDescription() << std::endl;
		return false;
	}
	catch (std::runtime_error &e)
	{
		// Error handling.
		std::cerr << "An exception occurred: " << e.what() << std::endl;
		return false;
	}
	catch (...)
	{
		// Error handling.
		std::cerr << "An unknown exception occurred: " << std::endl;
		return false;
	}
}

int main(int argc, char* argv[])
{
	int exitCode = 0;

	Pylon::PylonAutoInitTerm autoInitTerm;

	try
	{
		bool batchMode = false;
		bool parseMode = false;
		string parsePrefix = PARSE_PREFIX_DEFAULT;
		string rawFileName = "";
		uint32_t rawWidth = 0;
		uint32_t rawHeight = 0;
		int rawPixelType_int = 0;
		int newFileFormat_int = 0;
		Pylon::EPixelType rawPixelType;
		Pylon::EImageFileFormat newFileFormat;		

		if (argc > 1)
		{
			for (int i = 0; i < argc; i++)
			{
				std::string argument = string(argv[i]);

				if (argument.find(".raw") != std::string::npos && i == 1)
				{
					rawFileName = string(argv[i]);
					parseMode = true;
					batchMode = false;
					pauseBeforeExit = false;
					break;
				}

				if (argument.find("--") != std::string::npos)
				{
					if (string(argv[i]) == "--help")
					{
						PrintHelpMenu();
						cout << endl;
						cout << endl;
						cout << "Press Enter to exit." << endl;
						while (cin.get() != '\n');
						return 1;
					}
					else if (string(argv[i]) == "--batch")
					{
						batchMode = true;
					}
					else if (string(argv[i]) == "--parse")
					{
						parseMode = true;
					}
					else if (string(argv[i]) == "--parseprefix")
					{
						parsePrefix = string(argv[i + 1]);
					}
					else if (string(argv[i]) == "--file")
					{
						rawFileName = string(argv[i + 1]);
					}
					else if (string(argv[i]) == "--width")
					{
						std::string::size_type sz;
						rawWidth = stoi(string(argv[i + 1]), &sz, 10);
					}
					else if (string(argv[i]) == "--height")
					{
						std::string::size_type sz;
						rawHeight = stoi(string(argv[i + 1]), &sz, 10);
					}
					else if (string(argv[i]) == "--pixeltype")
					{
						std::string::size_type sz;
						rawPixelType_int = stoi(string(argv[i + 1]), &sz, 10);
					}
					else if (string(argv[i]) == "--fileformat")
					{
						std::string::size_type sz;
						newFileFormat_int = stoi(string(argv[i + 1]), &sz, 10);
					}
					else if (string(argv[i]) == "--silent")
					{
						silent = true;
						pauseBeforeExit = false;
					}
					else
					{
						cout << endl << "INVALID OPTION: " << argument << endl;
						cout << endl;
						cout << endl;
						cout << "Press Enter to exit." << endl;
						while (cin.get() != '\n');
						return 1;
					}
				}
			}
		}
		else
		{
			std::cout << "PylonRawFileConverter" << std::endl;
			std::cout << "(c) 2019 Matthew Breit - matt.breit@baslerweb.com or matt.breit@gmail.com" << std::endl;
			std::cout << "Run \"PylonRawFileConverter --help\" for instructions and options." << std::endl;
			std::cout << std::endl;
			std::cout << "Enter Filename of .raw Image (or enter \"batch\" to convert all files in directory): ";
			std::cin >> rawFileName;

			if (rawFileName == "batch")
			{
				batchMode = true;
				std::cout << std::endl;
				std::cout << "**** Batch mode selected. All images MUST have the same Width, Height, Pixel Type, and Target File Format! ****" << std::endl;;
			}

			std::cout << "Enter Image Width: ";
			std::cin >> rawWidth;

			std::cout << "Enter Image Height: ";
			std::cin >> rawHeight;

			std::cout << "Select Pixel Type from list below: " << std::endl;
			std::cout << " 1 : PixelType_Mono8" << std::endl;
			std::cout << " 2 : PixelType_Mono10" << std::endl;
			std::cout << " 3 : PixelType_Mono12" << std::endl;
			std::cout << " 4 : PixelType_BayerBG8" << std::endl;
			std::cout << " 5 : PixelType_BayerBG12" << std::endl;
			std::cout << " 6 : PixelType_BayerGB8" << std::endl;
			std::cout << " 7 : PixelType_BayerGB12" << std::endl;
			std::cout << " 8 : PixelType_BayerGR8" << std::endl;
			std::cout << " 9 : PixelType_BayerGR12" << std::endl;
			std::cout << " 10: PixelType_BayerRG8" << std::endl;
			std::cout << " 11: PixelType_BayerRG12" << std::endl;
			std::cout << " 12: PixelType_RGB8packed" << std::endl;
			std::cout << " 13: PixelType_BGR8packed" << std::endl;
			std::cout << " 14: PixelType_YUV422_YUYV_Packed" << std::endl;
			std::cout << " 15: PixelFormat_YCbCr422_8" << std::endl;
			std::cout << "Enter Selection: ";
			std::cin >> rawPixelType_int;

			std::cout << "Select Target File Format to convert to: " << std::endl;
			std::cout << " 1: TIFF" << std::endl;
			std::cout << " 2: PNG" << std::endl;
#ifdef PYLON_WIN_BUILD
			std::cout << " 3: BMP" << std::endl;
			std::cout << " 4: JPG" << std::endl;
#endif
			std::cout << "Enter Selection: ";
			std::cin >> newFileFormat_int;
		}

		if (batchMode == true)
		{
			std::vector<std::string> fileNames;
#ifdef PYLON_WIN_BUILD
			std::string searchPath = "*.raw";
			WIN32_FIND_DATAA fd;
			HANDLE hFind = ::FindFirstFileA(searchPath.c_str(), &fd);
			if (hFind != INVALID_HANDLE_VALUE)
			{
				do {
					// read all (real) files in current folder
					// , delete '!' read other 2 default folder . and ..
					if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
					{
						fileNames.push_back(fd.cFileName);
					}
				} while (::FindNextFileA(hFind, &fd));
				::FindClose(hFind);
			}
#endif
#ifndef PYLON_WIN_BUILD
			std::string searchPath = "./";
			// Read the files names in the directory
			DIR *pDirectory;
			struct dirent *pDirectoryEntry;
			pDirectory = opendir(searchPath.c_str());

			if (pDirectory != NULL)
			{
				while ((pDirectoryEntry = readdir(pDirectory)) != NULL)
				{
					std::string fileName = std::string(pDirectoryEntry->d_name);
					if (fileName != "." && fileName != ".." && fileName.find(".raw") != std::string::npos)
						fileNames.push_back(fileName);
				}
			}
#endif
			for (size_t i = 0; i < fileNames.size(); i++)
			{
				rawFileName = fileNames[i];

				if (silent == false)
				{
					std::cout << std::endl;
					std::cout << "Converting File: " << rawFileName << "..." << std::endl;
				}

				if (parseMode == true)
				{
					if (ParseFileName(rawFileName, parsePrefix, PARSE_NUM_FIELDS, rawWidth, rawHeight, rawPixelType_int, newFileFormat_int) == false)
						throw std::runtime_error("ParseFileName() failed.");
				}

				rawPixelType = PixelTypeFromInt(rawPixelType_int);
				newFileFormat = FileFormatFromInt(newFileFormat_int);

				if (RawFileConverter(rawFileName, rawWidth, rawHeight, rawPixelType, newFileFormat) == false)
					throw std::runtime_error("RawFileConverter() failed.");
			}
		}
		else
		{
			if (silent == false)
			{
				std::cout << std::endl;
				std::cout << "Converting File: " << rawFileName << "..." << std::endl;
			}

			if (parseMode == true)
			{
				if (ParseFileName(rawFileName, parsePrefix, PARSE_NUM_FIELDS, rawWidth, rawHeight, rawPixelType_int, newFileFormat_int) == false)
					throw std::runtime_error("ParseFileName() failed.");
			}

			rawPixelType = PixelTypeFromInt(rawPixelType_int);
			newFileFormat = FileFormatFromInt(newFileFormat_int);

			if (RawFileConverter(rawFileName, rawWidth, rawHeight, rawPixelType, newFileFormat) == false)
				throw std::runtime_error("RawFileConverter() failed.");
		}

	}
	catch (GenICam::GenericException &e)
	{
		// Error handling.
		std::cerr << "An exception occurred: " << e.GetDescription() << std::endl;
		exitCode = 1;
		pauseBeforeExit = true;
	}
	catch (std::runtime_error &e)
	{
		// Error handling.
		std::cerr << "An exception occurred: " << e.what() << std::endl;
		exitCode = 1;
		pauseBeforeExit = true;
	}
	catch (...)
	{
		// Error handling.
		std::cerr << "An unknown exception occurred. " << std::endl;
		exitCode = 1;
		pauseBeforeExit = true;
	}

	// Comment the following two lines to disable waiting on exit.
	if (pauseBeforeExit == true)
	{
		std::cerr << std::endl << "Press Enter to exit." << std::endl;
		while (std::cin.get() != '\n');
		std::cerr << std::endl << "Press Enter to exit." << std::endl;
		while (std::cin.get() != '\n');
	}

	return exitCode;
}