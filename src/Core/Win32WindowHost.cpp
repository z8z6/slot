#include "Core/Win32WindowHost.h"

#include <WindowsX.h>
#include <cmath>

using namespace z8;

Win32WindowHost::~Win32WindowHost() { DetachWindow(); }

void Win32WindowHost::AttachWindow(HWND window, EventTarget &inputTarget,
                                   HWND coordinateWindow) {
  DetachWindow();
  AttachedWindow = window;
  InputTarget = &inputTarget;
  InputCoordinateWindow = coordinateWindow ? coordinateWindow : window;
  HasPointerPosition = false;
  if (!AttachedWindow)
    return;
  SetWindowLongPtrW(AttachedWindow, GWLP_USERDATA,
                    reinterpret_cast<LONG_PTR>(this));
  SetWindowLongPtrW(AttachedWindow, GWLP_WNDPROC,
                    reinterpret_cast<LONG_PTR>(WindowProcedure));
}

void Win32WindowHost::DetachWindow() {
  if (AttachedWindow && IsWindow(AttachedWindow) &&
      reinterpret_cast<Win32WindowHost *>(
          GetWindowLongPtrW(AttachedWindow, GWLP_USERDATA)) == this)
    SetWindowLongPtrW(AttachedWindow, GWLP_USERDATA, 0);
  AttachedWindow = nullptr;
  InputCoordinateWindow = nullptr;
  InputTarget = nullptr;
  HasPointerPosition = false;
}

bool Win32WindowHost::DispatchInputMessage(HWND window, UINT message,
                                           WPARAM wParam, LPARAM lParam,
                                           LRESULT &result) {
  if (!InputTarget)
    return false;
  switch (message) {
  case WM_SETCURSOR:
    if (LOWORD(lParam) != HTCLIENT)
      return false;
    if (POINT point{}; GetCursorPos(&point)) {
      ScreenToClient(InputCoordinateWindow, &point);
      const float scale = ResolveCoordinateScale();
      MouseMovArgs args;
      args.X = static_cast<int>(std::lround(point.x / scale));
      args.Y = static_cast<int>(std::lround(point.y / scale));
      // 使用系统 DPI 感知光标；UI 只提供平台无关语义，避免主/浮动窗口
      // 分别维护一份易漂移的 IDC_* 映射。
      SetCursor(LoadCursorW(nullptr,
                            ResolveSystemCursor(InputTarget->GetMouseCursor(
                                args))));
      result = TRUE;
      return true;
    }
    return false;
  case WM_LBUTTONDOWN:
  case WM_MBUTTONDOWN:
  case WM_RBUTTONDOWN:
  case WM_XBUTTONDOWN: {
    SetCapture(window);
    const auto args = ResolveMouseArgs(window, message, wParam, lParam);
    InputTarget->OnMouseDown(args);
    result = 0;
    return true;
  }
  case WM_MOUSEMOVE: {
    const auto args = ResolveMouseArgs(window, message, wParam, lParam);
    const bool dragging =
        (args.State & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON | MK_XBUTTON1 |
                       MK_XBUTTON2)) != 0;
    if (dragging)
      InputTarget->OnMouseDrag(args);
    else
      InputTarget->OnMouseMove(args);
    result = 0;
    return true;
  }
  case WM_LBUTTONUP:
  case WM_MBUTTONUP:
  case WM_RBUTTONUP:
  case WM_XBUTTONUP: {
    const auto args = ResolveMouseArgs(window, message, wParam, lParam);
    InputTarget->OnMouseUp(args);
    if ((args.State & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON | MK_XBUTTON1 |
                       MK_XBUTTON2)) == 0 &&
        GetCapture() == window) {
      // ReleaseCapture 会同步产生 WM_CAPTURECHANGED；正常完成的 MouseUp 已经
      // 结束手势，不能再把它误报为 capture-lost 并关闭刚打开的 Menu。
      ReleasingCapture = true;
      ReleaseCapture();
      ReleasingCapture = false;
    }
    result = 0;
    return true;
  }
  case WM_MOUSEWHEEL: {
    POINT point{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
    ScreenToClient(InputCoordinateWindow, &point);
    const float scale = ResolveCoordinateScale();
    InputTarget->OnMouseWheel(
        {static_cast<unsigned>(GET_KEYSTATE_WPARAM(wParam)),
         static_cast<int>(std::lround(point.x / scale)),
         static_cast<int>(std::lround(point.y / scale)),
         GET_WHEEL_DELTA_WPARAM(wParam)});
    result = 0;
    return true;
  }
  case WM_CAPTURECHANGED:
    if (!ReleasingCapture && reinterpret_cast<HWND>(lParam) != window) {
      HasPointerPosition = false;
      InputTarget->OnPointerCaptureLost();
    }
    result = 0;
    return true;
  case WM_KEYDOWN:
    InputTarget->OnKeyDown(KeyArgs(wParam, lParam));
    result = 0;
    return true;
  case WM_KEYUP:
    InputTarget->OnKeyUp(KeyArgs(wParam, lParam));
    result = 0;
    return true;
  case WM_CHAR:
    InputTarget->OnTextInput(static_cast<wchar_t>(wParam));
    result = 0;
    return true;
  case WM_SYSKEYDOWN:
    // Alt/F10 只用于切换系统菜单时会触发蜂鸣；编辑器没有原生菜单栏，其他
    // system key 仍作为普通键送入统一事件目标。
    if (wParam != VK_MENU && wParam != VK_F10)
      InputTarget->OnKeyDown(KeyArgs(wParam, lParam));
    result = 0;
    return true;
  case WM_SYSKEYUP:
    if (wParam != VK_MENU)
      InputTarget->OnKeyUp(KeyArgs(wParam, lParam));
    result = 0;
    return true;
  case WM_MENUCHAR:
    result = MAKELRESULT(0, MNC_CLOSE);
    return true;
  case WM_SYSCOMMAND:
    if ((wParam & 0xFFF0) == SC_KEYMENU) {
      result = 0;
      return true;
    }
    return false;
  default:
    return false;
  }
}

