#pragma once

#include "CallbackEntry.g.h"

namespace winrt::StarlightGUI::implementation
{
    struct CallbackEntry : CallbackEntryT<CallbackEntry>
    {
		CallbackEntry() = default;

		hstring Type();
		void Type(hstring const& value);

		hstring Entry();
		void Entry(hstring const& value);

		ULONG64 EntryULong();
		void EntryULong(ULONG64 value);

		hstring Handle();
		void Handle(hstring const& value);

		ULONG64 HandleULong();
		void HandleULong(ULONG64 value);

		hstring Module();
		void Module(hstring const& value);

    private:
        hstring m_type{ L"" };
		hstring m_entry{ L"" };
		ULONG64 m_entryULong{ 0 };
		hstring m_handle{ L"" };
		ULONG64 m_handleULong{ 0 };
		hstring m_module{ L"" };
    };
}

namespace winrt::StarlightGUI::factory_implementation
{
    struct CallbackEntry : CallbackEntryT<CallbackEntry, implementation::CallbackEntry>
    {
    };
}