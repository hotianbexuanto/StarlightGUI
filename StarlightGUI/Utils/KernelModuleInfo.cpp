#include "pch.h"
#include "KernelModuleInfo.h"
#if __has_include("KernelModuleInfo.g.cpp")
#include "KernelModuleInfo.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml::Media;

namespace winrt::StarlightGUI::implementation
{

    hstring KernelModuleInfo::Name()
    {
        return m_name;
    }

    void KernelModuleInfo::Name(hstring const& value)
    {
        m_name = value;
    }

    hstring KernelModuleInfo::Path() {
        return m_path;
    }

    void KernelModuleInfo::Path(hstring const& value) {
        m_path = value;
    }

    hstring KernelModuleInfo::ImageBase()
    {
        return m_imageBase;
    }

    void KernelModuleInfo::ImageBase(hstring const& value)
    {
        m_imageBase = value;
    }

    ULONG64 KernelModuleInfo::ImageBaseULong()
    {
        return m_imageBaseULong;
    }

    void KernelModuleInfo::ImageBaseULong(ULONG64 const& value)
    {
        m_imageBaseULong = value;
    }

    hstring KernelModuleInfo::Size()
    {
        return m_size;
    }

    void KernelModuleInfo::Size(hstring const& value)
    {
        m_size = value;
    }

    ULONG64 KernelModuleInfo::SizeULong()
    {
        return m_sizeULong;
    }

    void KernelModuleInfo::SizeULong(ULONG64 const& value)
    {
        m_sizeULong = value;
    }

    hstring KernelModuleInfo::DriverObject()
    {
        return m_driverObject;
    }

    void KernelModuleInfo::DriverObject(hstring const& value)
    {
        m_driverObject = value;
    }

    ULONG64 KernelModuleInfo::DriverObjectULong()
    {
        return m_driverObjectULong;
    }

    void KernelModuleInfo::DriverObjectULong(ULONG64 const& value)
    {
        m_driverObjectULong = value;
    }

    hstring KernelModuleInfo::LoadOrder()
    {
        return m_loadOrder;
    }

    void KernelModuleInfo::LoadOrder(hstring const& value)
    {
        m_loadOrder = value;
    }

    ULONG64 KernelModuleInfo::LoadOrderULong()
    {
        return m_loadOrderULong;
    }

    void KernelModuleInfo::LoadOrderULong(ULONG64 const& value)
    {
        m_loadOrderULong = value;
    }
}