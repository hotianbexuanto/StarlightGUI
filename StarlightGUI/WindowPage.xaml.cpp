#include "pch.h"
#include "WindowPage.xaml.h"
#if __has_include("WindowPage.g.cpp")
#include "WindowPage.g.cpp"
#endif


#include <winrt/Microsoft.UI.Composition.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Media.Imaging.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.System.h>
#include <winrt/Windows.UI.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.Graphics.Imaging.h>
#include <winrt/Windows.Foundation.h>
#include <shellapi.h>
#include <sstream>
#include <iomanip>
#include <mutex>
#include <unordered_set>
#include <InfoWindow.xaml.h>
#include <MainWindow.xaml.h>
#include <Psapi.h>

using namespace winrt;
using namespace WinUI3Package;
using namespace Microsoft::UI::Text;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Microsoft::UI::Xaml::Media;
using namespace Microsoft::UI::Xaml::Media::Imaging;
using namespace Windows::Storage::Streams;
using namespace Windows::Graphics::Imaging;
using namespace Windows::System;

namespace winrt::StarlightGUI::implementation
{
    static std::unordered_map<hstring, std::optional<winrt::Microsoft::UI::Xaml::Media::ImageSource>> iconCache;
    static std::chrono::steady_clock::time_point lastRefresh;
    static HDC hdc{ nullptr };
    static int safeAcceptedImage = -1;
    static bool loaded;

    WindowPage::WindowPage() {
        InitializeComponent();

        loaded = false;

        WindowListView().ItemsSource(m_windowList);
        WindowListView().ItemContainerTransitions().Clear();
        WindowListView().ItemContainerTransitions().Append(EntranceThemeTransition());
		ShowVisibleOnlyCheckBox().IsChecked(m_showVisibleOnly);

        this->Loaded([this](auto&&, auto&&) {
            hdc = GetDC(NULL);
            LoadWindowList();
            loaded = true;
            });

        this->Unloaded([this](auto&&, auto&&) {
            ReleaseDC(NULL, hdc);
            });

        LOG_INFO(L"WindowPage", L"WindowPage initialized.");
    }

