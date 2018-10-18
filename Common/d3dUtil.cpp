//***************************************************************************************
// d3dUtil.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include "d3dUtil.h"
#include <WICTextureLoader.h>
#include <DDSTextureLoader.h>
#include <wincodec.h>
#include <wrl\client.h>

bool g_WIC2 = false;
BOOL WINAPI InitializeWICFactory(PINIT_ONCE, PVOID, PVOID* ifactory) noexcept
{
#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8) || defined(_WIN7_PLATFORM_UPDATE)
	HRESULT hr = CoCreateInstance(
		CLSID_WICImagingFactory2,
		nullptr,
		CLSCTX_INPROC_SERVER,
		__uuidof(IWICImagingFactory2),
		ifactory
	);

	if (SUCCEEDED(hr))
	{
		// WIC2 is available on Windows 10, Windows 8.x, and Windows 7 SP1 with KB 2670838 installed
		g_WIC2 = true;
		return TRUE;
	}
	else
	{
		hr = CoCreateInstance(
			CLSID_WICImagingFactory1,
			nullptr,
			CLSCTX_INPROC_SERVER,
			__uuidof(IWICImagingFactory),
			ifactory
		);
		return SUCCEEDED(hr) ? TRUE : FALSE;
	}
#else
	return SUCCEEDED(CoCreateInstance(
		CLSID_WICImagingFactory,
		nullptr,
		CLSCTX_INPROC_SERVER,
		__uuidof(IWICImagingFactory),
		ifactory)) ? TRUE : FALSE;
#endif
}
IWICImagingFactory* _GetWIC()
{
	static INIT_ONCE s_initOnce = INIT_ONCE_STATIC_INIT;

	IWICImagingFactory* factory = nullptr;
	InitOnceExecuteOnce(&s_initOnce, InitializeWICFactory, nullptr, reinterpret_cast<LPVOID*>(&factory));
	return  factory;
}

ID3D11ShaderResourceView* d3dHelper::CreateDDSTexture2DArraySRV(
	ID3D11Device* device, ID3D11DeviceContext* context,
	std::vector<std::wstring>& filenames)
{
	//
	// Load the texture elements individually from file.  These textures
	// won't be used by the GPU (0 bind flags), they are just used to 
	// load the image data from file.  We use the STAGING usage so the
	// CPU can read the resource.
	//

	UINT size = filenames.size();

	std::vector<ID3D11Texture2D*> srcTex(size);
	for (UINT i = 0; i < size; ++i)
	{
		DirectX::CreateDDSTextureFromFileEx(device, filenames[i].c_str(), 0, D3D11_USAGE_STAGING, 0, D3D11_CPU_ACCESS_READ, 0, false, (ID3D11Resource**)&srcTex[i], NULL);
		//DirectX::CreateDDSTextureFromFile(device, filenames[i].c_str(), (ID3D11Resource**)&srcTex[i], NULL);
	}

	//
	// Create the texture array.  Each element in the texture 
	// array has the same format/dimensions.
	//

	D3D11_TEXTURE2D_DESC texElementDesc;
	srcTex[0]->GetDesc(&texElementDesc);

	D3D11_TEXTURE2D_DESC texArrayDesc;
	texArrayDesc.Width = texElementDesc.Width;
	texArrayDesc.Height = texElementDesc.Height;
	texArrayDesc.MipLevels = texElementDesc.MipLevels;
	texArrayDesc.ArraySize = size;
	texArrayDesc.Format = texElementDesc.Format;
	texArrayDesc.SampleDesc.Count = 1;
	texArrayDesc.SampleDesc.Quality = 0;
	texArrayDesc.Usage = D3D11_USAGE_DEFAULT;
	texArrayDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texArrayDesc.CPUAccessFlags = 0;
	texArrayDesc.MiscFlags = 0;

	ID3D11Texture2D* texArray = 0;
	HR(device->CreateTexture2D(&texArrayDesc, 0, &texArray));

	//
	// Copy individual texture elements into texture array.
	//

	// for each texture element...
	for (UINT texElement = 0; texElement < size; ++texElement)
	{
		// for each mipmap level...
		for (UINT mipLevel = 0; mipLevel < texElementDesc.MipLevels; ++mipLevel)
		{
			D3D11_MAPPED_SUBRESOURCE mappedTex2D;
			HR(context->Map(srcTex[texElement], mipLevel, D3D11_MAP_READ, 0, &mappedTex2D));

			context->UpdateSubresource(texArray,
				D3D11CalcSubresource(mipLevel, texElement, texElementDesc.MipLevels),
				0, mappedTex2D.pData, mappedTex2D.RowPitch, mappedTex2D.DepthPitch);

			context->Unmap(srcTex[texElement], mipLevel);
		}
	}

	//
	// Create a resource view to the texture array.
	//

	D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
	viewDesc.Format = texArrayDesc.Format;
	viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	viewDesc.Texture2DArray.MostDetailedMip = 0;
	viewDesc.Texture2DArray.MipLevels = texArrayDesc.MipLevels;
	viewDesc.Texture2DArray.FirstArraySlice = 0;
	viewDesc.Texture2DArray.ArraySize = size;

	ID3D11ShaderResourceView* texArraySRV = 0;
	HR(device->CreateShaderResourceView(texArray, &viewDesc, &texArraySRV));

	//
	// Cleanup--we only need the resource view.
	//

	ReleaseCOM(texArray);

	for (UINT i = 0; i < size; ++i)
		ReleaseCOM(srcTex[i]);

	return texArraySRV;
}

