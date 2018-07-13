#include <DirectXTemplate.h>
#include <algorithm>
#include <chrono>
#include <memory>
#include "Effects.h"
using namespace DirectX;

const LONG g_WindowWidth = 1280;
const LONG g_WindowHeight = 720;
LPCWSTR g_WindowClassName = L"DirectXWindowClass";
LPCWSTR g_WindowName = L"DirectX Template";
HWND g_WindowHandle = 0;

const BOOL g_EnableVSync = TRUE;

// Direct3D device and swap chain.
ID3D11Device* g_d3dDevice = nullptr;
ID3D11DeviceContext* g_d3dDeviceContext = nullptr;
IDXGISwapChain* g_d3dSwapChain = nullptr;

// Render target view for the back buffer of the swap chain.
ID3D11RenderTargetView* g_d3dRenderTargetView = nullptr;
// Depth/stencil view for use as a depth buffer.
ID3D11DepthStencilView* g_d3dDepthStencilView = nullptr;

ID3D11ShaderResourceView* g_textureShaderResourceView = nullptr;

// A texture to associate to the depth stencil view.
ID3D11Texture2D* g_d3dDepthStencilBuffer = nullptr;

// Define the functionality of the depth/stencil stages.
ID3D11DepthStencilState* g_d3dDepthStencilState = nullptr;
// Define the functionality of the rasterizer stage.
ID3D11RasterizerState* g_d3dRasterizerState = nullptr;
ID3D11SamplerState* g_d3dSamplerState = nullptr;
D3D11_VIEWPORT g_Viewport = { 0 };


// Vertex buffer data
ID3D11InputLayout* g_d3dInputLayout = nullptr;
ID3D11Buffer* g_d3dVertexBuffer = nullptr;
ID3D11Buffer* g_d3dIndexBuffer = nullptr;
ID3D11InputLayout* g_d3dInstancedInputLayout = nullptr;
ID3D11Buffer* g_d3dInstancedVertexBuffer_Instances = nullptr;
ID3D11Buffer* g_d3dInstancedVertexBuffer_Vertices = nullptr;
ID3D11Buffer* g_d3dInstancedIndexBuffer = nullptr;

// Shader data
ID3D11VertexShader* g_d3dVertexShader = nullptr;
ID3D11VertexShader* g_d3dInstancedVertexShader = nullptr;
ID3D11PixelShader* g_d3dPixelShader = nullptr;


// Shader resources
enum ConstanBuffer
{
    CB_Frame,
    CB_Object,
    NumConstantBuffers
};

ID3D11Buffer* g_d3dConstantBuffers[NumConstantBuffers];

// Demo parameters
XMMATRIX g_WorldMatrix;
XMMATRIX g_ViewMatrix;
XMMATRIX g_ProjectionMatrix;

// Vertex data for a colored cube.
struct VertexPosNormColTex
{
    XMFLOAT3 Position;
    XMFLOAT3 Normal;
    XMFLOAT3 Color;
	XMFLOAT2 Texture;
};

// Per-instance data (must be 16 byte aligned)
struct alignas(16) PlaneInstanceData
{
	XMMATRIX WorldMatrix;
	XMMATRIX InverseTransposeWorldMatrix;
};

struct alignas(16) PerObjectTransformData
{
    XMMATRIX WorldMatrix;
    XMMATRIX InverseTransposeWorldMatrix;
    XMMATRIX WorldViewProjectMatrix;
} g_PerObjTransformData;

// A structure to hold the data for a per-object constant buffer
// defined in the vertex shader.
struct PerFrameConstantBufferData
{
	XMMATRIX ViewProjectionMatrix;
} g_PerFrameTransformData;

VertexPosNormColTex g_Vertices[8] =
{
    { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT2(1,0) }, // 0
    { XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1,1) }, // 1
    { XMFLOAT3(1.0f,  1.0f, -1.0f),  XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 0.0f),  XMFLOAT2(0,1) }, // 2
    { XMFLOAT3(1.0f, -1.0f, -1.0f),  XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f),  XMFLOAT2(0,0) }, // 3
    { XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0,0) }, // 4
    { XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 1.0f), XMFLOAT2(0,1) }, // 5
    { XMFLOAT3(1.0f,  1.0f,  1.0f),  XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f),  XMFLOAT2(1,1) }, // 6
    { XMFLOAT3(1.0f, -1.0f,  1.0f),  XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 1.0f),  XMFLOAT2(1,0) }  // 7
};

WORD g_Indicies[36] =
{
    0, 1, 2, 0, 2, 3,
    4, 6, 5, 4, 7, 6,
    4, 5, 1, 4, 1, 0,
    3, 2, 6, 3, 6, 7,
    1, 5, 6, 1, 6, 2,
    4, 0, 3, 4, 3, 7
};

