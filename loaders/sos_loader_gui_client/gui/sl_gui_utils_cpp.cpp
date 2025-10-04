/*
 * -------------------------------------------------------------------------------------
 * Carnegie Mellon University
 * Copyright (C) 2023 - 2023 Carnegie Mellon University. All Rights Reserved.
 * 
 * This software is the proprietary information of Carnegie Mellon University.
 * Use is subject to license terms.
 * 
 * Any reproduction, modification, distribution, or disclosure of this software, or any part of it, without the express
 * written consent of Carnegie Mellon University. is strictly prohibited.
 * -------------------------------------------------------------------------------------
 * 
 * Authors: Miao Yu
 *          Virgil Gligor
 */



#include "sl_gui_utils_cpp.h"

#if defined(__WINDOWS__)
#include <iostream>
#include <string>
#include <vector> // Include vector for std::vector

#include <windows.h>

std::wstring SL_GUI_UTILS::ConvertCharToLPCWSTR(const char* charStr)
{
    if (charStr == nullptr)
        return L"";

    // Determine the required buffer size
    int bufferSize = MultiByteToWideChar(CP_UTF8, 0, charStr, -1, nullptr, 0);
    if (bufferSize == 0)
    {
        // Handle error
        std::cerr << "Error in MultiByteToWideChar: " << GetLastError() << std::endl;
        return L"";
    }

    // Allocate buffer
    std::vector<wchar_t> wideStr(bufferSize);

    // Perform the conversion
    int result = MultiByteToWideChar(CP_UTF8, 0, charStr, -1, wideStr.data(), bufferSize);
    if (result == 0)
    {
        // Handle error
        std::cerr << "Error in MultiByteToWideChar: " << GetLastError() << std::endl;
        return L"";
    }

    return std::wstring(wideStr.data());
}

std::string SL_GUI_UTILS::ConvertWCharToChar(const wchar_t* wideStr)
{
    if (wideStr == nullptr)
        return "";

    // Determine the required buffer size for the converted string.
    int bufferSize = WideCharToMultiByte(CP_UTF8, 0, wideStr, -1, nullptr, 0, nullptr, nullptr);
    if (bufferSize == 0)
    {
        std::cerr << "Error in WideCharToMultiByte: " << GetLastError() << std::endl;
        return "";
    }

    // Allocate a buffer for the char string.
    std::vector<char> charStr(bufferSize);

    // Perform the conversion.
    int result = WideCharToMultiByte(CP_UTF8, 0, wideStr, -1, charStr.data(), bufferSize, nullptr, nullptr);
    if (result == 0)
    {
        std::cerr << "Error in WideCharToMultiByte: " << GetLastError() << std::endl;
        return "";
    }

    return std::string(charStr.data());
}
#endif // __WINDOWS__