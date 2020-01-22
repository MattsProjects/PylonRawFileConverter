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
		Pylon::CPylonImage tempImage;

		LoadRaw(fileName.c_str(), tempImage, imageWidth, imageHeight, imagePixelFormat);

		std::string newFileName = "";
		size_t lastdot = fileName.find_last_of(".");

		if (lastdot == std::string::npos)
			newFileName = fileName;
		else
			newFileName = fileName.substr(0, lastdot);

		switch (destinationFileFormat)
		{
#ifdef PYLON_WIN_BUILD

			case Pylon::EImageFileFormat::ImageFileFormat_Bmp:
				newFileName.append(".bmp");
				break;
			case Pylon::EImageFileFormat::ImageFileFormat_Jpeg:
				newFileName.append(".jpg");
				break;
#endif
			case Pylon::EImageFileFormat::ImageFileFormat_Tiff:
				newFileName.append(".tiff");
				break;
			case Pylon::EImageFileFormat::ImageFileFormat_Png:
				newFileName.append(".png");
				break;
			case Pylon::EImageFileFormat::ImageFileFormat_Raw:
				newFileName.append(".raw");
				break;
			default:
				newFileName.append(".png");
				break;
		}
		std::cout << "Converting and Saving Image..." << std::endl;
		Pylon::CImagePersistence::Save(destinationFileFormat, newFileName.c_str(), tempImage);
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

