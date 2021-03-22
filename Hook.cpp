#include "Hook.hpp"
#include <Windows.h>
#include "time.h"
#include "VtuberDelegate.hpp"
#include "View.hpp"
#include"EventManager.hpp"

#include <tchar.h>
#include <string>
#include <thread>
#include <sstream>
#include <fstream>
#include <iostream>
#include <optional>

#ifndef HID_USAGE_PAGE_GENERIC
#define HID_USAGE_PAGE_GENERIC ((USHORT)0x01)
#endif

#ifndef HID_USAGE_GENERIC_MOUSE
#define HID_USAGE_GENERIC_MOUSE ((USHORT)0x02)
#endif

static HHOOK hhkLowLevelKybd;
static HHOOK hhkLowLevelMs;

/* These are the actual letter bound to the key binding (e.g, AZERTY A is the same as QWERTY A)
 * A better way to tackle this would be to use scan codes, where scan code Q is
 * "the key next to tab on
 */
enum class Key : DWORD {
  A = 0x41,
};

static std::optional<int> HookCode(DWORD code) {
  /*
  Translate the return code from hook and 
  return the std::string rep of the the code.
  ex. 0x88 -> "[SHIFT]"
  caps = Caps lock on
  shift = Shift key pressed
  WinUser.h = define statments
  LINK = https://msdn.microsoft.com/en-us/library/dd375731(v=VS.85).aspx
  */
  // Char keys for ASCI
  // No VM Def in header
  // Why do a map when we can just do math B)
  // A - Z alphabet
  if (code >= 0x41 or code <= 0x5a) { return code - 0x41; }
  // Number Keyboard 0 - 9
  if (code >= 0x30 or code <= 0x39) { return code - 0x30 + 26; }

  switch (code) {
    case VK_SPACE: return 36;
    case VK_LSHIFT: return 37;
    case VK_LCONTROL: return 38;
    case VK_F1: return 39;
    case VK_F2: return 40;
    case VK_F3: return 41;
    case VK_F4: return 42;
    case VK_F5: return 43;
    case VK_F6: return 44;
    case VK_F7: return 45;
    case VK_F8: return 46;
    case VK_F9: return 47;
    case VK_F10: return 48;
    case VK_F11: return 49;
    case VK_F12: return 50;
    case VK_UP: return 51;
    case VK_DOWN: return 52;
    case VK_RIGHT: return 53;
    case VK_LEFT: return 54;
    case VK_OEM_COMMA: return 55;
    case VK_OEM_PERIOD: return 56;
    case VK_OEM_4: return 57;
    case VK_OEM_6: return 58;
    case VK_RSHIFT: return 59;
    case VK_RCONTROL: return 60;
  }
  return std::nullopt;
}

LRESULT CALLBACK Hook::KeyboardHookProc(int code, WPARAM key, LPARAM kbhook) {
  if (code != HC_ACTION) { return CallNextHookEx(nullptr, code, key, kbhook); }
  auto manager = VtuberDelegate::GetInstance()->GetView()->GetEventManager();
  auto hook = reinterpret_cast<KBDLLHOOKSTRUCT*>(kbhook);

  switch (code) {
    case WM_KEYUP: {
      if (auto result = HookCode(hook->vkCode)) {
        manager->KeyEventUp(*result);
      }
    }
    case WM_KEYDOWN: {
      if (auto result = HookCode(hook->vkCode)) {
        manager->KeyEventDown(*result);
      }
    }
  }
  return CallNextHookEx(nullptr, code, key, kbhook);
}

LRESULT CALLBACK Hook::MouseHookProc(int code, WPARAM mouse, LPARAM lParam) {
  auto manager = VtuberDelegate::GetInstance()->GetView()->GetEventManager();
  if (code != HC_ACTION) { return CallNextHookEx(nullptr, code, mouse,lParam); }
  switch(mouse) {
    case WM_RBUTTONDOWN:
      manager->RightButtonDown();
      break;
    case WM_LBUTTONDOWN:
      manager->LeftButtonDown();
      break;
    case WM_RBUTTONUP:
      manager->RightButtonUp();
      break;
    case WM_LBUTTONUP:
      manager->LeftButtonUp();
      break;
  }

  return CallNextHookEx(nullptr, code, mouse, lParam);
}

