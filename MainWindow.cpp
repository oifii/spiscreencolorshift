/*
 * Copyright (c) 2010-2016 Stephane Poirier
 *
 * stephane.poirier@oifii.org
 *
 * Stephane Poirier
 * 3532 rue Ste-Famille, #3
 * Montreal, QC, H2X 2L1
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "MainWindow.h"
#include "TimerEventHandler.h"
#include "UIAnimationSample.h"

#include <UXTheme.h>

#include <mmsystem.h> //for timeSetEvent()
#include "resource.h"

using namespace Gdiplus;

const DOUBLE COLOR_MIN = 0.0;
const DOUBLE COLOR_MAX = 255.0;

float global_duration_sec=180;
float global_sleeptimepershift_sec=5.0;
int global_x=100;
int global_y=200;
int global_xwidth=400;
int global_yheight=400;
BYTE global_alpha=200;
int global_titlebardisplay=1; //0 for off, 1 for on
int global_acceleratoractive=0; //0 for off, 1 for on
int global_menubardisplay=0; //0 for off, 1 for on

HWND global_hwnd=NULL;
MMRESULT global_timer=0;
DWORD global_startstamp_ms;


void CALLBACK StartGlobalProcess(UINT uTimerID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{

	DWORD nowstamp_ms = GetTickCount();
	while( (global_duration_sec<0.0f) || ((nowstamp_ms-global_startstamp_ms)/1000.0f)<global_duration_sec )
	{
		Sleep((int)(global_sleeptimepershift_sec*1000));
		
		/*
		CMainWindow* pMainWindow = reinterpret_cast<CMainWindow *>(static_cast<LONG_PTR>(GetWindowLongPtrW(global_hwnd,GWLP_USERDATA)));
		HRESULT hr = pMainWindow->ChangeColor(
			pMainWindow->RandomFromRange(COLOR_MIN, COLOR_MAX),
			pMainWindow->RandomFromRange(COLOR_MIN, COLOR_MAX),
			pMainWindow->RandomFromRange(COLOR_MIN, COLOR_MAX)
			);
		*/
		PostMessage(global_hwnd, WM_LBUTTONDOWN, 0, 0);

		nowstamp_ms = GetTickCount();
	}
	PostMessage(global_hwnd, WM_DESTROY, 0, 0);
}



CMainWindow::CMainWindow() :
    m_hwnd(NULL),
    m_pAnimationManager(NULL),
    m_pAnimationTimer(NULL),
    m_pTransitionLibrary(NULL),
    m_pAnimationVariableRed(NULL),
    m_pAnimationVariableGreen(NULL),
    m_pAnimationVariableBlue(NULL)
{
}

CMainWindow::~CMainWindow()
{
    // Animated Variables

    SafeRelease(&m_pAnimationVariableRed);
    SafeRelease(&m_pAnimationVariableGreen);
    SafeRelease(&m_pAnimationVariableBlue);

    // Animation

    SafeRelease(&m_pAnimationManager);
    SafeRelease(&m_pAnimationTimer);
    SafeRelease(&m_pTransitionLibrary);
}

// Creates the CMainWindow window

