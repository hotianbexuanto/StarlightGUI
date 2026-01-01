#pragma once

#include "KernelModuleInfo.g.h"

namespace winrt::StarlightGUI::implementation
{
    struct KernelModuleInfo : KernelModuleInfoT<KernelModuleInfo>
    {
        KernelModuleInfo() = default;

        hstring Name();
        void Name(hstring const& value);

        hstring Path();
        void Path(hstring const& value);

        hstring ImageBase();
        void ImageBase(hstring const& value);

        ULONG64 ImageBaseULong();
        void ImageBaseULong(ULONG64 const& value);

        hstring Size();
        void Size(hstring const& value);

        ULONG64 SizeULong();
        void SizeULong(ULONG64 const& value);

        hstring DriverObject();
        void DriverObject(hstring const& value);

        ULONG64 DriverObjectULong();
        void DriverObjectULong(ULONG64 const& value);

        hstring LoadOrder();
        void LoadOrder(hstring const& value);

        ULONG64 LoadOrderULong();
        void LoadOrderULong(ULONG64 const& value);

    private:
        hstring m_name{ L"" };
        hstring m_path{ L"" };
        hstring m_imageBase{ L"" };
        ULONG64 m_imageBaseULong{ 0 };
        hstring m_size{ L"" };
        ULONG64 m_sizeULong{ 0 };
        hstring m_driverObject{ L"" };
        ULONG64 m_driverObjectULong{ 0 };
        hstring m_loadOrder{ L"" };
        ULONG64 m_loadOrderULong{ 0 };
    };
}

namespace winrt::StarlightGUI::factory_implementation
{
    struct KernelModuleInfo : KernelModuleInfoT<KernelModuleInfo, implementation::KernelModuleInfo>
    {
    };
}