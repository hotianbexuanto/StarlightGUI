#include "pch.h"
#include "GridSplitter.h"
#if __has_include("GridSplitter.g.cpp")
#include "GridSplitter.g.cpp"
#endif

#include <winrt/Microsoft.UI.Input.h>
#include <algorithm>
#include <cmath>

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Microsoft::UI::Xaml::Input;
using namespace Microsoft::UI::Xaml::Media;

namespace winrt::StarlightGUI::implementation
{
    namespace
    {
        constexpr double kDragTolerance = 0.5;

        bool IsFixedLength(GridLength const& length)
        {
            return length.GridUnitType != GridUnitType::Auto && length.GridUnitType != GridUnitType::Star;
        }

        bool IsFlexibleLength(GridLength const& length)
        {
            return length.GridUnitType == GridUnitType::Star;
        }
    }

    GridSplitter::GridSplitter()
    {
        // 纯代码控件：不依赖 XAML 模板，只做指针拖拽。
        HorizontalAlignment(HorizontalAlignment::Stretch);
        VerticalAlignment(VerticalAlignment::Stretch);
        Background(SolidColorBrush(Windows::UI::Colors::Transparent()));
        IsTabStop(false);

        auto pressedToken = PointerPressed({ this, &GridSplitter::OnPointerPressed });
        auto movedToken = PointerMoved({ this, &GridSplitter::OnPointerMoved });
        auto releasedToken = PointerReleased({ this, &GridSplitter::OnPointerReleased });
        auto enteredToken = PointerEntered({ this, &GridSplitter::OnPointerEntered });
        auto exitedToken = PointerExited({ this, &GridSplitter::OnPointerExited });
        auto captureLostToken = PointerCaptureLost({ this, &GridSplitter::OnCaptureLost });

        // 避免 C4100 之类的告警：这些 token 的生命周期跟随控件本身。
        (void)pressedToken; (void)movedToken; (void)releasedToken;
        (void)enteredToken; (void)exitedToken; (void)captureLostToken;
    }

    void GridSplitter::ResizeDirection(winrt::StarlightGUI::GridSplitterDirection const& value)
    {
        m_direction = value;
    }

    void GridSplitter::ResizeBehavior(winrt::StarlightGUI::GridSplitterBehavior const& value)
    {
        m_behavior = value;
    }

    void GridSplitter::MinColumnWidth(double value)
    {
        m_minColumnWidth = value < 0.0 ? 0.0 : value;
    }

    double GridSplitter::PointerPosition(Grid const& grid, PointerRoutedEventArgs const& e) const
    {
        auto point = e.GetCurrentPoint(grid).Position();
        return IsHorizontal() ? point.X : point.Y;
    }

    double GridSplitter::PixelLength(Grid const& grid, int index) const
    {
        if (!grid || index < 0) return 0.0;

        if (IsHorizontal()) {
            auto definitions = grid.ColumnDefinitions();
            if (index >= (int)definitions.Size()) return 0.0;

            auto columnDef = definitions.GetAt(index);
            auto width = columnDef.Width();
            if (IsFixedLength(width)) return width.Value;

            if (IsFlexibleLength(width)) {
                double starTotal = 0.0;
                double fixedTotal = 0.0;
                for (auto const& definition : definitions) {
                    auto definitionWidth = definition.Width();
                    if (IsFlexibleLength(definitionWidth)) starTotal += definitionWidth.Value;
                    else if (IsFixedLength(definitionWidth)) fixedTotal += definitionWidth.Value;
                }

                double available = grid.ActualWidth() - fixedTotal;
                if (available < 0.0) available = 0.0;
                if (starTotal <= 0.0) return columnDef.ActualWidth();
                return available * (width.Value / starTotal);
            }

            return columnDef.ActualWidth();
        }

        auto rowDefinitions = grid.RowDefinitions();
        if (index >= (int)rowDefinitions.Size()) return 0.0;

        auto rowDef = rowDefinitions.GetAt(index);
        auto height = rowDef.Height();
        if (IsFixedLength(height)) return height.Value;

        if (IsFlexibleLength(height)) {
            double starTotal = 0.0;
            double fixedTotal = 0.0;
            for (auto const& definition : rowDefinitions) {
                auto definitionHeight = definition.Height();
                if (IsFlexibleLength(definitionHeight)) starTotal += definitionHeight.Value;
                else if (IsFixedLength(definitionHeight)) fixedTotal += definitionHeight.Value;
            }

            double available = grid.ActualHeight() - fixedTotal;
            if (available < 0.0) available = 0.0;
            if (starTotal <= 0.0) return rowDef.ActualHeight();
            return available * (height.Value / starTotal);
        }

        return rowDef.ActualHeight();
    }