// Vertices for a unit plane.
VertexPosNormColTex g_PlaneVerts[4] =
{
	{ XMFLOAT3(-0.5f, 0.0f,  0.5f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) }, // 0
	{ XMFLOAT3(0.5f, 0.0f,  0.5f), XMFLOAT3(0.0f, 1.0f, 0.0f),  XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) }, // 1
	{ XMFLOAT3(0.5f, 0.0f, -0.5f), XMFLOAT3(0.0f, 1.0f, 0.0f),  XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) }, // 2
	{ XMFLOAT3(-0.5f, 0.0f, -0.5f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) }  // 3
};

// Index buffer for the unit plane.
WORD g_PlaneIndex[6] =
{
	0, 1, 3, 1, 2, 3
};

// Forward declarations.
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

template< class ShaderClass >
ShaderClass* LoadShader(const std::wstring& fileName, const std::string& entryPoint, const std::string& profile);

bool LoadContent();
void UnloadContent();

void Update(float deltaTime);
void Render();
void Cleanup();

/**
* Initialize the application window.
*/
int InitApplication(HINSTANCE hInstance, int cmdShow)
{
    WNDCLASSEX wndClass = { 0 };
    wndClass.cbSize = sizeof(WNDCLASSEX);
    wndClass.style = CS_HREDRAW | CS_VREDRAW;
    wndClass.lpfnWndProc = &WndProc;
    wndClass.hInstance = hInstance;
    wndClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wndClass.lpszMenuName = nullptr;
    wndClass.lpszClassName = g_WindowClassName;

    if (!RegisterClassEx(&wndClass))
    {
        return -1;
    }

    RECT windowRect = { 0, 0, g_WindowWidth, g_WindowHeight };
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    g_WindowHandle = CreateWindow(g_WindowClassName, g_WindowName,
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        nullptr, nullptr, hInstance, nullptr);

    if (!g_WindowHandle)
    {
        return -1;
    }

    ShowWindow(g_WindowHandle, cmdShow);
    UpdateWindow(g_WindowHandle);

    return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT paintStruct;
    HDC hDC;

    switch (message)
    {
    case WM_PAINT:
    {
        hDC = BeginPaint(hwnd, &paintStruct);
        EndPaint(hwnd, &paintStruct);
    }
    break;
    case WM_DESTROY:
    {
        PostQuitMessage(0);
    }
    break;
    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }

    return 0;
}

/**
* The main application loop.
*/
int Run()
{
    MSG msg = { 0 };

    static DWORD previousTime = timeGetTime();
    static const float targetFramerate = 30.0f;
    static const float maxTimeStep = 1.0f / targetFramerate;

    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            DWORD currentTime = timeGetTime();
            float deltaTime = (currentTime - previousTime) / 1000.0f;
            previousTime = currentTime;

            // Cap the delta time to the max time step (useful if your 
            // debugging and you don't want the deltaTime value to explode.
            deltaTime = std::min<float>(deltaTime, maxTimeStep);

            Update(deltaTime);
            Render();
        }
    }

    return static_cast<int>(msg.wParam);
}

