// LoadPylonRawFile.h
// Loads a Pylon-Saved .raw file from disk.
// Note: This is designed to work with .raw file saved _only_ by Pylon.
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

namespace LoadPylonRawFile
{
	void Load(const Pylon::String_t& fileName, Pylon::CPylonImage& image, uint32_t width, uint32_t height, Pylon::EPixelType pixelType)
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
				else if (Pylon::BitPerPixel(pixelType) == 10)
					imageSize = (width * height) * 1.25;
				else if (Pylon::BitPerPixel(pixelType) == 12)
					imageSize = (width * height) * 1.5;
				else if (Pylon::BitPerPixel(pixelType) == 16)
					imageSize = (width * height) * 2;
				else
				{
					errorMessage.append("Less than 8 bits per pixel or more than 16bits per pixel is not supported yet.");
					myFile.close();
					throw std::runtime_error(errorMessage.c_str());
				}

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
}