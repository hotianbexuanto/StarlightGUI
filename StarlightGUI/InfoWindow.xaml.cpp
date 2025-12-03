#include "pch.h"
#include "InfoWindow.xaml.h"
#if __has_include("InfoWindow.g.cpp")
#include "InfoWindow.g.cpp"
#endif

#include <winrt/Microsoft.UI.h>
#include <winrt/Microsoft.UI.Windowing.h>
#include <winrt/Microsoft.UI.Dispatching.h>
#include <winrt/Microsoft.UI.Composition.SystemBackdrops.h>
#include <winrt/Windows.UI.h>
#include <Windows.h>
#include <winrt/Windows.UI.Xaml.Interop.h>
#include <winrt/Microsoft.UI.Xaml.Media.Imaging.h>
#include <MainWindow.xaml.h>
#include <Utils/ProcessInfo.h>

using namespace winrt;
using namespace WinUI3Package;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Microsoft::UI::Xaml::Media::Imaging;
using namespace Microsoft::UI::Dispatching;
using namespace Microsoft::UI::Composition::SystemBackdrops;

namespace winrt::StarlightGUI::implementation
{
    static HWND globalHWND;
    InfoWindow* g_infoWindowInstance = nullptr;
    winrt::StarlightGUI::ProcessInfo processForInfoWindow = nullptr;

    InfoWindow::InfoWindow() {
        InitializeComponent();

        auto windowNative{ this->try_as<::IWindowNative>() };
        HWND hWnd{ 0 };
        windowNative->get_WindowHandle(&hWnd);
        globalHWND = hWnd;

        SetWindowPos(hWnd, g_mainWindowInstance->GetWindowHandle(), 0, 0, 1200, 800, SWP_NOMOVE);

        this->ExtendsContentIntoTitleBar(true);
        this->SetTitleBar(AppTitleBar());

        // 外观
        LoadBackdrop();
        LoadNavigation();

        g_infoWindowInstance = this;

        for (auto& window : g_mainWindowInstance->m_openWindows) {
            if (window) {
                window.Close();
            }
        }
        g_mainWindowInstance->m_openWindows.push_back(*this);

        MainFrame().Navigate(xaml_typename<StarlightGUI::Process_ThreadPage>());
        RootNavigation().SelectedItem(RootNavigation().MenuItems().GetAt(0));
        ProcessName().Text(processForInfoWindow.Name());
        this->Title(processForInfoWindow.Name());
    }

    void InfoWindow::RootNavigation_ItemInvoked(Microsoft::UI::Xaml::Controls::NavigationView sender, Microsoft::UI::Xaml::Controls::NavigationViewItemInvokedEventArgs args)
    {
        auto invokedItem = args.InvokedItem().try_as<winrt::hstring>();

        if (invokedItem == L"线程")
        {
            MainFrame().Navigate(xaml_typename<StarlightGUI::Process_ThreadPage>());
            RootNavigation().SelectedItem(RootNavigation().MenuItems().GetAt(0));
        }
        else if (invokedItem == L"句柄")
        {
            MainFrame().Navigate(xaml_typename<StarlightGUI::Process_HandlePage>());
            RootNavigation().SelectedItem(RootNavigation().MenuItems().GetAt(1));
        }
        else if (invokedItem == L"模块")
        {
            MainFrame().Navigate(xaml_typename<StarlightGUI::Process_ModulePage>());
            RootNavigation().SelectedItem(RootNavigation().MenuItems().GetAt(2));
        }
    }

    void InfoWindow::LoadBackdrop()
    {
        auto background_type = ReadConfig("background_type", "Static");

        if (background_type == "Mica") {
            micaBackdrop = CustomMicaBackdrop();
            this->SystemBackdrop(micaBackdrop);

            auto mica_type = ReadConfig("mica_type", "BaseAlt");
            if (mica_type == "Base") {
                micaBackdrop.Kind(MicaKind::Base);
            }
            else {
                micaBackdrop.Kind(MicaKind::BaseAlt);
            }
        }
        else if (background_type == "Acrylic") {
            acrylicBackdrop = CustomAcrylicBackdrop();
            this->SystemBackdrop(acrylicBackdrop);

            auto acrylic_type = ReadConfig("acrylic_type", "Default");
            if (acrylic_type == "Base") {
                acrylicBackdrop.Kind(DesktopAcrylicKind::Base);
            }
            else if (acrylic_type == "Thin") {
                acrylicBackdrop.Kind(DesktopAcrylicKind::Thin);
            }
            else {
                acrylicBackdrop.Kind(DesktopAcrylicKind::Default);
            }
        }
        else
        {
            this->SystemBackdrop(nullptr);
        }
    }

    void InfoWindow::LoadNavigation()
    {
        auto navigation_style = ReadConfig("navigation_style", "LeftCompact");

        if (navigation_style == "Left") {
            RootNavigation().PaneDisplayMode(NavigationViewPaneDisplayMode::Left);
        }
        else if (navigation_style == "Top") {
            RootNavigation().PaneDisplayMode(NavigationViewPaneDisplayMode::Top);
        }
        else
        {
            RootNavigation().PaneDisplayMode(NavigationViewPaneDisplayMode::LeftCompact);
        }
    }


    HWND InfoWindow::GetWindowHandle()
    {
        return globalHWND;
    }
}
