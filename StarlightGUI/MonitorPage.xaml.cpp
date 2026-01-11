#include "pch.h"
#include "MonitorPage.xaml.h"
#if __has_include("MonitorPage.g.cpp")
#include "MonitorPage.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls::Primitives;
using namespace WinUI3Package;

namespace winrt::StarlightGUI::implementation
{
    static std::vector<SegmentedItem> items;
    static std::vector<winrt::StarlightGUI::ObjectEntry> partitions;
	static std::vector<winrt::StarlightGUI::ObjectEntry> fullRecordedObjects;
    static std::vector<winrt::StarlightGUI::CallbackEntry> fullRecordedCallbacks;

    MonitorPage::MonitorPage() {
        InitializeComponent();

        ObjectListView().ItemsSource(m_objectList);
        CallbackListView().ItemsSource(m_callbackList);
        
        if (!list_animation) {
            ObjectListView().ItemContainerTransitions().Clear();
			CallbackListView().ItemContainerTransitions().Clear();
        }

        ObjectTreeView().ItemsSource(m_itemList);

        if (!KernelInstance::IsRunningAsAdmin()) {
			CallbackSegmentedItem().IsEnabled(false);
        }

        // SelectionChanged 会在进入时触发一次，特么的不知道为啥，不管他
        Loaded([this](auto&&, auto&&) {
			HandleSegmentedChange(0);
            });
    }

    winrt::Windows::Foundation::IAsyncAction MonitorPage::LoadPartitionList(std::wstring path) {
        if (segmentedIndex != 0) co_return;

        std::vector<winrt::StarlightGUI::ObjectEntry> partitionsInPath;

        KernelInstance::EnumObjectsByDirectory(path, partitionsInPath);
        for (const auto& object : partitionsInPath) {
            if (object.Type() == L"Directory") {
                partitions.push_back(object);
                co_await LoadPartitionList(object.Path().c_str());
            }
        }

        co_return;
    }

    winrt::Windows::Foundation::IAsyncAction MonitorPage::LoadObjectList() {
        if (m_isLoading || segmentedIndex != 0 || !ObjectTreeView().SelectedItem() || partitions.size() == 0) {
            co_return;
        }
        m_isLoading = true;

        LOG_INFO(__WFUNCTION__, L"Loading object list...");

        auto lifetime = get_strong();
        int32_t index = ObjectTreeView().SelectedIndex();
        hstring query = SearchBox().Text();

        co_await winrt::resume_background();

        std::vector<winrt::StarlightGUI::ObjectEntry> objects;

        // 获取对象逻辑
        winrt::StarlightGUI::ObjectEntry& selectedPartition = partitions[index];
        KernelInstance::EnumObjectsByDirectory(selectedPartition.Path().c_str(), objects);

		fullRecordedObjects = objects;

        co_await wil::resume_foreground(DispatcherQueue());

        m_objectList.Clear();
        for (const auto& object : objects) {
            bool shouldRemove = query.empty() ? false : ApplyFilter(object.Name(), query);
            if (shouldRemove) continue;

            if (object.Name().empty()) object.Name(L"(未知)");
            if (object.Type().empty()) object.Type(L"(未知)");
            if (object.CreationTime().empty()) object.CreationTime(L"(未知)");
            if (!object.Link().empty()) object.Path(object.Link());

            m_objectList.Append(object);
        }

        LOG_INFO(__WFUNCTION__, L"Loaded object list, %d entry(s) in total.", m_objectList.Size());
        m_isLoading = false;
    }

    winrt::Windows::Foundation::IAsyncAction MonitorPage::LoadCallbackList() {
        if (m_isLoading || segmentedIndex != 1 || !KernelInstance::IsRunningAsAdmin()) {
            co_return;
        }
        m_isLoading = true;

        LOG_INFO(__WFUNCTION__, L"Loading callback list...");

        auto lifetime = get_strong();
        hstring query = SearchBox().Text();

        co_await winrt::resume_background();

        std::vector<winrt::StarlightGUI::CallbackEntry> callbacks;

		KernelInstance::EnumCallbacks(callbacks);

        fullRecordedCallbacks = callbacks;

        co_await wil::resume_foreground(DispatcherQueue());

        m_callbackList.Clear();
        for (const auto& callback : callbacks) {
            bool shouldRemove = query.empty() ? false : ApplyFilter(callback.Type(), query);
            if (shouldRemove) continue;

            if (callback.Type().empty()) callback.Type(L"(未知)");
            if (callback.Module().empty()) callback.Module(L"(未知)");

            m_callbackList.Append(callback);
        }

        LOG_INFO(__WFUNCTION__, L"Loaded callback list, %d entry(s) in total.", m_callbackList.Size());
        m_isLoading = false;
    }

