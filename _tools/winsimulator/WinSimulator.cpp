/***************************************************************************
 *   Copyright (C) 2011 by Yury Kuchura                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   As a special exception, if other files instantiate templates or use   *
 *   macros or inline functions from this file, or you compile this file   *
 *   and link it with other works to produce a work based on this file,    *
 *   this file does not by itself cause the resulting work to be covered   *
 *   by the GNU General Public License. However the source code for this   *
 *   file must still be made available in accordance with the GNU General  *
 *   Public License. This exception does not invalidate any other reasons  *
 *   why a work based on this file might be covered by the GNU General     *
 *   Public License.                                                       *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>   *
 ***************************************************************************/

//===============================================================
// DrawingContext.cpp : entry point for Windows application
//===============================================================

#include "stdafx.h"
#include "WinSimulator.h"
#include "winbackend.h"
#include "window.h"
#include "mxgui/entry.h"
#include "mxgui/display.h"
#include "mxgui/config/mxgui_settings.h"
#include "mxgui/misc_inst.h"
#include "mxgui/level2/input.h"

#define MAX_LOADSTRING 100

// Module variables:
static HINSTANCE hInst;                                // current instance
static TCHAR szTitle[MAX_LOADSTRING];                    // The title bar text
static TCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
static Window win;
static HBITMAP hBmp;
static HDC hDC; //Drawing context
static LONG clntWidth;
static LONG clntHeight;
static HANDLE hDCMutex;
static volatile LONG updateCounter = 0; //nonzero if updates are available in drawing context
static HWND hStatusBar;
static HWND hMainWnd;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);


//Copy memory context to window
static void RedrawContext (HDC hDestinationDC)
{
    WaitForSingleObject(hDCMutex, INFINITE);
    BitBlt(hDestinationDC, 0, 0, clntWidth, clntHeight, hDC, 0, 0, SRCCOPY);
    ReleaseMutex(hDCMutex);
}

//Convert 16-bit color to RGB used in Windows' drawing context
static COLORREF ConvertColor(unsigned short color)
{
    BYTE r = ((color>>11)&0x1F)<<3;
    BYTE g = ((color>>5)&0x3F)<<2;
    BYTE b = (color&0x1F)<<3;
    return RGB(r, g, b);
}

/** Function called by application to draw pixel
    @param [in] x X-coordinate
    @param [in] y Y-coordinate
    @param [in] color mxgui color for pixel
*/
void DrawPixel(short x, short y, mxgui::Color color)
{
    WaitForSingleObject(hDCMutex, INFINITE);
    SetPixel(hDC, x, y, ConvertColor(color));
    InterlockedIncrement(&updateCounter);
    ReleaseMutex(hDCMutex);
}

// WinMain
int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    MSG msg;
    HACCEL hAccelTable;

    // Initialize global strings
    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_DRAWINGCONTEXT, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DRAWINGCONTEXT));

    mxgui::Display& display = mxgui::Display::instance();
    {
        mxgui::DrawingContext dc(display);
        dc.beginPixel();
        dc.clear(mxgui::black);
    }

    // Main message loop:
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

//Register window class
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style            = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra        = 0;
    wcex.cbWndExtra        = 0;
    wcex.hInstance        = hInstance;
    wcex.hIcon            = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DRAWINGCONTEXT));
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground    = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName    = MAKEINTRESOURCE(IDC_DRAWINGCONTEXT);
    wcex.lpszClassName    = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassEx(&wcex);
}

