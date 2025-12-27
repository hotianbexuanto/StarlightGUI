#pragma once

#include "WindowInfo.g.h"

namespace winrt::StarlightGUI::implementation
{
    struct WindowInfo : WindowInfoT<WindowInfo>
    {
        WindowInfo() = default;

        hstring Name();
        void Name(hstring const& value);

		hstring Description();
		void Description(hstring const& value);

        hstring Process();
        void Process(hstring const& value);

		hstring ClassName();
		void ClassName(hstring const& value);

        uint64_t Hwnd();
		void Hwnd(uint64_t value);

		int32_t FromPID();
		void FromPID(int32_t value);

        int32_t WindowStyle();
        void WindowStyle(int32_t value);

        int32_t WindowStyleEx();
        void WindowStyleEx(int32_t value);

        winrt::Microsoft::UI::Xaml::Media::ImageSource Icon();
        void Icon(winrt::Microsoft::UI::Xaml::Media::ImageSource const& value);

    private:
        hstring m_name{ L"" };
        hstring m_description{ L"" };
        hstring m_process{ L"" };
        hstring m_className{ L"" };
        uint64_t m_hwnd{ 0 };
		int32_t m_fromPID{ 0 };
        int32_t m_windowStyle{ 0 };
        int32_t m_windowStyleEx{ 0 };
        winrt::Microsoft::UI::Xaml::Media::ImageSource m_icon{ nullptr };
    };
}

namespace winrt::StarlightGUI::factory_implementation
{
    struct WindowInfo : WindowInfoT<WindowInfo, implementation::WindowInfo>
    {
    };
}