/**
* Initialize the DirectX device and swap chain.
*/
int InitDirectX(HINSTANCE hInstance, BOOL vSync)
{
    // A window handle must have been created already.
    assert(g_WindowHandle != 0);

    RECT clientRect;
    GetClientRect(g_WindowHandle, &clientRect);

    // Compute the exact client dimensions. This will be used
    // to initialize the render targets for our swap chain.
    unsigned int clientWidth = clientRect.right - clientRect.left;
    unsigned int clientHeight = clientRect.bottom - clientRect.top;

    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

    swapChainDesc.BufferCount = 1;
    swapChainDesc.BufferDesc.Width = clientWidth;
    swapChainDesc.BufferDesc.Height = clientHeight;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.RefreshRate = { 0,1 };//QueryRefreshRate(clientWidth, clientHeight, vSync);
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = g_WindowHandle;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swapChainDesc.Windowed = TRUE;

    UINT createDeviceFlags = 0;
#if _DEBUG
    createDeviceFlags = D3D11_CREATE_DEVICE_DEBUG;
#endif

    // These are the feature levels that we will accept.
    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1
    };

    // This will be the feature level that 
    // is used to create our device and swap chain.
    D3D_FEATURE_LEVEL featureLevel;

    HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE,
        nullptr, createDeviceFlags, featureLevels, _countof(featureLevels),
        D3D11_SDK_VERSION, &swapChainDesc, &g_d3dSwapChain, &g_d3dDevice, &featureLevel,
        &g_d3dDeviceContext);

    if (hr == E_INVALIDARG)
    {
        hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE,
            nullptr, createDeviceFlags, &featureLevels[1], _countof(featureLevels) - 1,
            D3D11_SDK_VERSION, &swapChainDesc, &g_d3dSwapChain, &g_d3dDevice, &featureLevel,
            &g_d3dDeviceContext);
    }

    if (FAILED(hr))
    {
        return -1;
    }

    // Next initialize the back buffer of the swap chain and associate it to a 
    // render target view.
    ID3D11Texture2D* backBuffer;
    hr = g_d3dSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);
    if (FAILED(hr))
    {
        return -1;
    }

    hr = g_d3dDevice->CreateRenderTargetView(backBuffer, nullptr, &g_d3dRenderTargetView);
    if (FAILED(hr))
    {
        return -1;
    }

    SafeRelease(backBuffer);

    // Create the depth buffer for use with the depth/stencil view.
    D3D11_TEXTURE2D_DESC depthStencilBufferDesc;
    ZeroMemory(&depthStencilBufferDesc, sizeof(D3D11_TEXTURE2D_DESC));

    depthStencilBufferDesc.ArraySize = 1;
    depthStencilBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthStencilBufferDesc.CPUAccessFlags = 0; // No CPU access required.
    depthStencilBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilBufferDesc.Width = clientWidth;
    depthStencilBufferDesc.Height = clientHeight;
    depthStencilBufferDesc.MipLevels = 1;
    depthStencilBufferDesc.SampleDesc.Count = 1;
    depthStencilBufferDesc.SampleDesc.Quality = 0;
    depthStencilBufferDesc.Usage = D3D11_USAGE_DEFAULT;

    hr = g_d3dDevice->CreateTexture2D(&depthStencilBufferDesc, nullptr, &g_d3dDepthStencilBuffer);
    if (FAILED(hr))
    {
        return -1;
    }

    hr = g_d3dDevice->CreateDepthStencilView(g_d3dDepthStencilBuffer, nullptr, &g_d3dDepthStencilView);
    if (FAILED(hr))
    {
        return -1;
    }

    // Setup depth/stencil state.
    D3D11_DEPTH_STENCIL_DESC depthStencilStateDesc;
    ZeroMemory(&depthStencilStateDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));

    depthStencilStateDesc.DepthEnable = TRUE;
    depthStencilStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilStateDesc.DepthFunc = D3D11_COMPARISON_LESS;
    depthStencilStateDesc.StencilEnable = FALSE;

    hr = g_d3dDevice->CreateDepthStencilState(&depthStencilStateDesc, &g_d3dDepthStencilState);

    // Setup rasterizer state.
    D3D11_RASTERIZER_DESC rasterizerDesc;
    ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));

    rasterizerDesc.AntialiasedLineEnable = FALSE;
    rasterizerDesc.CullMode = D3D11_CULL_BACK;
    rasterizerDesc.DepthBias = 0;
    rasterizerDesc.DepthBiasClamp = 0.0f;
    rasterizerDesc.DepthClipEnable = TRUE;
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.FrontCounterClockwise = FALSE;
    rasterizerDesc.MultisampleEnable = FALSE;
    rasterizerDesc.ScissorEnable = FALSE;
    rasterizerDesc.SlopeScaledDepthBias = 0.0f;

    // Create the rasterizer state object.
    hr = g_d3dDevice->CreateRasterizerState(&rasterizerDesc, &g_d3dRasterizerState);
    if (FAILED(hr))
    {
        return -1;
    }

    // Initialize the viewport to occupy the entire client area.
    g_Viewport.Width = static_cast<float>(clientWidth);
    g_Viewport.Height = static_cast<float>(clientHeight);
    g_Viewport.TopLeftX = 0.0f;
    g_Viewport.TopLeftY = 0.0f;
    g_Viewport.MinDepth = 0.0f;
    g_Viewport.MaxDepth = 1.0f;


    // Create a sampler state for texture sampling in the pixel shader
    D3D11_SAMPLER_DESC samplerDesc;
    ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));

    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDesc.BorderColor[0] = 1.0f;
    samplerDesc.BorderColor[1] = 1.0f;
    samplerDesc.BorderColor[2] = 1.0f;
    samplerDesc.BorderColor[3] = 1.0f;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = 0;

    hr = g_d3dDevice->CreateSamplerState(&samplerDesc, &g_d3dSamplerState);
    if (FAILED(hr))
    {
        MessageBoxA(nullptr, "Failed to create texture sampler.", "Error", MB_OK | MB_ICONERROR);
        return false;
    }
    return 0;
}

