#pragma once

#include "GridSplitter.g.h"

namespace winrt::StarlightGUI::implementation
{
    // 原生 WinUI3 列/行分隔条，用于替代不受支持的 XamlToolkit GridSplitter。
    // 用法与原版本一致：作为 Grid 的直接子元素，并设置 Grid.Column（列）或 Grid.Row（行）。
    struct GridSplitter : GridSplitterT<GridSplitter>
    {
        GridSplitter();

        winrt::StarlightGUI::GridSplitterDirection ResizeDirection() const { return m_direction; }
        void ResizeDirection(winrt::StarlightGUI::GridSplitterDirection const& value);

        winrt::StarlightGUI::GridSplitterBehavior ResizeBehavior() const { return m_behavior; }
        void ResizeBehavior(winrt::StarlightGUI::GridSplitterBehavior const& value);

        double MinColumnWidth() const { return m_minColumnWidth; }
        void MinColumnWidth(double value);

    private:
        void OnPointerPressed(winrt::Windows::Foundation::IInspectable const& sender,
            winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);
        void OnPointerMoved(winrt::Windows::Foundation::IInspectable const& sender,
            winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);
        void OnPointerReleased(winrt::Windows::Foundation::IInspectable const& sender,
            winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);
        void OnPointerEntered(winrt::Windows::Foundation::IInspectable const& sender,
            winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);
        void OnPointerExited(winrt::Windows::Foundation::IInspectable const& sender,
            winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);
        void OnCaptureLost(winrt::Windows::Foundation::IInspectable const& sender,
            winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);

        void EndDrag();
        void SetResizeCursor(bool active) noexcept;
        bool IsHorizontal() const { return m_direction == winrt::StarlightGUI::GridSplitterDirection::Columns; }
        double PointerPosition(winrt::Microsoft::UI::Xaml::Controls::Grid const& grid,
            winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e) const;
        double PixelLength(winrt::Microsoft::UI::Xaml::Controls::Grid const& grid, int index) const;
        winrt::Microsoft::UI::Xaml::GridLength MakeLength(
            winrt::Microsoft::UI::Xaml::Controls::Grid const& grid, int index, double pixels) const;

        winrt::StarlightGUI::GridSplitterDirection m_direction{ winrt::StarlightGUI::GridSplitterDirection::Columns };
        winrt::StarlightGUI::GridSplitterBehavior m_behavior{ winrt::StarlightGUI::GridSplitterBehavior::BasedOnAlignment };
        double m_minColumnWidth{ 48.0 };

        bool m_dragging{ false };
        double m_dragStart{ 0.0 };
        double m_startLengths[2]{ 0.0, 0.0 };
        int m_previousIndex{ -1 };
        int m_nextIndex{ -1 };
    };
}

namespace winrt::StarlightGUI::factory_implementation
{
    struct GridSplitter : GridSplitterT<GridSplitter, implementation::GridSplitter>
    {
    };
}