HRESULT CMainWindow::Initialize(HINSTANCE hInstance)
{
    // Register the window class

    WNDCLASSEX wcex = {sizeof(wcex)};
    wcex.style         = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc   = CMainWindow::WndProc;
    wcex.cbWndExtra    = sizeof(LONG_PTR);
    wcex.hInstance     = hInstance;
    //wcex.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAIN_ICON));
	wcex.hIconSm	   = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAIN_ICON_SMALL));
    wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wcex.lpszClassName = L"SPISCREENCOLORSHIFT";
    RegisterClassEx(&wcex);

    // Create the CMainWindow window

	if(global_titlebardisplay)
	{
		m_hwnd = CreateWindow(wcex.lpszClassName, L"spiscreencolorshift",
			WS_OVERLAPPEDWINDOW,
			//CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			global_x, global_y, global_xwidth, global_yheight,
			NULL, NULL, hInstance, this);
	}
	else
	{
		m_hwnd = CreateWindow(wcex.lpszClassName, L"spiscreencolorshift",
			WS_POPUP,
			//CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			global_x, global_y, global_xwidth, global_yheight,
			NULL, NULL, hInstance, this);
	}

    HRESULT hr = m_hwnd ? S_OK : E_FAIL;

    if (SUCCEEDED(hr))
    {
		//spi, begin
		global_hwnd = m_hwnd;
		SetWindowLong(global_hwnd, GWL_EXSTYLE, GetWindowLong(global_hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
		SetLayeredWindowAttributes(global_hwnd, 0, global_alpha, LWA_ALPHA);
		//spi, end


		// Initialize Animation
        hr = InitializeAnimation();
        if (SUCCEEDED(hr))
        {
            hr = CreateAnimationVariables();
            if (SUCCEEDED(hr))
            {
                // Display the window
                ShowWindow(m_hwnd, SW_SHOWNORMAL);
                UpdateWindow(m_hwnd);

                // Fade in with Red
                hr = ChangeColor(COLOR_MAX, COLOR_MIN, COLOR_MIN);
            }
        }
		//spi, begin
		global_timer=timeSetEvent(500,25,(LPTIMECALLBACK)&StartGlobalProcess,0,TIME_ONESHOT);
		//spi, end
    }

    return hr;
}

// Invalidates the client area for redrawing

HRESULT CMainWindow::Invalidate()
{
    BOOL bResult = InvalidateRect(m_hwnd,NULL,FALSE);
    
    HRESULT hr = bResult ? S_OK : E_FAIL;

    return hr;
}

// Creates and initializes the main animation components

HRESULT CMainWindow::InitializeAnimation()
{
    // Create Animation Manager

    HRESULT hr = CoCreateInstance(CLSID_UIAnimationManager,NULL,CLSCTX_INPROC_SERVER,IID_PPV_ARGS(&m_pAnimationManager));
    if (SUCCEEDED(hr))
    {
        // Create Animation Timer

        hr = CoCreateInstance(CLSID_UIAnimationTimer,NULL,CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_pAnimationTimer));
        if (SUCCEEDED(hr))
        {
            // Create Animation Transition Library

            hr = CoCreateInstance(CLSID_UIAnimationTransitionLibrary,NULL,CLSCTX_INPROC_SERVER,IID_PPV_ARGS(&m_pTransitionLibrary));
            if (SUCCEEDED(hr))
            {
                // Connect the animation manager to the timer.
                // UI_ANIMATION_IDLE_BEHAVIOR_DISABLE tells the timer to shut itself
                // off when there is nothing to animate.

                IUIAnimationTimerUpdateHandler *pTimerUpdateHandler;
                hr = m_pAnimationManager->QueryInterface(IID_PPV_ARGS(&pTimerUpdateHandler));
                if (SUCCEEDED(hr))
                {
                    hr = m_pAnimationTimer->SetTimerUpdateHandler(pTimerUpdateHandler,UI_ANIMATION_IDLE_BEHAVIOR_DISABLE);
                    pTimerUpdateHandler->Release();
                    if (SUCCEEDED(hr))
                    {
                        // Create and set the Timer Event Handler

                        IUIAnimationTimerEventHandler *pTimerEventHandler;
                        hr = CTimerEventHandler::CreateInstance(this,&pTimerEventHandler);
                        if (SUCCEEDED(hr))
                        {
                            hr = m_pAnimationTimer->SetTimerEventHandler(pTimerEventHandler);
                            pTimerEventHandler->Release();
                        }
                    }
                }
            }
        }
    }
    
    return hr;
}

// Creates the RGB animation variables for the background color

