#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif

#include <winrt/Microsoft.UI.h>
#include <winrt/Microsoft.UI.Windowing.h>
#include <winrt/Microsoft.UI.Dispatching.h>
#include <winrt/Microsoft.UI.Composition.SystemBackdrops.h>
#include <winrt/Windows.ApplicationModel.h>
#include <winrt/Windows.UI.h>
#include <winrt/Windows.UI.Xaml.Interop.h>
#include <winrt/Windows.Web.Http.h>
#include <winrt/Windows.Web.Http.Headers.h>
#include <winrt/Windows.Data.Json.h>
#include <winrt/Microsoft.UI.Xaml.Media.Imaging.h>
#include "UpdateDialog.xaml.h"

using namespace winrt;
using namespace WinUI3Package;
using namespace Windows::UI;
using namespace Windows::Graphics;
using namespace Windows::Web::Http;
using namespace Windows::Data::Json;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;
using namespace Windows::ApplicationModel;
using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Microsoft::UI::Xaml::Media::Imaging;
using namespace Microsoft::UI::Dispatching;
using namespace Microsoft::UI::Composition::SystemBackdrops;

namespace winrt::StarlightGUI::implementation
{
    MainWindow* g_mainWindowInstance = nullptr;
    static HWND globalHWND;

    MainWindow::MainWindow()
    {
        InitializeComponent();

        auto windowNative{ this->try_as<::IWindowNative>() };
        HWND hWnd{ 0 };
        windowNative->get_WindowHandle(&hWnd);
        globalHWND = hWnd;

        this->ExtendsContentIntoTitleBar(true);
        this->SetTitleBar(AppTitleBar());
        this->AppWindow().TitleBar().PreferredHeightOption(winrt::Microsoft::UI::Windowing::TitleBarHeightOption::Tall);

        int32_t width = ReadConfig("window_width", 1200);
        int32_t height = ReadConfig("window_height", 800);

        this->AppWindow().Resize(SizeInt32{ width, height });

        // 外观
        LoadBackdrop();
        LoadBackground();
        LoadNavigation();

        g_mainWindowInstance = this;

        // Home page
        MainFrame().Navigate(xaml_typename<StarlightGUI::HomePage>());
        RootNavigation().SelectedItem(RootNavigation().MenuItems().GetAt(0));

        Activated([this](auto&&, auto&&) {
            if (!loaded) {
                if (!KernelInstance::IsRunningAsAdmin()) {
                    CreateInfoBarAndDisplay(L"警告", L"当前正以常规模式运行，大部分功能将无法使用或功能残缺。欲使用完整功能请以管理员身份运行！", InfoBarSeverity::Warning, g_mainWindowInstance);
                }
                else {
                    CreateInfoBarAndDisplay(L"信息", L"正在加载模块，这可能需要一点时间...", InfoBarSeverity::Informational, g_mainWindowInstance);
                }
                LoadModules();
                try {
                    CheckUpdate();
                }
                catch (hresult_error) {
                    CreateInfoBarAndDisplay(L"警告", L"检查更新失败！", InfoBarSeverity::Warning, g_mainWindowInstance);
                }
				loaded = true;
            }
            });


        Closed([this](auto&&, auto&&) {
            int32_t width = this->AppWindow().Size().Width;
            int32_t height = this->AppWindow().Size().Height;

            SaveConfig("window_width", width);
            SaveConfig("window_height", height);

            LOG_INFO(L"MainWindow", L"Saved window size.");
            LOGGER_CLOSE();
            });

        LOG_INFO(L"MainWindow", L"MainWindow initialized.");
    }

    MainWindow::~MainWindow()
    {
        for (auto& window : m_openWindows) {
            if (window) {
                window.Close();
            }
        }
    }

