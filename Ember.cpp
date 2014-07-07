// Ember.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Ember.h"
#include "dx.h"
#include "application.h"
#include "fileview.h"
#include <DirectXMath.h>
using namespace DirectX;
using namespace KennyKerr;
using namespace KennyKerr::DirectWrite;
using namespace KennyKerr::Direct2D;
using namespace KennyKerr::Direct3D;
using namespace KennyKerr::Dxgi;

Color const COLOR_WHITE( 1.0f, 1.0f, 1.0f );
Color const COLOR_SKYBLUE( 0.529f, 0.808f, 0.922f );

struct ModelViewProjection
{
	XMFLOAT4X4 model;
	XMFLOAT4X4 view;
	XMFLOAT4X4 projection;
};

// Used to send per-vertex data to the vertex shader.
struct VertexPositionColor
{
	XMFLOAT3 pos;
	XMFLOAT3 color;
};

class EmberApplication : public Application
{
	TextFormat _textFormat;
	SolidColorBrush _whiteBrush;
	VertexShader _vertexShader;
	InputLayout _inputLayout;
	PixelShader _pixelShader;
	Buffer _constantBuffer;
	Buffer _vertexBuffer;
	Buffer _indexBuffer;
	ModelViewProjection _constants;
	unsigned _indexCount;

	void Initialize() override
	{
		_textFormat = _dwFactory.CreateTextFormat( L"Candara", 100.0f );
	}

	void OnDeviceAquired() override
	{
		__super::OnDeviceAquired();

		_whiteBrush = _d2dContext.CreateSolidColorBrush( COLOR_WHITE );

		{
			fileview file( "SimpleVS.cso" );

			const InputElementDescription vertexDesc[] =
			{
				InputElementDescription{ Format::R32G32B32_FLOAT, "POSITION", 0, 0, 0 },
				InputElementDescription{ Format::R32G32B32_FLOAT, "COLOR", 0, 0, 12 },
			};

			_vertexShader = _d3dDevice.CreateVertexShader( file.begin(), file.size() );
			_inputLayout = _d3dDevice.CreateInputLayout( vertexDesc, _countof( vertexDesc ), file.begin(), file.size() );
		}

		{
			fileview file( "SimplePS.cso" );
			_pixelShader = _d3dDevice.CreatePixelShader( file.begin(), file.size() );
		}

		_constantBuffer = _d3dDevice.CreateBuffer( BufferDescription{ sizeof( ModelViewProjection ), BindFlag::ConstantBuffer } );

		const VertexPositionColor cubeVertices[] =
		{
			{ XMFLOAT3( -0.5f, -0.5f, -0.5f ), XMFLOAT3( 0.0f, 0.0f, 0.0f ) },
			{ XMFLOAT3( -0.5f, -0.5f, 0.5f ), XMFLOAT3( 0.0f, 0.0f, 1.0f ) },
			{ XMFLOAT3( -0.5f, 0.5f, -0.5f ), XMFLOAT3( 0.0f, 1.0f, 0.0f ) },
			{ XMFLOAT3( -0.5f, 0.5f, 0.5f ), XMFLOAT3( 0.0f, 1.0f, 1.0f ) },
			{ XMFLOAT3( 0.5f, -0.5f, -0.5f ), XMFLOAT3( 1.0f, 0.0f, 0.0f ) },
			{ XMFLOAT3( 0.5f, -0.5f, 0.5f ), XMFLOAT3( 1.0f, 0.0f, 1.0f ) },
			{ XMFLOAT3( 0.5f, 0.5f, -0.5f ), XMFLOAT3( 1.0f, 1.0f, 0.0f ) },
			{ XMFLOAT3( 0.5f, 0.5f, 0.5f ), XMFLOAT3( 1.0f, 1.0f, 1.0f ) },
		};

		_vertexBuffer = _d3dDevice.CreateBuffer( BufferDescription{ sizeof( cubeVertices), BindFlag::VertexBuffer }, SubresourceData{ &cubeVertices } );

		// Load mesh indices. Each trio of indices represents
		// a triangle to be rendered on the screen.
		// For example: 0,2,1 means that the vertices with indexes
		// 0, 2 and 1 from the vertex buffer compose the 
		// first triangle of this mesh.
		static const unsigned short cubeIndices[] =
		{
			0, 2, 1, // -x
			1, 2, 3,

			4, 5, 6, // +x
			5, 7, 6,

			0, 1, 5, // -y
			0, 5, 4,

			2, 6, 7, // +y
			2, 7, 3,

			0, 4, 6, // -z
			0, 6, 2,

			1, 3, 7, // +z
			1, 7, 5,
		};

		_indexCount = _countof( cubeIndices );
		_indexBuffer = _d3dDevice.CreateBuffer( BufferDescription{ sizeof( cubeIndices ), BindFlag::IndexBuffer }, SubresourceData{ &cubeIndices } );

		OnWindowChanged();
	}

