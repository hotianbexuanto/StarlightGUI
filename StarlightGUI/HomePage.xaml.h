#pragma once

#include "HomePage.g.h"
#include <pdh.h>
#include <nvml.h>

namespace winrt::StarlightGUI::implementation
{
    struct HomePage : public HomePageT<HomePage>
    {
        HomePage();

        void SetGreetingText();
        winrt::fire_and_forget SetUserProfile();
        winrt::fire_and_forget FetchHitokoto();
        void SetupClock();
        void OnClockTick(IInspectable const&, IInspectable const&);
        void UpdateClock();

        winrt::fire_and_forget UpdateGauges();

        winrt::Microsoft::UI::Xaml::DispatcherTimer clockTimer;

        inline static winrt::hstring greeting;
        inline static winrt::hstring username;
        inline static winrt::hstring hitokoto;
        inline static winrt::Microsoft::UI::Xaml::Media::Imaging::BitmapImage avatar{ nullptr };

        // –‘ƒ‹œ‘ æ
        inline static std::unordered_map<int, hstring> adpt_name_map;
        inline static hstring cpu_manufacture, disk_manufacture, gpu_manufacture, netadpt_manufacture;
        inline static bool initialized, isNvidia, virtualization, isNetSend = false;
        inline static double cache_l1, cache_l2, cache_l3;
        inline static int adptIndex;
        inline static nvmlDevice_t device;
        inline static PDH_HQUERY query;
        inline static PDH_HCOUNTER counter_cpu_time, counter_cpu_freq, counter_cpu_process, counter_cpu_thread, counter_cpu_syscall;
        inline static PDH_HCOUNTER counter_mem_cached, counter_mem_committed, counter_mem_read, counter_mem_write, counter_mem_input, counter_mem_output;
        inline static PDH_HCOUNTER counter_disk_time, counter_disk_trans, counter_disk_read, counter_disk_write, counter_disk_io;
        inline static PDH_HCOUNTER counter_gpu_time;
        inline static PDH_HCOUNTER counter_net_send, counter_net_receive, counter_net_packet_send, counter_net_packet_receive;

        void NextAdapterName_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void ChangeMode_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
    };
}
namespace winrt::StarlightGUI::factory_implementation
{
    struct HomePage : public HomePageT<HomePage, implementation::HomePage>
    {
    };
}
