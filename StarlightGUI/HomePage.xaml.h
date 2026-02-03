#pragma once

#include "HomePage.g.h"
#include <pdh.h>
#include <nvidia/nvml.h>

namespace winrt::StarlightGUI::implementation
{
    struct HomePage : public HomePageT<HomePage>
    {
        HomePage();

        slg::coroutine SetGreetingText();
        slg::coroutine SetUserProfile();
        slg::coroutine FetchHitokoto();
        slg::coroutine SetupClock();
        slg::coroutine OnClockTick(IInspectable const&, IInspectable const&);
        slg::coroutine UpdateClock();

        slg::coroutine UpdateGauges();

        winrt::Microsoft::UI::Xaml::DispatcherTimer clockTimer;

        inline static bool infoInitialized;
        inline static hstring greeting;
        inline static hstring username;
        inline static hstring hitokoto;
        inline static winrt::Microsoft::UI::Xaml::Media::Imaging::BitmapImage avatar{ nullptr };

        // 性能显示
        inline static std::unordered_map<int, hstring> adpt_name_map;
        inline static hstring cpu_manufacture = L"", disk_manufacture = L"", gpu_manufacture = L"", netadpt_manufacture = L"";
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
