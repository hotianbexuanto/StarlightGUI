#include "pch.h"
#include "Utils.h"
#include "Config.h"
#include "MainWindow.xaml.h"
#include "InfoWindow.xaml.h"
#include <winrt/Microsoft.UI.Composition.SystemBackdrops.h>
#include <winrt/Microsoft.UI.Input.h>
#include <unordered_map>
#include <shellapi.h>
#include <cstring>
#include <cctype>
#include <algorithm>

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Input;
using namespace Microsoft::UI::Xaml::Media;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Microsoft::UI::Composition::SystemBackdrops;
using namespace winrt::StarlightGUI::implementation;

namespace slg {
    std::unordered_map<std::wstring, ImageSource>& GetShellIconCacheStore()
    {
        static std::unordered_map<std::wstring, ImageSource> cache;
        return cache;
    }

    coroutine::coroutine() = default;

    coroutine coroutine::promise_type::get_return_object() const noexcept { return {}; }

    void coroutine::promise_type::return_void() const noexcept {}

    std::suspend_never coroutine::promise_type::initial_suspend() const noexcept { return {}; }

    std::suspend_never coroutine::promise_type::final_suspend() const noexcept { return {}; }

    void coroutine::promise_type::unhandled_exception() const noexcept
    {
        try {
            std::rethrow_exception(std::current_exception());
        }
        catch (const hresult_error& e) {
            LOG_ERROR(L"App", L"===== Unhandled exception detected! =====");
            LOG_ERROR(L"App", L"Type: 'hresult_error'");
            LOG_ERROR(L"App", L"Code: %d", e.code().value);
            LOG_ERROR(L"App", L"Message: %s", e.message().c_str());
            LOG_ERROR(L"App", L"=========================================");
        }
        catch (const std::exception& e) {
            LOG_ERROR(L"App", L"===== Unhandled exception detected! =====");
            LOG_ERROR(L"App", L"Type: 'std::exception'");
            LOG_ERROR(L"App", L"Message: %hs", e.what());
            LOG_ERROR(L"App", L"=========================================");
        }
        catch (...) {
            LOG_ERROR(L"App", L"===== Unhandled exception detected! =====");
            LOG_ERROR(L"App", L"Type: OTHER/UNKNOWN");
            LOG_ERROR(L"App", L"This should not happen!");
            LOG_ERROR(L"App", L"=========================================");
        }
    }
    Styles GetStyles()
    {
        auto resources = Application::Current().Resources();
        return {
            unbox_value<Style>(resources.TryLookup(box_value(L"MenuFlyoutItemStyle"))),
            unbox_value<Style>(resources.TryLookup(box_value(L"MenuFlyoutSubItemStyle")))
        };
    }

    MenuFlyoutItem CreateMenuItem(
        Styles const& styles,
        hstring const& glyph,
        hstring const& text,
        RoutedEventHandler const& click)
    {
        MenuFlyoutItem item;
        item.Style(styles.Item);
        item.Icon(CreateFontIcon(glyph));
        item.Text(text);
        if (click) item.Click(click);
        return item;
    }

    MenuFlyoutItem CreateMenuItem(
        Styles const& styles,
        hstring const& text,
        RoutedEventHandler const& click)
    {
        MenuFlyoutItem item;
        item.Style(styles.Item);
        item.Text(text);
        if (click) item.Click(click);
        return item;
    }

    MenuFlyoutSubItem CreateMenuSubItem(
        Styles const& styles,
        hstring const& glyph,
        hstring const& text)
    {
        MenuFlyoutSubItem item;
        item.Style(styles.SubItem);
        item.Icon(CreateFontIcon(glyph));
        item.Text(text);
        return item;
    }

    MenuFlyoutSubItem CreateMenuSubItem(
        Styles const& styles,
        hstring const& text)
    {
        MenuFlyoutSubItem item;
        item.Style(styles.SubItem);
        item.Text(text);
        return item;
    }

    void ShowAt(
        MenuFlyout const& flyout,
        ListView const& listView,
        RightTappedRoutedEventArgs const& e)
    {
        flyout.ShowAt(listView, e.GetPosition(listView));
    }
    FontIcon CreateFontIcon(hstring glyph) {
        FontIcon fontIcon;
        fontIcon.Glyph(glyph);
        fontIcon.FontFamily(FontFamily(L"Segoe Fluent Icons"));
        fontIcon.FontSize(16);

        return fontIcon;
    }