    void MainWindow::RootNavigation_ItemInvoked(Microsoft::UI::Xaml::Controls::NavigationView sender, Microsoft::UI::Xaml::Controls::NavigationViewItemInvokedEventArgs args)
    {
        if (args.IsSettingsInvoked())
        {
            MainFrame().Navigate(xaml_typename<StarlightGUI::SettingsPage>());
            return;
        }

        auto invokedItem = args.InvokedItem().try_as<winrt::hstring>();

        if (invokedItem == L"主页")
        {
            MainFrame().Navigate(xaml_typename<StarlightGUI::HomePage>());
            RootNavigation().SelectedItem(RootNavigation().MenuItems().GetAt(0));
        }
        else if (invokedItem == L"任务管理") {
            MainFrame().Navigate(xaml_typename<StarlightGUI::TaskPage>());
            RootNavigation().SelectedItem(RootNavigation().MenuItems().GetAt(1));
        }
        else if (invokedItem == L"模块管理") {
            MainFrame().Navigate(xaml_typename<StarlightGUI::KernelModulePage>());
            RootNavigation().SelectedItem(RootNavigation().MenuItems().GetAt(2));
        }
        else if (invokedItem == L"文件管理") {
            MainFrame().Navigate(xaml_typename<StarlightGUI::FilePage>());
            RootNavigation().SelectedItem(RootNavigation().MenuItems().GetAt(3));
        }
        else if (invokedItem == L"窗口管理") {
            MainFrame().Navigate(xaml_typename<StarlightGUI::WindowPage>());
            RootNavigation().SelectedItem(RootNavigation().MenuItems().GetAt(4));
        }
        else if (invokedItem == L"系统功能") {
            MainFrame().Navigate(xaml_typename<StarlightGUI::UtilityPage>());
            RootNavigation().SelectedItem(RootNavigation().MenuItems().GetAt(5));
        }
        else if (invokedItem == L"关于") {
            MainFrame().Navigate(xaml_typename<StarlightGUI::HelpPage>());
            RootNavigation().SelectedItem(RootNavigation().FooterMenuItems().GetAt(0));
        }
    }

