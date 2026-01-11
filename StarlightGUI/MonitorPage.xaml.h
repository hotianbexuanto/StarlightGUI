#pragma once

#include "MonitorPage.g.h"

namespace winrt::StarlightGUI::implementation
{
    struct MonitorPage : MonitorPageT<MonitorPage>
    {
        MonitorPage();

        void ObjectTreeView_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e);
        void ObjectListView_Tapped(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::TappedRoutedEventArgs const& e);
        winrt::fire_and_forget RefreshButton_Click(IInspectable const&, RoutedEventArgs const&);

        winrt::Windows::Foundation::IAsyncAction LoadItemList();
        winrt::Windows::Foundation::IAsyncAction LoadPartitionList(std::wstring path);
        winrt::Windows::Foundation::IAsyncAction LoadObjectList();
        winrt::Windows::Foundation::IAsyncAction WaitAndReloadAsync(int interval);

        void ColumnHeader_Click(IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);

        winrt::fire_and_forget ObjectApplySort(bool& isAscending, const std::string& column);
        void SearchBox_TextChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        bool ObjectApplyFilter(const winrt::StarlightGUI::ObjectEntry& object, hstring& query);

        winrt::Windows::Foundation::Collections::IObservableVector<winrt::StarlightGUI::ObjectEntry> m_objectList{
            winrt::multi_threaded_observable_vector<winrt::StarlightGUI::ObjectEntry>()
        };
        winrt::Windows::Foundation::Collections::IObservableVector<winrt::WinUI3Package::SegmentedItem> m_itemList{
            winrt::multi_threaded_observable_vector<winrt::WinUI3Package::SegmentedItem>()
        };

        bool m_isLoadingObjects = false;
        winrt::Microsoft::UI::Xaml::DispatcherTimer reloadTimer;

        inline static bool m_object_isNameAscending = true;
        inline static bool m_object_isTypeAscending = true;
        inline static bool currentSortingOption;
        inline static std::string currentSortingType;
    };
}

namespace winrt::StarlightGUI::factory_implementation
{
    struct MonitorPage : MonitorPageT<MonitorPage, implementation::MonitorPage>
    {
    };
}
