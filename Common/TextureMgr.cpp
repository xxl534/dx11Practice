#include "d3dUtil.h"
#include "TextureMgr.h"
#include "WICTextureLoader.h"
#include "DDSTextureLoader.h"
TextureMgr::TextureMgr() : md3dDevice(0)
{
}

TextureMgr::~TextureMgr()
{
	for(auto it = mTextureSRV.begin(); it != mTextureSRV.end(); ++it)
    {
		ReleaseCOM(it->second);
    }

	mTextureSRV.clear();
}

void TextureMgr::Init(ID3D11Device* device)
{
	md3dDevice = device;
}

ID3D11ShaderResourceView* TextureMgr::CreateTexture(std::wstring filename)
{
	ID3D11ShaderResourceView* srv = 0;

	// Does it already exist?
	if( mTextureSRV.find(filename) != mTextureSRV.end() )
	{
		srv = mTextureSRV[filename];
	}
	else
	{
		if (StrEndWith(filename, L".dds"))
		{
			HR(DirectX::CreateDDSTextureFromFile(md3dDevice, filename.c_str(), 0, &srv, 0));
		}
		else
		{
			HR(DirectX::CreateWICTextureFromFile(md3dDevice, filename.c_str(), 0, &srv, 0));
		}
		

		mTextureSRV[filename] = srv;
	}

	return srv;
}
 