bool LoadContent()
{
	assert(g_d3dDevice);

	D3D11_SUBRESOURCE_DATA resourceData;
	ZeroMemory(&resourceData, sizeof(D3D11_SUBRESOURCE_DATA));

	HRESULT hr;

	{// Create an initialize the simple vertex buffer.
		D3D11_BUFFER_DESC vertexBufferDesc;
		ZeroMemory(&vertexBufferDesc, sizeof(D3D11_BUFFER_DESC));

		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.ByteWidth = sizeof(VertexPosNormColTex) * _countof(g_Vertices);
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;

		resourceData.pSysMem = g_Vertices;

		hr = g_d3dDevice->CreateBuffer(&vertexBufferDesc, &resourceData, &g_d3dVertexBuffer);
		if (FAILED(hr))
		{
			MessageBoxA(nullptr, "Failed to create vertex buffer for simple vertex shader.", "Error", MB_OK | MB_ICONERROR);
			return false;
		}
	}

	{// Create and initialize the simple index buffer.
		D3D11_BUFFER_DESC indexBufferDesc;
		ZeroMemory(&indexBufferDesc, sizeof(D3D11_BUFFER_DESC));

		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.ByteWidth = sizeof(WORD) * _countof(g_Indicies);
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		resourceData.pSysMem = g_Indicies;

		hr = g_d3dDevice->CreateBuffer(&indexBufferDesc, &resourceData, &g_d3dIndexBuffer);
		if (FAILED(hr))
		{
			MessageBoxA(nullptr, "Failed to create index buffer for simple vertex shader.", "Error", MB_OK | MB_ICONERROR);
			return false;
		}
	}

	{// Create the constant buffers for the variables defined in the vertex shaders.
		D3D11_BUFFER_DESC constantBufferDesc;
		ZeroMemory(&constantBufferDesc, sizeof(D3D11_BUFFER_DESC));

		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constantBufferDesc.ByteWidth = sizeof(XMMATRIX);
		constantBufferDesc.CPUAccessFlags = 0;
		constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;

		hr = g_d3dDevice->CreateBuffer(&constantBufferDesc, nullptr, &g_d3dConstantBuffers[CB_Frame]);
		if (FAILED(hr))
		{
			MessageBoxA(nullptr, "Failed to create instanced vertex shader cbuffer.", "Error", MB_OK | MB_ICONERROR);
			return false;
		}

		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constantBufferDesc.ByteWidth = sizeof(PerObjectTransformData);
		constantBufferDesc.CPUAccessFlags = 0;
		constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;

		hr = g_d3dDevice->CreateBuffer(&constantBufferDesc, nullptr, &g_d3dConstantBuffers[CB_Object]);
		if (FAILED(hr))
		{
			MessageBoxA(nullptr, "Failed to create simple vertex shader cbuffer.", "Error", MB_OK | MB_ICONERROR);
			return false;
		}
	}

	{// Load the compiled vertex shader.
		ID3DBlob* vertexShaderBlob;
#if _DEBUG
		LPCWSTR compiledVertexShaderObject = L"SimpleVertexShader_d.cso";
#else
		LPCWSTR compiledVertexShaderObject = L"SimpleVertexShader.cso";
#endif

		hr = D3DReadFileToBlob(compiledVertexShaderObject, &vertexShaderBlob);
		if (FAILED(hr))
		{
			return false;
		}

		hr = g_d3dDevice->CreateVertexShader(vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), nullptr, &g_d3dVertexShader);
		if (FAILED(hr))
		{
			return false;
		}

		{// Create the input layout for the simple vertex shader.
			D3D11_INPUT_ELEMENT_DESC vertexLayoutDesc[] =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			};

			hr = g_d3dDevice->CreateInputLayout(vertexLayoutDesc, _countof(vertexLayoutDesc), vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), &g_d3dInputLayout);
			if (FAILED(hr))
			{
				return false;
			}

			SafeRelease(vertexShaderBlob);
		}
	}

	{// Load the compiled pixel shader.
		ID3DBlob* pixelShaderBlob;
#if _DEBUG
		LPCWSTR compiledPixelShaderObject = L"SimplePixelShader_d.cso";
#else
		LPCWSTR compiledPixelShaderObject = L"SimplePixelShader.cso";
#endif

		hr = D3DReadFileToBlob(compiledPixelShaderObject, &pixelShaderBlob);
		if (FAILED(hr))
		{
			MessageBoxA(g_WindowHandle, "Failed to load pixel shader blob.", "Error", MB_OK | MB_ICONERROR);
			return false;
		}

		hr = g_d3dDevice->CreatePixelShader(pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize(), nullptr, &g_d3dPixelShader);
		if (FAILED(hr))
		{
			MessageBoxA(g_WindowHandle, "Failed to create pixel shader.", "Error", MB_OK | MB_ICONERROR);
			return false;
		}

		SafeRelease(pixelShaderBlob);
	}

	{// Setup the projection matrix.
		RECT clientRect;
		GetClientRect(g_WindowHandle, &clientRect);

		// Compute the exact client dimensions.
		// This is required for a correct projection matrix.
		float clientWidth = static_cast<float>(clientRect.right - clientRect.left);
		float clientHeight = static_cast<float>(clientRect.bottom - clientRect.top);

		g_ProjectionMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0f), clientWidth / clientHeight, 0.1f, 100.0f);
	}

	{// Load textures
		auto effectFactory = std::unique_ptr<EffectFactory>(new EffectFactory(g_d3dDevice));
		effectFactory->SetDirectory(L"..\\assets");

		try
		{
			effectFactory->CreateTexture(L"container.jpg", g_d3dDeviceContext, &g_textureShaderResourceView);
		}
		catch (std::exception&)
		{
			MessageBoxA(nullptr, "Failed to load texture.", "Error", MB_OK | MB_ICONERROR);
			return false;
		}
	}

	{// Create and setup the per-instance buffer data

		// Start with the plane (quad) vertex data.
		{
			// Create and initialize a vertex buffer for a plane.
			D3D11_BUFFER_DESC vertexBufferDesc;
			ZeroMemory(&vertexBufferDesc, sizeof(D3D11_BUFFER_DESC));

			vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			vertexBufferDesc.ByteWidth = sizeof(g_PlaneVerts);
			vertexBufferDesc.CPUAccessFlags = 0;
			vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;

			D3D11_SUBRESOURCE_DATA resourceData;
			ZeroMemory(&resourceData, sizeof(D3D11_SUBRESOURCE_DATA));

			resourceData.pSysMem = g_PlaneVerts;

			hr = g_d3dDevice->CreateBuffer(&vertexBufferDesc, &resourceData, &g_d3dInstancedVertexBuffer_Vertices);
			if (FAILED(hr))
			{
				MessageBoxA(g_WindowHandle, "Failed to create vertex buffer.", "Error", MB_OK | MB_ICONERROR);
				return false;
			}
		}

		// Move onto the plane (quad) instance data.
		const int numInstances = 6;
		PlaneInstanceData* planeInstanceData = (PlaneInstanceData*)_aligned_malloc(sizeof(PlaneInstanceData) * numInstances, 16);

		float scalePlane = 20.0f;
		float translateOffset = scalePlane / 2.0f;
		XMMATRIX scaleMatrix = XMMatrixScaling(scalePlane, 1.0f, scalePlane);
		XMMATRIX translateMatrix = XMMatrixTranslation(0, 0, 0);
		XMMATRIX rotateMatrix = XMMatrixRotationX(0.0f);

		// Floor plane.
		XMMATRIX worldMatrix = scaleMatrix * rotateMatrix * translateMatrix;
		planeInstanceData[0].WorldMatrix = worldMatrix;
		planeInstanceData[0].InverseTransposeWorldMatrix = XMMatrixTranspose(XMMatrixInverse(nullptr, worldMatrix));

		// Back wall plane.
		translateMatrix = XMMatrixTranslation(0, translateOffset, translateOffset);
		rotateMatrix = XMMatrixRotationX(XMConvertToRadians(-90));
		worldMatrix = scaleMatrix * rotateMatrix * translateMatrix;

		planeInstanceData[1].WorldMatrix = worldMatrix;
		planeInstanceData[1].InverseTransposeWorldMatrix = XMMatrixTranspose(XMMatrixInverse(nullptr, worldMatrix));

		// Ceiling plane.
		translateMatrix = XMMatrixTranslation(0, translateOffset * 2.0f, 0);
		rotateMatrix = XMMatrixRotationX(XMConvertToRadians(180));
		worldMatrix = scaleMatrix * rotateMatrix * translateMatrix;

		planeInstanceData[2].WorldMatrix = worldMatrix;
		planeInstanceData[2].InverseTransposeWorldMatrix = XMMatrixTranspose(XMMatrixInverse(nullptr, worldMatrix));

		// Front wall plane.
		translateMatrix = XMMatrixTranslation(0, translateOffset, -translateOffset);
		rotateMatrix = XMMatrixRotationX(XMConvertToRadians(90));
		worldMatrix = scaleMatrix * rotateMatrix * translateMatrix;

		planeInstanceData[3].WorldMatrix = worldMatrix;
		planeInstanceData[3].InverseTransposeWorldMatrix = XMMatrixTranspose(XMMatrixInverse(nullptr, worldMatrix));

		// Left wall plane.
		translateMatrix = XMMatrixTranslation(-translateOffset, translateOffset, 0);
		rotateMatrix = XMMatrixRotationZ(XMConvertToRadians(-90));
		worldMatrix = scaleMatrix * rotateMatrix * translateMatrix;

		planeInstanceData[4].WorldMatrix = worldMatrix;
		planeInstanceData[4].InverseTransposeWorldMatrix = XMMatrixTranspose(XMMatrixInverse(nullptr, worldMatrix));

		// Right wall plane.
		translateMatrix = XMMatrixTranslation(translateOffset, translateOffset, 0);
		rotateMatrix = XMMatrixRotationZ(XMConvertToRadians(90));
		worldMatrix = scaleMatrix * rotateMatrix * translateMatrix;

		planeInstanceData[5].WorldMatrix = worldMatrix;
		planeInstanceData[5].InverseTransposeWorldMatrix = XMMatrixTranspose(XMMatrixInverse(nullptr, worldMatrix));

		{// Create the per-instance instance buffer.
			D3D11_BUFFER_DESC instanceBufferDesc;
			ZeroMemory(&instanceBufferDesc, sizeof(D3D11_BUFFER_DESC));

			instanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			instanceBufferDesc.ByteWidth = sizeof(PlaneInstanceData) * numInstances;
			instanceBufferDesc.CPUAccessFlags = 0;
			instanceBufferDesc.Usage = D3D11_USAGE_DEFAULT;

			ZeroMemory(&resourceData, sizeof(D3D11_SUBRESOURCE_DATA));
			resourceData.pSysMem = planeInstanceData;

			hr = g_d3dDevice->CreateBuffer(&instanceBufferDesc, &resourceData, &g_d3dInstancedVertexBuffer_Instances);

			_aligned_free(planeInstanceData);

			if (FAILED(hr))
			{
				MessageBoxA(g_WindowHandle, "Failed to create instanced vertex buffer.", "Error", MB_OK | MB_ICONERROR);
				return false;
			}
		}
	}

	{// Create the per-instance index buffer.
		D3D11_BUFFER_DESC instancedIndexBufferDesc;
		ZeroMemory(&instancedIndexBufferDesc, sizeof(D3D11_BUFFER_DESC));

		instancedIndexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		instancedIndexBufferDesc.ByteWidth = sizeof(WORD) * _countof(g_PlaneIndex);
		instancedIndexBufferDesc.CPUAccessFlags = 0;
		instancedIndexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		resourceData.pSysMem = g_PlaneIndex;

		hr = g_d3dDevice->CreateBuffer(&instancedIndexBufferDesc, &resourceData, &g_d3dInstancedIndexBuffer);
		if (FAILED(hr))
		{
			MessageBoxA(g_WindowHandle, "Failed to create instanced index buffer.", "Error", MB_OK | MB_ICONERROR);
			return false;
		}
	}

	{ // Load the compiled instanced vertex shader.
		ID3DBlob* instancedVertexShaderBlob;
#if _DEBUG
		LPCWSTR compiledInstancedVertexShaderObject = L"InstancedVertexShader_d.cso";
#else
		LPCWSTR compiledInstancedVertexShaderObject = L"InstancedVertexShader.cso";
#endif

		hr = D3DReadFileToBlob(compiledInstancedVertexShaderObject, &instancedVertexShaderBlob);
		if (FAILED(hr))
		{
			MessageBoxA(g_WindowHandle, "Failed to load instanced vertex shader blob.", "Error", MB_OK | MB_ICONERROR);
			return false;
		}

		hr = g_d3dDevice->CreateVertexShader(instancedVertexShaderBlob->GetBufferPointer(), instancedVertexShaderBlob->GetBufferSize(), nullptr, &g_d3dInstancedVertexShader);
		if (FAILED(hr))
		{
			MessageBoxA(g_WindowHandle, "Failed to create instanced vertex shader.", "Error", MB_OK | MB_ICONERROR);
			return false;
		}

		// Create the input layout for rendering instanced vertex data.
		D3D11_INPUT_ELEMENT_DESC instancedVertexLayoutDesc[] =
		{
			// Per-vertex data.
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			// Per-instance data.
			{ "WORLDMATRIX", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLDMATRIX", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLDMATRIX", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLDMATRIX", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "INVERSETRANSPOSEWORLDMATRIX", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "INVERSETRANSPOSEWORLDMATRIX", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "INVERSETRANSPOSEWORLDMATRIX", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "INVERSETRANSPOSEWORLDMATRIX", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		};

		hr = g_d3dDevice->CreateInputLayout(instancedVertexLayoutDesc, _countof(instancedVertexLayoutDesc), instancedVertexShaderBlob->GetBufferPointer(),
											instancedVertexShaderBlob->GetBufferSize(), &g_d3dInstancedInputLayout);
		if (FAILED(hr))
		{
			MessageBoxA(g_WindowHandle, "Failed to create input layout.", "Error", MB_OK | MB_ICONERROR);
			return false;
		}

		SafeRelease(instancedVertexShaderBlob);
	}

    return true;
}

void Update(float deltaTime)
{
	static XMVECTOR eyePosition = XMVectorSet(0, -5, -10, 1);
	const float speed = 4.0f;
	if (GetKeyState('A') & 0x8000) /*check if high-order bit is set (1 << 15)*/
	{
		eyePosition = XMVectorSetX(eyePosition, XMVectorGetX(eyePosition) - speed * deltaTime);
	}
	if (GetKeyState('D') & 0x8000) /*check if high-order bit is set (1 << 15)*/
	{
		eyePosition = XMVectorSetX(eyePosition, XMVectorGetX(eyePosition) + speed * deltaTime);
	}
	if (GetKeyState('Q') & 0x8000) /*check if high-order bit is set (1 << 15)*/
	{
		eyePosition = XMVectorSetY(eyePosition, XMVectorGetY(eyePosition) - speed * deltaTime);
	}
	if (GetKeyState('E') & 0x8000) /*check if high-order bit is set (1 << 15)*/
	{
		eyePosition = XMVectorSetY(eyePosition, XMVectorGetY(eyePosition) + speed * deltaTime);
	}
	if (GetKeyState('W') & 0x8000) /*check if high-order bit is set (1 << 15)*/
	{
		eyePosition = XMVectorSetZ(eyePosition, XMVectorGetZ(eyePosition) + speed * deltaTime);
	}
	if (GetKeyState('S') & 0x8000) /*check if high-order bit is set (1 << 15)*/
	{
		eyePosition = XMVectorSetZ(eyePosition, XMVectorGetZ(eyePosition) - speed * deltaTime);
	}

	XMVECTOR focusPoint = XMVectorSetZ(eyePosition, XMVectorGetZ(eyePosition) + 0.1f);
    XMVECTOR upDirection = XMVectorSet(0, 1, 0, 0);
    g_ViewMatrix = XMMatrixLookAtLH(eyePosition, focusPoint, upDirection);

	const XMMATRIX viewProjectionMatrix = g_ViewMatrix * g_ProjectionMatrix;
	g_PerFrameTransformData.ViewProjectionMatrix = viewProjectionMatrix;


    //static float angle = 0.0f;
    //angle += 90.0f * deltaTime;
    //XMVECTOR rotationAxis = XMVectorSet(0, 1, 1, 0);
    //g_WorldMatrix = XMMatrixRotationAxis(rotationAxis, XMConvertToRadians(angle));

    //g_PerObjTransformData.WorldMatrix = g_WorldMatrix;
    //g_PerObjTransformData.WorldViewProjectMatrix = XMMatrixMultiply(g_WorldMatrix, viewProjectionMatrix);
    //g_PerObjTransformData.InverseTransposeWorldMatrix = XMMatrixTranspose(XMMatrixInverse(nullptr, g_WorldMatrix));
    
    //g_d3dDeviceContext->UpdateSubresource(g_d3dConstantBuffers[CB_Object], 0, nullptr, &g_PerObjTransformData, 0, 0);
}

// Clear the color and depth buffers.
void Clear(const FLOAT clearColor[4], FLOAT clearDepth, UINT8 clearStencil)
{
    g_d3dDeviceContext->ClearRenderTargetView(g_d3dRenderTargetView, clearColor);
    g_d3dDeviceContext->ClearDepthStencilView(g_d3dDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, clearDepth, clearStencil);
}

void Present(bool vSync)
{
    if (vSync)
    {
        g_d3dSwapChain->Present(1, 0);
    }
    else
    {
        g_d3dSwapChain->Present(0, 0);
    }
}

void Render()
{
    assert(g_d3dDevice);
    assert(g_d3dDeviceContext);

    Clear(Colors::CornflowerBlue, 1.0f, 0);

	{ // Instanced render walls.
		const UINT vertexStride[2] = { sizeof(VertexPosNormColTex), sizeof(PlaneInstanceData) };
		const UINT offset[2] = { 0, 0 };
		ID3D11Buffer* buffers[2] = { g_d3dInstancedVertexBuffer_Vertices, g_d3dInstancedVertexBuffer_Instances };

		g_d3dDeviceContext->IASetVertexBuffers(0, 2, buffers, vertexStride, offset);
		g_d3dDeviceContext->IASetInputLayout(g_d3dInstancedInputLayout);
		g_d3dDeviceContext->IASetIndexBuffer(g_d3dInstancedIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
		g_d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		g_d3dDeviceContext->VSSetShader(g_d3dInstancedVertexShader, nullptr, 0);
		g_d3dDeviceContext->VSSetConstantBuffers(0, 1, &g_d3dConstantBuffers[CB_Frame]);

		g_d3dDeviceContext->PSSetShader(g_d3dPixelShader, nullptr, 0);

		g_d3dDeviceContext->OMSetRenderTargets(1, &g_d3dRenderTargetView, g_d3dDepthStencilView);
		g_d3dDeviceContext->OMSetDepthStencilState(g_d3dDepthStencilState, 0);

		g_d3dDeviceContext->RSSetState(g_d3dRasterizerState);
		g_d3dDeviceContext->RSSetViewports(1, &g_Viewport);

		g_d3dDeviceContext->UpdateSubresource(g_d3dConstantBuffers[CB_Frame], 0, nullptr, &g_PerFrameTransformData, 0, 0);

		g_d3dDeviceContext->DrawIndexedInstanced(_countof(g_PlaneIndex), 6, 0, 0, 0);
	}
    //const UINT vertexStride = sizeof(VertexPosNormColTex);
    //const UINT offset = 0;

    //g_d3dDeviceContext->IASetVertexBuffers(0, 1, &g_d3dVertexBuffer, &vertexStride, &offset);
    //g_d3dDeviceContext->IASetInputLayout(g_d3dInputLayout);
    //g_d3dDeviceContext->IASetIndexBuffer(g_d3dIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
    //g_d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    //g_d3dDeviceContext->VSSetShader(g_d3dVertexShader, nullptr, 0);
    //g_d3dDeviceContext->VSSetConstantBuffers(0, 3, g_d3dConstantBuffers);

    //g_d3dDeviceContext->RSSetState(g_d3dRasterizerState);
    //g_d3dDeviceContext->RSSetViewports(1, &g_Viewport);

    //g_d3dDeviceContext->PSSetShader(g_d3dPixelShader, nullptr, 0);
    //g_d3dDeviceContext->PSSetSamplers(0,1,&g_d3dSamplerState);
    //g_d3dDeviceContext->PSSetShaderResources(0, 1, &g_textureShaderResourceView);

    //g_d3dDeviceContext->OMSetRenderTargets(1, &g_d3dRenderTargetView, g_d3dDepthStencilView);
    //g_d3dDeviceContext->OMSetDepthStencilState(g_d3dDepthStencilState, 1);

    //g_d3dDeviceContext->DrawIndexed(_countof(g_Indicies), 0, 0);

    Present(g_EnableVSync);
}

void UnloadContent()
{
    SafeRelease(g_d3dConstantBuffers[CB_Frame]);
    SafeRelease(g_d3dConstantBuffers[CB_Object]);
    SafeRelease(g_d3dIndexBuffer);
    SafeRelease(g_d3dVertexBuffer);
    SafeRelease(g_d3dInputLayout);
    SafeRelease(g_d3dVertexShader);
	SafeRelease(g_d3dInstancedIndexBuffer);
	SafeRelease(g_d3dInstancedVertexBuffer_Instances);
	SafeRelease(g_d3dInstancedVertexBuffer_Vertices);
	SafeRelease(g_d3dInstancedInputLayout);
	SafeRelease(g_d3dInstancedVertexShader);
    SafeRelease(g_d3dPixelShader);
    SafeRelease(g_textureShaderResourceView);
}

void Cleanup()
{
    SafeRelease(g_d3dDepthStencilView);
    SafeRelease(g_d3dRenderTargetView);
    SafeRelease(g_d3dDepthStencilBuffer);
    SafeRelease(g_d3dDepthStencilState);
    SafeRelease(g_d3dRasterizerState);
    SafeRelease(g_d3dSamplerState);
    SafeRelease(g_d3dSwapChain);
    SafeRelease(g_d3dDeviceContext);

    //ID3D11Debug *d3dDebug = nullptr;
    //if (SUCCEEDED(g_d3dDevice->QueryInterface(__uuidof(ID3D11Debug), (void**)&d3dDebug)))
    //{
    //    d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
    //    d3dDebug->Release();
    //}

    SafeRelease(g_d3dDevice);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE prevInstance, LPWSTR cmdLine, int cmdShow)
{
    UNREFERENCED_PARAMETER(prevInstance);
    UNREFERENCED_PARAMETER(cmdLine);

    // Check for DirectX Math library support.
    if (!XMVerifyCPUSupport())
    {
        MessageBox(nullptr, TEXT("Failed to verify DirectX Math library support."), TEXT("Error"), MB_OK);
        return -1;
    }

    if (InitApplication(hInstance, cmdShow) != 0)
    {
        MessageBox(nullptr, TEXT("Failed to create applicaiton window."), TEXT("Error"), MB_OK);
        return -1;
    }

    if (InitDirectX(hInstance, g_EnableVSync) != 0)
    {
        MessageBox(nullptr, TEXT("Failed to create DirectX device and swap chain."), TEXT("Error"), MB_OK);
        return -1;
    }

    if (!LoadContent())
    {
        MessageBox(nullptr, TEXT("Failed to load content."), TEXT("Error"), MB_OK);
        return -1;
    }

    int returnCode = Run();

    UnloadContent();
    Cleanup();

    return returnCode;
}