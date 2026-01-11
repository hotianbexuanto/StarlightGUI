#include "pch.h"
#include "CallbackEntry.h"
#if __has_include("CallbackEntry.g.cpp")
#include "CallbackEntry.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml::Media;

namespace winrt::StarlightGUI::implementation
{

    hstring CallbackEntry::Type()
    {
        return m_type;
	}

    void CallbackEntry::Type(hstring const& value)
    {
        m_type = value;
	}

    hstring CallbackEntry::Entry()
    {
        return m_entry;
	}

    void CallbackEntry::Entry(hstring const& value)
    {
        m_entry = value;
	}

    ULONG64 CallbackEntry::EntryULong()
    {
        return m_entryULong;
	}

    void CallbackEntry::EntryULong(ULONG64 value)
    {
        m_entryULong = value;
	}

    hstring CallbackEntry::Handle()
    {
        return m_handle;
	}

    void CallbackEntry::Handle(hstring const& value)
    {
        m_handle = value;
	}

    ULONG64 CallbackEntry::HandleULong()
    {
        return m_handleULong;
	}

    void CallbackEntry::HandleULong(ULONG64 value)
    {
        m_handleULong = value;
	}

    hstring CallbackEntry::Module()
    {
        return m_module;
	}

    void CallbackEntry::Module(hstring const& value)
    {
        m_module = value;
	}
}