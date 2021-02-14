#include "snipper.hpp"

//set instance to null
Snipper * Snipper::m_pInstance = NULL;

/* Creates the window, sets and allocates global variables */
Snipper::Snipper(){

  m_pInstance = this;
  nScreenWidth = GetSystemMetrics(SM_CXSCREEN);
	nScreenHeight = GetSystemMetrics(SM_CYSCREEN);

  // creates the window and pen
  SetUpWindow();
  
  // set the pPixels array for screenshot
  pPixels = new RGBQUAD[nScreenWidth*nScreenHeight];

  // setting bitmap info for transfering bitmap to RGB pixel array
  bmi = { 0 };
	bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
	bmi.bmiHeader.biWidth = nScreenWidth;
	bmi.bmiHeader.biHeight = nScreenHeight * -1;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;

  // initial rect values
  rect.bottom=nScreenHeight/2;
  rect.top=nScreenHeight/2;
  rect.left=nScreenWidth/2;
  rect.right=nScreenWidth/2;

}

 // deallocates memory and destroys the window
Snipper::~Snipper(){
  delete[] pixelsSnip;
  delete[] pPixels;
  DeleteObject(hPen);
  DestroyWindow(windowHandle);
  UnregisterClass(TEXT("Snipper"),NULL);
}

/* Takes a screenshot and makes user take a snipshot and returns a pointer to RGB pixel array of the snipshot 
    delay is the time in miliseconds passed between function call and snipshot. It's simply Sleep(delay) 
    NOTE: If user presses ESC key instead of taking a snipshot, NULL is returned*/
const RGBQUAD* Snipper::TakeASnip(int delay){

  //reset the snipshot array
  delete[] pixelsSnip;
  pixelsSnip=NULL;

  Sleep(delay);
  TakeScreenShot();
  UpdateWindow(windowHandle); //Sends WM_PAINT message to window (updates the window with the screenshot)
  BringWindow(); // brings window to the front

  
  // message loop

  MSG messages;
  int ret;
  
  while(true)
  {
    ret = GetMessage(&messages, NULL, 0, 0);

    //escape is set when user took a snipshot
    //cancel is set when user presses ESC key

    //resets the flags and breaks the message loop
    if(escape  || cancel){
      draw=true;
      escape=false;
      break;
    }
    TranslateMessage(&messages);
    DispatchMessage(&messages);
  }

  //hide window
  ShowWindow(windowHandle,SW_HIDE);

  // if user pressed ESC key, returns NULL
  if(cancel){
    cancel=true;
    return NULL;
  }

  //Setting snipshot's width and height
  width = rect.right-rect.left-1;
  height = rect.bottom-rect.top-1;

  //creates a RGBQUAD array for snipshot
  pixelsSnip = new RGBQUAD[width*height];

  //fills the snipshot's pixel array from screenshot's pixel array
  int count=0;
  for(int y=rect.top+1;y<rect.bottom;y++){
    for(int x=rect.left+1;x<rect.right;x++){
      pixelsSnip[count]=pPixels[y*nScreenWidth+x];
      count++;
    }
  }

  // returns the pointer to the snipshot's RGB pixel array
  return pixelsSnip;
}

/* Creates a window */
void Snipper::SetUpWindow(){ 

  // creating the window's class
  WNDCLASS windowClass={0};
	windowClass.hbrBackground=(HBRUSH)GetStockObject(WHITE_BRUSH);
	windowClass.hCursor=LoadCursor(NULL, IDC_CROSS);
	windowClass.hInstance=NULL;
	windowClass.lpfnWndProc=WndProc;
	windowClass.lpszClassName=TEXT("Snipper");
	windowClass.style=CS_HREDRAW | CS_VREDRAW;
	if (!RegisterClass(&windowClass))
    MessageBoxA(NULL, "Could not register class", "Error", MB_OK);

  // creating the window
	windowHandle = CreateWindowA(
    "Snipper",
    "Snipper (Press ESC to Quit)",
    WS_POPUP, //borderless
    0, //x coordinate of window start point
    0, //y start point
    nScreenWidth, //width of window
    nScreenHeight, //height of the window
    NULL, //handles and such, not needed
    NULL,
    NULL,
    NULL
  );

  //creating the pen that's used for snipshot's frame
  hPen=CreatePen(PS_SOLID,5,RGB(247,94,0));

}

