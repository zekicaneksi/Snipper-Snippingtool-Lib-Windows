#ifndef SNIPPER_H
#define SNIPEPR_H

#include <windows.h>

using namespace std;

class Snipper{

public:

  int width; //width of the latest snipshot
  int height; //height of the latest snipshot

  // Creates the window, sets and allocates global variables
  Snipper();
  
  // Destroys the window and deallocates variables
  ~Snipper();

  /* Takes a screenshot and makes user take a snipshot and returns a pointer to RGB pixel array of the snipshot 
    delay is the time in miliseconds passed between function call and snipshot. It's simply Sleep(delay) 
    NOTE: If user presses ESC key instead of taking a snipshot, NULL is returned*/
  const RGBQUAD* TakeASnip(int delay);

private:

  /* GLOBALS */
  int nScreenWidth, nScreenHeight;

  HWND windowHandle; //handle of window
  HPEN hPen; // pen of window

  HBITMAP hBitmap; //handle of the screenshot's bitmap (compatible with screen)
  BITMAP bm; //bitmap of the screenshot (compatible with window)
  BITMAPINFO bmi; // used for bitmap to rgb
  RGBQUAD* pPixels = NULL; // RGB pixel array of the screenshot

  POINT cornerFirst, cornerSecond; //corners of the snipshot
  
  RGBQUAD* pixelsSnip = NULL;  //array of snipshot's rgb values
  RECT rect; // coordinates of the snipshot rectangle
  RECT memRect; // coordinates of the previus snipshot rectangle (used to clean up the previous rectangle when WM_MOUSEMOVE)

  /* flags for operations */
  bool draw = true; //flag for drawing the screenshot into window
  bool escape=false; //flag for breaking GetMessage loop
  bool cancel=false; //flag used if user pressed ESC key
  bool animate=false; //flag used to activate WM_MOUSEMOVE
  
  
  /* Creates the window and pen*/
  void SetUpWindow();

  /*Takes a screenshot */
  void TakeScreenShot();

  /* Shows the window to user */
  void BringWindow();

  /* Windows Procedure method */  
  LRESULT CALLBACK ActualWndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

  /* kind of a pointer that calls the actual Windows Procedure function (ActualWndProc) */
  static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
  static Snipper *m_pInstance;
};

#endif