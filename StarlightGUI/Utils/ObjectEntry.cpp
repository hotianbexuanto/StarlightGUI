#include "pch.h"
#include "ObjectEntry.h"
#if __has_include("ObjectEntry.g.cpp")
#include "ObjectEntry.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml::Media;

namespace winrt::StarlightGUI::implementation
{

    hstring ObjectEntry::Name()
    {
        return m_name;
    }

    void ObjectEntry::Name(hstring const& value)
    {
        m_name = value;
    }

    hstring ObjectEntry::Path()
    {
        return m_path;
    }

    void ObjectEntry::Path(hstring const& value)
    {
        m_path = value;
    }

    hstring ObjectEntry::Type()
    {
        return m_type;
	}

    void ObjectEntry::Type(hstring const& value)
    {
        m_type = value;
	}

    bool ObjectEntry::Permanent()
    {
        return m_permanent;
	}

    void ObjectEntry::Permanent(bool value)
    {
        m_permanent = value;
	}

    ULONG ObjectEntry::References()
    {
        return m_references;
	}

    void ObjectEntry::References(ULONG value)
    {
        m_references = value;
	}

    ULONG ObjectEntry::Handles()
    {
        return m_handles;
	}

    void ObjectEntry::Handles(ULONG value)
    {
        m_handles = value;
	}

    ULONG ObjectEntry::PagedPool()
    {
        return m_pagedPool;
	}

    void ObjectEntry::PagedPool(ULONG value)
    {
        m_pagedPool = value;
	}

    ULONG ObjectEntry::NonPagedPool()
    {
        return m_nonPagedPool;
	}

    void ObjectEntry::NonPagedPool(ULONG value)
    {
        m_nonPagedPool = value;
	}

    hstring ObjectEntry::CreationTime()
    {
        return m_creationTime;
	}

    void ObjectEntry::CreationTime(hstring const& value)
    {
        m_creationTime = value;
	}

    hstring ObjectEntry::Link()
    {
        return m_link;
	}

    void ObjectEntry::Link(hstring const& value)
    {
        m_link = value;
	}

    hstring ObjectEntry::EventType()
    {
        return m_eventType;
    }

    void ObjectEntry::EventType(hstring const& value)
    {
        m_eventType = value;
	}

    bool ObjectEntry::EventSignaled()
    {
        return m_eventSignaled;
	}

    void ObjectEntry::EventSignaled(bool value)
    {
        m_eventSignaled = value;
	}

    ULONG ObjectEntry::MutantHoldCount()
    {
        return m_mutantHoldCount;
    }

    void ObjectEntry::MutantHoldCount(ULONG value)
    {
        m_mutantHoldCount = value;
	}

    bool ObjectEntry::MutantAbandoned()
    {
        return m_mutantAbandoned;
    }

    void ObjectEntry::MutantAbandoned(bool value)
    {
        m_mutantAbandoned = value;
    }

    ULONG ObjectEntry::SemaphoreCount()
    {
        return m_semaphoreCount;
	}

    void ObjectEntry::SemaphoreCount(ULONG value)
    {
        m_semaphoreCount = value;
	}

    ULONG ObjectEntry::SemaphoreLimit()
    {
        return m_semaphoreLimit;
	}

    void ObjectEntry::SemaphoreLimit(ULONG value)
    {
        m_semaphoreLimit = value;
	}

    ULONG64 ObjectEntry::SectionBaseAddress()
    {
        return m_sectionBaseAddress;
	}

    void ObjectEntry::SectionBaseAddress(ULONG64 value)
    {
        m_sectionBaseAddress = value;
	}

    ULONG64 ObjectEntry::SectionMaximumSize()
    {
        return m_sectionMaximumSize;
	}
    void ObjectEntry::SectionMaximumSize(ULONG64 value)
    {
        m_sectionMaximumSize = value;
	}

    ULONG ObjectEntry::SectionAttributes()
    {
        return m_sectionAttributes;
	}

    void ObjectEntry::SectionAttributes(ULONG value)
    {
        m_sectionAttributes = value;
	}

    ULONG64 ObjectEntry::TimerRemainingTime()
    {
        return m_timerRemainingTime;
	}

    void ObjectEntry::TimerRemainingTime(ULONG64 value)
    {
        m_timerRemainingTime = value;
	}

    bool ObjectEntry::TimerState()
    {
        return m_timerState;
	}

    void ObjectEntry::TimerState(bool value)
    {
        m_timerState = value;
	}

    LONG ObjectEntry::IoCompletionDepth()
    {
        return m_ioCompletionDepth;
	}

    void ObjectEntry::IoCompletionDepth(LONG value)
    {
        m_ioCompletionDepth = value;
	}
}