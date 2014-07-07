#pragma once

#include "window.h"
#include "dx.h"

class Application
{
public:

	Application() :
		_initialized( false ),
		_isRunning( true ),
		_exitCode( -1 )
	{
	}


	int Run( HINSTANCE hinst, LPCTSTR title );
	void Exit( int exitCode );

protected:

	virtual void Initialize() {}
	virtual void OnDeviceAquired() {}
	virtual void OnDeviceLost() {}
	virtual void OnWindowChanged();

	virtual void Update() {}
	virtual void Draw() {}

	Win32::Window _window;

	KennyKerr::Direct3D::Device1 _d3dDevice;
	KennyKerr::Direct3D::DeviceContext _d3dContext;
	KennyKerr::Direct3D::RenderTargetView _d3dRenderTargetView;
	KennyKerr::Direct3D::DepthStencilView _d3dDepthStencilView;

	KennyKerr::Direct2D::Factory1 _d2dFactory;
	KennyKerr::Direct2D::Device _d2dDevice;
	KennyKerr::Direct2D::DeviceContext _d2dContext;

	KennyKerr::DirectWrite::Factory2 _dwFactory;

	KennyKerr::Dxgi::SwapChain1 _swapChain;

	double _elapsedTotalSecs;
	double _elapsedLastFrameSecs;

private:
	bool _initialized;
	bool _isRunning;
	int _exitCode;

	void InitializeDX();

	LRESULT WndProc( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam );
};
