// SPDX-License-Identifier: GPL-2.0-only OR Proprietary
// Author: Miao Yu (Carnegie Mellon University)
// Author: Virgil Gligor (Carnegie Mellon University)

#ifndef SOSLOADER_GUI_UTILS_CPP_H
#define SOSLOADER_GUI_UTILS_CPP_H

#include <string>

class SL_GUI_UTILS
{
#if defined(__WINDOWS__)
public:
    static std::wstring ConvertCharToLPCWSTR(const char* charStr);
    static std::string ConvertWCharToChar(const wchar_t* wideStr);
#endif // __WINDOWS__
};

#endif // SOSLOADER_GUI_UTILS_CPP_H