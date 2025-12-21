#include "pch.h"
#include "WindowInfo.h"
#if __has_include("WindowInfo.g.cpp")
#include "WindowInfo.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml::Media;

namespace winrt::StarlightGUI::implementation
{

    hstring WindowInfo::Name()
    {
        return m_name;
    }

    void WindowInfo::Name(hstring const& value)
    {
        m_name = value;
    }

    hstring WindowInfo::Description()
    {
        return m_description;
	}

    void WindowInfo::Description(hstring const& value)
    {
        m_description = value;
    }

    hstring WindowInfo::Process()
    {
        return m_process;
    }

    void WindowInfo::Process(hstring const& value)
    {
        m_process = value;
    }

    hstring WindowInfo::ClassName()
    {
        return m_className;
    }

    void WindowInfo::ClassName(hstring const& value)
    {
        m_className = value;
	}

    uint64_t WindowInfo::Hwnd()
    {
        return m_hwnd;
	}

    void WindowInfo::Hwnd(uint64_t value)
    {
        m_hwnd = value;
	}

    int32_t WindowInfo::FromPID() 
    {
        return m_fromPID;
	}

    void WindowInfo::FromPID(int32_t value)
    {
        m_fromPID = value;
    }

    ImageSource WindowInfo::Icon()
    {
        return m_icon;
    }

    void WindowInfo::Icon(ImageSource const& value)
    {
        m_icon = value;
    }
}