    void WindowPage::WindowListView_RightTapped(IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::RightTappedRoutedEventArgs const& e)
    {
        auto listView = WindowListView();

        if (auto fe = e.OriginalSource().try_as<FrameworkElement>())
        {
            auto container = FindParent<ListViewItem>(fe);
            if (container)
            {
                listView.SelectedItem(container.Content());
            }
        }

        if (!listView.SelectedItem()) return;

        auto item = listView.SelectedItem().as<winrt::StarlightGUI::WindowInfo>();
        WINDOWINFO idk{};

        auto style = unbox_value<Microsoft::UI::Xaml::Style>(Application::Current().Resources().TryLookup(box_value(L"MenuFlyoutItemStyle")));
        auto styleSub = unbox_value<Microsoft::UI::Xaml::Style>(Application::Current().Resources().TryLookup(box_value(L"MenuFlyoutSubItemStyle")));

        MenuFlyout menuFlyout;

        MenuFlyoutItem item1_1;
        item1_1.Style(style);
        item1_1.Icon(CreateFontIcon(L"\ue711"));
        item1_1.Text(L"关闭窗口");
        item1_1.Click([this, item](IInspectable const& sender, RoutedEventArgs const& e) -> winrt::Windows::Foundation::IAsyncAction {
            if (PostMessageW((HWND)item.Hwnd(), WM_DESTROY, 0, 0)) {
                CreateInfoBarAndDisplay(L"成功", L"成功关闭窗口: " + item.Name() + L" (" + to_hstring(item.Hwnd()) + L")", InfoBarSeverity::Success, g_mainWindowInstance);
                WaitAndReloadAsync(1000);
            }
            else CreateInfoBarAndDisplay(L"失败", L"无法关闭窗口: " + item.Name() + L" (" + to_hstring(item.Hwnd()) + L"), 错误码: " + to_hstring((int)GetLastError()), InfoBarSeverity::Error, g_mainWindowInstance);
            co_return;
            });

        MenuFlyoutItem item1_2;
        item1_2.Style(style);
        item1_2.Icon(CreateFontIcon(L"\ue8f0"));
        item1_2.Text(L"关闭窗口 (结束任务)");
        item1_2.Click([this, item](IInspectable const& sender, RoutedEventArgs const& e) -> winrt::Windows::Foundation::IAsyncAction {
            if (TaskUtils::EndTaskByWindow((HWND)item.Hwnd())) {
                CreateInfoBarAndDisplay(L"成功", L"成功关闭窗口: " + item.Name() + L" (" + to_hstring(item.Hwnd()) + L")", InfoBarSeverity::Success, g_mainWindowInstance);
                WaitAndReloadAsync(1000);
            }
            else CreateInfoBarAndDisplay(L"失败", L"无法关闭窗口: " + item.Name() + L" (" + to_hstring(item.Hwnd()) + L"), 错误码: " + to_hstring((int)GetLastError()), InfoBarSeverity::Error, g_mainWindowInstance);
            co_return;
            });

        MenuFlyoutItem item1_3;
        item1_3.Style(style);
        item1_3.Icon(CreateFontIcon(L"\ue945"));
        item1_3.Text(L"关闭窗口 (内核)");
        item1_3.Click([this, item](IInspectable const& sender, RoutedEventArgs const& e) -> winrt::Windows::Foundation::IAsyncAction {
            DWORD pid;
			GetWindowThreadProcessId((HWND)item.Hwnd(), &pid);
            if (KernelInstance::_ZwTerminateProcess(pid)) {
                CreateInfoBarAndDisplay(L"成功", L"成功关闭窗口: " + item.Name() + L" (" + to_hstring(item.Hwnd()) + L")", InfoBarSeverity::Success, g_mainWindowInstance);
                WaitAndReloadAsync(1000);
            }
            else CreateInfoBarAndDisplay(L"失败", L"无法关闭窗口: " + item.Name() + L" (" + to_hstring(item.Hwnd()) + L"), 错误码: " + to_hstring((int)GetLastError()), InfoBarSeverity::Error, g_mainWindowInstance);
            co_return;
            });

        // 分割线1
        MenuFlyoutSeparator separator1;

        // 选项2.1
        MenuFlyoutSubItem item2_1;
        item2_1.Style(styleSub);
        item2_1.Icon(CreateFontIcon(L"\ue912"));
        item2_1.Text(L"设置窗口状态");
        MenuFlyoutItem item2_1_sub1;
        item2_1_sub1.Style(style);
        item2_1_sub1.Icon(CreateFontIcon(L"\ueb1d"));
        item2_1_sub1.Text(L"显示");
        item2_1_sub1.Click([this, item](IInspectable const& sender, RoutedEventArgs const& e) -> winrt::Windows::Foundation::IAsyncAction {
            if (ShowWindow((HWND)item.Hwnd(), SW_SHOW) || GetLastError() == 0) {
                CreateInfoBarAndDisplay(L"成功", L"成功显示窗口: " + item.Name() + L" (" + to_hstring(item.Hwnd()) + L")", InfoBarSeverity::Success, g_mainWindowInstance);
                WaitAndReloadAsync(1000);
            }
            else CreateInfoBarAndDisplay(L"失败", L"无法显示窗口: " + item.Name() + L" (" + to_hstring(item.Hwnd()) + L"), 错误码: " + to_hstring((int)GetLastError()), InfoBarSeverity::Error, g_mainWindowInstance);
            co_return;
            });
        item2_1.Items().Append(item2_1_sub1);
        MenuFlyoutItem item2_1_sub2;
        item2_1_sub2.Style(style);
        item2_1_sub2.Icon(CreateFontIcon(L"\ueb19"));
        item2_1_sub2.Text(L"隐藏");
        item2_1_sub2.Click([this, item](IInspectable const& sender, RoutedEventArgs const& e) -> winrt::Windows::Foundation::IAsyncAction {
            if (ShowWindow((HWND)item.Hwnd(), SW_HIDE) || GetLastError() == 0) {
                CreateInfoBarAndDisplay(L"成功", L"成功隐藏窗口: " + item.Name() + L" (" + to_hstring(item.Hwnd()) + L")", InfoBarSeverity::Success, g_mainWindowInstance);
                WaitAndReloadAsync(1000);
            }
            else CreateInfoBarAndDisplay(L"失败", L"无法隐藏窗口: " + item.Name() + L" (" + to_hstring(item.Hwnd()) + L"), 错误码: " + to_hstring((int)GetLastError()), InfoBarSeverity::Error, g_mainWindowInstance);
            co_return;
            });
        item2_1.Items().Append(item2_1_sub2);
        MenuFlyoutItem item2_1_sub3;
        item2_1_sub3.Style(style);
        item2_1_sub3.Icon(CreateFontIcon(L"\ue740"));
        item2_1_sub3.Text(L"最大化");
        item2_1_sub3.Click([this, item](IInspectable const& sender, RoutedEventArgs const& e) -> winrt::Windows::Foundation::IAsyncAction {
            if (PostMessageW((HWND)item.Hwnd(), WM_SYSCOMMAND, SC_MAXIMIZE, 0) == ERROR_SUCCESS) {
                CreateInfoBarAndDisplay(L"成功", L"成功最大化窗口: " + item.Name() + L" (" + to_hstring(item.Hwnd()) + L")", InfoBarSeverity::Success, g_mainWindowInstance);
                WaitAndReloadAsync(1000);
            }
            else CreateInfoBarAndDisplay(L"失败", L"无法最大化窗口: " + item.Name() + L" (" + to_hstring(item.Hwnd()) + L"), 错误码: " + to_hstring((int)GetLastError()), InfoBarSeverity::Error, g_mainWindowInstance);
            co_return;
            });
        item2_1.Items().Append(item2_1_sub3);
        MenuFlyoutItem item2_1_sub4;
        item2_1_sub4.Style(style);
        item2_1_sub4.Icon(CreateFontIcon(L"\ue73f"));
        item2_1_sub4.Text(L"最小化");
        item2_1_sub4.Click([this, item](IInspectable const& sender, RoutedEventArgs const& e) -> winrt::Windows::Foundation::IAsyncAction {
            if (PostMessageW((HWND)item.Hwnd(), WM_SYSCOMMAND, SC_MINIMIZE, 0) == ERROR_SUCCESS) {
                CreateInfoBarAndDisplay(L"成功", L"成功最小化窗口: " + item.Name() + L" (" + to_hstring(item.Hwnd()) + L")", InfoBarSeverity::Success, g_mainWindowInstance);
                WaitAndReloadAsync(1000);
            }
            else CreateInfoBarAndDisplay(L"失败", L"无法最小化窗口: " + item.Name() + L" (" + to_hstring(item.Hwnd()) + L"), 错误码: " + to_hstring((int)GetLastError()), InfoBarSeverity::Error, g_mainWindowInstance);
            co_return;
            });
        item2_1.Items().Append(item2_1_sub4);

        // 选项2.2
        MenuFlyoutItem item2_2;
        item2_2.Style(style);
        item2_2.Icon(CreateFontIcon(L"\ue754"));
        item2_2.Text(L"在任务栏闪烁");
        item2_2.Click([this, item](IInspectable const& sender, RoutedEventArgs const& e) -> winrt::Windows::Foundation::IAsyncAction {
            if (FlashWindow((HWND)item.Hwnd(), FALSE) || GetLastError() == 0) {
                CreateInfoBarAndDisplay(L"成功", L"成功在任务栏闪烁窗口: " + item.Name() + L" (" + to_hstring(item.Hwnd()) + L")", InfoBarSeverity::Success, g_mainWindowInstance);
                WaitAndReloadAsync(1000);
            }
            else CreateInfoBarAndDisplay(L"失败", L"无法在任务栏闪烁窗口: " + item.Name() + L" (" + to_hstring(item.Hwnd()) + L"), 错误码: " + to_hstring((int)GetLastError()), InfoBarSeverity::Error, g_mainWindowInstance);
            co_return;
            });

        // 选项2.3
        MenuFlyoutItem item2_3;
        item2_3.Style(style);
        item2_3.Icon(CreateFontIcon(L"\ue75c"));
        item2_3.Text(L"重绘窗口");
        item2_3.Click([this, item](IInspectable const& sender, RoutedEventArgs const& e) -> winrt::Windows::Foundation::IAsyncAction {
            if (UpdateWindow((HWND)item.Hwnd()) || GetLastError() == 0) {
                CreateInfoBarAndDisplay(L"成功", L"成功重绘窗口: " + item.Name() + L" (" + to_hstring(item.Hwnd()) + L")", InfoBarSeverity::Success, g_mainWindowInstance);
                WaitAndReloadAsync(1000);
            }
            else CreateInfoBarAndDisplay(L"失败", L"无法重绘窗口: " + item.Name() + L" (" + to_hstring(item.Hwnd()) + L"), 错误码: " + to_hstring((int)GetLastError()), InfoBarSeverity::Error, g_mainWindowInstance);
            co_return;
            });

		menuFlyout.Items().Append(item1_1);
        menuFlyout.Items().Append(item1_2);
        menuFlyout.Items().Append(item1_3);
		menuFlyout.Items().Append(separator1);
        menuFlyout.Items().Append(item2_1);
        menuFlyout.Items().Append(item2_2);
        menuFlyout.Items().Append(item2_3);

        menuFlyout.ShowAt(listView, e.GetPosition(listView));
    }

