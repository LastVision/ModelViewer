#pragma once
#include <algorithm>
#include <DL_Debug.h>
#include "Matrix.h"
#include <string>
#include "Vector.h"
#include <ShlObj.h>
#include <sstream>

namespace CU
{
	inline std::string ToLower(const std::string& aString)
	{
		std::string data = aString;
		std::transform(data.begin(), data.end(), data.begin(), ::tolower);
		return data;
	}

	inline std::string ToLower(const char* aString)
	{
		std::string data = aString;
		std::transform(data.begin(), data.end(), data.begin(), ::tolower);
		return data;
	}

	inline float Clip(float aNumber, float aLower, float aUpper)
	{
		return fmax(aLower, fmin(aNumber, aUpper));
	}

	inline int ClipInt(int aNumber, int aLower, int aUpper)
	{
		return max(aLower, min(aNumber, aUpper));
	}

	inline std::string GetSubString(std::string aStringToReadFrom, char aCharToFind, bool aReadAfterChar)
	{
		std::string toReturn;
		if (aReadAfterChar == false)
		{
			toReturn = aStringToReadFrom.substr(0, aStringToReadFrom.find(aCharToFind));
		}
		else if (aReadAfterChar == true)
		{
			toReturn = aStringToReadFrom.substr(aStringToReadFrom.find_first_of(aCharToFind) + 1);
		}

		return toReturn;
	}

	inline std::string GetSubString(const std::string& aStringToReadFrom, const std::string& aWordToFind, bool aReadAfterChar
		, int someCharsToSkip = 0)
	{
		std::string toReturn;
		if (aStringToReadFrom.rfind(aWordToFind) != std::string::npos)
		{
			if (aReadAfterChar == false)
			{
				return toReturn = aStringToReadFrom.substr(0, aStringToReadFrom.rfind(aWordToFind));
			}
			else if (aReadAfterChar == true)
			{
				return toReturn = aStringToReadFrom.substr(aStringToReadFrom.rfind(aWordToFind) + (someCharsToSkip - 1));
			}
		}
		return aStringToReadFrom;
	}
	inline std::string GetMyDocumentFolderPath()
	{
		char documents[MAX_PATH];
		HRESULT hr = SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, documents);
		if (hr != S_OK) 
		{
			DL_ASSERT("Can't get document folder!");
		}
		std::stringstream ss;
		ss << documents;
		ss << "\\Distortion Games\\";
		return ss.str();
	}

	//If OptionalExtension is blank, the outputstring will have the same extension as the input string
	//OptionalExtension needs to be entered without a period, "xml", NOT ".xml"
	inline std::string GetGeneratedDataFolderFilePath(const std::string& aFilePath, const std::string& anOptionalNewExtension = "")
	{
		std::string pathWithoutData = GetSubString(aFilePath, "Data", true, 5);
		//std::string pathWithoutData(aFilePath.begin() + 14, aFilePath.end());

		if (anOptionalNewExtension != "")
		{
			int extensionIndex = pathWithoutData.find_last_of(".");
			pathWithoutData = std::string(pathWithoutData.begin(), pathWithoutData.begin() + extensionIndex + 1);
			pathWithoutData += anOptionalNewExtension;
		}

		std::string realDataFilePath = "GeneratedData/";
		realDataFilePath += pathWithoutData;

		size_t found = realDataFilePath.find_first_of('\\');

		while (found != std::string::npos)
		{
			realDataFilePath[found] = '/';
			found = realDataFilePath.find_first_of('\\', found + 1);
		}

		return realDataFilePath;
	}

	//If OptionalExtension is blank, the outputstring will have the same extension as the input string
	//OptionalExtension needs to be entered without a period, "xml", NOT ".xml"
	inline std::string GetRealDataFolderFilePath(const std::string& aFilePath, const std::string& anOptionalNewExtension = "")
	{
		std::string pathWithoutData = GetSubString(aFilePath, "Data", true, 1);
		//std::string pathWithoutData(aFilePath.begin() + 14, aFilePath.end());

		if (anOptionalNewExtension != "")
		{
			int extensionIndex = pathWithoutData.find_last_of(".");
			pathWithoutData = std::string(pathWithoutData.begin(), pathWithoutData.begin() + extensionIndex + 1);
			pathWithoutData += anOptionalNewExtension;
		}

		std::string realDataFilePath;
		realDataFilePath = pathWithoutData;
		
		size_t found = realDataFilePath.find_first_of('\\');

		while (found != std::string::npos)
		{
			realDataFilePath[found] = '/';
			found = realDataFilePath.find_first_of('\\', found + 1);
		}

		return realDataFilePath;
	}

	inline void BuildFoldersInPath(const std::string& aPath)
	{
		unsigned int slashIndex = aPath.find_first_of("/");

		while (slashIndex != std::string::npos)
		{
			std::string folder(aPath.begin(), aPath.begin() + slashIndex);
			CreateDirectory(folder.c_str(), NULL);
			slashIndex = aPath.find_first_of("/", slashIndex + 1);
		}
	}

	inline std::string Concatenate(const char* aFormattedString, ...)
	{
		char buffer[1024];
		va_list args;
		va_start(args, aFormattedString);
		vsprintf_s(buffer, aFormattedString, args);
		perror(buffer);
		va_end(args);

		return buffer;
	}

	inline float StringToFloat(const std::string& aString)
	{
		const char* value = aString.c_str();
		char* end;
		float floatValue = strtof(value, &end);

		DL_ASSERT_EXP(value != end, "Error reading float value: " + aString);
		return floatValue;
	}

	inline int StringToInt(const std::string& aString)
	{
		const char* value = aString.c_str();
		char* end;
		int intValue = strtol(value, &end, 10);

		DL_ASSERT_EXP(value != end, "Error reading int value: " + aString);
		return intValue;
	}

	inline void TrimWhiteSpacesAtBeginAndEnd(std::string& aString)
	{
		if (aString.length() <= 0)
		{
			return;
		}

		unsigned int begin = 0;
		while (aString[begin] == ' ')
		{
			++begin;
		}

		unsigned int end = aString.length();
		if (end > begin)
		{
			while (aString[end-1] == ' ')
			{
				--end;
			}
		}
		if (begin != 0 || end != aString.length())
		{
			aString = std::string(aString.begin() + begin, aString.begin() + end);
		}
	}


	inline Matrix44<float> GetOrientation(Matrix44<float>& aMatrixToRotate, const Vector3<float>& aRotationInDegrees)
	{
		//to radians:
		Vector3<float> rotation(aRotationInDegrees);
		rotation *= 0.0174532925f;
		
		aMatrixToRotate = Matrix44<float>::CreateRotateAroundZ(rotation.z) * aMatrixToRotate;
		aMatrixToRotate = Matrix44<float>::CreateRotateAroundY(rotation.y) * aMatrixToRotate;
		aMatrixToRotate = Matrix44<float>::CreateRotateAroundX(rotation.x) * aMatrixToRotate;

		return aMatrixToRotate;
	}

	inline bool IsValidTextureSize(UINT aValue)
	{
		return aValue == 2 || aValue == 4 || aValue == 8 || aValue == 16 || aValue == 32 || aValue == 64 || aValue == 128
			|| aValue == 256 || aValue == 512 || aValue == 1024 || aValue == 2048 || aValue == 4096 || aValue == 8192
			|| aValue == 1024 * 6;
	}
}