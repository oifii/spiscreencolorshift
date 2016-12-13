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

#include <GDIPlus.h>
#include <UXTheme.h>

using namespace Gdiplus;


extern float global_duration_sec;
extern float global_sleeptimepershift_sec;
extern int global_x;
extern int global_y;
extern int global_xwidth;
extern int global_yheight;
extern BYTE global_alpha;
extern int global_titlebardisplay; 
extern int global_acceleratoractive; 
extern int global_menubardisplay; 

extern DWORD global_startstamp_ms;



PCHAR*
    CommandLineToArgvA(
        PCHAR CmdLine,
        int* _argc
        )
    {
        PCHAR* argv;
        PCHAR  _argv;
        ULONG   len;
        ULONG   argc;
        CHAR   a;
        ULONG   i, j;

        BOOLEAN  in_QM;
        BOOLEAN  in_TEXT;
        BOOLEAN  in_SPACE;

        len = strlen(CmdLine);
        i = ((len+2)/2)*sizeof(PVOID) + sizeof(PVOID);

        argv = (PCHAR*)GlobalAlloc(GMEM_FIXED,
            i + (len+2)*sizeof(CHAR));

        _argv = (PCHAR)(((PUCHAR)argv)+i);

        argc = 0;
        argv[argc] = _argv;
        in_QM = FALSE;
        in_TEXT = FALSE;
        in_SPACE = TRUE;
        i = 0;
        j = 0;

        while( a = CmdLine[i] ) {
            if(in_QM) {
                if(a == '\"') {
                    in_QM = FALSE;
                } else {
                    _argv[j] = a;
                    j++;
                }
            } else {
                switch(a) {
                case '\"':
                    in_QM = TRUE;
                    in_TEXT = TRUE;
                    if(in_SPACE) {
                        argv[argc] = _argv+j;
                        argc++;
                    }
                    in_SPACE = FALSE;
                    break;
                case ' ':
                case '\t':
                case '\n':
                case '\r':
                    if(in_TEXT) {
                        _argv[j] = '\0';
                        j++;
                    }
                    in_TEXT = FALSE;
                    in_SPACE = TRUE;
                    break;
                default:
                    in_TEXT = TRUE;
                    if(in_SPACE) {
                        argv[argc] = _argv+j;
                        argc++;
                    }
                    _argv[j] = a;
                    j++;
                    in_SPACE = FALSE;
                    break;
                }
            }
            i++;
        }
        _argv[j] = '\0';
        argv[argc] = NULL;

        (*_argc) = argc;
        return argv;
    }

// Entry point to the application

int APIENTRY WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow
    )
{
	global_startstamp_ms = GetTickCount();

	LPSTR *szArgList;
	int nArgs;


	szArgList = CommandLineToArgvA(GetCommandLineA(), &nArgs);
	if( NULL == szArgList )
	{
		//wprintf(L"CommandLineToArgvW failed\n");
		return FALSE;
	}
	if(nArgs>1)
	{
		global_duration_sec = atof(szArgList[1]);
	}
	if(nArgs>2)
	{
		global_sleeptimepershift_sec = atof(szArgList[2]);
	}
	if(nArgs>3)
	{
		global_x = atoi(szArgList[3]);
	}
	if(nArgs>4)
	{
		global_y = atoi(szArgList[4]);
	}
	if(nArgs>5)
	{
		global_xwidth = atoi(szArgList[5]);
	}
	if(nArgs>6)
	{
		global_yheight = atoi(szArgList[6]);
	}
	if(nArgs>7)
	{
		global_alpha = atoi(szArgList[7]);
	}
	if(nArgs>8)
	{
		global_titlebardisplay = atoi(szArgList[8]);
	}
	if(nArgs>9)
	{
		global_menubardisplay = atoi(szArgList[9]);
	}
	if(nArgs>10)
	{
		global_acceleratoractive = atoi(szArgList[10]);
	}
	LocalFree(szArgList);


    HRESULT hr = CoInitialize(NULL);
    if (SUCCEEDED(hr))
    {
        if (SUCCEEDED(BufferedPaintInit()))
        {
            ULONG_PTR gdiplusToken;
            GdiplusStartupInput gdiplusStartupInput;
            
            if (GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL) == Ok)
            {
                {
                    CMainWindow mainWindow;
                    hr = mainWindow.Initialize(hInstance);
                    if (SUCCEEDED(hr))
                    {
                        MSG msg;
                        while (GetMessage(&msg, NULL, 0, 0) > 0)
                        {
                            TranslateMessage(&msg);
                            DispatchMessage(&msg);
                        }
                    }
                }
                
                GdiplusShutdown(gdiplusToken);
            }
            
            BufferedPaintUnInit();
        }

        CoUninitialize();
    }

    return 0;
}