    void WindowPage::WindowListView_ContainerContentChanging(
        winrt::Microsoft::UI::Xaml::Controls::ListViewBase const& sender,
        winrt::Microsoft::UI::Xaml::Controls::ContainerContentChangingEventArgs const& args)
    {
        if (args.InRecycleQueue())
            return;

        // 将 Tag 设到容器上，便于 ListViewItemPresenter 通过 TemplatedParent 绑定
        if (auto itemContainer = args.ItemContainer())
            itemContainer.Tag(sender.Tag());
    }

    winrt::Windows::Foundation::IAsyncAction WindowPage::LoadWindowList()
    {
        if (m_isLoadingWindows) {
            co_return;
        }
        m_isLoadingWindows = true;

        LOG_INFO(__WFUNCTION__, L"Loading window list...");
        m_windowList.Clear();
        LoadingRing().IsActive(true);

        auto start = std::chrono::steady_clock::now();

        auto lifetime = get_strong();

        winrt::hstring query = SearchBox().Text();

        co_await winrt::resume_background();

        std::vector<winrt::StarlightGUI::WindowInfo> windows;
        windows.reserve(500);

        co_await GetWindowInfoAsync(windows);
        LOG_INFO(__WFUNCTION__, L"Enumerated windows, %d entry(s).", windows.size());

        lastRefresh = std::chrono::steady_clock::now();

        co_await wil::resume_foreground(DispatcherQueue());

        for (const auto& window : windows) {
            bool shouldRemove = query.empty() ? false : ApplyFilter(window, query);
            if (shouldRemove) continue;

            if (!(HWND)window.Hwnd()) continue;

            co_await GetWindowIconAsync(window);

            if (window.Name().empty()) window.Name(L"(未知)");
            if (window.Process().empty()) window.Process(L"(未知)");
            if (window.ClassName().empty()) window.ClassName(L"(未知)");

            m_windowList.Append(window);
        }

        // 恢复排序
        ApplySort(currentSortingOption, currentSortingType);

        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        // 更新窗口数量文本
        std::wstringstream countText;
        countText << L"共 " << m_windowList.Size() << L" 个窗口 (" << duration << " ms)";
        WindowCountText().Text(countText.str());

        LoadingRing().IsActive(false);

        LOG_INFO(__WFUNCTION__, L"Loaded window list, %d entry(s) in total.", m_windowList.Size());
        m_isLoadingWindows = false;
    }

