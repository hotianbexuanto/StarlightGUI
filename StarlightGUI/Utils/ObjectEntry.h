#pragma once

#include "ObjectEntry.g.h"

namespace winrt::StarlightGUI::implementation
{
    struct ObjectEntry : ObjectEntryT<ObjectEntry>
    {
        ObjectEntry() = default;

        hstring Name();
        void Name(hstring const& value);

        hstring Path();
        void Path(hstring const& value);

		hstring Type();
		void Type(hstring const& value);

        bool Permanent();
        void Permanent(bool value);

        ULONG References();
        void References(ULONG value);

        ULONG Handles();
        void Handles(ULONG value);

        ULONG PagedPool();
        void PagedPool(ULONG value);

        ULONG NonPagedPool();
        void NonPagedPool(ULONG value);

        hstring CreationTime();
        void CreationTime(hstring const& value);

        hstring Link();
		void Link(hstring const& value);

		hstring EventType();
		void EventType(hstring const& value);

		bool EventSignaled();
		void EventSignaled(bool value);

		ULONG MutantHoldCount();
		void MutantHoldCount(ULONG value);

		bool MutantAbandoned();
		void MutantAbandoned(bool value);

		ULONG SemaphoreCount();
		void SemaphoreCount(ULONG value);

		ULONG SemaphoreLimit();
		void SemaphoreLimit(ULONG value);

		ULONG64 SectionBaseAddress();
		void SectionBaseAddress(ULONG64 value);

		ULONG64 SectionMaximumSize();
		void SectionMaximumSize(ULONG64 value);

		ULONG SectionAttributes();
		void SectionAttributes(ULONG value);

		ULONG64 TimerRemainingTime();
		void TimerRemainingTime(ULONG64 value);

		bool TimerState();
		void TimerState(bool value);

		LONG IoCompletionDepth();
		void IoCompletionDepth(LONG value);

    private:
        hstring m_name{ L"" };
        hstring m_path{ L"" };
        hstring m_type{ L"" };
        bool m_permanent{ false };
        ULONG m_references{ 0 };
        ULONG m_handles{ 0 };
        ULONG m_pagedPool{ 0 };
        ULONG m_nonPagedPool{ 0 };
        hstring m_creationTime{ L"" };
        hstring m_link{ L"" };
		hstring m_eventType{ L"" };
		bool m_eventSignaled{ false };
		ULONG m_mutantHoldCount{ 0 };
		bool m_mutantAbandoned{ false };
		ULONG m_semaphoreCount{ 0 };
		ULONG m_semaphoreLimit{ 0 };
		ULONG64 m_sectionBaseAddress{ 0 };
		ULONG64 m_sectionMaximumSize{ 0 };
		ULONG m_sectionAttributes{ 0 };
		ULONG64 m_timerRemainingTime{ 0 };
		bool m_timerState{ false };
		LONG m_ioCompletionDepth{ 0 };
    };
}

namespace winrt::StarlightGUI::factory_implementation
{
    struct ObjectEntry : ObjectEntryT<ObjectEntry, implementation::ObjectEntry>
    {
    };
}