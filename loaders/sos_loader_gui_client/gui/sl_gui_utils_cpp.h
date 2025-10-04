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