LRESULT Hook::WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  switch (msg) {
    case WM_COMMAND: [[fallthrough]];
    case WM_PAINT:
      break;
    case WM_DESTROY:
      PostQuitMessage(0);
      break;
    case WM_INPUT: {
      UINT dwSize = 0;
      GetRawInputData((HRAWINPUT)lParam, (UINT)RID_INPUT, NULL,
        &dwSize,
        sizeof(RAWINPUTHEADER)); 

      std::unique_ptr<BYTE[]> lpbBuffer { new BYTE[dwSize] };
      GetRawInputData(
        (HRAWINPUT)lParam, (UINT)RID_INPUT, lpbBuffer.get(),
        (PUINT)&dwSize,
        sizeof(RAWINPUTHEADER));

      auto raw = reinterpret_cast<RAWINPUT*>(lpbBuffer.get());
      if (raw->header.dwType == RIM_TYPEMOUSE) {
        auto manager = VtuberDelegate::GetInstance()->GetView()->GetEventManager();

        int xPosRelative = raw->data.mouse.lLastX;
        int yPosRelative = raw->data.mouse.lLastY;
        manager->SetRelativeMouse(xPosRelative, yPosRelative);

      }
      break;
    }
  }
  return DefWindowProc(hwnd, msg, wParam, lParam);
}

void Hook::Strat() {
  isExist = false;
  th = new std::thread(&Hook::Run, this);
  th->detach();
}

void Hook::Stop() {
  isExist = true;
  UnhookWindowsHookEx(hhkLowLevelKybd);
  UnhookWindowsHookEx(hhkLowLevelMs);
}

void Hook::Run() {

  hhkLowLevelKybd =SetWindowsHookEx(WH_KEYBOARD_LL, Hook::KeyboardHookProc, 0, 0);
  hhkLowLevelMs =SetWindowsHookEx(WH_MOUSE_LL, Hook::MouseHookProc, 0, 0);


  WNDCLASSEX wcx {
    .cbSize = sizeof(wcx),
    .style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_NOCLOSE,
    .lpfnWndProc = WindowProc,
    .cbClsExtra = 0,
    .cbWndExtra = 0,
    .hInstance = GetModuleHandle(nullptr),
    .hIcon = LoadIcon(nullptr, IDI_APPLICATION),
    .hCursor = LoadCursor(nullptr, IDC_ARROW),
    .hbrBackground = static_cast<HBRUSH>(WHITE_BRUSH),
    .lpszMenuName = nullptr,
    .lpszClassName = _T("BONGOHOOK"),
    .hIconSm = nullptr
  };

  if (!RegisterClassEx(&wcx)) {
    printf("RegisterClassEx failed");
    return;
  }

  //set windows position
  int OSDleft = GetSystemMetrics(SM_CXSCREEN) / 2 - 300;
  int OSDTop = GetSystemMetrics(SM_CYSCREEN) / 2;

  m_hWnd = CreateWindowEx(
    WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_TOPMOST |
      WS_EX_TRANSPARENT | WS_EX_NOACTIVATE, 
    wcx.lpszClassName,
    NULL,
    WS_VISIBLE | WS_POPUP, 
    OSDleft,
    OSDTop,
    300,
    300,
    nullptr,
    nullptr,
    wcx.hInstance,
    nullptr);

  if (!m_hWnd) {
    //fail to creat window;
    return;
  }

  //regist raw input device
  RAWINPUTDEVICE Rid[1];
  Rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
  Rid[0].usUsage = HID_USAGE_GENERIC_MOUSE;
  Rid[0].dwFlags = RIDEV_INPUTSINK;
  Rid[0].hwndTarget = m_hWnd;
  if (RegisterRawInputDevices(Rid, 1, sizeof(Rid[0])) == FALSE) {
    return;
  };

  //message loop
  BOOL bRet;
  MSG msg;
  while ((bRet = GetMessage(&msg, 0, 0, 0)) != 0) {
    if (bRet == -1||isExist) {
      break;
    } else {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }
}
