//=============================================================================
//
// Spook
//
// SPOOK.C
//
// This file is the main module of the Spook program destined to spook whoever
// poor soul happens to install it in his/her PC.
//
//=============================================================================
#pragma option -WES
#define STRICT
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <mem.h>
#pragma hdrstop

#define IDT_SPOOK               1

#define MAX_TIME                32767   // about 33 seconds
#define MIN_MOVES               100
#define MAX_MOVES               50
#define MAX_SPOOKS              50

HINSTANCE hAppInstance;
UINT nSpooks;
UINT cxScreen, cyScreen;
UINT cxDelta, cyDelta;
char szAppName[] = "spook";
char szAppCaption[] = "Spook!";
char szClassName[] = "SpookClass";

extern VOID WINAPI RepaintScreen(VOID);

//
// Spook
//
// Purpose:
//    Spooks the hell outta the user.
// In:
//    none
// Out:
//    none
//
#pragma argsused
void Spook(void)
{
  HCURSOR hOldCursor;
  HDC hDC;
  UINT nMoves;
  POINT p, dp, np, p2, p3;
  RECT r;

  hOldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

  hDC = GetDC(NULL);

  SelectObject(hDC, GetStockObject(BLACK_BRUSH));
  for(p.x = 0; p.x < cxScreen; p.x += cxDelta)
  {
    MoveTo(hDC, p.x, 0);
    LineTo(hDC, p.x, cyScreen);
  }

  for(p.y = 0; p.y < cyScreen; p.y += cyDelta)
  {
    MoveTo(hDC, 0, p.y);
    LineTo(hDC, cxScreen, p.y);
  }
  r.left = r.top = 0;
  r.right = cxDelta;
  r.bottom = cyDelta;
  FillRect(hDC, &r, GetStockObject(BLACK_BRUSH));

  p.x = p.y = 0;
  for(nMoves = max((rand()%MAX_MOVES)+1, MIN_MOVES); nMoves; nMoves--)
  {
    np = p;
    if(rand()&1)
    {
      dp.x = (rand()&1)?1:-1;
      dp.y = 0;
      if((dp.x < 0 && p.x == 0) || (dp.x > 0 && p.x == cxScreen-cxDelta))
        dp.x = -dp.x;
      np.x = p.x + cxDelta*dp.x;
    }
    else
    {
      dp.x = 0;
      dp.y = (rand()&1)?1:-1;
      if((dp.y < 0 && p.y == 0) || (dp.y > 0 && p.y == cyScreen-cyDelta))
        dp.y = -dp.y;
      np.y = p.y + cyDelta*dp.y;
    }
    p3 = np;
    while(np.x != p.x || np.y != p.y)
    {
      p2.x = np.x-dp.x;
      p2.y = np.y-dp.y;
      BitBlt(hDC, p2.x, p2.y, cxDelta+1, cyDelta+1, hDC, np.x, np.y, SRCCOPY);
      np = p2;
    }
    p = p3;
  }

  ReleaseDC(NULL, hDC);

  SetCursor(hOldCursor);

  MessageBox(NULL, "You have been spooked!", szAppCaption, MB_TASKMODAL|MB_ICONINFORMATION|MB_OK);

  RepaintScreen();
}

//
// SpookWndProc
//
// Purpose:
//    Handles messages sent to a window
// In:
//    hWnd = handle of the window
//    uMsg = message ID
//    wParam, lParam = parameters of the message
// Out:
//    depends on the message
//
#pragma argsused
LRESULT CALLBACK _export SpookWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch(uMsg)
  {
    case WM_TIMER:
      //
      // Kill the timer that invoked us
      //
      KillTimer(hWnd, IDT_SPOOK);

      //
      // Do the thing(tm)!
      //
      Spook();

      //
      // Set up a new timer for next spook attack
      //
      nSpooks--;
      if(nSpooks)
        SetTimer(hWnd, IDT_SPOOK, (rand()%MAX_TIME)+1, NULL);
      else
        goto Kill;
      return(0);

    case WM_CREATE:
      SetTimer(hWnd, IDT_SPOOK, (rand()%MAX_TIME)+1, NULL);
      return(0);

    case WM_DESTROY:
      KillTimer(hWnd, IDT_SPOOK);
Kill:
      MessageBox(hWnd, "Ok, I give up!", szAppCaption, MB_TASKMODAL|MB_ICONINFORMATION|MB_OK);
      PostQuitMessage(0);
      return(0);
  }
  return(DefWindowProc(hWnd, uMsg, wParam, lParam));
}

