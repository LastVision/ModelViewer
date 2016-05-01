#include "stdafx.h"
#include "SpriteProxy.h"

namespace Prism
{
	SpriteProxy::SpriteProxy()
		: mySprite(nullptr)
	{
	}

	void SpriteProxy::Render(const CU::Vector2<float>& aPosition, const CU::Vector2<float>& aScale, const CU::Vector4<float>& aColor)
	{
		if (mySprite != nullptr)
		{
			mySprite->SetSize(mySize, myHotspot);
			mySprite->Render(aPosition, aScale, aColor);
		}
	}

	void SpriteProxy::SetSize(const CU::Vector2<float> aTextureSize, const CU::Vector2<float>& aHotSpot)
	{
		mySize = aTextureSize;
		myHotspot = aHotSpot;
	}

	const CU::Vector2<float>& SpriteProxy::GetSize() const
	{
		/*if (mySprite != nullptr)
		{
			return mySprite->GetSize();
		}*/

		return mySize;
	}

	void SpriteProxy::ResizeTexture(ID3D11Texture2D* aSrcTexture)
	{
		if (mySprite != nullptr)
		{
			mySprite->ResizeTexture(aSrcTexture);
		}
	}

	void SpriteProxy::Rotate(float aRadians)
	{
		if (mySprite != nullptr)
		{
			mySprite->Rotate(aRadians);
		}
	}

	void SpriteProxy::CopyFromD3DTexture(ID3D11Texture2D* aTexture)
	{
		if (mySprite != nullptr)
		{
			mySprite->CopyFromD3DTexture(aTexture);
		}
	}

	const CU::Vector2<float>& SpriteProxy::GetHotspot() const
	{
		/*if (mySprite != nullptr)
		{
			return mySprite->GetHotspot();
		}*/

		return myHotspot;
	}
}