HRESULT CMainWindow::CreateAnimationVariables()
{
    const DOUBLE INITIAL_RED = COLOR_MAX;
    const DOUBLE INITIAL_GREEN = COLOR_MAX;
    const DOUBLE INITIAL_BLUE = COLOR_MAX;

    HRESULT hr = m_pAnimationManager->CreateAnimationVariable(INITIAL_RED,&m_pAnimationVariableRed);
    if (SUCCEEDED(hr))
    {
        hr = m_pAnimationVariableRed->SetLowerBound(COLOR_MIN);
        if (SUCCEEDED(hr))
        {
            hr = m_pAnimationVariableRed->SetUpperBound(COLOR_MAX);
            if (SUCCEEDED(hr))
            {
                hr = m_pAnimationManager->CreateAnimationVariable(INITIAL_GREEN,&m_pAnimationVariableGreen);
                if (SUCCEEDED(hr))
                {
                    hr = m_pAnimationVariableGreen->SetLowerBound(COLOR_MIN);
                    if (SUCCEEDED(hr))
                    {
                        hr = m_pAnimationVariableGreen->SetUpperBound(COLOR_MAX);
                        if (SUCCEEDED(hr))
                        {
                            hr = m_pAnimationManager->CreateAnimationVariable(INITIAL_BLUE,&m_pAnimationVariableBlue);
                            if (SUCCEEDED(hr))
                            {
                                hr = m_pAnimationVariableBlue->SetLowerBound(COLOR_MIN);
                                if (SUCCEEDED(hr))
                                {
                                    hr = m_pAnimationVariableBlue->SetUpperBound(COLOR_MAX);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    return hr;
}

// The window message handler

LRESULT CALLBACK CMainWindow::WndProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
    const int MESSAGE_PROCESSED = 0;

    if (uMsg == WM_CREATE)
    {
        LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
        CMainWindow *pMainWindow = (CMainWindow *)pcs->lpCreateParams;

        SetWindowLongPtrW(hwnd,GWLP_USERDATA,PtrToUlong(pMainWindow));

        return MESSAGE_PROCESSED;
    }

    CMainWindow *pMainWindow = reinterpret_cast<CMainWindow *>(static_cast<LONG_PTR>(GetWindowLongPtrW(hwnd,GWLP_USERDATA)));

    if (pMainWindow != NULL)
    {
        switch (uMsg)
        {
        case WM_LBUTTONDOWN:
            {
                pMainWindow->OnLButtonDown();
            }
            return MESSAGE_PROCESSED;

        case WM_PAINT:
        case WM_DISPLAYCHANGE:
            {
                PAINTSTRUCT ps;
                BeginPaint(hwnd,&ps);

                pMainWindow->OnPaint(ps.hdc, ps.rcPaint);

                EndPaint(hwnd, &ps);
            }
            return MESSAGE_PROCESSED;

        case WM_DESTROY:
            {
                pMainWindow->OnDestroy();
            
                PostQuitMessage(0);
            }
            return MESSAGE_PROCESSED;
        }
    }
    return DefWindowProc(hwnd,uMsg,wParam,lParam);
}

//  Called whenever the client area needs to be drawn

HRESULT CMainWindow::OnPaint(HDC hdc,const RECT &rcPaint)
{
    HRESULT hr = S_OK;

    HDC hdcBuffer;
    HPAINTBUFFER hpb = BeginBufferedPaint(hdc,&rcPaint,BPBF_COMPATIBLEBITMAP, NULL,&hdcBuffer);
    if (hpb)
    {
        hr = DrawClientArea(hdcBuffer, rcPaint);

        // Can only return one failure HRESULT
        HRESULT hrEndDraw = EndBufferedPaint(hpb,TRUE);
        if (SUCCEEDED(hr))
        {
            hr = hrEndDraw;
        }
    }
    else
    {
        // An error occurred, default to unbuffered painting
        
        hr = DrawClientArea(hdc, rcPaint);
    }

    return hr;
}

// When the left mouse button is clicked on the client area, schedule
// animations to change the background color of the window

HRESULT CMainWindow::OnLButtonDown()
{
    HRESULT hr = ChangeColor(
        RandomFromRange(COLOR_MIN, COLOR_MAX),
        RandomFromRange(COLOR_MIN, COLOR_MAX),
        RandomFromRange(COLOR_MIN, COLOR_MAX)
        );

    return hr; 
}

// Prepares for window destruction

void CMainWindow::OnDestroy()
{
    if (m_pAnimationTimer != NULL)
    {
        // Clear the timer event handler, to ensure that the one that points to this object is released
    
        (void)m_pAnimationTimer->SetTimerEventHandler(NULL);
    }
}

// Draws the contents of the client area

HRESULT CMainWindow::DrawClientArea(
    HDC hdc,
    const RECT &rcPaint
    )
{
    Graphics graphics(hdc);
    HRESULT hr = HrFromStatus(graphics.SetSmoothingMode(
        SmoothingModeAntiAlias
        ));
    if (SUCCEEDED(hr))
    {
        RectF rectPaint(
            static_cast<REAL>(rcPaint.left),
            static_cast<REAL>(rcPaint.top),
            static_cast<REAL>(rcPaint.right - rcPaint.left),
            static_cast<REAL>(rcPaint.bottom - rcPaint.top)
            );
    
        hr = DrawBackground(
            graphics,
            rectPaint
            );
    }
    
    return hr;
}

// Fills the background with the color specified by the animation variables

HRESULT CMainWindow::DrawBackground(
    Graphics &graphics,
    const RectF &rectPaint
    )
{
    // Get the RGB animation variable values

    INT32 red;
    HRESULT hr = m_pAnimationVariableRed->GetIntegerValue(
        &red
        );
    if (SUCCEEDED(hr))
    {
        INT32 green;
        hr = m_pAnimationVariableGreen->GetIntegerValue(
            &green
            );
        if (SUCCEEDED(hr))
        {
            INT32 blue;
            hr = m_pAnimationVariableBlue->GetIntegerValue(
                &blue
                );
            if (SUCCEEDED(hr))
            {
                // Set the RGB of the background brush to the new animated value

                SolidBrush brushBackground(Color(
                    static_cast<BYTE>(red),
                    static_cast<BYTE>(green),
                    static_cast<BYTE>(blue)
                    ));

                // Paint the background

                hr = HrFromStatus(graphics.FillRectangle(
                    &brushBackground,
                    rectPaint
                    ));
            }
        }
    }
    
    return hr;
}

// Animates the background color to a new value

HRESULT CMainWindow::ChangeColor(DOUBLE red, DOUBLE green, DOUBLE blue)
{
    const UI_ANIMATION_SECONDS DURATION = 0.5; //500ms
    //const UI_ANIMATION_SECONDS DURATION = 0.1; //100ms
    const DOUBLE ACCELERATION_RATIO = 0.5;
    const DOUBLE DECELERATION_RATIO = 0.5;

    // Create a storyboard

    IUIAnimationStoryboard *pStoryboard = NULL;
    HRESULT hr = m_pAnimationManager->CreateStoryboard(
        &pStoryboard
        );
    if (SUCCEEDED(hr))
    {
        // Create transitions for the RGB animation variables

        IUIAnimationTransition *pTransitionRed;
        hr = m_pTransitionLibrary->CreateAccelerateDecelerateTransition(
            DURATION,
            red,
            ACCELERATION_RATIO,
            DECELERATION_RATIO,
            &pTransitionRed
            );
        if (SUCCEEDED(hr))
        {
            IUIAnimationTransition *pTransitionGreen;
            hr = m_pTransitionLibrary->CreateAccelerateDecelerateTransition(
                DURATION,
                green,
                ACCELERATION_RATIO,
                DECELERATION_RATIO,
                &pTransitionGreen
                );
            if (SUCCEEDED(hr))
            {
                IUIAnimationTransition *pTransitionBlue;
                hr = m_pTransitionLibrary->CreateAccelerateDecelerateTransition(
                    DURATION,
                    blue,
                    ACCELERATION_RATIO,
                    DECELERATION_RATIO,
                    &pTransitionBlue
                    );
                if (SUCCEEDED(hr))
                {
                    // Add transitions to the storyboard

                    hr = pStoryboard->AddTransition(
                        m_pAnimationVariableRed,
                        pTransitionRed
                        );
                    if (SUCCEEDED(hr))
                    {
                        hr = pStoryboard->AddTransition(
                            m_pAnimationVariableGreen,
                            pTransitionGreen
                            );
                        if (SUCCEEDED(hr))
                        {
                            hr = pStoryboard->AddTransition(
                                m_pAnimationVariableBlue,
                                pTransitionBlue
                                );
                            if (SUCCEEDED(hr))
                            {
                                // Get the current time and schedule the storyboard for play

                                UI_ANIMATION_SECONDS secondsNow;
                                hr = m_pAnimationTimer->GetTime(
                                    &secondsNow
                                    );
                                if (SUCCEEDED(hr))
                                {
                                    hr = pStoryboard->Schedule(
                                        secondsNow
                                        );
                                }
                            }
                        }
                    }

                    pTransitionBlue->Release();
                }

                pTransitionGreen->Release();
            }

            pTransitionRed->Release();
        }

        pStoryboard->Release();
    }

    return hr;
}

// Generate a random value in the given range

DOUBLE CMainWindow::RandomFromRange(
    DOUBLE minimum,
    DOUBLE maximum
    )
{
     return minimum + (maximum - minimum) * rand() / RAND_MAX;
}

// Convert a GDI+ Status to an HRESULT
        
HRESULT CMainWindow::HrFromStatus(
    Status status
    )
{
    return (status == Ok ? S_OK : E_FAIL);
}