//
// InitApplication
//
// Purpose:
//    Initializes this application.
// In:
//    none
// Out:
//    TRUE = success, FALSE = failure
//
BOOL InitApplication(VOID)
{
  char szRunLine[512];
  LPSTR pFound, pScan;
  WNDCLASS wc;

  GetProfileString("windows", "run", "", szRunLine, sizeof(szRunLine));
  _fstrlwr(szRunLine);
  pFound = _fstrstr(szRunLine, szAppName);
  if(pFound != NULL)
  {
    pScan = pFound;
    while(*pScan != '\0' && *pScan != ' ')
      pScan++;
    while(*pScan != '\0' && *pScan == ' ')
      pScan++;
    GetProfileString("windows", "run", "", szRunLine, sizeof(szRunLine));
    lstrcpy(pFound, pScan);
    WriteProfileString("windows", "run", szRunLine);
  }

  __memset__(&wc, 0, sizeof(wc));
  wc.style = CS_HREDRAW|CS_VREDRAW;
  wc.lpfnWndProc = SpookWndProc;
  wc.hInstance = hAppInstance;
  wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
  wc.lpszMenuName = NULL;
  wc.lpszClassName = szClassName;
  RegisterClass(&wc);
  return(TRUE);
}

//
// InitInstance
//
// Purpose:
//    Initializes this instance of an application.
// In:
//    hPrevInstance = instance handle of a previous instance
//    pszCmdLine -> command line
//    nCmdShow = window visibility
// Out:
//    TRUE = success, FALSE = failure
//
#pragma argsused
BOOL InitInstance(HINSTANCE hPrevInstance, LPCSTR pszCmdLine, int nCmdShow)
{
  HWND hWnd;

  if(hPrevInstance != NULL)
  {
    hWnd = FindWindow(szClassName, NULL);
    SendMessage(hWnd, WM_DESTROY, 0, 0);
    return(FALSE);
  }

  hWnd = CreateWindow(szClassName, NULL, WS_POPUPWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hAppInstance, NULL);
  if(hWnd == NULL)
    return(FALSE);

  srand((unsigned)GetCurrentTime());
  nSpooks = (rand()%MAX_SPOOKS)+1;

  cxScreen = GetSystemMetrics(SM_CXSCREEN);
  cyScreen = GetSystemMetrics(SM_CYSCREEN);

  cxDelta = cxScreen/10;
  cyDelta = cyScreen/10;

  return(TRUE);
}

//
// WinMain
//
// Purpose:
//    Called by Windows upon application initialization.
// In:
//    hThisInstance = instance handle of this application
//    hPrevInstance = instance handle of a previous instance of this
//     application, or NULL if no previous instance executing at this time
//    lpszCmdLine -> command line arguments
//    nCmdShow = window show command
// Out:
//    return code - not used by Windows at this time
//
#pragma argsused
int PASCAL WinMain(HINSTANCE hThisInstance, HINSTANCE hPrevInstance, LPSTR pszCmdLine, int nCmdShow)
{
  MSG msg;

  hAppInstance = hThisInstance;
  if(hPrevInstance == NULL)
  {
    if(!InitApplication())
      return(-1);
  }

  if(!InitInstance(hPrevInstance, pszCmdLine, nCmdShow))
    return(-1);

  while(GetMessage(&msg, NULL, 0, 0))
  {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return(0);
}