    GridLength GridSplitter::MakeLength(Grid const& grid, int index, double pixels) const
    {
        GridUnitType unitType = GridUnitType::Pixel;

        if (IsHorizontal()) {
            unitType = grid.ColumnDefinitions().GetAt(index).Width().GridUnitType;
        }
        else {
            unitType = grid.RowDefinitions().GetAt(index).Height().GridUnitType;
        }

        if (unitType == GridUnitType::Star) {
            return GridLengthHelper::FromValueAndType(pixels, GridUnitType::Star);
        }
        return GridLengthHelper::FromPixels(pixels);
    }

    void GridSplitter::OnPointerEntered(IInspectable const&, PointerRoutedEventArgs const&)
    {
        SetResizeCursor(true);
    }

    void GridSplitter::OnPointerExited(IInspectable const&, PointerRoutedEventArgs const&)
    {
        SetResizeCursor(false);
    }

    void GridSplitter::OnCaptureLost(IInspectable const&, PointerRoutedEventArgs const&)
    {
        EndDrag();
    }

    void GridSplitter::OnPointerPressed(IInspectable const&, PointerRoutedEventArgs const& e)
    {
        if (m_dragging) return;

        auto grid = Parent().try_as<Grid>();
        if (!grid) return;

        // 分隔条自身所在的行/列：列方向看 Grid.Column，行方向看 Grid.Row。
        int index = IsHorizontal() ? Grid::GetColumn(*this) : Grid::GetRow(*this);
        int count = IsHorizontal() ? (int)grid.ColumnDefinitions().Size() : (int)grid.RowDefinitions().Size();
        if (count < 2) return;
        if (index < 0 || index >= count) return;

        // BasedOnAlignment 与原实现一致：拖拽当前列/行与其右侧/下方的那一列/行。
        int current = index;
        int next = index + 1;
        if (next >= count) {
            current = index - 1;
            next = index;
        }
        if (current < 0 || next >= count) return;

        m_previousIndex = current;
        m_nextIndex = next;
        m_startLengths[0] = PixelLength(grid, current);
        m_startLengths[1] = PixelLength(grid, next);
        m_dragStart = PointerPosition(grid, e);
        m_dragging = true;

        CapturePointer(e.Pointer());
        e.Handled(true);
    }

    void GridSplitter::OnPointerMoved(IInspectable const&, PointerRoutedEventArgs const& e)
    {
        if (!m_dragging) return;

        auto grid = Parent().try_as<Grid>();
        if (!grid) { EndDrag(); return; }

        int count = IsHorizontal() ? (int)grid.ColumnDefinitions().Size() : (int)grid.RowDefinitions().Size();
        if (m_previousIndex < 0 || m_nextIndex >= count) { EndDrag(); return; }

        double delta = PointerPosition(grid, e) - m_dragStart;
        if (std::abs(delta) < kDragTolerance) { e.Handled(true); return; }

        auto previousDef = IsHorizontal()
            ? grid.ColumnDefinitions().GetAt(m_previousIndex).Width()
            : grid.RowDefinitions().GetAt(m_previousIndex).Height();
        auto nextDef = IsHorizontal()
            ? grid.ColumnDefinitions().GetAt(m_nextIndex).Width()
            : grid.RowDefinitions().GetAt(m_nextIndex).Height();

        double previousPixels = m_startLengths[0] + delta;
        double nextPixels = m_startLengths[1] - delta;

        if (IsFixedLength(previousDef) || IsFixedLength(nextDef)) {
            double minWidth = m_minColumnWidth;
            if (previousPixels < minWidth || nextPixels < minWidth) return;
        }
        else {
            if (previousPixels < 0.0 || nextPixels < 0.0) return;
        }

        auto newPrevious = MakeLength(grid, m_previousIndex, previousPixels);
        auto newNext = MakeLength(grid, m_nextIndex, nextPixels);

        if (IsHorizontal()) {
            grid.ColumnDefinitions().GetAt(m_previousIndex).Width(newPrevious);
            grid.ColumnDefinitions().GetAt(m_nextIndex).Width(newNext);
        }
        else {
            grid.RowDefinitions().GetAt(m_previousIndex).Height(newPrevious);
            grid.RowDefinitions().GetAt(m_nextIndex).Height(newNext);
        }

        e.Handled(true);
    }

    void GridSplitter::OnPointerReleased(IInspectable const&, PointerRoutedEventArgs const& e)
    {
        if (!m_dragging) return;

        ReleasePointerCapture(e.Pointer());
        EndDrag();
        e.Handled(true);
    }

    void GridSplitter::EndDrag()
    {
        m_dragging = false;
        m_previousIndex = -1;
        m_nextIndex = -1;
    }

    void GridSplitter::SetResizeCursor(bool active) noexcept
    {
        try {
            using Microsoft::UI::Input::InputSystemCursorShape;
            auto shape = active
                ? (IsHorizontal() ? InputSystemCursorShape::SizeWestEast : InputSystemCursorShape::SizeNorthSouth)
                : InputSystemCursorShape::Arrow;
            ProtectedCursor(Microsoft::UI::Input::InputSystemCursor::Create(shape));
        }
        catch (...) {
            // 该 API 在少数环境下不可用，忽略即可。
        }
    }
}