    winrt::fire_and_forget MainWindow::LoadBackdrop()
    {
        std::string background_type = ReadConfig("background_type", "Static");
        std::string option = "*";

        if (background_type == "Mica") {
            CustomMicaBackdrop micaBackdrop = CustomMicaBackdrop();

            this->SystemBackdrop(micaBackdrop);

            option = ReadConfig("mica_type", "BaseAlt");
            if (option == "Base") {
                micaBackdrop.Kind(MicaKind::Base);
            }
            else {
                micaBackdrop.Kind(MicaKind::BaseAlt);
            }
        }
        else if (background_type == "Acrylic") {
            CustomAcrylicBackdrop acrylicBackdrop = CustomAcrylicBackdrop();

            this->SystemBackdrop(acrylicBackdrop);

            option = ReadConfig("acrylic_type", "Default");
            if (option == "Base") {
                acrylicBackdrop.Kind(DesktopAcrylicKind::Base);
            }
            else if (option == "Thin") {
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

        LOG_INFO(L"MainWindow", L"Loaded backdrop async with options: [%s, %s]", std::wstring(background_type.begin(), background_type.end()).c_str(), std::wstring(option.begin(), option.end()).c_str());
        co_return;
    }

    winrt::fire_and_forget MainWindow::LoadBackground()
    {
        std::string background_image = ReadConfig("background_image", "");

        HANDLE hFile = CreateFileA(background_image.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

        if (hFile != INVALID_HANDLE_VALUE) {
            CloseHandle(hFile);

            try {
                StorageFile file = co_await StorageFile::GetFileFromPathAsync(to_hstring(background_image));

                if (file && file.IsAvailable() && (file.FileType() == L".png" || file.FileType() == L".jpg" || file.FileType() == L".bmp" || file.FileType() == L".jpeg")) {
                    ImageBrush brush;
                    BitmapImage bitmapImage;
                    auto& stream = co_await file.OpenReadAsync();
                    bitmapImage.SetSource(stream);
                    brush.ImageSource(bitmapImage);

                    int opacity = ReadConfig("image_opacity", 20);
                    std::string stretch = ReadConfig("image_stretch", "UniformToFill");

                    brush.Stretch(stretch == "None" ? Stretch::None : stretch == "Uniform" ? Stretch::Uniform : stretch == "Fill" ? Stretch::Fill : Stretch::UniformToFill);
                    brush.Opacity(opacity / 100.0);

                    MainWindowGrid().Background(brush);

                    LOG_INFO(L"MainWindow", L"Loaded background async with options: [%s, %d, %s]", to_hstring(background_image).c_str(), opacity, to_hstring(stretch).c_str());
                }
            }
            catch (hresult_error) {
                SolidColorBrush brush;
                brush.Color(Colors::Transparent());

                MainWindowGrid().Background(brush);
                LOG_ERROR(L"MainWindow", L"Unable to load window backgroud! Applying transparent brush instead.");
            }
        }
        else {
            SolidColorBrush brush;
            brush.Color(Colors::Transparent());

            MainWindowGrid().Background(brush);
            LOG_ERROR(L"MainWindow", L"Background file does not exist. Applying transparent brush instead.");
        }
        co_return;
    }

    winrt::fire_and_forget MainWindow::LoadNavigation()
    {
        std::string navigation_style = ReadConfig("navigation_style", "LeftCompact");

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

        LOG_INFO(L"MainWindow", L"Loaded navigation async with options: [%s]", to_hstring(navigation_style).c_str());
        co_return;
    }

    winrt::fire_and_forget MainWindow::CheckUpdate()
    {
        if (!ReadConfig("check_update", true)) co_return;

        auto weak_this = get_weak();

        int currentBuildNumber = unbox_value<int>(Application::Current().Resources().TryLookup(box_value(L"BuildNumber")));
        int latestBuildNumber = 0;

        co_await winrt::resume_background();

        HttpClient client;
        Uri uri(L"https://pastebin.com/raw/kz5qViYF");

        // 防止获取旧数据
        client.DefaultRequestHeaders().Append(L"Cache-Control", L"no-cache");
        client.DefaultRequestHeaders().Append(L"If-None-Match", L"");

        LOG_INFO(L"Updater", L"Sending update check request...");
        hstring result = co_await client.GetStringAsync(uri);

        auto json = Windows::Data::Json::JsonObject::Parse(result);
        latestBuildNumber = json.GetNamedNumber(L"build_number");

        if (auto strong_this = weak_this.get()) {
            co_await wil::resume_foreground(DispatcherQueue());

            LOG_INFO(L"Updater", L"Current: %d, Latest: %d", currentBuildNumber, latestBuildNumber);

            if (latestBuildNumber == 0) {
                LOG_WARNING(L"Updater", L"Latest = 0, check failed.");
                CreateInfoBarAndDisplay(L"警告", L"检查更新失败！", InfoBarSeverity::Warning, g_mainWindowInstance);
            }
            else if (latestBuildNumber == currentBuildNumber) {
                LOG_INFO(L"Updater", L"Latest = current, we are on the latest version.");
                CreateInfoBarAndDisplay(L"信息", L"你正在使用最新版本的 Starlight GUI！", InfoBarSeverity::Informational, g_mainWindowInstance);

                if (ReadConfig("last_announcement_date", 0) < GetDateAsInt()) {
                    auto dialog = winrt::make<winrt::StarlightGUI::implementation::UpdateDialog>();
                    dialog.IsUpdate(false);
                    dialog.LatestVersion(json.GetNamedString(L"an_update_time"));
                    dialog.SetAnLine(1, json.GetNamedString(L"an_line1"));
                    dialog.SetAnLine(2, json.GetNamedString(L"an_line2"));
                    dialog.SetAnLine(3, json.GetNamedString(L"an_line3"));
                    dialog.XamlRoot(MainWindowGrid().XamlRoot());
                    co_await dialog.ShowAsync();
                }
            }
            else if (latestBuildNumber > currentBuildNumber) {
                LOG_INFO(L"Updater", L"Latest > current, new version avaliable. Calling up update dialog.");
                CreateInfoBarAndDisplay(L"信息", L"检测到新版本的 Starlight GUI！", InfoBarSeverity::Informational, g_mainWindowInstance);
                auto dialog = winrt::make<winrt::StarlightGUI::implementation::UpdateDialog>();
                dialog.IsUpdate(true);
                dialog.LatestVersion(json.GetNamedString(L"version"));
                dialog.XamlRoot(MainWindowGrid().XamlRoot());

                auto result = co_await dialog.ShowAsync();

                if (result == ContentDialogResult::Primary) {
                    hstring channel = L"";
                    switch (dialog.Channel()) {
                    case 0:
                        channel = json.GetNamedString(L"download_link");
                        break;
                    case 1:
                        channel = json.GetNamedString(L"download_link2");
                        break;
                    default:
                        channel = json.GetNamedString(L"download_link");
                        break;
                    }
                    Uri target(channel);
                    auto result = co_await Launcher::LaunchUriAsync(target);

                    if (result) {
                        CreateInfoBarAndDisplay(L"成功", L"已在浏览器打开网页！", InfoBarSeverity::Success, g_mainWindowInstance);
                    }
                    else {
                        CreateInfoBarAndDisplay(L"失败", L"无法打开网页！", InfoBarSeverity::Error, g_mainWindowInstance);
                    }
                }
            }
            else if (latestBuildNumber < currentBuildNumber) {
                LOG_INFO(L"Updater", L"Latest < current, maybe we are on a dev environment.", kernelPath.c_str());
                CreateInfoBarAndDisplay(L"信息", L"你正在使用 Starlight GUI 的开发版本！", InfoBarSeverity::Informational, g_mainWindowInstance);
            }
        }

        co_return;
    }

    winrt::fire_and_forget MainWindow::LoadModules() {
        auto strong = get_strong();

        RootNavigation().IsEnabled(false);

        LOG_INFO(L"DriverUtils", L"Loading necessary modules...");

        if (kernelPath.empty() || astralPath.empty() || axBandPath.empty()) {
            try {
                co_await winrt::resume_background();

                auto& appFolder = Package::Current().InstalledLocation();
                auto& assetsFolder = co_await appFolder.GetFolderAsync(L"Assets");
                auto& kernelFile = co_await assetsFolder.GetFileAsync(L"kernel.sys");
                auto& astralFile = co_await assetsFolder.GetFileAsync(L"AstralX.sys");
                auto& axBandFile = co_await assetsFolder.GetFileAsync(L"AxBand.dll");

                if (kernelFile && KernelInstance::IsRunningAsAdmin()) {
                    kernelPath = kernelFile.Path();

                    LOG_INFO(L"DriverUtils", L"Kernel.sys path [%s], load it.", kernelPath.c_str());
                    DriverUtils::LoadKernelDriver(kernelPath.c_str(), unused);
                }

                if (astralFile && KernelInstance::IsRunningAsAdmin()) {
                    astralPath = astralFile.Path();

                    LOG_INFO(L"DriverUtils", L"AstralX.sys path [%s], load it.", astralPath.c_str());
                    DriverUtils::LoadDriver(astralPath.c_str(), L"AstralX", unused);
                }

                if (axBandFile) {
                    axBandPath = axBandFile.Path();

                    LOG_INFO(L"DriverUtils", L"AxBand.dll path [%s].", axBandPath.c_str());
                }

                LOG_SUCCESS(L"DriverUtils", L"Loaded successfully.", kernelPath.c_str());
                co_await wil::resume_foreground(DispatcherQueue());
                CreateInfoBarAndDisplay(L"成功", L"模块加载成功！", InfoBarSeverity::Success, g_mainWindowInstance);
            }
            catch (winrt::hresult_error e) {
                LOG_ERROR(L"DriverUtils", L"Error while loading modules!", kernelPath.c_str());
                LOG_ERROR(L"DriverUtils", L"%s", e.message().c_str());
                CreateInfoBarAndDisplay(L"警告", L"一个或多个模块文件未找到或无法加载，部分功能可能不可用！", InfoBarSeverity::Warning, g_mainWindowInstance);
            }
        }

        RootNavigation().IsEnabled(true);
    }

    HWND MainWindow::GetWindowHandle()
    {
        return globalHWND;
    }
}