    winrt::Windows::Foundation::IAsyncAction WindowPage::GetWindowInfoAsync(std::vector<winrt::StarlightGUI::WindowInfo>& windows)
    {
        EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
            std::vector<winrt::StarlightGUI::WindowInfo>& windowsRef = *reinterpret_cast<std::vector<winrt::StarlightGUI::WindowInfo>*>(lParam);
            if (!IsWindow(hwnd)) return TRUE;

            if (m_showVisibleOnly && !IsWindowVisible(hwnd)) return TRUE;

            int length = GetWindowTextLengthW(hwnd);
            if (length == 0) return TRUE;

            std::wstring windowTitle(length + 1, '\0');
            GetWindowTextW(hwnd, &windowTitle[0], length + 1);

            DWORD processId;
            GetWindowThreadProcessId(hwnd, &processId);
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ, FALSE, processId);
            std::wstring processName;
            if (hProcess) {
                wchar_t processNameTemp[MAX_PATH];
                if (GetModuleFileNameExW(hProcess, nullptr, processNameTemp, MAX_PATH)) {
                    processName = processNameTemp;
                }
                CloseHandle(hProcess);
            }

            std::wstring className;
            wchar_t classNameTmp[MAX_PATH];
            GetClassNameW(hwnd, &classNameTmp[0], MAX_PATH);
            className = classNameTmp;

