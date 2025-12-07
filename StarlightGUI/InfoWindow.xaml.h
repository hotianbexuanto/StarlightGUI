#pragma once

#include "InfoWindow.g.h"
#include <Utils/ProcessInfo.h>

namespace winrt::StarlightGUI::implementation
{
    struct InfoWindow : InfoWindowT<InfoWindow>
    {
        InfoWindow();

        HWND GetWindowHandle();

        void RootNavigation_ItemInvoked(Microsoft::UI::Xaml::Controls::NavigationView sender, Microsoft::UI::Xaml::Controls::NavigationViewItemInvokedEventArgs args);

        // Õ‚π€
        winrt::fire_and_forget LoadBackdrop();
        winrt::fire_and_forget LoadBackground();
        winrt::fire_and_forget LoadNavigation();
    };

    extern winrt::StarlightGUI::ProcessInfo processForInfoWindow;
    extern InfoWindow* g_infoWindowInstance;
}

namespace winrt::StarlightGUI::factory_implementation
{
    struct InfoWindow : InfoWindowT<InfoWindow, implementation::InfoWindow>
    {
    };
}
