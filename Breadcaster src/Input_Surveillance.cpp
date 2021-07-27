#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include "Input_Surveillance.h"
#include"Broadcast_UDP.h"
//#define _WIN32_WINNT 0x050

Input_Surveillance::Input_Surveillance()
{
    hhkLowLevelKybd  = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, 0, 0);
    hhLowLevelMouse = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, 0, 0);
}

Input_Surveillance::~Input_Surveillance()
{
    UnhookWindowsHookEx(hhkLowLevelKybd);
    UnhookWindowsHookEx(hhLowLevelMouse);
}

bool Input_Surveillance::IsInputDone()
{
    return false;
}

LRESULT CALLBACK Input_Surveillance::LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)  
{
    BOOL fEatKeystroke = FALSE;
    POINT CursorPos;
    Broadcast_UDP BU_Obj;
    int message_array[3];
    if (nCode == HC_ACTION)
    {
        switch (wParam)
        {
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP:
        
            PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT)lParam;
            GetCursorPos(&CursorPos);
            if (fEatKeystroke = (p->vkCode == p->vkCode) && CursorPos.x < 0)  //redirect a to b
            {

                if ((wParam == WM_KEYDOWN) || (wParam == WM_SYSKEYDOWN))
                {
                    message_array[0] = 1;
                    message_array[1] = p->vkCode;
                    message_array[2] = 0;
                    BU_Obj.Broadcast(message_array);
                }
                else if ((wParam == WM_KEYUP) || (wParam == WM_SYSKEYUP))
                {
					message_array[0] = 2;
					message_array[1] = p->vkCode;
					message_array[2] = 0;
                    BU_Obj.Broadcast(message_array);
                }
                break;
            }
            break;
        }
    }
    return(fEatKeystroke ? 1 : CallNextHookEx(NULL, nCode, wParam, lParam));
}

LRESULT Input_Surveillance::LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    BOOL fEatKeystroke = FALSE;
    POINT CursorPos;
    Broadcast_UDP BU_Obj;
    INPUT Input = { 0 };
    int message_array[3];
    if (nCode == HC_ACTION)
    {
        switch (wParam)
        {
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
        case WM_MOUSEWHEEL:

          MSLLHOOKSTRUCT* mouseInfo = (MSLLHOOKSTRUCT*)lParam;
          GetCursorPos(&CursorPos);
          if (fEatKeystroke = (&mouseInfo == &mouseInfo) && CursorPos.x < 0)  //redirect a to b
          {
              if (wParam == WM_LBUTTONDOWN)
              {
				  message_array[0] = 0;
				  message_array[1] = MOUSEEVENTF_LEFTDOWN;
				  message_array[2] = 0;
                  BU_Obj.Broadcast(message_array);
              }
              else if (wParam == WM_LBUTTONUP)
              {
                  message_array[0] = 0;
                  message_array[1] = MOUSEEVENTF_LEFTUP;
                  message_array[2] = 0;
                  BU_Obj.Broadcast(message_array);
              }

              if (wParam == WM_RBUTTONDOWN)
              {
                  message_array[0] = 0;
                  message_array[1] = MOUSEEVENTF_RIGHTDOWN;
                  message_array[2] = 0;
                  BU_Obj.Broadcast(message_array);
              }
              else if (wParam == WM_RBUTTONUP)
              {
				  message_array[0] = 0;
                  message_array[1] = MOUSEEVENTF_RIGHTUP;
                  message_array[2] = 0;
                  BU_Obj.Broadcast(message_array);
              }

              if (wParam == WM_MBUTTONDOWN)
              {
				  message_array[0] = 0;
				  message_array[1] = MOUSEEVENTF_MIDDLEDOWN;
				  message_array[2] = 0;
                  BU_Obj.Broadcast(message_array);
              }
              else if (wParam == WM_MBUTTONUP) 
              {
				  message_array[0] = 0;
				  message_array[1] = MOUSEEVENTF_MIDDLEUP;
				  message_array[2] = 0;
                  BU_Obj.Broadcast(message_array);
              }

              if ((short)HIWORD(mouseInfo->mouseData)>0)
              {
				  message_array[0] = 4;
				  message_array[1] = 600;
				  message_array[2] = 0;
                  BU_Obj.Broadcast(message_array);
              }
              else if((short)HIWORD(mouseInfo->mouseData) < 0)
              {
				  message_array[0] = 0;
				  message_array[1] = -120;
				  message_array[2] = 0;
                  BU_Obj.Broadcast(message_array);
              }
              break;
          }
          break;
        }
    }
    return(fEatKeystroke ? 1 : CallNextHookEx(NULL, nCode, wParam, lParam));
}