//Create window
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // Store instance handle in our global variable
    DWORD style = WS_CAPTION | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_CLIPCHILDREN;
    DWORD exStyle = WS_EX_APPWINDOW; //Show on taskbar

    InitCommonControls();

    hDCMutex = CreateMutex(NULL, FALSE, NULL);

    hMainWnd = CreateWindowEx(exStyle, szWindowClass, szTitle, style,
       CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
    if (!hMainWnd)
    {
        MessageBox(NULL, _T("CreateWindowEx failed"), _T("Error"), MB_OK);
        return FALSE;
    }

    //Calculate window rectangle to fit exactly the client area we need
    RECT rect;
    rect.top = 0; rect.left=0;
    rect.right = mxgui::SIMULATOR_DISP_WIDTH;
    rect.bottom = mxgui::SIMULATOR_DISP_HEIGHT;
    if (!AdjustWindowRectEx(&rect, style, TRUE, exStyle))
    {
        MessageBox(NULL, _T("AdjustWindowRectEx failed"), _T("Error"), MB_OK);
        return FALSE;
    }

    //Resize window to fit the client area we need
    SetWindowPos( hMainWnd, NULL, 0, 0, rect.right - rect.left, rect.bottom - rect.top,
                  SWP_NOZORDER | SWP_NOMOVE ) ;

    //Create status bar - oops, it overlaps client area...
    //hStatusBar = CreateStatusWindow(WS_CHILD | WS_VISIBLE, "Status bar placeholder", hMainWnd, 111);

    ShowWindow(hMainWnd, nCmdShow);
    UpdateWindow(hMainWnd);

    //Create drawing context
    RECT clientRect;
    GetClientRect(hMainWnd, &clientRect);
    clntWidth = clientRect.right - clientRect.left;
    clntHeight = clientRect.bottom - clientRect.top;
    HDC tmpHDC = GetDC(hMainWnd);
    hDC = CreateCompatibleDC(tmpHDC);
    hBmp = CreateCompatibleBitmap(tmpHDC, clntWidth, clntHeight);
    ReleaseDC(hMainWnd, tmpHDC);
    SelectObject(hDC, hBmp);
    SetBkMode(hDC, TRANSPARENT); //Background remains untouched

    SetTimer(hMainWnd, 1, 20, NULL); //Send WM_TIMER message every 20 ms
    return TRUE;
}

//Processes messages for the main window.
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int wmId, wmEvent, x=0, y=0;
    PAINTSTRUCT ps;
    HDC hdc;
    RECT rcClient;                 // client area rectangle 
    POINT ptClientUL;              // client upper left corner 
    POINT ptClientLR;              // client lower right corner 
    POINTS pt;                     // point 

    switch (message)
    {
    case WM_COMMAND:
        wmId    = LOWORD(wParam);
        wmEvent = HIWORD(wParam);
        // Parse the menu selections:
        switch (wmId)
        {
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        case IDM_BUTTONA:
            win.ButtonAEvent();
            break;
        case IDM_BUTTONB:
            win.ButtonBEvent();
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;

    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        RedrawContext(hdc);
        EndPaint(hWnd, &ps);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    case WM_TIMER:
        if (0 != InterlockedExchange(&updateCounter, 0))
        {
            hdc = GetDC(hWnd); 
            RedrawContext(hdc);
            ReleaseDC(hWnd, hdc); 
        }
        WinBackend::instance().start();
        break;

    case WM_LBUTTONDOWN: 
        // Capture mouse input. 
        SetCapture(hWnd); 
        // Retrieve the screen coordinates of the client area, 
        // and convert them into client coordinates. 
        GetClientRect(hWnd, &rcClient); 
        ptClientUL.x = rcClient.left; 
        ptClientUL.y = rcClient.top; 
        // Add one to the right and bottom sides, because the 
        // coordinates retrieved by GetClientRect do not 
        // include the far left and lowermost pixels. 
        ptClientLR.x = rcClient.right+1; 
        ptClientLR.y = rcClient.bottom+1; 
        ClientToScreen(hWnd, &ptClientUL); 
        ClientToScreen(hWnd, &ptClientLR); 
        // Copy the client coordinates of the client area 
        // to the rcClient structure. Confine the mouse cursor 
        // to the client area by passing the rcClient structure 
        // to the ClipCursor function. 
        SetRect(&rcClient, ptClientUL.x, ptClientUL.y, 
                ptClientLR.x, ptClientLR.y); 
        ClipCursor(&rcClient); 
        // Convert the cursor coordinates into a POINTS 
        // structure
        pt = MAKEPOINTS(lParam);
        //And set mxgui event
        win.mousePressEvent(pt.x, pt.y);
        return 0; 

    case WM_MOUSEMOVE: 
        if (wParam & MK_LBUTTON) 
        {
            pt = MAKEPOINTS(lParam); 
            win.mouseMoveEvent(pt.x, pt.y);
        }
        break; 

    case WM_LBUTTONUP: 
        pt = MAKEPOINTS(lParam); 
        win.mouseReleaseEvent(pt.x, pt.y);
        ClipCursor(NULL); 
        ReleaseCapture(); 
        return 0; 

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
