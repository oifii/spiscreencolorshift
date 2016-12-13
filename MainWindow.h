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

#pragma once

#include <UIAnimation.h>
#include <GDIPlus.h>

// Main application window

class CMainWindow
{
public:

    CMainWindow();
    ~CMainWindow();

    HRESULT Initialize(
        HINSTANCE hInstance
        );

    HRESULT Invalidate();

protected:

    HRESULT InitializeAnimation();
    HRESULT CreateAnimationVariables();

    static LRESULT CALLBACK WndProc(
        HWND hwnd,
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam
        );

    HRESULT OnPaint(
        HDC hdc,
        const RECT &rcPaint
        );
    
    HRESULT OnLButtonDown();
    
    void OnDestroy();

    HRESULT DrawClientArea(
        HDC hdc,
        const RECT &rcPaint
        );

    HRESULT DrawBackground(
        Gdiplus::Graphics &graphics,
        const Gdiplus::RectF &rectPaint
        );

    HRESULT ChangeColor(
        DOUBLE red,
        DOUBLE green,
        DOUBLE blue
        );

    DOUBLE RandomFromRange(
        DOUBLE minimum,
        DOUBLE maximum
        );

    HRESULT HrFromStatus(
        Gdiplus::Status status
        );

private:

    HWND m_hwnd;

    // Animation components

    IUIAnimationManager *m_pAnimationManager;
    IUIAnimationTimer *m_pAnimationTimer;
    IUIAnimationTransitionLibrary *m_pTransitionLibrary;

    // Animated Variables

    IUIAnimationVariable *m_pAnimationVariableRed;
    IUIAnimationVariable *m_pAnimationVariableGreen;
    IUIAnimationVariable *m_pAnimationVariableBlue;
};
