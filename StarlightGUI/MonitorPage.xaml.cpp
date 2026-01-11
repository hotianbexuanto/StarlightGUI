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

    MonitorPage::MonitorPage() {
        InitializeComponent();

        ObjectListView().ItemsSource(m_objectList);
        ObjectListView().ItemContainerTransitions().Clear();
        ObjectListView().ItemContainerTransitions().Append(EntranceThemeTransition());

        ObjectTreeView().ItemsSource(m_itemList);

        // SelectionChanged 会在进入时触发一次，特么的不知道为啥，不管他
        Loaded([this](auto&&, auto&&) -> IAsyncAction {
            partitions.clear();
            items.clear();
            winrt::StarlightGUI::ObjectEntry root = winrt::make<winrt::StarlightGUI::implementation::ObjectEntry>();
            root.Name(L"\\");
            root.Type(L"Directory");
            root.Path(L"\\");
            partitions.push_back(root);
            co_await LoadPartitionList(L"\\");
            co_await LoadItemList();
            co_await LoadObjectList();
            m_isLoadingObjects = false; // 为啥啊
            });
    }

    winrt::Windows::Foundation::IAsyncAction MonitorPage::LoadPartitionList(std::wstring path) {
        std::vector<winrt::StarlightGUI::ObjectEntry> partitionsInPath;

        KernelInstance::EnumObjectByDirectory(path, partitionsInPath);
        for (const auto& object : partitionsInPath) {
            if (object.Type() == L"Directory") {
                partitions.push_back(object);
                co_await LoadPartitionList(object.Path().c_str());
            }
        }

        co_return;
    }

    winrt::Windows::Foundation::IAsyncAction MonitorPage::LoadObjectList() {
        if (m_isLoadingObjects) {
            co_return;
        }
        m_isLoadingObjects = true;

        LOG_INFO(__WFUNCTION__, L"Loading object list...");

        if (!ObjectTreeView().SelectedItem() || partitions.size() == 0) co_return;

        auto lifetime = get_strong();
        int32_t index = ObjectTreeView().SelectedIndex();
        hstring query = SearchBox().Text();

        co_await winrt::resume_background();

        std::vector<winrt::StarlightGUI::ObjectEntry> objects;

        // 获取对象逻辑
        winrt::StarlightGUI::ObjectEntry& selectedPartition = partitions[index];
        KernelInstance::EnumObjectByDirectory(selectedPartition.Path().c_str(), objects);

		fullRecordedObjects = objects;

        co_await wil::resume_foreground(DispatcherQueue());

        m_objectList.Clear();
        for (const auto& object : objects) {
            bool shouldRemove = query.empty() ? false : ObjectApplyFilter(object, query);
            if (shouldRemove) continue;

            if (object.Name().empty()) object.Name(L"(未知)");
            if (object.Type().empty()) object.Type(L"(未知)");
            if (object.CreationTime().empty()) object.CreationTime(L"(未知)");
            if (!object.Link().empty()) object.Path(object.Link());

            m_objectList.Append(object);
        }

        LOG_INFO(__WFUNCTION__, L"Loaded object list, %d entry(s) in total.", m_objectList.Size());
        m_isLoadingObjects = false;
    }

    void MonitorPage::ObjectListView_Tapped(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::TappedRoutedEventArgs const& e)
    {
        if (!ObjectListView().SelectedItem()) return;
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
        LoadObjectList();
    }

    winrt::Windows::Foundation::IAsyncAction MonitorPage::LoadItemList() {
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
            ObjectApplySort(m_object_isNameAscending, "Name");
        }
        else if (columnName == L"Object_Type")
        {
            ObjectApplySort(m_object_isTypeAscending, "Type");
        }
    }

    // 排序切换
    winrt::fire_and_forget MonitorPage::ObjectApplySort(bool& isAscending, const std::string& column)
    {
        NameHeaderButton().Content(box_value(L"名称"));
        TypeHeaderButton().Content(box_value(L"类型"));

        if (column == "Name") {
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
        else if (column == "Type") {
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

        m_objectList.Clear();
        for (auto& object : fullRecordedObjects) {
            m_objectList.Append(object);
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

    bool MonitorPage::ObjectApplyFilter(const winrt::StarlightGUI::ObjectEntry& object, hstring& query) {
        std::wstring name = object.Name().c_str();
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
        co_await LoadObjectList();
        RefreshButton().IsEnabled(true);
        co_return;
    }

    winrt::Windows::Foundation::IAsyncAction MonitorPage::WaitAndReloadAsync(int interval) {
        auto lifetime = get_strong();

        reloadTimer.Stop();
        reloadTimer.Interval(std::chrono::milliseconds(interval));
        reloadTimer.Tick([this](auto&&, auto&&) {
            LoadObjectList();
            reloadTimer.Stop();
            });
        reloadTimer.Start();

        co_return;
    }
}