int main(int argc, char* argv[])
{
	int exitCode = 0;

	Pylon::PylonAutoInitTerm autoInitTerm;

	try
	{
		std::string myFileName = "";
		uint32_t myWidth = 0;
		uint32_t myHeight = 0;
		int intPixelType = 0;
		int intFileFormat = 0;
		Pylon::EPixelType myPixelType;
		Pylon::EImageFileFormat myFileFormat;

		if (argc > 1)
		{
			std::string arg1 = std::string(argv[1]);
			if (arg1 == "?")
			{
				std::cout << "PylonSample_RawFileConverter:" << std::endl;
				std::cout << "  Converts a Pylon Viewer .raw image file to a different format like .png, .tiff, .jpg, .bmp." << std::endl;
				std::cout << std::endl;
				std::cout << "Usage:" << std::endl;
				std::cout << "  1. Manual: Simply run program and follow instructions." << std::endl;
				std::cout << "  2. Automatic: PylonSample_RawFileConverter.exe filename" << std::endl;
				std::cout << "     filename must be prefixed and formatted as follows:" << std::endl;
				std::cout << "     rawimage_width_height_PixelType_FileFormat.raw" << std::endl;
				std::cout << "     Example: rawimage_2448_2048_4_4.raw" << std::endl;
				std::cout << std::endl;
				std::cout << "PixelType List: " << std::endl;
				std::cout << "  1 : PixelType_Mono8" << std::endl;
				std::cout << "  2 : PixelType_Mono10" << std::endl;
				std::cout << "  3 : PixelType_Mono12" << std::endl;
				std::cout << "  4 : PixelType_BayerBG8" << std::endl;
				std::cout << "  5 : PixelType_BayerBG12" << std::endl;
				std::cout << "  6 : PixelType_BayerGB8" << std::endl;
				std::cout << "  7 : PixelType_BayerGB12" << std::endl;
				std::cout << "  8 : PixelType_BayerGR8" << std::endl;
				std::cout << "  9 : PixelType_BayerGR12" << std::endl;
				std::cout << "  10: PixelType_BayerRG8" << std::endl;
				std::cout << "  11: PixelType_BayerRG12" << std::endl;
				std::cout << "  12: PixelType_RGB8packed" << std::endl;
				std::cout << "  13: PixelType_BGR8packed" << std::endl;
				std::cout << "  14: PixelType_YUV422_YUYV_Packed" << std::endl;
				std::cout << "  15: PixelFormat_YCbCr422_8" << std::endl;
				std::cout << std::endl;
				std::cout << "FileFormat List: " << std::endl;
				std::cout << "  1: BMP" << std::endl;
				std::cout << "  2: TIFF" << std::endl;
				std::cout << "  3: JPG" << std::endl;
				std::cout << "  4: PNG" << std::endl;
				return 0;
			}

			if (arg1 == "-m")
			{
				myFileName.append(std::string(argv[2]).c_str());
				std::string::size_type sz;
				myWidth = std::stoi(std::string(argv[3]), &sz, 10);
				myHeight = std::stoi(std::string(argv[4]), &sz, 10);
				intPixelType = std::stoi(std::string(argv[5]), &sz, 10);
				intFileFormat = std::stoi(std::string(argv[6]), &sz, 10);
			}
			else
			{
				std::string inputFileName = std::string(argv[1]);
				std::string prefix = "rawimage";
				size_t prefixPos = inputFileName.find(prefix);
				if (prefixPos != std::string::npos)
					inputFileName.erase(0, prefixPos);

				if (inputFileName.substr(0, prefix.length()) == prefix)
				{
					std::string s = inputFileName;
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

					if (info.size() != 6)
					{
						throw std::runtime_error("unable to parse file name. Please check format matches: raw_width_height_pixeltype_fileformat_.raw.");
					}

					myFileName = inputFileName;
					std::string::size_type sz;
					myWidth = std::stoi(info[1], &sz, 10);
					myHeight = std::stoi(info[2], &sz, 10);
					intPixelType = std::stoi(info[3], &sz, 10);
					intFileFormat = std::stoi(info[4], &sz, 10);
				}
				else
				{
					std::cout << inputFileName << std::endl;
					std::string ermsg = "File Name Invalid. File must start with \"";
					ermsg.append(prefix);
					ermsg.append("\"");
					throw std::runtime_error(ermsg.c_str());
				}
			}

			std::cout << "File Name  : " << myFileName << std::endl;
			std::cout << "Width      : " << myWidth << std::endl;
			std::cout << "Height     : " << myHeight << std::endl;
			std::cout << "PixelType  : " << intPixelType << std::endl;
			std::cout << "FileFormat : " << intFileFormat << std::endl;
		}
		else
		{
			std::cout << "Enter Filename of Pylon .raw Image (or enter \"batch\" to select all .raw files found): ";
			std::cin >> myFileName;

			if (myFileName == "batch")
			{
				std::cout << std::endl;
				std::cout << "**** Batch mode selected. All images MUST have the same Width, Height, Pixel Type, and Target File Format! ****" << std::endl;;
			}

			std::cout << "Enter Image Width: ";
			std::cin >> myWidth;

			std::cout << "Enter Image Height: ";
			std::cin >> myHeight;

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
			std::cout << "Selection: ";
			std::cin >> intPixelType;

			std::cout << "Select Target File Format to convert to: " << std::endl;
			std::cout << " 1: TIFF" << std::endl;
			std::cout << " 2: PNG" << std::endl;
#ifdef PYLON_WIN_BUILD
			std::cout << " 3: BMP" << std::endl;
			std::cout << " 4: JPG" << std::endl;
#endif
			std::cout << "Selection: ";
			std::cin >> intFileFormat;
		}

		switch (intPixelType)
		{
			case 1:
				myPixelType = Pylon::EPixelType::PixelType_Mono8;
				break;
			case 2:
				myPixelType = Pylon::EPixelType::PixelType_Mono10;
				break;
			case 3:
				myPixelType = Pylon::EPixelType::PixelType_Mono12;
				break;
			case 4:
				myPixelType = Pylon::EPixelType::PixelType_BayerBG8;
				break;
			case 5:
				myPixelType = Pylon::EPixelType::PixelType_BayerBG12;
				break;
			case 6:
				myPixelType = Pylon::EPixelType::PixelType_BayerGB8;
				break;
			case 7:
				myPixelType = Pylon::EPixelType::PixelType_BayerGB12;
				break;
			case 8:
				myPixelType = Pylon::EPixelType::PixelType_BayerGR8;
				break;
			case 9:
				myPixelType = Pylon::EPixelType::PixelType_BayerGR12;
				break;
			case 10:
				myPixelType = Pylon::EPixelType::PixelType_BayerRG8;
				break;
			case 11:
				myPixelType = Pylon::EPixelType::PixelType_BayerRG12;
				break;
			case 12:
				myPixelType = Pylon::EPixelType::PixelType_RGB8packed;
				break;
			case 13:
				myPixelType = Pylon::EPixelType::PixelType_BGR8packed;
				break;
			case 14:
				myPixelType = Pylon::EPixelType::PixelType_YUV422_YUYV_Packed;
				break;
			case 15:
				myPixelType = Pylon::EPixelType::PixelType_YUV422_YUYV_Packed;
				break;
			default:
				throw std::runtime_error("Invalid Pixel Type Selection");
				break;
		}

		switch (intFileFormat)
		{

			case 1:
				myFileFormat = Pylon::EImageFileFormat::ImageFileFormat_Tiff;
				break;
			case 2:
				myFileFormat = Pylon::EImageFileFormat::ImageFileFormat_Png;
				break;
#ifdef PYLON_WIN_BUILD
			case 3:
				myFileFormat = Pylon::EImageFileFormat::ImageFileFormat_Bmp;
				break;
			case 4:
				myFileFormat = Pylon::EImageFileFormat::ImageFileFormat_Jpeg;
				break;
#endif
			default:
				throw std::runtime_error("Invalid File Format Selection");
				break;
		}

		if (myFileName == "")
			throw std::runtime_error("No Filename Given");
		if (myWidth == 0)
			throw std::runtime_error("Width must be greater than 0.");
		if (myHeight == 0)
			throw std::runtime_error("Height must be greater than 0.");

		if (myFileName == "batch")
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
				std::cout << "Converting File: " << fileNames[i] << "..." << std::endl;
				if (RawFileConverter(fileNames[i], myWidth, myHeight, myPixelType, myFileFormat) == false)
					throw std::runtime_error("RawFileConverter() failed.");
			}
		}
		else
		{
			if (RawFileConverter(myFileName, myWidth, myHeight, myPixelType, myFileFormat) == false)
				throw std::runtime_error("RawFileConverter() failed.");
		}

	}
	catch (GenICam::GenericException &e)
	{
		// Error handling.
		std::cerr << "An exception occurred: " << e.GetDescription() << std::endl;
		exitCode = 1;
	}
	catch (std::runtime_error &e)
	{
		// Error handling.
		std::cerr << "An exception occurred: " << e.what() << std::endl;
		exitCode = 1;
	}
	catch (...)
	{
		// Error handling.
		std::cerr << "An unknown exception occurred. " << std::endl;
		exitCode = 1;
	}

	// Comment the following two lines to disable waiting on exit.
	std::cerr << std::endl << "Press Enter to exit." << std::endl;
	while (std::cin.get() != '\n');
	while (std::cin.get() != '\n');

	return exitCode;
}
