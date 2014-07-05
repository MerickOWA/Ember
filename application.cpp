#include "stdafx.h"
#include "application.h"
#include <chrono>

using namespace std::chrono;
using namespace Win32;
using namespace KennyKerr;
using namespace KennyKerr::Dxgi;
using namespace KennyKerr::Direct2D;
using namespace KennyKerr::Direct3D;

int Application::Run( HINSTANCE hinst, LPCTSTR title )
{
	_window = WindowCreation{
		WindowClass{ hinst, _T( "AppWindow" ) }
		.WndProc<Application, &Application::WndProc>()
		.Style( CS_HREDRAW | CS_VREDRAW )
		.Cursor( IDC_ARROW )
		.Register()
	}
	.Title( title )
	.Param( this )
	.Create();

		InitializeDX();
		CreateDeviceIndependentResources();
		CreateDeviceResources();

		auto timer = high_resolution_clock{};
		auto timerStart = timer.now(),
			timerLast = timerStart;

		// Main message loop:
		while ( _isRunning )
		{
			auto timerNow = timer.now();
			_elapsedTotalSecs = duration_cast<duration<double>>(timerNow - timerStart).count();
			_elapsedLastFrameSecs = duration_cast<duration<double>>(timerNow - timerLast).count();

			Update();
			Draw();
			PumpMessages();
		}

		return _exitCode;
}

void Application::Exit( int exitCode )
{
	_isRunning = false;
	_exitCode = exitCode;
}


void Application::InitializeDX()
{
	_d3dDevice = Direct3D::CreateDevice();
	_d3dContext = _d3dDevice.GetImmediateContext();

	SwapChainDescription1 description;
	description.SwapEffect = SwapEffect::Discard;

	auto dxgiFactory = _d3dDevice.GetDxgiFactory();
	_swapChain = dxgiFactory.CreateSwapChainForHwnd( _d3dDevice, _window.Get(), description );

	_d2dFactory = Direct2D::CreateFactory();
	_dwFactory = DirectWrite::CreateFactory();

	RenderTargetProperties renderTargetProps;
	renderTargetProps.PixelFormat.AlphaMode = AlphaMode::Premultiplied;

	_d2dDevice = _d2dFactory.CreateDevice( _d3dDevice );
	_d2dContext = _d2dDevice.CreateDeviceContext();
	_d2dContext.SetTarget( _d2dContext.CreateBitmapFromDxgiSurface( _swapChain ) );
}

void Application::OnWindowChanged()
{
	if ( _swapChain )
	{
		_d2dContext.SetTarget();
		_swapChain.ResizeBuffers();
		_d2dContext.SetTarget( _d2dContext.CreateBitmapFromDxgiSurface( _swapChain ) );
	}
}

LRESULT Application::WndProc( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam )
{
	switch ( msg )
	{
	case WM_DESTROY:
		Exit( 0 );
		return 0;

	case WM_SIZE:
		OnWindowChanged();
		break;

	case WM_PAINT:
		Draw();
		break;

	default:
		break;
	}

	return ::DefWindowProc( hwnd, msg, wparam, lparam );
}