            DWORD windowStyle = 0;
            DWORD windowStyleEx = 0;

            WINDOWINFO pwndInfo = { 0 };
            pwndInfo.cbSize = sizeof(WINDOWINFO);
            if (GetWindowInfo(hwnd, &pwndInfo)) {
                windowStyle = pwndInfo.dwStyle;
                windowStyleEx = pwndInfo.dwExStyle;
            }

            winrt::StarlightGUI::WindowInfo windowInfo = winrt::make<winrt::StarlightGUI::implementation::WindowInfo>();
            windowInfo.Name(windowTitle);
            windowInfo.Process(processName);
            windowInfo.ClassName(className);
            windowInfo.FromPID(processId);
            windowInfo.WindowStyle(windowStyle);
            windowInfo.WindowStyleEx(windowStyleEx);
            windowInfo.Hwnd((uint64_t)hwnd);
            windowInfo.Description(ExtractFileName(processName) + L" / " + className);
            windowsRef.push_back(windowInfo);

            return TRUE;
            }, reinterpret_cast<LPARAM>(&windows));

        co_return;
    }

    winrt::Windows::Foundation::IAsyncAction WindowPage::GetWindowIconAsync(const winrt::StarlightGUI::WindowInfo& window) {
        if (iconCache.find(window.Name()) == iconCache.end()) {
			// 获取窗口图标 ICON
            HICON hIcon = (HICON)GetClassLongPtrW((HWND)window.Hwnd(), GCLP_HICON);
            if (!hIcon)
                hIcon = (HICON)GetClassLongPtrW((HWND)window.Hwnd(), GCLP_HICONSM);
            if (!hIcon)
				hIcon = (HICON)LoadImageW(NULL, MAKEINTRESOURCEW(32512), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
            if (!hIcon) co_return;
            ICONINFO iconInfo;
            if (GetIconInfo(hIcon, &iconInfo)) {
                BITMAP bmp;
                GetObject(iconInfo.hbmColor, sizeof(bmp), &bmp);
                BITMAPINFOHEADER bmiHeader = { 0 };
                bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
                bmiHeader.biWidth = bmp.bmWidth;
                bmiHeader.biHeight = bmp.bmHeight;
                bmiHeader.biPlanes = 1;
                bmiHeader.biBitCount = 32;
                bmiHeader.biCompression = BI_RGB;

                int dataSize = bmp.bmWidthBytes * bmp.bmHeight;
                std::vector<BYTE> buffer(dataSize);

                GetDIBits(hdc, iconInfo.hbmColor, 0, bmp.bmHeight, buffer.data(), reinterpret_cast<BITMAPINFO*>(&bmiHeader), DIB_RGB_COLORS);

                winrt::Microsoft::UI::Xaml::Media::Imaging::WriteableBitmap writeableBitmap(bmp.bmWidth, bmp.bmHeight);

                // 将数据写入 WriteableBitmap
                uint8_t* data = writeableBitmap.PixelBuffer().data();
                int rowSize = bmp.bmWidth * 4;
                for (int i = 0; i < bmp.bmHeight; ++i) {
                    int srcOffset = i * rowSize;
                    int dstOffset = (bmp.bmHeight - 1 - i) * rowSize;
                    std::memcpy(data + dstOffset, buffer.data() + srcOffset, rowSize);
                }

                DeleteObject(iconInfo.hbmColor);
                DeleteObject(iconInfo.hbmMask);
                DestroyIcon(hIcon);

                // 将图标缓存到 map 中
                iconCache[window.Name()] = writeableBitmap.as<winrt::Microsoft::UI::Xaml::Media::ImageSource>();
                window.Icon(writeableBitmap);
            }
        }
        else {
            if (iconCache[window.Name()].has_value()) {
                window.Icon(iconCache[window.Name()].value());
            }
            else {
                LOG_WARNING(__WFUNCTION__, L"File icon path (%s) does not have a value!", window.Name().c_str());
            }
        }

        co_return;
    }

    // 暂时实现不了，不管他
    bool WindowPage::SetWindowZBID(HWND hwnd, ZBID zbid) {
        HMODULE hDll = LoadLibraryW(axBandPath.c_str());
        if (hDll)
        {
			LOG_INFO(L"AxBand", L"Loaded AxBand.dll successfully.");
            typedef BOOL(*PFN_SetWindowBandEx)(HWND, HWND, DWORD);

            PFN_SetWindowBandEx pSetWindowBandEx =
                (PFN_SetWindowBandEx)GetProcAddress(hDll, "SetWindowBandEx");

            if (pSetWindowBandEx)
            {
                LOG_INFO(L"AxBand", L"Loaded SetWindowBandEx function successfully.");

                BOOL result = pSetWindowBandEx(hwnd, NULL, zbid);

                LOG_INFO(L"AxBand", L"Set window band to %d successfully.", zbid);
                FreeLibrary(hDll);

                return result;
            }

            FreeLibrary(hDll);
        }

        return false;
    }

    void WindowPage::ColumnHeader_Click(IInspectable const& sender, RoutedEventArgs const& e)
    {
        Button clickedButton = sender.as<Button>();
        winrt::hstring columnName = clickedButton.Tag().as<winrt::hstring>();

        if (columnName == L"Name")
        {
            ApplySort(m_isNameAscending, "Name");
        }
        else if (columnName == L"Hwnd")
        {
            ApplySort(m_isHwndAscending, "Hwnd");
        }
    }

    // 排序切换
    winrt::fire_and_forget WindowPage::ApplySort(bool& isAscending, const std::string& column)
    {
        NameHeaderButton().Content(box_value(L"窗口"));

        std::vector<winrt::StarlightGUI::WindowInfo> sortedWindows;

        for (auto& window : m_windowList) {
            sortedWindows.push_back(window);
        }

        if (column == "Name") {
            if (isAscending) {
                NameHeaderButton().Content(box_value(L"窗口 ↓"));
                std::sort(sortedWindows.begin(), sortedWindows.end(), [](auto a, auto b) {
                    std::wstring aName = a.Name().c_str();
                    std::wstring bName = b.Name().c_str();
                    std::transform(aName.begin(), aName.end(), aName.begin(), ::towlower);
                    std::transform(bName.begin(), bName.end(), bName.begin(), ::towlower);

                    return aName < bName;
                    });

            }
            else {
                NameHeaderButton().Content(box_value(L"窗口 ↑"));
                std::sort(sortedWindows.begin(), sortedWindows.end(), [](auto a, auto b) {
                    std::wstring aName = a.Name().c_str();
                    std::wstring bName = b.Name().c_str();
                    std::transform(aName.begin(), aName.end(), aName.begin(), ::towlower);
                    std::transform(bName.begin(), bName.end(), bName.begin(), ::towlower);

                    return aName > bName;
                    });
            }
        }
        else if (column == "Hwnd") {
            if (isAscending) {
                HwndHeaderButton().Content(box_value(L"HWND ↓"));
                std::sort(sortedWindows.begin(), sortedWindows.end(), [](auto a, auto b) {
                    return a.Hwnd() < b.Hwnd();
                    });

            }
            else {
                HwndHeaderButton().Content(box_value(L"HWND ↑"));
                std::sort(sortedWindows.begin(), sortedWindows.end(), [](auto a, auto b) {
                    return a.Hwnd() > b.Hwnd();
                    });
            }
        }

        m_windowList.Clear();
        for (auto& window : sortedWindows) {
            m_windowList.Append(window);
        }

        isAscending = !isAscending;
        currentSortingOption = !isAscending;
        currentSortingType = column;

        co_return;
    }

    void WindowPage::ShowVisibleOnlyCheckBox_Checked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
    {
        if (!loaded) return;
		m_showVisibleOnly = ShowVisibleOnlyCheckBox().IsChecked().GetBoolean();
        if (m_showVisibleOnly) {
            LOG_INFO(__WFUNCTION__, L"Show visible only is checked.");
        }
        else {
            LOG_INFO(__WFUNCTION__, L"Show visible only is unchecked.");
		}
        LoadWindowList();
    }

    void WindowPage::SearchBox_TextChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
    {
        if (!loaded) return;

        WaitAndReloadAsync(200);
    }

    bool WindowPage::ApplyFilter(const winrt::StarlightGUI::WindowInfo& window, hstring& query) {
        std::wstring name = window.Name().c_str();
        std::wstring queryWStr = query.c_str();

        // 不比较大小写
        std::transform(name.begin(), name.end(), name.begin(), ::towlower);
        std::transform(queryWStr.begin(), queryWStr.end(), queryWStr.begin(), ::towlower);

        bool result = name.find(queryWStr) == std::wstring::npos;

        return result;
    }


    winrt::fire_and_forget WindowPage::RefreshButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        RefreshButton().IsEnabled(false);
        co_await LoadWindowList();
        RefreshButton().IsEnabled(true);
        co_return;
    }

    winrt::Windows::Foundation::IAsyncAction WindowPage::WaitAndReloadAsync(int interval) {
        auto lifetime = get_strong();

        reloadTimer.Stop();
        reloadTimer.Interval(std::chrono::milliseconds(interval));
        reloadTimer.Tick([this](auto&&, auto&&) {
            LoadWindowList();
            reloadTimer.Stop();
            });
        reloadTimer.Start();

        co_return;
    }

    template <typename T>
    T WindowPage::FindParent(DependencyObject const& child)
    {
        DependencyObject parent = VisualTreeHelper::GetParent(child);
        while (parent && !parent.try_as<T>())
        {
            parent = VisualTreeHelper::GetParent(parent);
        }
        return parent.try_as<T>();
    }
}
