#include "BlurFilter.h"
#include "Effects.h"

BlurFilter::BlurFilter()
	:mWidth(0)
	,mHeight(0)
	,mFormat(DXGI_FORMAT_UNKNOWN)
	,mBlurredOutputTexSRV(NULL)
	,mBlurredOutputTexUAV(NULL)
{

}

BlurFilter::~BlurFilter()
{
	ReleaseCOM(mBlurredOutputTexSRV);
	ReleaseCOM(mBlurredOutputTexUAV);
}

ID3D11ShaderResourceView* 
BlurFilter::GetBlurredOutput()
{
	return mBlurredOutputTexSRV;
}

void BlurFilter::SetGaussianWeights(float sigma)
{
	float d = 2.f * sigma  * sigma;
	float weights[9];
	float sum = 0.f;
	for (int i = 0; i < 9; ++i)
	{
		float x = (float)(i - 4);
		weights[i] = expf(-x*x / d);
		sum += weights[i];
	}

	for (int i = 0; i < 9; ++i)
	{
		weights[i] /= sum;
	}
	Effects::BlurFX->SetWeights(weights);
}

void BlurFilter::SetWeights(const float weights[9])
{
	Effects::BlurFX->SetWeights(weights);
}

void BlurFilter::BlurInplace(ID3D11DeviceContext * dc, ID3D11ShaderResourceView * inputSRV, ID3D11UnorderedAccessView * inputUAV, int blurCount)
{
	for (int i = 0; i < blurCount; ++i)
	{
		D3DX11_TECHNIQUE_DESC techDesc;
		Effects::BlurFX->HorzBlurTech->GetDesc(&techDesc);
		for (UINT p = 0; p < techDesc.Passes; ++p)
		{
			Effects::BlurFX->SetInputMap(inputSRV);
			Effects::BlurFX->SetOutputMap(mBlurredOutputTexUAV);
			Effects::BlurFX->HorzBlurTech->GetPassByIndex(p)->Apply(0, dc);

			// How many groups do we need to dispatch to cover a row of pixels, where each
			// group covers 256 pixels (the 256 is defined in the ComputeShader).
			UINT numGroupsX = (UINT)ceilf(mWidth / 256.f);
			dc->Dispatch(numGroupsX, mHeight, 1);
		}

		//Unbind the input texture from the compute stage for good housekeeping.
		ID3D11ShaderResourceView* nullSRV[1] = { NULL };
		dc->CSSetShaderResources(0, 1, nullSRV);

		//Unbind output from compute stage( we are going to use this output as an input in the next pass
		//and a resource cannot be both an output and input at the same time.
		ID3D11UnorderedAccessView* nullUAV[1] = { NULL };
		dc->CSSetUnorderedAccessViews(0, 1, nullUAV, 0);

		Effects::BlurFX->VertBlurTech->GetDesc(&techDesc);
		for(UINT p = 0; p < techDesc.Passes; ++p )
		{
			Effects::BlurFX->SetInputMap(mBlurredOutputTexSRV);
			Effects::BlurFX->SetOutputMap(inputUAV);
			Effects::BlurFX->VertBlurTech->GetPassByIndex(p)->Apply(0, dc);

			UINT numGroupsY = (UINT)ceilf(mHeight / 256.f);
			dc->Dispatch(mWidth, numGroupsY, 1);
		}
		dc->CSSetShaderResources(0, 1, nullSRV);
		dc->CSSetUnorderedAccessViews(0, 1, nullUAV, NULL);
	}

	//Disable compute shader;
	dc->CSSetShader(NULL, NULL, NULL);
}

void BlurFilter::Init(ID3D11Device* device, UINT width, UINT height, DXGI_FORMAT format)
{
	ReleaseCOM(mBlurredOutputTexSRV);
	ReleaseCOM(mBlurredOutputTexUAV);

	mWidth = width;
	mHeight = height;
	mFormat = format;

	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.Format = format;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	ID3D11Texture2D* tex = NULL;
	HR(device->CreateTexture2D(&texDesc, 0, &tex));

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	HR(device->CreateShaderResourceView(tex, &srvDesc, &mBlurredOutputTexSRV));

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	uavDesc.Format = format;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;
	HR(device->CreateUnorderedAccessView(tex, &uavDesc, &mBlurredOutputTexUAV));
	
	ReleaseCOM(tex);

}
