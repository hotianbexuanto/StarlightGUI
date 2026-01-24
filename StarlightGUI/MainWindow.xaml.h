#pragma once

#include "MainWindow.g.h"
#include <winrt/Microsoft.UI.Composition.SystemBackdrops.h>
#include <winrt/Windows.System.h>
#include <winrt/Windows.UI.h>

namespace winrt::StarlightGUI::implementation
{
    struct MainWindow : MainWindowT<MainWindow>
    {
        MainWindow();
        ~MainWindow();

        void RootNavigation_ItemInvoked(Microsoft::UI::Xaml::Controls::NavigationView sender, Microsoft::UI::Xaml::Controls::NavigationViewItemInvokedEventArgs args);

        HWND GetWindowHandle();

        // 外观
        winrt::fire_and_forget LoadBackdrop();
        winrt::fire_and_forget LoadBackground();
        winrt::fire_and_forget LoadNavigation();

        // 驱动和模块
        winrt::fire_and_forget LoadModules();
        winrt::fire_and_forget CheckUpdate();

        // 窗口
        static LRESULT CALLBACK MainWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

        std::vector<winrt::StarlightGUI::InfoWindow> m_openWindows;

        inline static bool loaded = false;
        inline static HWND globalHWND;
    };

    extern MainWindow* g_mainWindowInstance;
}

namespace winrt::StarlightGUI::factory_implementation
{
    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {
    };
}