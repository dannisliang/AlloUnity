#include "CubemapExtractionPlugin.h"
#include "CubemapFaceD3D11.h"
#include <boost/thread/mutex.hpp>

#if SUPPORT_D3D11

static AVPixelFormat avPixel2DXGIFormat(DXGI_FORMAT format)
{
	switch (format)
	{
	case DXGI_FORMAT_R8G8B8A8_TYPELESS:
		return AV_PIX_FMT_RGBA;
	default:
		return AV_PIX_FMT_NONE;
	}
}

CubemapFaceD3D11::CubemapFaceD3D11(boost::uint32_t width,
                                   boost::uint32_t height,
	                               int index,
								   boost::chrono::system_clock::time_point presentationTime,
                                   void* pixels,
	                               Allocator& allocator,
	                               ID3D11Texture2D* gpuTexturePtr,
	                               ID3D11Texture2D* cpuTexturePtr,
	                               D3D11_MAPPED_SUBRESOURCE resource,
	                               D3D11_TEXTURE2D_DESC& description)
	:
	CubemapFace(width,
	            height,
	            index,
	            avPixel2DXGIFormat(description.Format),
	            presentationTime,
				pixels,
	            allocator),
	gpuTexturePtr(gpuTexturePtr),
	cpuTexturePtr(cpuTexturePtr),
	resource(resource)
{
}

CubemapFaceD3D11* CubemapFaceD3D11::create(ID3D11Texture2D* texturePtr,
	                                       int index,
	                                       Allocator& allocator)
{

	ID3D11Texture2D* gpuTexturePtr = (ID3D11Texture2D*)texturePtr;

	D3D11_TEXTURE2D_DESC textureDescription;
	gpuTexturePtr->GetDesc(&textureDescription);

	UINT width = textureDescription.Width;
	UINT height = textureDescription.Height;

	textureDescription.BindFlags = 0;
	textureDescription.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	textureDescription.Usage = D3D11_USAGE_STAGING;

	ID3D11Texture2D* cpuTexturePtr;
	HRESULT hr = g_D3D11Device->CreateTexture2D(&textureDescription, NULL, &cpuTexturePtr);

	ID3D11DeviceContext* g_D3D11DeviceContext = NULL;
	g_D3D11Device->GetImmediateContext(&g_D3D11DeviceContext);
	//HRESULT hr2 = g_D3D11Device->CreateDeferredContext(0, &g_D3D11DeviceContext);

	D3D11_MAPPED_SUBRESOURCE resource;
	ZeroMemory(&resource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	unsigned int subresource = D3D11CalcSubresource(0, 0, 0);
	hr = g_D3D11DeviceContext->Map(cpuTexturePtr, subresource, D3D11_MAP_READ, 0, &resource);
	g_D3D11DeviceContext->Unmap(cpuTexturePtr, subresource);

	void* pixels = allocator.allocate(width * height * 4);
	void* addr = allocator.allocate(sizeof(CubemapFaceD3D11));

	return new (addr) CubemapFaceD3D11(width,
		                               height,
		                               index,
									   boost::chrono::system_clock::now(),
									   pixels,
						               allocator,
		                               gpuTexturePtr,
		                               cpuTexturePtr,
		                               resource,
		                               textureDescription);
}

#endif