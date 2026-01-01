#pragma once

#include "KCTInfo.g.h"

namespace winrt::StarlightGUI::implementation
{
    struct KCTInfo : KCTInfoT<KCTInfo>
    {
        KCTInfo() = default;

        hstring Name();
        void Name(hstring const& value);

        hstring Address();
        void Address(hstring const& value);

    private:
        hstring m_name{ L"" };
        hstring m_address{ L"" };
    };
}

namespace winrt::StarlightGUI::factory_implementation
{
    struct KCTInfo : KCTInfoT<KCTInfo, implementation::KCTInfo>
    {
    };
}