    void MonitorPage::ObjectListView_Tapped(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::TappedRoutedEventArgs const& e)
    {
        if (!ObjectListView().SelectedItem() || segmentedIndex != 0) return;
        auto item = ObjectListView().SelectedItem().as<winrt::StarlightGUI::ObjectEntry>();

        // 获取信息
        BOOL status = KernelInstance::GetObjectDetails(item.Path().c_str(), item.Type().c_str(), item);

        Flyout flyout;
        StackPanel flyoutPanel;

        // 基本信息
        GroupBox basicInfoBox;
        StackPanel basicInfoPanel;
        basicInfoBox.Header(box_value(L"基本信息"));
        basicInfoBox.Margin(ThicknessHelper::FromLengths(0, 0, 0, 10));
        TextBlock name;
        name.Text(L"名称: " + item.Name());
        TextBlock type;
        type.Text(L"类型: " + item.Type());
        TextBlock path;
        path.Text(L"完整路径: " + item.Path());
        CheckBox permanent;
        permanent.Content(box_value(L"永久"));
        permanent.IsChecked(item.Permanent());
        permanent.IsEnabled(false);
        basicInfoPanel.Children().Append(name);
        basicInfoPanel.Children().Append(type);
        basicInfoPanel.Children().Append(path);
        basicInfoPanel.Children().Append(permanent);
        basicInfoBox.Content(basicInfoPanel);

        // 引用信息
        GroupBox referencesBox;
        StackPanel referencesPanel;
        referencesBox.Header(box_value(L"引用信息"));
        referencesBox.Margin(ThicknessHelper::FromLengths(0, 0, 0, 10));
        TextBlock references;
        references.Text(L"引用: " + std::to_wstring(item.References()));
        TextBlock handles;
        handles.Text(L"句柄: " + std::to_wstring(item.Handles()));
        referencesPanel.Children().Append(references);
        referencesPanel.Children().Append(handles);
        referencesBox.Content(referencesPanel);

        // 配额信息
        GroupBox quotaBox;
        StackPanel quotaPanel;
        quotaBox.Header(box_value(L"配额信息"));
        quotaBox.Margin(ThicknessHelper::FromLengths(0, 0, 0, 10));
        TextBlock paged;
        paged.Text(L"分页池: " + FormatMemorySize(item.PagedPool()));
        TextBlock nonPaged;
        nonPaged.Text(L"非分页池: " + FormatMemorySize(item.NonPagedPool()));
        quotaPanel.Children().Append(paged);
        quotaPanel.Children().Append(nonPaged);
        quotaBox.Content(quotaPanel);

        // 详细信息
        bool flag = false;
        GroupBox detailBox;
        StackPanel detailPanel;
        detailBox.Margin(ThicknessHelper::FromLengths(0, 0, 0, 10));
        if (item.Type() == L"SymbolicLink") {
            detailBox.Header(box_value(L"符号链接"));
            TextBlock creationTime;
            creationTime.Text(L"创建时间: " + item.CreationTime());
            TextBlock linkTarget;
            linkTarget.Text(L"链接: " + item.Link());
            detailPanel.Children().Append(creationTime);
            detailPanel.Children().Append(linkTarget);
        }
        else if (item.Type() == L"Event") {
            detailBox.Header(box_value(L"事件"));
            TextBlock eventType;
            eventType.Text(L"事件类型: " + item.EventType());
            TextBlock eventSignaled;
            hstring state = item.EventSignaled() ? L"TRUE" : L"FALSE";
            eventSignaled.Text(L"触发: " + state);
            detailPanel.Children().Append(eventType);
            detailPanel.Children().Append(eventSignaled);
        }
        else if (item.Type() == L"Mutant") {
            detailBox.Header(box_value(L"互斥体"));
			TextBlock mutantHoldCount;
			mutantHoldCount.Text(L"持有数: " + to_hstring(item.MutantHoldCount()));
            TextBlock mutantAbandoned;
            hstring state = item.MutantAbandoned() ? L"TRUE" : L"FALSE";
            mutantAbandoned.Text(L"遗弃: " + state);
			detailPanel.Children().Append(mutantHoldCount);
            detailPanel.Children().Append(mutantAbandoned);
        }
        else if (item.Type() == L"Semaphore") {
            detailBox.Header(box_value(L"信号量"));
			TextBlock semaphoreCount;
            semaphoreCount.Text(L"当前量: " + to_hstring(item.SemaphoreCount()));
            TextBlock semaphoreLimit;
			semaphoreLimit.Text(L"最大量: " + to_hstring(item.SemaphoreLimit()));
            detailPanel.Children().Append(semaphoreCount);
			detailPanel.Children().Append(semaphoreLimit);
        }
        else if (item.Type() == L"Section") {
            detailBox.Header(box_value(L"区域"));
            TextBlock sectionBaseAddress;
            sectionBaseAddress.Text(L"基址: " + ULongToHexString(item.SectionBaseAddress()));
			TextBlock sectionMaximumSize;
            sectionMaximumSize.Text(L"大小: " + FormatMemorySize(item.SectionMaximumSize()));
            TextBlock sectionAttributes;
            hstring attr = item.SectionAttributes() == 0x200000 ? L"SEC_BASED" : item.SectionAttributes() == 0x800000 ? L"SEC_FILE" : item.SectionAttributes() == 0x4000000 
				? L"SEC_RESERVE" : item.SectionAttributes() == 0x8000000 ? L"SEC_COMMIT" : item.SectionAttributes() == 0x1000000 ? L"SEC_IMAGE" : L"NULL";
			sectionAttributes.Text(L"属性: " + attr);
			detailPanel.Children().Append(sectionBaseAddress);
			detailPanel.Children().Append(sectionMaximumSize);
			detailPanel.Children().Append(sectionAttributes);
        }
        else if (item.Type() == L"Timer") {
            detailBox.Header(box_value(L"计时器"));
            TextBlock timerRemainingTime;
            timerRemainingTime.Text(L"剩余时间: " + to_hstring(item.TimerRemainingTime() * 100) + L"ns");
            TextBlock timerState;
            hstring state = item.TimerState() ? L"TRUE" : L"FALSE";
            timerState.Text(L"触发: " + state);
            detailPanel.Children().Append(timerRemainingTime);
            detailPanel.Children().Append(timerState);
		}
        else if (item.Type() == L"IoCompletion") {
            detailBox.Header(box_value(L"I/O 完成端口"));
            TextBlock ioCompletionDepth;
            ioCompletionDepth.Text(L"深度: " + to_hstring(item.IoCompletionDepth()));
            detailPanel.Children().Append(ioCompletionDepth);
		}
        else {
            flag = true;
        }
        detailBox.Content(detailPanel);
		detailBox.Visibility(flag ? Visibility::Collapsed : Visibility::Visible);
        if (!status && !flag) {
            TextBlock errorText;
            errorText.Text(L"获取信息时出错，部分内容可能不完整！");
            errorText.Foreground(Microsoft::UI::Xaml::Media::SolidColorBrush(Microsoft::UI::Colors::OrangeRed()));
            flyoutPanel.Children().Append(errorText);
        }

        flyoutPanel.Children().Append(basicInfoBox);
        flyoutPanel.Children().Append(referencesBox);
        flyoutPanel.Children().Append(quotaBox);
        flyoutPanel.Children().Append(detailBox);
        flyout.Content(flyoutPanel);

        FlyoutShowOptions options;
        options.ShowMode(FlyoutShowMode::Auto);
        options.Position(e.GetPosition(ObjectListView()));

		FlyoutHelper::SetAcrylicWorkaround(flyout, true);

        flyout.ShowAt(ObjectListView(), options);
    }


