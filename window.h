#pragma once

#include <exception>
#include <memory>
#include <Windows.h>

namespace Win32
{
	EXTERN_C IMAGE_DOS_HEADER __ImageBase;
	#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)

	inline void PumpMessages()
	{
		MSG msg;
		while ( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) != FALSE )
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
	}

	template < typename T, typename LRESULT( T::*Func )(HWND, UINT, WPARAM, LPARAM) >
	struct WndProcThunk
	{
		static LRESULT WINAPI Callback( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam )
		{
			T *pThis; // our "this" pointer will go here
			if ( msg == WM_NCCREATE ) {

				// Recover the "this" pointer which was passed as a parameter
				// to CreateWindow(Ex).
				auto lpcs = reinterpret_cast<LPCREATESTRUCT>(lparam);
				pThis = static_cast<T *>(lpcs->lpCreateParams);

				// Put the value in a safe place for future use
				SetWindowLongPtr( hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis) );
			}
			else {

				// Recover the "this" pointer from where our WM_NCCREATE handler
				// stashed it.
				pThis = reinterpret_cast<T*>(GetWindowLongPtr( hwnd, GWLP_USERDATA ));
			}

			if ( pThis != nullptr ) {
				// Now that we have recovered our "this" pointer, let the
				// member function finish the job.
				return (pThis->*Func)( hwnd, msg, wparam, lparam );
			}

			// We don't know what our "this" pointer is, so just do the default
			// thing. Hopefully, we didn't need to customize the behavior yet.
			return DefWindowProc( hwnd, msg, wparam, lparam );
		}
	};

	class WindowClass
	{
		UINT      _style;
		WNDPROC   _wndProc;
		int       _clsExtra;
		int       _wndExtra;
		HINSTANCE _hinstance;
		HICON     _hicon;
		HCURSOR   _hcursor;
		HBRUSH    _background;
		LPCTSTR   _menuName;
		LPCTSTR   _className;
		HICON     _hiconSm;
	public:
		static LRESULT WINAPI DummyCallback( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam )
		{
			return 0L;
		}

		WindowClass( HINSTANCE hinstance, LPCTSTR className )
			: _style( 0 ),
			_wndProc( nullptr ),
			_clsExtra( 0 ),
			_wndExtra( 0 ),
			_hinstance( hinstance ),
			_hicon( nullptr ),
			_hcursor( nullptr ),
			_background( nullptr ),
			_menuName( nullptr ),
			_className( className ),
			_hiconSm( nullptr )
		{
		}

		WindowClass &Style( UINT value )        { _style = value; return *this; }
		WindowClass &WndProc( WNDPROC value )   { _wndProc = value; return *this; }
		WindowClass &ClassExtra( int value )    { _clsExtra = value; return *this; }
		WindowClass &WindowExtra( int value )   { _wndExtra = value; return *this; }
		WindowClass &Icon( HICON value )        { _hicon = value; return *this; }
		WindowClass &Cursor( HCURSOR value )    { _hcursor = value; return *this; }
		WindowClass &Background( HBRUSH value ) { _background = value; return *this; }
		WindowClass &MenuName( LPCTSTR value )  { _menuName = value; return *this; }
		WindowClass &IconSmall( HICON value )   { _hiconSm = value; return *this; }

		WindowClass &Cursor( LPCTSTR name, HINSTANCE hinst = nullptr )
		{
			return Cursor( ::LoadCursor( hinst, name ) );
		}

		WindowClass &Background( UINT value )
		{
			return Background(reinterpret_cast<HBRUSH>(value + 1));
		}

		template < typename T, typename LRESULT (T::*Func)(HWND, UINT, WPARAM, LPARAM)>
		WindowClass &WndProc()
		{
			return WndProc( &WndProcThunk<T, Func>::Callback );
		}

		WindowClass &Register()
		{
			WNDCLASSEX wc = { sizeof( wc ) };
			wc.style = _style;
			wc.lpfnWndProc = _wndProc;
			wc.cbClsExtra = _clsExtra;
			wc.cbWndExtra = _wndExtra;
			wc.hInstance = _hinstance;
			wc.hIcon = _hicon;
			wc.hCursor = _hcursor;
			wc.hbrBackground = _background;
			wc.lpszMenuName = _menuName;
			wc.lpszClassName = _className;
			wc.hIconSm = _hiconSm;

			auto result = ::RegisterClassEx( &wc );
			if ( result == 0 )
			{
				auto error = GetLastError();
				throw std::exception( "Class registration failed!" );
			}

			return *this;
		}

		friend class WindowCreation;
	};

	class Window
	{
	public:

		Window( HWND hwnd = nullptr )
			: handle( hwnd )
		{
		}

		void Destroy()
		{
			DestroyWindow( this->handle );
		}

		HWND Get() const {
			return this->handle;
		}

	private:
		HWND handle;
	};

	class WindowCreation
	{
		DWORD _exstyle;
		LPCTSTR _className;
		LPCTSTR _title;
		int _x;
		int _y;
		int _width;
		int _height;
		HWND _parent;
		HMENU _menu;
		HINSTANCE _hinstance;
		DWORD _style;
		LPVOID _param;

	public:
		WindowCreation( HINSTANCE hinstance, LPCTSTR className )
			: _exstyle( 0 ),
			_className( className ),
			_title( nullptr ),
			_x( CW_USEDEFAULT ),
			_y( CW_USEDEFAULT ),
			_width( CW_USEDEFAULT ),
			_height( CW_USEDEFAULT ),
			_parent( nullptr ),
			_menu( nullptr ),
			_hinstance( hinstance ),
			_style( WS_OVERLAPPEDWINDOW | WS_VISIBLE ),
			_param( nullptr )
		{
		}

		WindowCreation( const WindowClass &wndClass )
			: WindowCreation( wndClass._hinstance, wndClass._className )
		{
		}

		WindowCreation& ExStyle( DWORD value ) { _exstyle = value; return *this; }
		WindowCreation& Title( LPCTSTR value ) { _title = value; return *this; }
		WindowCreation& X( int value )         { _x = value; return *this; }
		WindowCreation& Y( int value )         { _y = value; return *this; }
		WindowCreation& Width( int value )     { _width = value; return *this; }
		WindowCreation& Height( int value )    { _height = value; return *this; }
		WindowCreation& Parent( HWND value )   { _parent = value; return *this; }
		WindowCreation& Menu( HMENU value )    { _menu = value; return *this; }
		WindowCreation& Style( DWORD value )   { _style = value; return *this; }
		WindowCreation& Param( LPVOID value )  { _param = value; return *this; }

		Window Create()
		{
			auto hwnd = ::CreateWindowEx( _exstyle, _className, _title, _style, _x, _y, _width, _height, _parent, _menu, _hinstance, _param );

			return{ hwnd };
		}
	};
}
