#include "pch.h"
#include "SettingsPage.xaml.h"
#include "Utils/Config.h"
#if __has_include("SettingsPage.g.cpp")
#include "SettingsPage.g.cpp"
#endif
#include "MainWindow.xaml.h"
#include "InfoWindow.xaml.h"

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;

namespace winrt::StarlightGUI::implementation
{
    static bool loaded;
    static std::string background_type;
    static std::string mica_type;
    static std::string acrylic_type;
    static bool dangerous_confirm;
    static bool elevator_full_privileges;
    static bool bypass_signature;
    static std::string navigation_style;

    SettingsPage::SettingsPage()
    {
        InitializeComponent();

        loaded = false;

        // idk why it cant understand the fucking boolean
        background_type = ReadConfig("background_type", "Static");
        mica_type = ReadConfig("mica_type", "BaseAlt");
        acrylic_type = ReadConfig("acrylic_type", "Default");
        dangerous_confirm = ReadConfig("dangerous_confirm", true);
        elevator_full_privileges = ReadConfig("elevator_full_privileges", true);
        bypass_signature = ReadConfig("bypass_signature", false);
        navigation_style = ReadConfig("navigation_style", "LeftCompact");

        InitializeOptions();

        loaded = true;
    }

    void SettingsPage::InitializeOptions() {
        if (background_type == "Mica") {
            BackgroundComboBox().SelectedIndex(1);
        }
        else if (background_type == "Acrylic") {
            BackgroundComboBox().SelectedIndex(2);
        }
        else {
            BackgroundComboBox().SelectedIndex(0);
        }

        if (navigation_style == "Left") {
            NavigationComboBox().SelectedIndex(1);
        }
        else if (navigation_style == "Top") {
            NavigationComboBox().SelectedIndex(2);
        }
        else {
            NavigationComboBox().SelectedIndex(0);
        }

        if (mica_type == "Base") {
            MicaTypeComboBox().SelectedIndex(1);
        }
        else {
            MicaTypeComboBox().SelectedIndex(0);
        }

        if (acrylic_type == "Base") {
            AcrylicTypeComboBox().SelectedIndex(1);
        }
        else if (acrylic_type == "Thin") {
            AcrylicTypeComboBox().SelectedIndex(2);
        }
        else {
            AcrylicTypeComboBox().SelectedIndex(0);
        }

        DangerousConfirmButton().IsOn(dangerous_confirm);
        FullPrivilegesButton().IsOn(elevator_full_privileges);
        BypassSignatureButton().IsOn(bypass_signature);
    }

    void SettingsPage::BackgroundComboBox_SelectionChanged(IInspectable const& sender, SelectionChangedEventArgs const& e)
    {
        if (!loaded) return;

        if (BackgroundComboBox().SelectedIndex() == 0)
        {
            background_type = "Static";
        }
        else if (BackgroundComboBox().SelectedIndex() == 1)
        {
            background_type = "Mica";
        }
        else if (BackgroundComboBox().SelectedIndex() == 2)
        {
            background_type = "Acrylic";
        }
        SaveConfig("background_type", background_type);

        g_mainWindowInstance->LoadBackdrop();
    }

    void SettingsPage::MicaTypeComboBox_SelectionChanged(IInspectable const& sender, SelectionChangedEventArgs const& e)
    {
        if (!loaded) return;

        if (MicaTypeComboBox().SelectedIndex() == 0)
        {
            mica_type = "Base";
        }
        else
        {
            mica_type = "BaseAlt";
        }

        SaveConfig("mica_type", mica_type);

        g_mainWindowInstance->LoadBackdrop();
    }

    void SettingsPage::AcrylicTypeComboBox_SelectionChanged(IInspectable const& sender, SelectionChangedEventArgs const& e)
    {
        if (!loaded) return;

        if (AcrylicTypeComboBox().SelectedIndex() == 1)
        {
            acrylic_type = "Base";
        }
        else if (AcrylicTypeComboBox().SelectedIndex() == 2) 
        {
            acrylic_type = "Thin";
        }
        else
        {
            acrylic_type = "Default";
        }

        SaveConfig("acrylic_type", acrylic_type);

        g_mainWindowInstance->LoadBackdrop();
    }
    
    void SettingsPage::NavigationComboBox_SelectionChanged(IInspectable const& sender, SelectionChangedEventArgs const& e)
    {
        if (!loaded) return;

        if (NavigationComboBox().SelectedIndex() == 0)
        {
            navigation_style = "LeftCompact";

            g_mainWindowInstance->RootNavigation().PaneDisplayMode(NavigationViewPaneDisplayMode::LeftCompact);
        }
        else if (NavigationComboBox().SelectedIndex() == 1)
        {
            navigation_style = "Left";

            g_mainWindowInstance->RootNavigation().PaneDisplayMode(NavigationViewPaneDisplayMode::Left);
        }
        else if (NavigationComboBox().SelectedIndex() == 2)
        {
            navigation_style = "Top";

            g_mainWindowInstance->RootNavigation().PaneDisplayMode(NavigationViewPaneDisplayMode::Top);
        }
        SaveConfig("navigation_style", navigation_style);
    }

    void SettingsPage::DangerousConfirmButton_Toggled(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
    {
        dangerous_confirm = DangerousConfirmButton().IsOn();
        SaveConfig("dangerous_confirm", dangerous_confirm);
    }

    void SettingsPage::FullPrivilegesButton_Toggled(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
    {
        elevator_full_privileges = FullPrivilegesButton().IsOn();
        SaveConfig("elevator_full_privileges", elevator_full_privileges);
    }

    void SettingsPage::BypassSignatureButton_Toggled(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
    {
        bypass_signature = BypassSignatureButton().IsOn();
        SaveConfig("bypass_signature", bypass_signature);
    }
}