	void OnWindowChanged() override
	{
		__super::OnWindowChanged();

		auto outputSize = _d2dContext.GetSize();
		auto aspectRatio = outputSize.Width / outputSize.Height;
		auto fovAngleY = 70.0f * XM_PI / 180.0f;

		// Note that the OrientationTransform3D matrix is post-multiplied here
		// in order to correctly orient the scene to match the display orientation.
		// This post-multiplication step is required for any draw calls that are
		// made to the swap chain render target. For draw calls to other targets,
		// this transform should not be applied.

		// This sample makes use of a right-handed coordinate system using row-major matrices.
		auto perspectiveMatrix = XMMatrixPerspectiveFovRH( fovAngleY, aspectRatio, 0.01f, 100.0f );

		XMStoreFloat4x4( &_constants.projection, XMMatrixTranspose( perspectiveMatrix ) );

		// Eye is at (0,0.7,1.5), looking at point (0,-0.1,0) with the up-vector along the y-axis.
		static const XMVECTORF32 eye = { 0.0f, 0.7f, 1.5f, 0.0f };
		static const XMVECTORF32 at = { 0.0f, -0.1f, 0.0f, 0.0f };
		static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

		XMStoreFloat4x4( &_constants.view, XMMatrixTranspose( XMMatrixLookAtRH( eye, at, up ) ) );
	}

	virtual void Update() override
	{
		const auto degreesPerSecond = 45.0f;
		auto radiansPerSecond = XMConvertToRadians( degreesPerSecond );
		auto totalRotation = static_cast<float>(_elapsedTotalSecs * radiansPerSecond);
		auto radians = fmod( totalRotation, XM_2PI );
		XMStoreFloat4x4( &_constants.model, XMMatrixTranspose( XMMatrixRotationY( radians ) ) );
	}

	void Draw()	override
	{
		_d2dContext.BeginDraw();
		_d2dContext.Clear( COLOR_SKYBLUE );

		auto size = _d2dContext.GetSize();

		WCHAR text[] = L"Hello World";
		_d2dContext.DrawText( text, _countof( text ) - 1, _textFormat, RectF( 0, 0, size.Width, size.Height ), _whiteBrush );

		_d2dContext.FillEllipse( Direct2D::Ellipse{ Point2F{ size.Width / 2, size.Height / 2 }, 100.0, 100.0 }, _whiteBrush );

		_d3dContext.OMSetRenderTargets( 1, &_d3dRenderTargetView, _d3dDepthStencilView );

		_d3dContext.UpdateSubresource( _constantBuffer, &_constants );

		Buffer vertexBuffers[] = { _vertexBuffer };
		unsigned strides[] = { sizeof( VertexPositionColor ) };
		unsigned offsets[] = { 0 };
		_d3dContext.IASetVertexBuffers( 0, 1, vertexBuffers, strides, offsets );

		_d3dContext.IASetIndexBuffer( _indexBuffer );

		_d3dContext.IASetPrimitiveTopology( PrimitiveTopology::TriangleList );

		_d3dContext.IASetInputLayout( _inputLayout );

		_d3dContext.VSSetShader( _vertexShader );

		_d3dContext.VSSetConstantBuffers( 0, 1, &_constantBuffer );

		_d3dContext.PSSetShader( _pixelShader );

		_d3dContext.DrawIndexed( _indexCount );

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