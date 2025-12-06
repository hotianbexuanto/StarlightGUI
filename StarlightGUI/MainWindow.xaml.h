#pragma once

#include "MainWindow.g.h"
#include <winrt/Microsoft.UI.Composition.SystemBackdrops.h>
#include <winrt/Windows.System.h>
#include <winrt/Windows.UI.h>

namespace winrt::StarlightGUI::implementation
{
    extern winrt::WinUI3Package::CustomMicaBackdrop micaBackdrop;
    extern winrt::WinUI3Package::CustomAcrylicBackdrop acrylicBackdrop;

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

        std::vector<winrt::StarlightGUI::InfoWindow> m_openWindows;

    };

    extern MainWindow* g_mainWindowInstance;
}

namespace winrt::StarlightGUI::factory_implementation
{
    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {
    };
}