#pragma once

#include "pch.h"
#include <pdh.h>

namespace winrt::StarlightGUI::implementation {
    std::wstring GenerateRandomString(size_t length);

    int GenerateRandomNumber(size_t from, size_t to);

    int GetDateAsInt();

    std::wstring FixBackSplash(const hstring& hstr);

    std::wstring RemoveFromString(const hstring& hstr, const hstring& removeStr);

    std::wstring GetParentDirectory(const hstring& path);

    std::string WideStringToString(const hstring& hstr);

    std::wstring ULongToHexString(ULONG64 value);

    std::wstring ULongToHexString(ULONG64 value, int w, bool uppercase, bool prefix);

    std::wstring FormatMemorySize(double bytes);

    std::wstring ExtractFunctionName(const std::string& old);

    std::wstring ExtractFileName(const std::wstring& path);

    double GetValueFromCounter(PDH_HCOUNTER& counter);

    double GetValueFromCounterArray(PDH_HCOUNTER& counter);
}