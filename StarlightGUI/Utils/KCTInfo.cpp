#include "pch.h"
#include "KCTInfo.h"
#if __has_include("KCTInfo.g.cpp")
#include "KCTInfo.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml::Media;

namespace winrt::StarlightGUI::implementation
{

    hstring KCTInfo::Name()
    {
        return m_name;
    }

    void KCTInfo::Name(hstring const& value)
    {
        m_name = value;
    }

    hstring KCTInfo::Address()
    {
        return m_address;
    }

    void KCTInfo::Address(hstring const& value)
    {
        m_address = value;
    }
}