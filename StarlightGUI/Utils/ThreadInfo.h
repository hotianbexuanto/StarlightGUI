#pragma once

#include "ThreadInfo.g.h"

namespace winrt::StarlightGUI::implementation
{
	struct ThreadInfo : ThreadInfoT<ThreadInfo>
	{
		ThreadInfo() = default;

		int32_t Id() { return m_id; }
		void Id(int32_t value) { m_id = value; }

		hstring EThread() { return m_ethread; }
		void EThread(hstring const& value) { m_ethread = value; }

		hstring Address() { return m_address; }
		void Address(hstring const& value) { m_address = value; }

		hstring Status() { return m_status; }
		void Status(hstring const& value) { m_status = value; }

		int32_t Priority() { return m_priority; }
		void Priority(int32_t value) { m_priority = value; }

		hstring ModuleInfo() { return m_module; }
		void ModuleInfo(hstring const& value) { m_module = value; }

	private:
		int32_t m_id{ 0 };
		hstring m_ethread{ L"" };
		hstring m_address{ L"" };
		hstring m_status{ L"" };
		int32_t m_priority{ 0 };
		hstring m_module{ L"" };
	};
}

namespace winrt::StarlightGUI::factory_implementation
{
	struct ThreadInfo : ThreadInfoT<ThreadInfo, implementation::ThreadInfo>
	{
	};
}