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

Color const COLOR_WHITE(1.0f, 1.0f, 1.0f);
Color const COLOR_SKYBLUE(0.529f, 0.808f, 0.922f);
Color const COLOR_LIGHTGREY(0.75f, 0.75f, 0.75f);

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
    RasterizerState _rasterizerState;
    VertexShader _vertexShader;
    InputLayout _inputLayout;
    GeometryShader _geometryShader;
    PixelShader _pixelShader;
    Buffer _constantBuffer;
    Buffer _vertexBuffer;
    Buffer _indexBuffer;
    ModelViewProjection _constants;
    unsigned _indexCount;

    void Initialize() override
    {
    }

    void OnDeviceAquired() override
    {
        __super::OnDeviceAquired();

        _rasterizerState = _d3dDevice.CreateRasterizerState(RasterizerDescription{ Direct3D::FillMode::Wireframe });

        {
            fileview file("SimpleVS.cso");

            const InputElementDescription vertexDesc[] =
            {
                InputElementDescription { Format::R32G32B32_FLOAT, "POSITION", 0, 0, 0 },
                InputElementDescription { Format::R32G32B32_FLOAT, "COLOR", 0, 0, 12 },
            };

            _vertexShader = _d3dDevice.CreateVertexShader(file.begin(), file.size());
            _inputLayout = _d3dDevice.CreateInputLayout(vertexDesc, _countof(vertexDesc), file.begin(), file.size());
        }

        {
            fileview file("SimpleGS.cso");
            _geometryShader = _d3dDevice.CreateGeometryShader(file.begin(), file.size());
        }

        {
            fileview file("SimplePS.cso");
            _pixelShader = _d3dDevice.CreatePixelShader(file.begin(), file.size());
        }

        _constantBuffer = _d3dDevice.CreateBuffer(BufferDescription { sizeof(ModelViewProjection), BindFlag::ConstantBuffer });

        auto icosahedren_rings_lat = atan(0.5f);
        auto y = sin(icosahedren_rings_lat);
        auto ring_radius = cos(icosahedren_rings_lat);
        auto ring_rads = XMConvertToRadians(36.0f);

        const VertexPositionColor icosahedrenVertices[] =
        {
            { XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f) },

            { XMFLOAT3(ring_radius*sin(0 * ring_rads), y, ring_radius*cos(0 * ring_rads)), XMFLOAT3(1.0f, 0.0f, 0.0f) },
            { XMFLOAT3(ring_radius*sin(1 * ring_rads), -y, ring_radius*cos(1 * ring_rads)), XMFLOAT3(1.0f, 0.5f, 0.0f) },
            { XMFLOAT3(ring_radius*sin(2 * ring_rads), y, ring_radius*cos(2 * ring_rads)), XMFLOAT3(1.0f, 1.0f, 0.0f) },
            { XMFLOAT3(ring_radius*sin(3 * ring_rads), -y, ring_radius*cos(3 * ring_rads)), XMFLOAT3(0.0f, 1.0f, 0.0f) },
            { XMFLOAT3(ring_radius*sin(4 * ring_rads), y, ring_radius*cos(4 * ring_rads)), XMFLOAT3(0.0f, 1.0f, 1.0f) },
            { XMFLOAT3(ring_radius*sin(5 * ring_rads), -y, ring_radius*cos(5 * ring_rads)), XMFLOAT3(0.0f, 0.5f, 1.0f) },
            { XMFLOAT3(ring_radius*sin(6 * ring_rads), y, ring_radius*cos(6 * ring_rads)), XMFLOAT3(0.0f, 0.0f, 1.0f) },
            { XMFLOAT3(ring_radius*sin(7 * ring_rads), -y, ring_radius*cos(7 * ring_rads)), XMFLOAT3(0.5f, 0.0f, 1.0f) },
            { XMFLOAT3(ring_radius*sin(8 * ring_rads), y, ring_radius*cos(8 * ring_rads)), XMFLOAT3(1.0f, 0.0f, 1.0f) },
            { XMFLOAT3(ring_radius*sin(9 * ring_rads), -y, ring_radius*cos(9 * ring_rads)), XMFLOAT3(1.0f, 0.0f, 0.5f) },

            { XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) },
        };


        _vertexBuffer = _d3dDevice.CreateBuffer(BufferDescription { sizeof(icosahedrenVertices), BindFlag::VertexBuffer }, SubresourceData { &icosahedrenVertices });

        static const unsigned short icosahedrenIndices[] =
        {
            0, 3, 1,
            0, 5, 3,
            0, 7, 5,
            0, 9, 7,
            0, 1, 9,
            1, 3, 2,
            2, 3, 4,
            3, 5, 4,
            4, 5, 6,
            5, 7, 6,
            6, 7, 8,
            7, 9, 8,
            8, 9, 10,
            9, 1, 10,
            10, 1, 2,
            11, 2, 4,
            11, 4, 6,
            11, 6, 8,
            11, 8, 10,
            11, 10, 2,
        };

        _indexCount = _countof(icosahedrenIndices);
        _indexBuffer = _d3dDevice.CreateBuffer(BufferDescription { sizeof(icosahedrenIndices), BindFlag::IndexBuffer }, SubresourceData { &icosahedrenIndices });
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
        auto perspectiveMatrix = XMMatrixPerspectiveFovRH(fovAngleY, aspectRatio, 0.01f, 100.0f);

        XMStoreFloat4x4(&_constants.projection, XMMatrixTranspose(perspectiveMatrix));

        // Eye is at (0,0.7,1.5), looking at point (0,-0.1,0) with the up-vector along the y-axis.
        const XMVECTORF32 eye = { 0.0f, 0.7f, 2.5f, 0.0f };
        const XMVECTORF32 at = { 0.0f, -0.1f, 0.0f, 0.0f };
        const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

        XMStoreFloat4x4(&_constants.view, XMMatrixTranspose(XMMatrixLookAtRH(eye, at, up)));
    }

    virtual void Update() override
    {
        const auto degreesPerSecond = 45.0f;
        auto radiansPerSecond = XMConvertToRadians(degreesPerSecond);
        auto totalRotation = static_cast<float>(_elapsedTotalSecs * radiansPerSecond);
        auto radians = fmod(totalRotation, XM_2PI);
        XMStoreFloat4x4(&_constants.model, XMMatrixTranspose(XMMatrixRotationY(radians)));
    }

    void Draw() override
    {
        _d2dContext.BeginDraw();

        _d3dContext.ClearRenderTargetView(_d3dRenderTargetView, COLOR_LIGHTGREY);
        _d3dContext.ClearDepthStencilView(_d3dDepthStencilView);

        _d3dContext.UpdateSubresource(_constantBuffer, &_constants);

        _d3dContext.RSSetState(_rasterizerState);

        Buffer vertexBuffers[] = { _vertexBuffer };
        unsigned strides[] = { sizeof(VertexPositionColor) };
        unsigned offsets[] = { 0 };
        _d3dContext.IASetVertexBuffers(0, 1, vertexBuffers, strides, offsets);

        _d3dContext.IASetIndexBuffer(_indexBuffer);

        _d3dContext.IASetPrimitiveTopology(PrimitiveTopology::TriangleList);

        _d3dContext.IASetInputLayout(_inputLayout);

        _d3dContext.VSSetShader(_vertexShader);

        _d3dContext.VSSetConstantBuffers(0, 1, &_constantBuffer);

        _d3dContext.GSSetShader(_geometryShader);

        _d3dContext.PSSetShader(_pixelShader);

        _d3dContext.DrawIndexed(_indexCount);

        _d2dContext.EndDraw();
        _swapChain.Present();
    }
};

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    ComInitialize com;
    EmberApplication app;
    return app.Run(hInstance, _T("Ember"));
}