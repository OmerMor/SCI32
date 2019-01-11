

class Main
{
public:
  static HINSTANCE hInstance;
  static HINSTANCE hPrevInstance;
  static LPSTR lpszCmdLine;
  static int nCmdShow;
  static int MessageLoop( void );
};

class Window
{
protected:
    HWND hWnd;
public:
    // Provide (read) access to the window's handle in case it is needed
    // elsewhere.
    HWND GetHandle( void ) { return hWnd; }

    // Pure virtual function makes Window an abstract class.
    virtual LRESULT WndProc( UINT iMessage, WPARAM wParam, LPARAM lParam ) = 0;
};

class MainWindow : public Window
{
public:

    // Do not create unless previously registered.
    MainWindow( void );
    LRESULT WndProc( UINT iMessage, WPARAM wParam, LPARAM lParam );

};

