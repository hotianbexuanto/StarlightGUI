#pragma once

#include "UpdateDialog.g.h"

namespace winrt::StarlightGUI::implementation
{
    struct UpdateDialog : UpdateDialogT<UpdateDialog>
    {
        UpdateDialog();


        hstring LatestVersion() const { return m_latestVersion; }
        void LatestVersion(const hstring& value) { m_latestVersion = value; }

        void OnPrimaryButtonClick(winrt::Microsoft::UI::Xaml::Controls::ContentDialog const& sender,
            winrt::Microsoft::UI::Xaml::Controls::ContentDialogButtonClickEventArgs const& args);

    private:
        hstring m_latestVersion{ L"" };
    };
}

namespace winrt::StarlightGUI::factory_implementation
{
    struct UpdateDialog : UpdateDialogT<UpdateDialog, implementation::UpdateDialog>
    {
    };
}
