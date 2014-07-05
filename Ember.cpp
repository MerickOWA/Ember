// Ember.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Ember.h"
#include "dx.h"
#include "application.h"
#include "fileview.h"
using namespace KennyKerr;
using namespace KennyKerr::DirectWrite;
using namespace KennyKerr::Direct2D;
using namespace KennyKerr::Direct3D;

Color const COLOR_WHITE( 1.0f, 1.0f, 1.0f );
Color const COLOR_SkyBlue( 0.529f, 0.808f, 0.922f );

class EmberApplication : public Application
{
	TextFormat _textFormat;
	SolidColorBrush _whiteBrush;
	VertexShader _vertexShader;

	void CreateDeviceIndependentResources() override
	{
		_textFormat = _dwFactory.CreateTextFormat( L"Candara", 100.0f );
	}

	void CreateDeviceResources() override
	{
		_whiteBrush = _d2dContext.CreateSolidColorBrush( COLOR_WHITE );

		{
			fileview file( "SimpleVS.cso" );
			_vertexShader = _d3dDevice.CreateVertexShader( file.begin(), file.size() );
		}
	}

	virtual void Update() override
	{
	}

	void Draw()	override
	{
		_d2dContext.BeginDraw();
		_d2dContext.Clear( COLOR_SkyBlue );

		auto size = _d2dContext.GetSize();

		WCHAR text[] = L"Hello World";
		_d2dContext.DrawText( text, _countof( text ) - 1, _textFormat, RectF( 0, 0, size.Width, size.Height ), _whiteBrush );

		_d2dContext.FillEllipse( Direct2D::Ellipse{ Point2F{ size.Width / 2, size.Height / 2 }, 100.0, 100.0 }, _whiteBrush );

		_d2dContext.EndDraw();
		_swapChain.Present();
	}
};

int APIENTRY _tWinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nCmdShow )
{
	UNREFERENCED_PARAMETER( hPrevInstance );
	UNREFERENCED_PARAMETER( lpCmdLine );

	ComInitialize com;
	EmberApplication app;
	return app.Run( hInstance, _T( "Ember" ) );
}