    InfoBar CreateInfoBar(hstring title, hstring message, InfoBarSeverity severity, XamlRoot xamlRoot) {
        InfoBar infobar;

        infobar.Title(title);
        infobar.Message(message);
        infobar.Severity(severity);
        infobar.XamlRoot(xamlRoot);
        infobar.RequestedTheme(GetConfiguredElementTheme());
        infobar.HorizontalAlignment(HorizontalAlignment::Right);
        infobar.VerticalAlignment(VerticalAlignment::Top);

        if (severity == InfoBarSeverity::Informational) {
            infobar.Background(SolidColorBrush{ slg::GetConfiguredElementTheme() == ElementTheme::Dark ? Color{ 255,40,40,40 } : Color{ 255,240,240,240 } });
        }

        return infobar;
    }

    void DisplayInfoBar(InfoBar infobar, Panel parent, int time) {
        if (!infobar || !parent) return;

        // Entrance animation
        EdgeUIThemeTransition transition;
        TransitionCollection transitions;
        transitions.Append(transition);
        infobar.Transitions(transitions);

        // Add and display
        parent.Children().Append(infobar);
        infobar.IsOpen(true);

        // Auto close timer
        auto timer = DispatcherTimer();
        timer.Interval(std::chrono::milliseconds(time));
        timer.Tick([infobar, parent, timer](auto&&, auto&&) {
            // Run fade out animation first
            Storyboard storyboard;
            auto fadeOutAnimation = FadeOutThemeAnimation();
            Storyboard::SetTarget(fadeOutAnimation, infobar);
            storyboard.Children().Append(fadeOutAnimation.as<Timeline>());
            storyboard.Begin();

            // Then close and remove from parent
            auto timer2 = DispatcherTimer();
            timer2.Interval(std::chrono::milliseconds(300));
            timer2.Tick([infobar, parent, timer2](auto&&, auto&&) {
                infobar.IsOpen(false);
                uint32_t index;
                if (parent.Children().IndexOf(infobar, index)) {
                    parent.Children().RemoveAt(index);
                }
                timer2.Stop();
                });
            timer2.Start();

            timer.Stop();
            });
        timer.Start();
    }

    void CreateInfoBarAndDisplay(hstring title, hstring message, InfoBarSeverity severity, XamlRoot xamlRoot, Panel parent, int time) {
        DisplayInfoBar(CreateInfoBar(title, message, severity, xamlRoot), parent, time);
    }

    void CreateInfoBarAndDisplay(hstring title, hstring message, InfoBarSeverity severity, StarlightGUI::implementation::MainWindow* instance, int time) {
        DisplayInfoBar(CreateInfoBar(title, message, severity, instance->MainWindowGrid().XamlRoot()), instance->InfoBarPanel(), time);
    }

    void CreateInfoBarAndDisplay(hstring title, hstring message, InfoBarSeverity severity, StarlightGUI::implementation::InfoWindow* instance, int time) {
        if (!instance) return;
        DisplayInfoBar(CreateInfoBar(title, message, severity, instance->InfoWindowGrid().XamlRoot()), instance->InfoBarPanel(), time);
    }

    StarlightGUI::implementation::InfoWindow* GetInfoWindowForXamlRoot(XamlRoot const& xamlRoot)
    {
        if (!xamlRoot || !g_mainWindowInstance) return nullptr;

        for (auto const& window : g_mainWindowInstance->m_openWindows) {
            if (!window) continue;
            auto instance = winrt::get_self<StarlightGUI::implementation::InfoWindow>(window);
            if (instance->InfoWindowGrid().XamlRoot() == xamlRoot) return instance;
        }

        return nullptr;
    }

    ContentDialog CreateContentDialog(hstring title, hstring content, hstring closeMessage, XamlRoot xamlRoot) {
        ContentDialog dialog;

        dialog.Title(box_value(title));
        dialog.Content(box_value(content));
        dialog.CloseButtonText(closeMessage);
        dialog.XamlRoot(xamlRoot);
        dialog.RequestedTheme(GetConfiguredElementTheme());

        return dialog;
    }

