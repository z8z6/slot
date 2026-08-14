#pragma once

#include "Core/Event.h"

#include <windows.h>

namespace z8 {

/**
 * Win32 HWND 与保留式事件目标之间的共享适配层。
 *
 * 该类只负责 WNDPROC 对象转发、坐标归一化和输入状态，不拥有 HWND，也不
 * 参与交换链或窗口业务生命周期；主 Application 与 Floating host 因而可以
 * 复用同一套平台消息语义，同时保留各自的 resize/render 策略。
 */
class Win32WindowHost {
private:
  EventTarget *InputTarget = nullptr;
  HWND AttachedWindow = nullptr;
  HWND InputCoordinateWindow = nullptr;
  POINT LastPointer{};
  bool HasPointerPosition = false;
  bool ReleasingCapture = false;

  static MouseButton ResolveMouseButton(UINT message, WPARAM wParam);
  float ResolveCoordinateScale() const;
  MouseMovArgs ResolveMouseArgs(HWND sourceWindow, UINT message,
                                WPARAM wParam, LPARAM lParam);
  static LPCWSTR ResolveSystemCursor(MouseCursor cursor);
  static LRESULT CALLBACK WindowProcedure(HWND window, UINT message,
                                          WPARAM wParam, LPARAM lParam);

protected:
  Win32WindowHost() = default;
  virtual ~Win32WindowHost();

  /**
   * 绑定现有 HWND；coordinateWindow 决定事件坐标系，Floating window 可将
   * 本地客户坐标统一映射回主 Layout 客户区。
   */
  void AttachWindow(HWND window, EventTarget &inputTarget,
                    HWND coordinateWindow = nullptr);
  void DetachWindow();
  /** 处理共享输入消息；返回 true 表示 result 已包含窗口过程结果。 */
  bool DispatchInputMessage(HWND window, UINT message, WPARAM wParam,
                            LPARAM lParam, LRESULT &result);
  virtual LRESULT HandleWindowMessage(HWND window, UINT message,
                                      WPARAM wParam, LPARAM lParam) = 0;
};

} // namespace z8