/*Takes a screenshot and sets hBitmap and bm*/
void Snipper::TakeScreenShot(){

  //reset hBitmap
  DeleteObject(hBitmap);

  // get the device context of the screen
  HDC hScreenDC = GetDC(NULL);  
  // and a device context to put it in
  HDC hMemoryDC = CreateCompatibleDC(hScreenDC);

  // creates compatible bitmap
  hBitmap = CreateCompatibleBitmap(hScreenDC, nScreenWidth, nScreenHeight);

  // get a new bitmap
  HBITMAP hOldBitmap = (HBITMAP) SelectObject(hMemoryDC, hBitmap);

  BitBlt(hMemoryDC, 0, 0, nScreenWidth, nScreenHeight, hScreenDC, 0, 0, SRCCOPY);

  // saving the screenshot in hBitmap
  hBitmap = (HBITMAP) SelectObject(hMemoryDC, hOldBitmap);

  // transfering screenshot from bitmap (hBitmap) to rgb pixel array (pPixels)
  GetDIBits(
  hMemoryDC,
  hBitmap,
  0,
  nScreenHeight,
  pPixels,
  &bmi,
  DIB_RGB_COLORS
  );

  // setting bm via hBitmap
  GetObject(hBitmap, sizeof(bm), &bm);

  // clean up
  DeleteDC(hMemoryDC);
  ReleaseDC(NULL,hScreenDC);

}

/* Shows the window to user */
void Snipper::BringWindow(){

  //Activates and sets the windows as foreground
  SetForegroundWindow(windowHandle);

  //Bring window to front
  SetWindowPos(
    windowHandle,       // handle to window
    HWND_TOPMOST,  // placement-order handle
    0,     // horizontal position
    0,      // vertical position
    0,  // width
    0, // height
    SWP_SHOWWINDOW|SWP_NOSIZE|SWP_NOMOVE// window-positioning options
  );

}

/* kind of a pointer that calls the actual Windows Procedure function (ActualWndProc) */
LRESULT CALLBACK Snipper::WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam){
  return m_pInstance->ActualWndProc(hwnd, message, wparam, lparam);
}