    IAsyncOperation<bool> ShowConfirmDialog(hstring title, hstring content, hstring primaryMessage, hstring closeMessage, XamlRoot xamlRoot) {
        ContentDialog dialog;
        auto style = Application::Current().Resources().TryLookup(box_value(L"DefaultContentDialogStyle"));
        if (style) dialog.Style(style.as<Style>());

        dialog.Title(box_value(title));
        dialog.TitleTemplate(XamlReader::Load(LR"(
        <DataTemplate xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation">
            <StackPanel Orientation="Horizontal" Spacing="8">
                <FontIcon
                    Margin="0,5,0,0"
                    FontFamily="Segoe Fluent Icons"
                    FontSize="30"
                    Glyph="&#xe7ba;" />
                <TextBlock VerticalAlignment="Center" Text="{Binding}" />
            </StackPanel>
        </DataTemplate>
        )").as<DataTemplate>());
        dialog.Content(box_value(content));
        dialog.PrimaryButtonText(primaryMessage);
        dialog.CloseButtonText(closeMessage);
        dialog.DefaultButton(ContentDialogButton::Primary);
        dialog.XamlRoot(xamlRoot);
        dialog.RequestedTheme(GetConfiguredElementTheme());

        auto result = co_await dialog.ShowAsync();
        co_return result == ContentDialogResult::Primary;
    }

    DataTemplate GetContentDialogSuccessTemplate() {
        return XamlReader::Load(LR"(
        <DataTemplate xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation">
            <StackPanel Orientation="Horizontal" Spacing="8">
                <FontIcon Glyph="&#xec61;" FontSize="30" FontFamily="Segoe Fluent Icons" Foreground="Green" Margin="0,5,0,0"/>
                <TextBlock Text="{Binding}" VerticalAlignment="Center"/>
            </StackPanel>
        </DataTemplate>
    )").as<DataTemplate>();
    }

    DataTemplate GetContentDialogErrorTemplate() {
        return XamlReader::Load(LR"(
        <DataTemplate xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation">
            <StackPanel Orientation="Horizontal" Spacing="8">
                <FontIcon Glyph="&#xeb90;" FontSize="30" FontFamily="Segoe Fluent Icons" Foreground="OrangeRed" Margin="0,5,0,0"/>
                <TextBlock Text="{Binding}" VerticalAlignment="Center"/>
            </StackPanel>
        </DataTemplate>
    )").as<DataTemplate>();
    }

    DataTemplate GetContentDialogInfoTemplate() {
        return XamlReader::Load(LR"(
        <DataTemplate xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation">
            <StackPanel Orientation="Horizontal" Spacing="8">
                <FontIcon Glyph="&#xf167;" FontSize="30" FontFamily="Segoe Fluent Icons" Foreground="LightBlue" Margin="0,5,0,0"/>
                <TextBlock Text="{Binding}" VerticalAlignment="Center"/>
            </StackPanel>
        </DataTemplate>
    )").as<DataTemplate>();
    }

    DataTemplate GetTemplate(hstring xaml) {
        return XamlReader::Load(xaml).as<DataTemplate>();
    }

    bool CheckIllegalComboBoxAction(IInspectable const& sender, SelectionChangedEventArgs const& e) {
        auto cb = sender.as<ComboBox>();

        if (!cb) return true;

        int index = cb.SelectedIndex();
        int itemCount = cb.Items().Size();

        // 非法索引，返回true并重置索引
        if (index < 0 || index >= itemCount) {
            cb.SelectedIndex(0);
            return true; 
        }

        // 正常索引，返回false
        return false;
    }

    static ElementTheme GetSystemElementTheme()
    {
        DWORD lightTheme = 1;
        DWORD size = sizeof(lightTheme);
        auto result = RegGetValueW(
            HKEY_CURRENT_USER,
            L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
            L"AppsUseLightTheme",
            RRF_RT_REG_DWORD,
            nullptr,
            &lightTheme,
            &size);

        if (result == ERROR_SUCCESS) {
            return lightTheme == 0 ? ElementTheme::Dark : ElementTheme::Light;
        }

        return ElementTheme::Dark;
    }

    ElementTheme GetConfiguredElementTheme()
    {
        std::string themeValue = theme;
        std::transform(themeValue.begin(), themeValue.end(), themeValue.begin(), [](unsigned char c) { return (char)std::tolower(c); });

        if (themeValue == "light") {
            return ElementTheme::Light;
        }

        if (themeValue == "dark") {
            return ElementTheme::Dark;
        }

        return GetSystemElementTheme();
    }

    void ApplyConfiguredTheme()
    {
        auto targetTheme = GetConfiguredElementTheme();
		BOOL isDark = (targetTheme == ElementTheme::Dark) ? TRUE : FALSE;

        if (g_mainWindowInstance) {
            DwmSetWindowAttribute(g_mainWindowInstance->GetWindowHandle(), DWMWA_USE_IMMERSIVE_DARK_MODE, &isDark, sizeof(isDark));
            g_mainWindowInstance->MainWindowGrid().RequestedTheme(targetTheme);
            g_mainWindowInstance->RootNavigation().RequestedTheme(targetTheme);
            g_mainWindowInstance->AppTitleBar().RequestedTheme(targetTheme);
            g_mainWindowInstance->CaptionButtonThemeWorkaround().RequestedTheme(targetTheme);
            g_mainWindowInstance->LoadBackdrop();
        }

        if (g_mainWindowInstance) {
            for (auto const& window : g_mainWindowInstance->m_openWindows) {
                if (!window) continue;
                auto instance = winrt::get_self<StarlightGUI::implementation::InfoWindow>(window);
                DwmSetWindowAttribute(instance->GetWindowHandle(), DWMWA_USE_IMMERSIVE_DARK_MODE, &isDark, sizeof(isDark));
                instance->InfoWindowGrid().RequestedTheme(targetTheme);
                instance->RootNavigation().RequestedTheme(targetTheme);
                instance->AppTitleBar().RequestedTheme(targetTheme);
                instance->CaptionButtonThemeWorkaround().RequestedTheme(targetTheme);
                instance->LoadBackdrop();
            }
        }
    }

    ImageSource CreateImageSourceFromHIcon(HICON iconHandle, int iconSize, bool destroyIcon)
    {
        if (!iconHandle || iconSize <= 0) return nullptr;

        HDC screenDc = GetDC(nullptr);
        if (!screenDc) {
            if (destroyIcon) DestroyIcon(iconHandle);
            return nullptr;
        }

        BITMAPINFO bmi{};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = iconSize;
        bmi.bmiHeader.biHeight = -iconSize;
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;

        void* bits = nullptr;
        HBITMAP bitmapHandle = CreateDIBSection(screenDc, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);
        if (!bitmapHandle || !bits) {
            ReleaseDC(nullptr, screenDc);
            if (destroyIcon) DestroyIcon(iconHandle);
            return nullptr;
        }

        HDC memDc = CreateCompatibleDC(screenDc);
        if (!memDc) {
            DeleteObject(bitmapHandle);
            ReleaseDC(nullptr, screenDc);
            if (destroyIcon) DestroyIcon(iconHandle);
            return nullptr;
        }

        auto oldBitmap = SelectObject(memDc, bitmapHandle);
        std::memset(bits, 0, iconSize * iconSize * 4);
        DrawIconEx(memDc, 0, 0, iconHandle, iconSize, iconSize, 0, nullptr, DI_NORMAL);

        Imaging::WriteableBitmap bitmap(iconSize, iconSize);
        std::memcpy(bitmap.PixelBuffer().data(), bits, iconSize * iconSize * 4);

        SelectObject(memDc, oldBitmap);
        DeleteDC(memDc);
        DeleteObject(bitmapHandle);
        ReleaseDC(nullptr, screenDc);

        if (destroyIcon) DestroyIcon(iconHandle);
        return bitmap.as<ImageSource>();
    }

    ImageSource GetShellIconImage(
        std::wstring const& path,
        bool isDirectory,
        int iconSize,
        bool useFileAttributes,
        std::wstring const& cacheKey)
    {
        auto& cache = GetShellIconCacheStore();

        std::wstring key = cacheKey;
        if (key.empty()) key = (isDirectory ? L"dir:" : L"file:") + path;

        auto cacheIt = cache.find(key);
        if (cacheIt != cache.end()) return cacheIt->second;

        SHFILEINFO shellFileInfo{};
        UINT flags = SHGFI_ICON | SHGFI_SMALLICON;
        DWORD attrs = isDirectory ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL;

        bool status = false;
        if (useFileAttributes) {
            status = SHGetFileInfoW(path.c_str(), attrs, &shellFileInfo, sizeof(shellFileInfo), flags | SHGFI_USEFILEATTRIBUTES) != 0;
            if (!status) status = SHGetFileInfoW(L".", FILE_ATTRIBUTE_NORMAL, &shellFileInfo, sizeof(shellFileInfo), flags | SHGFI_USEFILEATTRIBUTES) != 0;
        }
        else {
            status = SHGetFileInfoW(path.c_str(), 0, &shellFileInfo, sizeof(shellFileInfo), flags) != 0;
            if (!status) status = SHGetFileInfoW(path.c_str(), attrs, &shellFileInfo, sizeof(shellFileInfo), flags | SHGFI_USEFILEATTRIBUTES) != 0;
            if (!status) status = SHGetFileInfoW(L".", FILE_ATTRIBUTE_NORMAL, &shellFileInfo, sizeof(shellFileInfo), flags | SHGFI_USEFILEATTRIBUTES) != 0;
        }

        if (!status || !shellFileInfo.hIcon) return nullptr;

        auto source = CreateImageSourceFromHIcon(shellFileInfo.hIcon, iconSize, true);
        if (source) cache.insert_or_assign(key, source);
        return source;
    }

    void ClearShellIconCache()
    {
        GetShellIconCacheStore().clear();
    }

    void ApplyHeaderColumnWidthsToRow(
        Grid const& headerGrid,
        Grid const& rowGrid,
        uint32_t rowOffset)
    {
        if (!headerGrid || !rowGrid) return;

        auto headerColumns = headerGrid.ColumnDefinitions();
        auto rowColumns = rowGrid.ColumnDefinitions();
        if (headerColumns.Size() == 0 || rowColumns.Size() < rowOffset + headerColumns.Size()) return;

        for (uint32_t i = 0; i < headerColumns.Size(); ++i) {
            rowColumns.GetAt(rowOffset + i).Width(headerColumns.GetAt(i).Width());
        }
    }

    void ApplyHeaderColumnWidthsToContainer(
        Grid const& headerGrid,
        ListViewItem const& itemContainer,
        uint32_t rowOffset)
    {
        if (!headerGrid || !itemContainer) return;

        auto rowGrid = itemContainer.ContentTemplateRoot().try_as<Grid>();
        if (!rowGrid) return;

        ApplyHeaderColumnWidthsToRow(headerGrid, rowGrid, rowOffset);
    }

    void SyncListViewColumnWidths(
        Grid const& headerGrid,
        Grid const& bodyGrid,
        ListView const& listView,
        uint32_t rowOffset,
        double epsilon)
    {
        if (!headerGrid || !bodyGrid || !listView) return;

        auto headerColumns = headerGrid.ColumnDefinitions();
        auto bodyColumns = bodyGrid.ColumnDefinitions();
        if (headerColumns.Size() == 0 || bodyColumns.Size() < headerColumns.Size()) return;

        static std::unordered_map<uint64_t, std::vector<double>> cachedHeaderWidths;
        uint64_t key = (uint64_t)get_abi(headerGrid);
        auto& lastWidths = cachedHeaderWidths[key];
        if (lastWidths.size() != headerColumns.Size()) {
            lastWidths.assign(headerColumns.Size(), -1.0);
        }

        bool changed = false;
        for (uint32_t i = 0; i < headerColumns.Size(); ++i) {
            double current = headerColumns.GetAt(i).ActualWidth();
            double previous = lastWidths[i];
            if (current > previous + epsilon || current + epsilon < previous) {
                changed = true;
                break;
            }
        }

        if (!changed) return;

        for (uint32_t i = 0; i < headerColumns.Size(); ++i) {
            auto headerColumn = headerColumns.GetAt(i);
            bodyColumns.GetAt(i).Width(headerColumn.Width());
            lastWidths[i] = headerColumn.ActualWidth();
        }

        std::vector<DependencyObject> pending{ listView };
        while (!pending.empty()) {
            auto parent = pending.back();
            pending.pop_back();

            int childCount = VisualTreeHelper::GetChildrenCount(parent);
            for (int i = 0; i < childCount; ++i) {
                auto child = VisualTreeHelper::GetChild(parent, i);
                if (auto itemContainer = child.try_as<ListViewItem>()) {
                    ApplyHeaderColumnWidthsToContainer(headerGrid, itemContainer, rowOffset);
                }
                else {
                    pending.push_back(child);
                }
            }
        }
    }

    namespace
    {
        // ── 表头分隔条的视觉常量（与应用的浅色/深色配色保持一致）──
        constexpr double kSplitterRestThickness = 1.0;
        constexpr double kSplitterActiveThickness = 3.0;
        constexpr double kSplitterActiveCornerRadius = 1.5;
        constexpr double kSplitterDragTolerance = 0.5;
        constexpr int kSplitterZIndex = 100;

        bool IsFixedGridLength(GridLength const& length)
        {
            return length.GridUnitType != GridUnitType::Auto && length.GridUnitType != GridUnitType::Star;
        }

        bool IsFlexibleGridLength(GridLength const& length)
        {
            return length.GridUnitType == GridUnitType::Star;
        }

        bool UseDarkSplitterPalette()
        {
            // 与应用的主题设置保持一致；跟随系统时用当前应用主题。
            try {
                if (GetConfiguredElementTheme() == ElementTheme::Dark) return true;
            }
            catch (...) {
            }

            try {
                if (auto app = Application::Current()) {
                    return app.RequestedTheme() == ApplicationTheme::Dark;
                }
            }
            catch (...) {
            }

            return false;
        }

        Windows::UI::Color SplitterRestColor()
        {
            return UseDarkSplitterPalette()
                ? Windows::UI::Color{ 0x3A, 0xE8, 0xE8, 0xE8 }
                : Windows::UI::Color{ 0x2E, 0x20, 0x20, 0x20 };
        }

        Windows::UI::Color SplitterActiveColor()
        {
            return UseDarkSplitterPalette()
                ? Windows::UI::Color{ 0xFF, 0x76, 0xB9, 0xFF }
                : Windows::UI::Color{ 0xFF, 0x00, 0x78, 0xD4 };
        }

        double PixelLengthOf(Grid const& grid, int index, bool isHorizontal)
        {
            if (!grid || index < 0) return 0.0;

            if (isHorizontal) {
                auto definitions = grid.ColumnDefinitions();
                if (index >= (int)definitions.Size()) return 0.0;

                auto definition = definitions.GetAt(index);
                auto width = definition.Width();
                if (IsFixedGridLength(width)) return width.Value;

                if (IsFlexibleGridLength(width)) {
                    double starTotal = 0.0;
                    double fixedTotal = 0.0;
                    for (auto const& item : definitions) {
                        auto itemWidth = item.Width();
                        if (IsFlexibleGridLength(itemWidth)) starTotal += itemWidth.Value;
                        else if (IsFixedGridLength(itemWidth)) fixedTotal += itemWidth.Value;
                    }

                    double available = grid.ActualWidth() - fixedTotal;
                    if (available < 0.0) available = 0.0;
                    if (starTotal <= 0.0) return definition.ActualWidth();
                    return available * (width.Value / starTotal);
                }

                return definition.ActualWidth();
            }

            auto rowDefinitions = grid.RowDefinitions();
            if (index >= (int)rowDefinitions.Size()) return 0.0;

            auto rowDefinition = rowDefinitions.GetAt(index);
            auto height = rowDefinition.Height();
            if (IsFixedGridLength(height)) return height.Value;

            if (IsFlexibleGridLength(height)) {
                double starTotal = 0.0;
                double fixedTotal = 0.0;
                for (auto const& item : rowDefinitions) {
                    auto itemHeight = item.Height();
                    if (IsFlexibleGridLength(itemHeight)) starTotal += itemHeight.Value;
                    else if (IsFixedGridLength(itemHeight)) fixedTotal += itemHeight.Value;
                }

                double available = grid.ActualHeight() - fixedTotal;
                if (available < 0.0) available = 0.0;
                if (starTotal <= 0.0) return rowDefinition.ActualHeight();
                return available * (height.Value / starTotal);
            }

            return rowDefinition.ActualHeight();
        }

        GridLength MakeGridLength(Grid const& grid, int index, bool isHorizontal, double pixels)
        {
            GridUnitType unitType = GridUnitType::Pixel;

            if (isHorizontal) {
                unitType = grid.ColumnDefinitions().GetAt(index).Width().GridUnitType;
            }
            else {
                unitType = grid.RowDefinitions().GetAt(index).Height().GridUnitType;
            }

            if (unitType == GridUnitType::Star) {
                return GridLengthHelper::FromValueAndType(pixels, GridUnitType::Star);
            }
            return GridLengthHelper::FromPixels(pixels);
        }

        void ApplySplitterVisual(Border const& line, SolidColorBrush const& brush, bool active, bool isHorizontal)
        {
            if (!line || !brush) return;

            brush.Color(active ? SplitterActiveColor() : SplitterRestColor());

            double thickness = active ? kSplitterActiveThickness : kSplitterRestThickness;
            if (isHorizontal) {
                line.Width(thickness);
            }
            else {
                line.Height(thickness);
            }

            double radius = active ? kSplitterActiveCornerRadius : 0.0;
            line.CornerRadius(Windows::UI::Xaml::CornerRadius{ radius, radius, radius, radius });
        }

    }

    void EnsureHeaderSplitters(
        winrt::Microsoft::UI::Xaml::Controls::Grid const& headerGrid,
        bool isHorizontal,
        double minColumnWidth)
    {
        if (!headerGrid) return;

        auto columns = headerGrid.ColumnDefinitions();
        if (columns.Size() < 2) return;

        // 双击复位用的初始列宽（整表头一份）。
        std::vector<GridLength> initialLengths;
        initialLengths.reserve(columns.Size());
        for (auto const& column : columns) {
            initialLengths.push_back(column.Width());
        }
        auto sharedInitial = std::make_shared<std::vector<GridLength>>(std::move(initialLengths));

        for (uint32_t index = 0; index + 1 < columns.Size(); ++index) {
            // 命中区：宽（或高）9px 的透明 Grid，骑在列/行边界上。
            Grid splitter;
            splitter.Background(SolidColorBrush(Windows::UI::Colors::Transparent()));
            Canvas::SetZIndex(splitter, kSplitterZIndex);

            if (isHorizontal) {
                splitter.Width(9);
                splitter.Margin(ThicknessHelper::FromLengths(0, 0, -5, 0));
                splitter.HorizontalAlignment(HorizontalAlignment::Right);
                splitter.VerticalAlignment(VerticalAlignment::Stretch);
            }
            else {
                splitter.Height(9);
                splitter.Margin(ThicknessHelper::FromLengths(0, 0, 0, -5));
                splitter.HorizontalAlignment(HorizontalAlignment::Stretch);
                splitter.VerticalAlignment(VerticalAlignment::Bottom);
            }

            // 可见部分：静止 1px 的分隔线，悬停/拖拽时加宽并变为强调色。
            Border line;
            SolidColorBrush lineBrush;
            lineBrush.Color(SplitterRestColor());
            line.Background(lineBrush);
            line.IsHitTestVisible(false);

            if (isHorizontal) {
                line.Width(kSplitterRestThickness);
                line.HorizontalAlignment(HorizontalAlignment::Center);
                line.VerticalAlignment(VerticalAlignment::Stretch);
            }
            else {
                line.Height(kSplitterRestThickness);
                line.HorizontalAlignment(HorizontalAlignment::Stretch);
                line.VerticalAlignment(VerticalAlignment::Center);
            }

            splitter.Children().Append(line);

            auto dragState = std::make_shared<bool>(false);
            auto hoverState = std::make_shared<bool>(false);
            auto previousIndex = std::make_shared<int>(-1);
            auto nextIndex = std::make_shared<int>(-1);
            auto startLengths = std::make_shared<std::array<double, 2>>(std::array<double, 2>{ 0.0, 0.0 });
            auto dragStart = std::make_shared<double>(0.0);

            auto applyVisual = [line, lineBrush, isHorizontal, hoverState, dragState]() {
                ApplySplitterVisual(line, lineBrush, *hoverState || *dragState, isHorizontal);
            };
            applyVisual();

            splitter.PointerEntered([hoverState, applyVisual, splitter, isHorizontal](IInspectable const&, PointerRoutedEventArgs const&) {
                *hoverState = true;
                applyVisual();
            });

            splitter.PointerExited([hoverState, dragState, applyVisual, splitter, isHorizontal](IInspectable const&, PointerRoutedEventArgs const&) {
                *hoverState = false;
                if (*dragState) return;
                applyVisual();
            });

            splitter.PointerPressed([headerGrid, index, isHorizontal, dragState, previousIndex, nextIndex, startLengths, dragStart, applyVisual, splitter](IInspectable const&, PointerRoutedEventArgs const& e) {
                if (*dragState) return;

                int count = isHorizontal ? (int)headerGrid.ColumnDefinitions().Size() : (int)headerGrid.RowDefinitions().Size();
                if (index < 0 || (int)index >= count) return;

                int current = (int)index;
                int next = current + 1;
                if (next >= count) {
                    current = (int)index - 1;
                    next = (int)index;
                }
                if (current < 0 || next >= count) return;

                *previousIndex = current;
                *nextIndex = next;
                (*startLengths)[0] = PixelLengthOf(headerGrid, current, isHorizontal);
                (*startLengths)[1] = PixelLengthOf(headerGrid, next, isHorizontal);

                Windows::Foundation::Point point = e.GetCurrentPoint(headerGrid).Position().as<Windows::Foundation::Point>();
                *dragStart = isHorizontal ? point.X : point.Y;
                *dragState = true;

                applyVisual();
                splitter.CapturePointer(e.Pointer());
                e.Handled(true);
            });

            splitter.PointerMoved([headerGrid, isHorizontal, dragState, previousIndex, nextIndex, startLengths, dragStart, minColumnWidth](IInspectable const&, PointerRoutedEventArgs const& e) {
                if (!*dragState) return;

                int count = isHorizontal ? (int)headerGrid.ColumnDefinitions().Size() : (int)headerGrid.RowDefinitions().Size();
                if (*previousIndex < 0 || *nextIndex >= count) { *dragState = false; return; }

                Windows::Foundation::Point point = e.GetCurrentPoint(headerGrid).Position().as<Windows::Foundation::Point>();
                double position = isHorizontal ? point.X : point.Y;
                double delta = position - *dragStart;
                if (std::abs(delta) < kSplitterDragTolerance) { e.Handled(true); return; }

                GridLength previousLength = isHorizontal
                    ? headerGrid.ColumnDefinitions().GetAt(*previousIndex).Width()
                    : headerGrid.RowDefinitions().GetAt(*previousIndex).Height();
                GridLength nextLength = isHorizontal
                    ? headerGrid.ColumnDefinitions().GetAt(*nextIndex).Width()
                    : headerGrid.RowDefinitions().GetAt(*nextIndex).Height();

                double previousPixels = (*startLengths)[0] + delta;
                double nextPixels = (*startLengths)[1] - delta;

                if (IsFixedGridLength(previousLength) || IsFixedGridLength(nextLength)) {
                    if (previousPixels < minColumnWidth || nextPixels < minColumnWidth) return;
                }
                else if (previousPixels < 0.0 || nextPixels < 0.0) {
                    return;
                }

                GridLength newPrevious = MakeGridLength(headerGrid, *previousIndex, isHorizontal, previousPixels);
                GridLength newNext = MakeGridLength(headerGrid, *nextIndex, isHorizontal, nextPixels);

                if (isHorizontal) {
                    headerGrid.ColumnDefinitions().GetAt(*previousIndex).Width(newPrevious);
                    headerGrid.ColumnDefinitions().GetAt(*nextIndex).Width(newNext);
                }
                else {
                    headerGrid.RowDefinitions().GetAt(*previousIndex).Height(newPrevious);
                    headerGrid.RowDefinitions().GetAt(*nextIndex).Height(newNext);
                }

                e.Handled(true);
            });

            splitter.PointerReleased([dragState, previousIndex, nextIndex, applyVisual, splitter, isHorizontal](IInspectable const&, PointerRoutedEventArgs const& e) {
                if (!*dragState) return;

                splitter.ReleasePointerCapture(e.Pointer());
                *dragState = false;
                *previousIndex = -1;
                *nextIndex = -1;
                applyVisual();
                e.Handled(true);
            });

            splitter.PointerCaptureLost([dragState, previousIndex, nextIndex, applyVisual](IInspectable const&, PointerRoutedEventArgs const&) {
                *dragState = false;
                *previousIndex = -1;
                *nextIndex = -1;
                applyVisual();
            });

            splitter.DoubleTapped([headerGrid, sharedInitial, isHorizontal](IInspectable const&, DoubleTappedRoutedEventArgs const& e) {
                auto definitions = headerGrid.ColumnDefinitions();
                auto count = std::min<uint32_t>((uint32_t)sharedInitial->size(), definitions.Size());
                for (uint32_t column = 0; column < count; ++column) {
                    definitions.GetAt(column).Width(sharedInitial->at(column));
                }
                e.Handled(true);
            });

            Grid::SetColumn(splitter, index);
            headerGrid.Children().Append(splitter);
        }
    }
}