MouseButton Win32WindowHost::ResolveMouseButton(UINT message,
                                                WPARAM wParam) {
  switch (message) {
  case WM_LBUTTONDOWN:
  case WM_LBUTTONUP:
    return MouseButton::Left;
  case WM_MBUTTONDOWN:
  case WM_MBUTTONUP:
    return MouseButton::Middle;
  case WM_RBUTTONDOWN:
  case WM_RBUTTONUP:
    return MouseButton::Right;
  case WM_XBUTTONDOWN:
  case WM_XBUTTONUP:
    return GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? MouseButton::X1
                                                  : MouseButton::X2;
  default:
    return MouseButton::None;
  }
}

MouseMovArgs Win32WindowHost::ResolveMouseArgs(HWND sourceWindow,
                                               UINT message, WPARAM wParam,
                                               LPARAM lParam) {
  POINT point{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
  if (sourceWindow != InputCoordinateWindow) {
    ClientToScreen(sourceWindow, &point);
    ScreenToClient(InputCoordinateWindow, &point);
  }
  const float scale = ResolveCoordinateScale();
  POINT logicalPoint{static_cast<LONG>(std::lround(point.x / scale)),
                     static_cast<LONG>(std::lround(point.y / scale))};
  MouseMovArgs args;
  args.State = static_cast<unsigned>(GET_KEYSTATE_WPARAM(wParam));
  args.X = logicalPoint.x;
  args.Y = logicalPoint.y;
  args.DeltaX = HasPointerPosition ? logicalPoint.x - LastPointer.x : 0;
  args.DeltaY = HasPointerPosition ? logicalPoint.y - LastPointer.y : 0;
  args.Button = ResolveMouseButton(message, wParam);
  LastPointer = logicalPoint;
  HasPointerPosition = true;
  return args;
}

float Win32WindowHost::ResolveCoordinateScale() const {
  if (!InputCoordinateWindow)
    return 1.0f;
  // 所有事件最终进入主 Layout 坐标系；即使消息来自 Floating HWND，也按
  // coordinateWindow 的 DPI 还原为同一套 96 DPI 逻辑像素。
  const UINT dpi = GetDpiForWindow(InputCoordinateWindow);
  return static_cast<float>(dpi ? dpi : USER_DEFAULT_SCREEN_DPI) /
         USER_DEFAULT_SCREEN_DPI;
}

LPCWSTR Win32WindowHost::ResolveSystemCursor(MouseCursor cursor) {
  switch (cursor) {
  case MouseCursor::SizeHorizontal:
    return IDC_SIZEWE;
  case MouseCursor::SizeVertical:
    return IDC_SIZENS;
  case MouseCursor::SizeDiagonalNorthwestSoutheast:
    return IDC_SIZENWSE;
  case MouseCursor::SizeDiagonalNortheastSouthwest:
    return IDC_SIZENESW;
  default:
    return IDC_ARROW;
  }
}

LRESULT CALLBACK Win32WindowHost::WindowProcedure(HWND window, UINT message,
                                                  WPARAM wParam,
                                                  LPARAM lParam) {
  auto *host = reinterpret_cast<Win32WindowHost *>(
      GetWindowLongPtrW(window, GWLP_USERDATA));
  if (host && message != WM_NCDESTROY)
    return host->HandleWindowMessage(window, message, wParam, lParam);
  if (message == WM_NCDESTROY)
    SetWindowLongPtrW(window, GWLP_USERDATA, 0);
  return DefWindowProcW(window, message, wParam, lParam);
}
