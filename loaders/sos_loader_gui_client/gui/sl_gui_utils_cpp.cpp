// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)



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