    void MonitorPage::ObjectTreeView_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e)
    {
        if (!IsLoaded() || segmentedIndex != 0) return;
        LoadObjectList();
    }

    winrt::Windows::Foundation::IAsyncAction MonitorPage::LoadItemList() {
        if (segmentedIndex != 0) co_return;
        m_itemList.Clear();

        std::sort(partitions.begin(), partitions.end(), [](auto a, auto b) {
            std::wstring aName = a.Path().c_str();
            std::wstring bName = b.Path().c_str();
            std::transform(aName.begin(), aName.end(), aName.begin(), ::towlower);
            std::transform(bName.begin(), bName.end(), bName.begin(), ::towlower);

            return aName < bName;
            });

        for (const auto& partition : partitions) {
            SegmentedItem item;
            TextBlock textBlock;
            textBlock.Text(partition.Path());
            item.Content(textBlock);
            item.HorizontalContentAlignment(HorizontalAlignment::Left);
            items.push_back(item);
        }

        for (const auto& item : items) {
            m_itemList.Append(item);
        }
        co_return;
    }

    void MonitorPage::ColumnHeader_Click(IInspectable const& sender, RoutedEventArgs const& e)
    {
        Button clickedButton = sender.as<Button>();
        winrt::hstring columnName = clickedButton.Tag().as<winrt::hstring>();

        if (columnName == L"Object_Name")
        {
            ApplySort(m_object_isNameAscending, "Object_Name");
        }
        else if (columnName == L"Object_Type")
        {
            ApplySort(m_object_isTypeAscending, "Object_Type");
        }
        else if (columnName == L"Callback_Type")
        {
			ApplySort(m_callback_isTypeAscending, "Callback_Type");
        }
        else if (columnName == L"Callback_Entry")
        {
            ApplySort(m_callback_isEntryAscending, "Callback_Entry");
        }
        else if (columnName == L"Callback_Handle")
        {
            ApplySort(m_callback_isHandleAscending, "Callback_Handle");
		}
    }

    // 排序切换
    winrt::fire_and_forget MonitorPage::ApplySort(bool& isAscending, const std::string& column)
    {
        NameHeaderButton().Content(box_value(L"名称"));
        TypeHeaderButton().Content(box_value(L"类型"));
		TypeHeaderButton2().Content(box_value(L"类型"));
		EntryHeaderButton().Content(box_value(L"入口"));
		HandleHeaderButton().Content(box_value(L"句柄"));

        if (column == "Object_Name") {
            if (isAscending) {
                NameHeaderButton().Content(box_value(L"名称 ↓"));
                std::sort(fullRecordedObjects.begin(), fullRecordedObjects.end(), [](auto a, auto b) {
                    std::wstring aName = a.Name().c_str();
                    std::wstring bName = b.Name().c_str();
                    std::transform(aName.begin(), aName.end(), aName.begin(), ::towlower);
                    std::transform(bName.begin(), bName.end(), bName.begin(), ::towlower);

                    return aName < bName;
                    });

            }
            else {
                NameHeaderButton().Content(box_value(L"名称 ↑"));
                std::sort(fullRecordedObjects.begin(), fullRecordedObjects.end(), [](auto a, auto b) {
                    std::wstring aName = a.Name().c_str();
                    std::wstring bName = b.Name().c_str();
                    std::transform(aName.begin(), aName.end(), aName.begin(), ::towlower);
                    std::transform(bName.begin(), bName.end(), bName.begin(), ::towlower);

                    return aName > bName;
                    });
            }
        }
        else if (column == "Object_Type") {
            if (isAscending) {
                TypeHeaderButton().Content(box_value(L"类型 ↓"));
                std::sort(fullRecordedObjects.begin(), fullRecordedObjects.end(), [](auto a, auto b) {
                    std::wstring aType = a.Type().c_str();
                    std::wstring bType = b.Type().c_str();
                    std::transform(aType.begin(), aType.end(), aType.begin(), ::towlower);
                    std::transform(bType.begin(), bType.end(), bType.begin(), ::towlower);

                    return aType < bType;
                    });

            }
            else {
                TypeHeaderButton().Content(box_value(L"类型 ↑"));
                std::sort(fullRecordedObjects.begin(), fullRecordedObjects.end(), [](auto a, auto b) {
                    std::wstring aType = a.Type().c_str();
                    std::wstring bType = b.Type().c_str();
                    std::transform(aType.begin(), aType.end(), aType.begin(), ::towlower);
                    std::transform(bType.begin(), bType.end(), bType.begin(), ::towlower);

                    return aType > bType;
                    });
            }
        }
        else if (column == "Callback_Type") {
            if (isAscending) {
                TypeHeaderButton2().Content(box_value(L"类型 ↓"));
                std::sort(fullRecordedCallbacks.begin(), fullRecordedCallbacks.end(), [](auto a, auto b) {
                    std::wstring aType = a.Type().c_str();
                    std::wstring bType = b.Type().c_str();
                    std::transform(aType.begin(), aType.end(), aType.begin(), ::towlower);
                    std::transform(bType.begin(), bType.end(), bType.begin(), ::towlower);
                    return aType < bType;
                    });
            }
            else {
                TypeHeaderButton2().Content(box_value(L"类型 ↑"));
                std::sort(fullRecordedCallbacks.begin(), fullRecordedCallbacks.end(), [](auto a, auto b) {
                    std::wstring aType = a.Type().c_str();
                    std::wstring bType = b.Type().c_str();
                    std::transform(aType.begin(), aType.end(), aType.begin(), ::towlower);
                    std::transform(bType.begin(), bType.end(), bType.begin(), ::towlower);
                    return aType > bType;
                    });
            }
		}
        else if (column == "Callback_Entry") {
            if (isAscending) {
                EntryHeaderButton().Content(box_value(L"入口 ↓"));
                std::sort(fullRecordedCallbacks.begin(), fullRecordedCallbacks.end(), [](auto a, auto b) {
                    return a.EntryULong() < b.EntryULong();
                    });
            }
            else {
                EntryHeaderButton().Content(box_value(L"入口 ↑"));
                std::sort(fullRecordedCallbacks.begin(), fullRecordedCallbacks.end(), [](auto a, auto b) {
                    return a.EntryULong() > b.EntryULong();
                    });
            }
        }
        else if (column == "Callback_Handle") {
            if (isAscending) {
                HandleHeaderButton().Content(box_value(L"句柄 ↓"));
                std::sort(fullRecordedCallbacks.begin(), fullRecordedCallbacks.end(), [](auto a, auto b) {
                    return a.HandleULong() < b.HandleULong();
                    });
            }
            else {
                HandleHeaderButton().Content(box_value(L"句柄 ↑"));
                std::sort(fullRecordedCallbacks.begin(), fullRecordedCallbacks.end(), [](auto a, auto b) {
                    return a.HandleULong() > b.HandleULong();
                    });
            }
		}

        switch (segmentedIndex) {
        case 0: {
            m_objectList.Clear();
            for (auto& object : fullRecordedObjects) {
                m_objectList.Append(object);
            }
            break;
        }
        case 1: {
            m_callbackList.Clear();
            for (auto& callback : fullRecordedCallbacks) {
                m_callbackList.Append(callback);
            }
            break;
        }
        }

        isAscending = !isAscending;
        currentSortingOption = !isAscending;
        currentSortingType = column;

        co_return;
    }

    void MonitorPage::SearchBox_TextChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
    {
        WaitAndReloadAsync(200);
    }

    bool MonitorPage::ApplyFilter(const hstring& target, const hstring& query) {
        std::wstring name = target.c_str();
        std::wstring queryWStr = query.c_str();

        // 不比较大小写
        std::transform(name.begin(), name.end(), name.begin(), ::towlower);
        std::transform(queryWStr.begin(), queryWStr.end(), queryWStr.begin(), ::towlower);

        bool result = name.find(queryWStr) == std::wstring::npos;

        return result;
    }

    winrt::fire_and_forget MonitorPage::RefreshButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        RefreshButton().IsEnabled(false);

        switch (segmentedIndex) {
        case 0: {
            co_await LoadObjectList();
            break;
        }
        case 1: {
            co_await LoadCallbackList();
			break;
        }
        }

        RefreshButton().IsEnabled(true);
        co_return;
    }

    winrt::Windows::Foundation::IAsyncAction MonitorPage::WaitAndReloadAsync(int interval) {
        auto lifetime = get_strong();

        reloadTimer.Stop();
        reloadTimer.Interval(std::chrono::milliseconds(interval));
        reloadTimer.Tick([this](auto&&, auto&&) {
			RefreshButton_Click(nullptr, nullptr);
            reloadTimer.Stop();
            });
        reloadTimer.Start();

        co_return;
    }

    void MonitorPage::MainSegmented_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e)
    {
        if (!IsLoaded()) return;
		if (!MainSegmented().SelectedItem()) return;
		HandleSegmentedChange(MainSegmented().SelectedIndex());
    }

    winrt::fire_and_forget MonitorPage::HandleSegmentedChange(int index) {
		if (!IsLoaded()) co_return;
		segmentedIndex = index;
        m_isLoading = false;

        // 清除列表以防止潜在的内存占用
        items.clear();
        partitions.clear();
		fullRecordedObjects.clear();
		fullRecordedCallbacks.clear();
        m_objectList.Clear();
		m_callbackList.Clear();

		Grid& visibleGrid = ObjectGrid();
        ObjectGrid().Visibility(Visibility::Collapsed);
        CallbackGrid().Visibility(Visibility::Collapsed);
        switch (index) {
        case 0: {
            visibleGrid = ObjectGrid();
            winrt::StarlightGUI::ObjectEntry root = winrt::make<winrt::StarlightGUI::implementation::ObjectEntry>();
            root.Name(L"\\");
            root.Type(L"Directory");
            root.Path(L"\\");
            partitions.push_back(root);
            co_await LoadPartitionList(L"\\");
            co_await LoadItemList();
            co_await LoadObjectList();
            ObjectTreeView().SelectedIndex(0);
            break;
        }
        case 1: {
            visibleGrid = CallbackGrid();
			co_await LoadCallbackList();
            break;
        }
        }
		visibleGrid.Visibility(Visibility::Visible);

        co_return;
    }
}
