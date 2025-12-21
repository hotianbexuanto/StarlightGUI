#include "pch.h"
#include "HomePage.xaml.h"
#if __has_include("HomePage.g.cpp")
#include "HomePage.g.cpp"
#endif

#include <winrt/Windows.System.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.Web.Http.h>
#include <winrt/Windows.Data.Json.h>
#include <winrt/Windows.System.UserProfile.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Globalization.h>
#include <winrt/Windows.ApplicationModel.h>
#include <winrt/Microsoft.UI.h>
#include <winrt/Microsoft.UI.Windowing.h>
#include <winrt/Microsoft.UI.Dispatching.h>
#include <winrt/Microsoft.UI.Xaml.Media.Imaging.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <random>
#include <chrono>
#include "MainWindow.xaml.h"

using namespace winrt;
using namespace Windows::Web::Http;
using namespace Windows::Data::Json;
using namespace Windows::System;
using namespace Windows::Storage;
using namespace Windows::Foundation;
using namespace Windows::Globalization;
using namespace Windows::ApplicationModel;
using namespace Microsoft::UI::Dispatching;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Microsoft::UI::Xaml::Media::Imaging;

namespace winrt::StarlightGUI::implementation
{
    HomePage::HomePage()
    {
        InitializeComponent();

        SetGreetingText();
        SetUserProfile();
        try {
            FetchHitokoto();
        }
        catch (hresult_error) {
            hitokoto = L"无法加载内容... :(";
        }
        SetupClock();

        LOG_INFO(L"HomePage", L"HomePage initialized.");
    }

    void HomePage::SetGreetingText()
    {
        if (greeting.empty()) {
            std::vector<hstring> greetings = {
                L"欢迎回来",
                L"你好",
                L"Hi",
                L"TimeFormat",
            };
            greeting = greetings[GenerateRandomNumber(0, greetings.size() - 1)];

            DateTime currentDateTime;
            Calendar calendar;
            calendar.SetToNow();
            int currentHour = calendar.Hour();

            if (greeting == L"TimeFormat") {
                if (currentHour < 12)
                {
                    greeting = L"上午好";
                }
                else if (currentHour < 18)
                {
                    greeting = L"下午好";
                }
                else if (currentHour >= 18)
                {
                    greeting = L"晚上好";
                }
            }
        }

        AppIntroduction().Text(L"欢迎使用 Starlight GUI！");
    }

    winrt::fire_and_forget HomePage::SetUserProfile()
    {
        auto weak_this = get_weak();

        if (username.empty() || avatar == nullptr) {
            co_await winrt::resume_background();

            auto users = co_await User::FindAllAsync(UserType::LocalUser, UserAuthenticationStatus::LocallyAuthenticated);

            if (users != nullptr && users.Size() > 0)
            {
                auto user = users.GetAt(0);
                auto picture = co_await user.GetPictureAsync(UserPictureSize::Size64x64);

                if (picture != nullptr)
                {
                    auto stream = co_await picture.OpenReadAsync();

                    if (stream != nullptr)
                    {
                        co_await wil::resume_foreground(DispatcherQueue());

                        BitmapImage bitmapImage;
                        co_await bitmapImage.SetSourceAsync(stream);
                        avatar = bitmapImage;

                        LOG_INFO(__WFUNCTION__, L"Got user account picture.");
                    }
                }

                co_await winrt::resume_background();

                auto displayName = co_await user.GetPropertyAsync(KnownUserProperties::DisplayName());

                if (displayName != nullptr && !displayName.as<winrt::hstring>().empty())
                {
                    username = displayName.as<winrt::hstring>();
                    LOG_INFO(__WFUNCTION__, L"Got user account name.");
                }
            }
        }


        if (auto strong_this = weak_this.get()) {
            co_await wil::resume_foreground(DispatcherQueue());
            UserAvatar().ImageSource(avatar.as<winrt::Microsoft::UI::Xaml::Media::ImageSource>());
            WelcomeText().Text(greeting + L", " + username + L"！");
        }
    }

    winrt::fire_and_forget HomePage::FetchHitokoto()
    {
        auto weak_this = get_weak();

        if (hitokoto.empty()) {
            co_await winrt::resume_background();

            // Async request to get hitokoto text
            HttpClient client;
            Uri uri(L"https://v1.hitokoto.cn/?c=a&c=b&c=c&c=d&c=i&c=k");

            LOG_INFO(__WFUNCTION__, L"Sending hitokoto request...");
            hstring result = co_await client.GetStringAsync(uri);

            // Read json object
            auto json = Windows::Data::Json::JsonObject::Parse(result);
            hitokoto = L"“" + json.GetNamedString(L"hitokoto") + L"”";

            LOG_INFO(__WFUNCTION__, L"Hitokoto result: %s", hitokoto.c_str());
        }

        if (auto strong_this = weak_this.get()) {
            co_await wil::resume_foreground(DispatcherQueue());
            HitokotoText().Text(hitokoto);
        }

        co_return;
    }

    void HomePage::SetupClock()
    {
        // Tick every sec
        m_clockTimer = DispatcherQueue().CreateTimer();
        m_clockTimer.Interval(std::chrono::seconds(1));
        m_clockTimer.Tick({ this, &HomePage::OnClockTick });
        m_clockTimer.Start();

        UpdateClock();
    }

    void HomePage::OnClockTick(IInspectable const& sender, IInspectable const&)
    {
        UpdateClock();
    }

    void HomePage::UpdateClock()
    {
        Calendar calendar;
        calendar.SetToNow();

        int hour = calendar.Hour();
        int minute = calendar.Minute();
        int second = calendar.Second();

        auto splitDigits = [](int value) -> std::pair<hstring, hstring> {
            int digit1 = value / 10;  // 十位
            int digit2 = value % 10;  // 个位
            return { to_hstring(digit1), to_hstring(digit2) };
            };

        auto hourDigits = splitDigits(hour);
        Hour1().Text(hourDigits.first);
        Hour2().Text(hourDigits.second);

        auto minuteDigits = splitDigits(minute);
        Minute1().Text(minuteDigits.first);
        Minute2().Text(minuteDigits.second);

        auto secondDigits = splitDigits(second);
        Second1().Text(secondDigits.first);
        Second2().Text(secondDigits.second);
    }
}