ID3D11ShaderResourceView* d3dHelper::CreateRandomTexture1DSRV(ID3D11Device* device)
{
	// 
	// Create the random data.
	//
	XMFLOAT4 randomValues[1024];

	for(int i = 0; i < 1024; ++i)
	{
		randomValues[i].x = MathHelper::RandF(-1.0f, 1.0f);
		randomValues[i].y = MathHelper::RandF(-1.0f, 1.0f);
		randomValues[i].z = MathHelper::RandF(-1.0f, 1.0f);
		randomValues[i].w = MathHelper::RandF(-1.0f, 1.0f);
	}

    D3D11_SUBRESOURCE_DATA initData;
    initData.pSysMem = randomValues;
	initData.SysMemPitch = 1024*sizeof(XMFLOAT4);
    initData.SysMemSlicePitch = 0;

	//
	// Create the texture.
	//
    D3D11_TEXTURE1D_DESC texDesc;
    texDesc.Width = 1024;
    texDesc.MipLevels = 1;
    texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    texDesc.Usage = D3D11_USAGE_IMMUTABLE;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = 0;
    texDesc.ArraySize = 1;

	ID3D11Texture1D* randomTex = 0;
    HR(device->CreateTexture1D(&texDesc, &initData, &randomTex));

	//
	// Create the resource view.
	//
    D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
	viewDesc.Format = texDesc.Format;
    viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
    viewDesc.Texture1D.MipLevels = texDesc.MipLevels;
	viewDesc.Texture1D.MostDetailedMip = 0;
	
	ID3D11ShaderResourceView* randomTexSRV = 0;
    HR(device->CreateShaderResourceView(randomTex, &viewDesc, &randomTexSRV));

	ReleaseCOM(randomTex);

	return randomTexSRV;
}

void 
DXErrorMessage(HRESULT hr)
{
	static WCHAR buffer[256];
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, hr, 0, buffer, 256, NULL);
	//wprintf(buffer);
#if defined(DEBUG) | defined(_DEBUG)
	assert(false);
#else
	MessageBox(0, buffer, NULL, 0);
#endif
	
}

template<int col> inline  
float MatrixCom(CXMMATRIX m, int row)
{
	return 0.f;
}
template<> inline
float MatrixCom<0>(CXMMATRIX m, int row)
{
	return ::XMVectorGetX(m.r[row]);
}
template<> inline
float MatrixCom<1>(CXMMATRIX m, int row)
{
	return ::XMVectorGetY(m.r[row]);
}
template<> inline
float MatrixCom<2>(CXMMATRIX m, int row)
{
	return ::XMVectorGetZ(m.r[row]);
}
template<> inline
float MatrixCom<3>(CXMMATRIX m, int row)
{
	return ::XMVectorGetW(m.r[row]);
}

void ExtractFrustumPlanes(XMFLOAT4 planes[6], CXMMATRIX M)
{
	//
	// Left
	//
#define Matrix4Com( row, col ) (MatrixCom<col>(M,row) )


	planes[0].x = Matrix4Com(0,3) + Matrix4Com(0,0);
	planes[0].y = Matrix4Com(1,3) + Matrix4Com(1,0);
	planes[0].z = Matrix4Com(2,3) + Matrix4Com(2,0);
	planes[0].w = Matrix4Com(3,3) + Matrix4Com(3,0);

	//
	// Right
	//
	planes[1].x = Matrix4Com(0,3) - Matrix4Com(0,0);
	planes[1].y = Matrix4Com(1,3) - Matrix4Com(1,0);
	planes[1].z = Matrix4Com(2,3) - Matrix4Com(2,0);
	planes[1].w = Matrix4Com(3,3) - Matrix4Com(3,0);

	//
	// Bottom
	//
	planes[2].x = Matrix4Com(0,3) + Matrix4Com(0,1);
	planes[2].y = Matrix4Com(1,3) + Matrix4Com(1,1);
	planes[2].z = Matrix4Com(2,3) + Matrix4Com(2,1);
	planes[2].w = Matrix4Com(3,3) + Matrix4Com(3,1);

	//
	// Top
	//
	planes[3].x = Matrix4Com(0,3) - Matrix4Com(0,1);
	planes[3].y = Matrix4Com(1,3) - Matrix4Com(1,1);
	planes[3].z = Matrix4Com(2,3) - Matrix4Com(2,1);
	planes[3].w = Matrix4Com(3,3) - Matrix4Com(3,1);

	//
	// Near
	//
	planes[4].x = Matrix4Com(0,2);
	planes[4].y = Matrix4Com(1,2);
	planes[4].z = Matrix4Com(2,2);
	planes[4].w = Matrix4Com(3,2);

	//
	// Far
	//
	planes[5].x = Matrix4Com(0,3) - Matrix4Com(0,2);
	planes[5].y = Matrix4Com(1,3) - Matrix4Com(1,2);
	planes[5].z = Matrix4Com(2,3) - Matrix4Com(2,2);
	planes[5].w = Matrix4Com(3,3) - Matrix4Com(3,2);
#undef Matrix4Com
	// Normalize the plane equations.
	for(int i = 0; i < 6; ++i)
	{
		XMVECTOR v = XMPlaneNormalize(XMLoadFloat4(&planes[i]));
		XMStoreFloat4(&planes[i], v);
	}
}