#include "stdafx.h"

#include "CommonHelper.h"
#include <d3dx11effect.h>
#include "Effect.h"
#include "Surface.h"
#include "Texture.h"
#include "TextureContainer.h"

Prism::Surface::Surface()
{
	myTextures.Init(8);
	myShaderResourceViews.Init(8);
	myFilePaths.Init(8);
	myShaderResourceNames.Init(8);
	myEmissive = false;
}

Prism::Surface::~Surface()
{
	myShaderResourceViews.RemoveAll();
	myFilePaths.RemoveAll();
	myTextures.RemoveAll();
	myShaderResourceNames.RemoveAll();
}

bool Prism::Surface::SetTexture(const std::string& aResourceName, const std::string& aFileName, bool aUseSRGB)
{
	aUseSRGB;

	DL_ASSERT_EXP(aFileName != ""
		, CU::Concatenate("Shader resource ( %s ) tried to use invalid filePath", aResourceName.c_str()));

	Texture* tex = TextureContainer::GetInstance()->GetTexture(aFileName);

	if (aResourceName == "AlbedoTexture")
	{
		int index = aFileName.find("_neutral.dds");
		if (index != std::string::npos)
		{
			std::string playerAlbedo(aFileName.begin(), aFileName.begin() + index);
			playerAlbedo += "_player.dds";

			std::string enemyAlbedo(aFileName.begin(), aFileName.begin() + index);
			enemyAlbedo += "_enemy.dds";

			myOwnerAlbedoTextures[0] = tex;
			myOwnerAlbedoTextures[1] = TextureContainer::GetInstance()->GetTexture(playerAlbedo);
			myOwnerAlbedoTextures[2] = TextureContainer::GetInstance()->GetTexture(enemyAlbedo);
		}
	}


	ID3DX11EffectShaderResourceVariable* shaderVar = myEffect->GetEffect()->GetVariableByName(aResourceName.c_str())->AsShaderResource();

	if (shaderVar->IsValid() == false)
	{
		std::string errorMsg = "Failed to get ShaderResource: " + aResourceName;
		//DL_MESSAGE_BOX(errorMsg.c_str(), "Surface Error", MB_ICONWARNING);
		RESOURCE_LOG(errorMsg.c_str());
		return false;
	}

	if (aResourceName == "EmissiveTexture")
	{
		myEmissive = true;
	}

	myTextures.Add(tex);
	myShaderResourceViews.Add(shaderVar);
	myFilePaths.Add(aFileName);
	myShaderResourceNames.Add(aResourceName);

	return true;
}

bool Prism::Surface::SetTexture(const std::string& aResourceName, ID3D11ShaderResourceView* aResource)
{
	aResource;
	ID3DX11EffectShaderResourceVariable* shaderVar = myEffect->GetEffect()->GetVariableByName(aResourceName.c_str())->AsShaderResource();
	if (shaderVar->IsValid() == false)
	{
		//DL_MESSAGE_BOX("Failed to get ShaderResource", "Surface Error", MB_ICONWARNING);
		RESOURCE_LOG("Failed to get ShaderResource");
		return false;
	}

	myShaderResourceViews.Add(shaderVar);
	myShaderResourceNames.Add(aResourceName);
	return true;
}

void Prism::Surface::ActivateAlbedo(eOwnerType aOwner)
{
	//return;

	DL_ASSERT_EXP(aOwner != eOwnerType::NOT_USED, "NOT_USED cannot be used to ActivateAlbedo");

	if (myOwnerAlbedoTextures[0] == nullptr || myOwnerAlbedoTextures[1] == nullptr
		|| myOwnerAlbedoTextures[2] == nullptr)
	{
		return;
	}

	for (int i = 0; i < myShaderResourceNames.Size(); ++i)
	{
		if (myShaderResourceNames[i] == "AlbedoTexture")
		{
			switch (aOwner)
			{
			case NEUTRAL:
				myTextures[i] = myOwnerAlbedoTextures[0];
				break;
			case PLAYER:
				myTextures[i] = myOwnerAlbedoTextures[1];
				break;
			case ENEMY:
				myTextures[i] = myOwnerAlbedoTextures[2];
				break;
			default:
				myTextures[i] = myOwnerAlbedoTextures[0];
				break;
			}	
		}
	}
}

void Prism::Surface::ReloadSurface()
{
	myTextures.RemoveAll();
	myShaderResourceViews.RemoveAll();

	for (int i = 0; i < myFilePaths.Size(); ++i)
	{
		Texture* tex = TextureContainer::GetInstance()->GetTexture(myFilePaths[i]);
		ID3DX11EffectShaderResourceVariable* shaderVar = myEffect->GetEffect()->GetVariableByName(myShaderResourceNames[i].c_str())->AsShaderResource();

		if (shaderVar->IsValid() == false)
		{
			std::string errorMsg = "Failed to get ShaderResource: " + myShaderResourceNames[i];
			//DL_MESSAGE_BOX(errorMsg.c_str(), "Surface Error", MB_ICONWARNING);
			RESOURCE_LOG(errorMsg.c_str());
		}

		myTextures.Add(tex);
		myShaderResourceViews.Add(shaderVar);
	}
}

bool Prism::Surface::VerifyTextures(const std::string& aModelPath)
{
	if (aModelPath.find("skySphere") != std::string::npos)
	{
		return true;
	}

	bool diffuse = false;
	bool albedo = false;
	bool normal = false;
	bool roughness = false;
	bool metal = false;
	bool ao = false;
	bool emissiveness = false;

	for (int i = 0; i < myShaderResourceNames.Size(); ++i)
	{
		if (myShaderResourceNames[i] == "DiffiuseTexture")
		{
			diffuse = true;
		}
		else if (myShaderResourceNames[i] == "AlbedoTexture")
		{
			albedo = true;
		}
		else if (myShaderResourceNames[i] == "NormalTexture")
		{
			normal = true;
		}
		else if (myShaderResourceNames[i] == "RoughnessTexture")
		{
			roughness = true;
		}
		else if (myShaderResourceNames[i] == "MetalnessTexture")
		{
			metal = true;
		}
		else if (myShaderResourceNames[i] == "AOTexture")
		{
			ao = true;
		}
		else if (myShaderResourceNames[i] == "EmissiveTexture")
		{
			emissiveness = true;
		}
	}


	if (diffuse == true) return true;

#ifndef _DEBUG
	DL_ASSERT_EXP(albedo == true, CU::Concatenate("Albedo missing from %s", aModelPath.c_str()));
	DL_ASSERT_EXP(normal == true, CU::Concatenate("NormalMap missing from %s", aModelPath.c_str()));
	DL_ASSERT_EXP(roughness == true, CU::Concatenate("Roughness missing from %s", aModelPath.c_str()));
	DL_ASSERT_EXP(metal == true, CU::Concatenate("Metalness missing from %s", aModelPath.c_str()));
	DL_ASSERT_EXP(ao == true, CU::Concatenate("AmbientOcclusion missing from %s", aModelPath.c_str()));
	DL_ASSERT_EXP(emissiveness == true, CU::Concatenate("Emissiveness missing from %s", aModelPath.c_str()));
#endif

	return true;
}

void Prism::Surface::Activate()
{
	Engine::GetInstance()->GetContex()->IASetPrimitiveTopology(myPrimitiveTopologyType);

	for (int i = 0; i < myShaderResourceViews.Size(); ++i)
	{
		myShaderResourceViews[i]->SetResource(myTextures[i]->GetShaderView());
	}
}