/* Windows Procedure method */
LRESULT CALLBACK Snipper::ActualWndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
  switch (message)
  {

    PAINTSTRUCT ps; //not used, created only to provide to BeginPaint()
    HDC hdc; // used to handle DC that's returned from BeginPaint()

    case WM_LBUTTONDOWN:{
    
    // sets the first corner of the snipshot
    cornerFirst.x = LOWORD(lparam); 
    cornerFirst.y = HIWORD(lparam);

    // activates the flag that's used for WM_MOUSEMOVE
    animate=true;

    }
    return 0;

    case WM_MOUSEMOVE:{

      if(animate){
        
        //sets the second corner
        cornerSecond.x=LOWORD(lparam);
        cornerSecond.y=HIWORD(lparam);

        //sets the memRect
        memRect=rect;

        //setting rect
        if(cornerSecond.x>=cornerFirst.x){
          rect.right=cornerSecond.x+1;
          rect.left=cornerFirst.x-1;
        }
        else{
          rect.right = cornerFirst.x+1;
          rect.left=cornerSecond.x-1;
        }
        if(cornerSecond.y>=cornerFirst.y){
          rect.bottom=cornerSecond.y+1;
          rect.top=cornerFirst.y-1;
        }
        else{
          rect.bottom=cornerFirst.y+1;
          rect.top=cornerSecond.y-1;
        }

        //setting memRect. +10's are for frame
        if(rect.right>memRect.right)
          memRect.right=rect.right+10;
        else
          memRect.right+=10;
        
        if(rect.left<memRect.left)
          memRect.left=rect.left-10;
        else
          memRect.left-=10;

        if(rect.top<memRect.top)
          memRect.top=rect.top-10;
        else
          memRect.top-=10;

        if(rect.bottom>memRect.bottom)
          memRect.bottom=rect.bottom+10;
        else
          memRect.bottom+=10;

        // sending WM_PAINT message to window to redraw the given rectangle area (from screenshot)
        InvalidateRect(hwnd,&memRect,false);
      }

    }
    return 0;

    case WM_LBUTTONUP:{

      // setting the second corner
      cornerSecond.x=LOWORD(lparam);
      cornerSecond.y=HIWORD(lparam);

      //setting rect
      if(cornerSecond.x>=cornerFirst.x){
        rect.right=cornerSecond.x+1;
        rect.left=cornerFirst.x-1;
      }
      else{
        rect.right = cornerFirst.x+1;
        rect.left=cornerSecond.x-1;
      }
      if(cornerSecond.y>=cornerFirst.y){
        rect.bottom=cornerSecond.y+1;
        rect.top=cornerFirst.y-1;
      }
      else{
        rect.bottom=cornerFirst.y+1;
        rect.top=cornerSecond.y-1;
      }

      // sending WM_PAINT message to window to redraw the given rectangle area (from screenshot)
      InvalidateRect(hwnd,&rect,false);

      // setting flag to break message loop
      escape=true;

      // resetting flag to not draw on WM_MOUSEMOVE
      animate=false;

    }
    return 0;

    case WM_PAINT:{

      //drawing the screenshot into window
      if(draw==true){

        // getting windows device context
        hdc = BeginPaint(hwnd, &ps);

        // creating DC compatible with window to hold bitmap
        HDC hdcMem = CreateCompatibleDC(hdc);

        // creating a HBITMAP to hold the previous bitmap and selecting hdcMem with hBitmap(handle to the screenshot's bitmap)
        HBITMAP hbmOld = (HBITMAP) SelectObject(hdcMem, hBitmap);

        // bitmap transfer to window from screenshot's bitmap
        BitBlt(hdc, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCCOPY);

        // cleaning up
        SelectObject(hdcMem, hbmOld);
        DeleteDC(hdcMem);
        
        EndPaint(hwnd, &ps);

        draw=false;
      
      }
      else {

        // getting windows device context
        hdc = BeginPaint(hwnd, &ps);

        // selecting pen used for frame with window's DC
        SelectObject(hdc,hPen);

        // creating DC compatible with window to hold bitmap
        HDC hdcMem = CreateCompatibleDC(hdc);

        // creating a HBITMAP to hold the previous bitmap and selecting hdcMem with hBitmap(handle to the screenshot's bitmap)
        HBITMAP hbmOld = (HBITMAP) SelectObject(hdcMem, hBitmap);

        // redrawing the given rectangle of window area from screenshot's bitmap (this is used to clean the frame rectangle from previous WM_MOUSEMOVE) 
        BitBlt(hdc, memRect.left, memRect.top, (memRect.right-memRect.left)+1, (memRect.bottom-memRect.top)+1, hdcMem, memRect.left, memRect.top, SRCCOPY);

        // cleaning up
        SelectObject(hdcMem, hbmOld);
        DeleteDC(hdcMem);

        // setting current position of pen
        MoveToEx(hdc, cornerSecond.x, cornerSecond.y, NULL); 

        // drawing a new frame between corners
        LineTo(hdc,cornerSecond.x,cornerFirst.y);
        LineTo(hdc,cornerFirst.x,cornerFirst.y);
        LineTo(hdc,cornerFirst.x,cornerSecond.y);
        LineTo(hdc,cornerSecond.x,cornerSecond.y);

        EndPaint(hwnd, &ps);

      }

    }
    return 0;

    // to cancel taking snipshot, press ESC 
    case WM_CHAR:
      if (wparam==VK_ESCAPE)
      {
        // flag, used to break message loop
        cancel=true;
      }
    return 0;

    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;
    default:
      return DefWindowProc(hwnd, message, wparam, lparam);
  }
}
