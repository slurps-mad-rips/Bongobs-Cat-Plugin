#pragma once
#include <Windows.h>
#include <thread>

class Hook {
public:
  Hook() noexcept = default;
  ~Hook() noexcept = default;

  void Strat();
  void Stop();

private:
  void Run();

  bool isExist;

  static LRESULT CALLBACK KeyboardHookProc(int, WPARAM wParam, LPARAM);
  static LRESULT CALLBACK MouseHookProc(int, WPARAM wParam,  LPARAM);
  static LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);

  HWND m_hWnd;

  std::thread *th;

};
