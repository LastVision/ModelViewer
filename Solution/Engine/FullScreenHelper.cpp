#include "stdafx.h"

#include <d3dx11effect.h>
#include "EngineEnums.h"
#include "FullScreenHelper.h"
#include "IndexBufferWrapper.h"


namespace Prism
{
	FullScreenHelper::FullScreenHelper()
	{
		CreateCombineData();
		CreateRenderToTextureData();
		CreateBloomData();

		CU::Vector2<float> screenSize(static_cast<float>(Engine::GetInstance()->GetWindowSize().x)
			, static_cast<float>(Engine::GetInstance()->GetWindowSize().y));
		myProcessingTexture = new Texture();
		myProcessingTexture->Init(screenSize.x, screenSize.y
			, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL
			, DXGI_FORMAT_R32G32B32A32_FLOAT);


		D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		D3DX11_PASS_DESC passDesc;
		myRenderToTextureData.myEffect->GetTechnique()->GetPassByIndex(0)->GetDesc(&passDesc);
		HRESULT hr = Engine::GetInstance()->GetDevice()->CreateInputLayout(vertexDesc
			, ARRAYSIZE(vertexDesc), passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &myVertexLayout);
		if (FAILED(hr) != S_OK)
		{
			DL_MESSAGE_BOX("Failed to CreateInputLayout", "FullScreenHelper::FullScreenHelper()", MB_ICONWARNING);
		}

		Engine::GetInstance()->SetDebugName(myVertexLayout, "FullScreenHelper::myVertexLayout");

		InitVertexBuffer(sizeof(VertexPosUV), D3D11_USAGE_IMMUTABLE, 0);
		InitIndexBuffer();

		CreateVertices();

		ZeroMemory(&myViewPort, sizeof(D3D11_VIEWPORT));

		myViewPort.TopLeftX = 0;
		myViewPort.TopLeftY = 0;
		myViewPort.Width = static_cast<FLOAT>(Engine::GetInstance()->GetWindowSize().x);
		myViewPort.Height = static_cast<FLOAT>(Engine::GetInstance()->GetWindowSize().y);
		myViewPort.MinDepth = 0.f;
		myViewPort.MaxDepth = 1.f;

		myClearColor[0] = 0.f;
		myClearColor[1] = 0.f;
		myClearColor[2] = 0.f;
		myClearColor[3] = 0.f;
	}
	
	
	FullScreenHelper::~FullScreenHelper()
	{
	}

	void FullScreenHelper::Process(Texture* aSource, Texture* aTarget, int aEffect)
	{
		ActivateBuffers();
		Engine::GetInstance()->DisableZBuffer();

		if (aEffect == ePostProcessing::NONE)
		{
			CopyTexture(aSource, aTarget);
			Engine::GetInstance()->EnableZBuffer();
			return;
		}

		Engine::GetInstance()->GetContex()->ClearRenderTargetView(myProcessingTexture->GetRenderTargetView(), myClearColor);
		CopyTexture(aSource, myProcessingTexture);

		if (aEffect & ePostProcessing::BLOOM)
		{
			Engine::GetInstance()->GetContex()->ClearRenderTargetView(
				myBloomData.myDownSampleTextures[0]->GetRenderTargetView(), myClearColor);
			Engine::GetInstance()->GetContex()->ClearRenderTargetView(
				myBloomData.myDownSampleTextures[1]->GetRenderTargetView(), myClearColor);
			Engine::GetInstance()->GetContex()->ClearRenderTargetView(
				myBloomData.myFinalTexture->GetRenderTargetView(), myClearColor);

			BloomEffect(myProcessingTexture);

			Engine::GetInstance()->RestoreViewPort();
			CombineTextures(myBloomData.myFinalTexture, aSource, myProcessingTexture, false);
		}
		
		CopyTexture(myProcessingTexture, aTarget);
		Engine::GetInstance()->EnableZBuffer();
	}

	void FullScreenHelper::RenderToScreen(Texture* aSource)
	{
		Engine::GetInstance()->RestoreViewPort();
		Engine::GetInstance()->SetBackBufferAsTarget();

		myRenderToTextureData.mySource->SetResource(aSource->GetShaderView());
		Render(myRenderToTextureData.myEffect);
	}

	void FullScreenHelper::RenderToScreen(Texture* aSource, Texture* aDepth)
	{
		Engine::GetInstance()->RestoreViewPort();
		Engine::GetInstance()->SetBackBufferAsTarget();

		ID3D11RenderTargetView* target = Engine::GetInstance()->GetDepthBuffer();
		ID3D11DepthStencilView* depth = aDepth->GetDepthStencilView();
		Engine::GetInstance()->GetContex()->OMSetRenderTargets(1, &target, depth);

		myRenderToTextureData.mySource->SetResource(aSource->GetShaderView());
		Render(myRenderToTextureData.myEffect);
	}

	void FullScreenHelper::CopyTexture(Texture* aSource, Texture* aTarget)
	{
		DL_ASSERT_EXP(aSource != aTarget, "[Combine]: Cant use Texture as both Source and Target");

		Engine::GetInstance()->GetContex()->CopyResource(aTarget->GetTexture(), aSource->GetTexture());
	}

	void FullScreenHelper::CombineTextures(Texture* aSourceA, Texture* aSourceB, Texture* aTarget, bool aUseDepth)
	{
		DL_ASSERT_EXP(aSourceA != aTarget, "[Combine]: Cant use Texture as both Source and Target");
		DL_ASSERT_EXP(aSourceB != aTarget, "[Combine]: Cant use Texture as both Source and Target");

		myCombineData.mySourceA->SetResource(aSourceA->GetShaderView());
		myCombineData.mySourceB->SetResource(aSourceB->GetShaderView());

		ID3D11RenderTargetView* target = aTarget->GetRenderTargetView();
		ID3D11DepthStencilView* depth = aTarget->GetDepthStencilView();
		Engine::GetInstance()->GetContex()->ClearRenderTargetView(target, myClearColor);
		Engine::GetInstance()->GetContex()->ClearDepthStencilView(depth, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		Engine::GetInstance()->GetContex()->OMSetRenderTargets(1, &target, depth);

		if (aUseDepth == true)
		{
			myCombineData.myDepthA->SetResource(aSourceA->GetDepthStencilShaderView());
			myCombineData.myDepthB->SetResource(aSourceB->GetDepthStencilShaderView());
			Render(myCombineData.myEffect, "Depth");
		}
		else
		{
			Render(myCombineData.myEffect, "NoDepth");
		}
	}

	void FullScreenHelper::CombineTextures(Texture* aSourceA, Texture* aDepthA
		, Texture* aSourceB, Texture* aDepthB, Texture* aTarget)
	{
		DL_ASSERT_EXP(aSourceA != aTarget, "[Combine]: Cant use Texture as both Source and Target");
		DL_ASSERT_EXP(aSourceB != aTarget, "[Combine]: Cant use Texture as both Source and Target");

		myCombineData.mySourceA->SetResource(aSourceA->GetShaderView());
		myCombineData.myDepthA->SetResource(aDepthA->GetDepthStencilShaderView());
		myCombineData.mySourceB->SetResource(aSourceB->GetShaderView());
		myCombineData.myDepthB->SetResource(aDepthB->GetDepthStencilShaderView());


		ID3D11RenderTargetView* target = aTarget->GetRenderTargetView();
		ID3D11DepthStencilView* depth = aTarget->GetDepthStencilView();
		Engine::GetInstance()->GetContex()->ClearRenderTargetView(target, myClearColor);
		Engine::GetInstance()->GetContex()->ClearDepthStencilView(depth, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		Engine::GetInstance()->GetContex()->OMSetRenderTargets(1, &target, depth);

		Render(myCombineData.myEffect, "Depth");
	}

	void FullScreenHelper::CreateCombineData()
	{
		myCombineData.myEffect = EffectContainer::GetInstance()->GetEffect(
			"Data/Resource/Shader/S_effect_combine.fx");

		myCombineData.mySourceA
			= myCombineData.myEffect->GetEffect()->GetVariableByName("SourceA")->AsShaderResource();
		myCombineData.myDepthA
			= myCombineData.myEffect->GetEffect()->GetVariableByName("DepthA")->AsShaderResource();
		myCombineData.mySourceB
			= myCombineData.myEffect->GetEffect()->GetVariableByName("SourceB")->AsShaderResource();
		myCombineData.myDepthB
			= myCombineData.myEffect->GetEffect()->GetVariableByName("DepthB")->AsShaderResource();
	}

	void FullScreenHelper::CreateRenderToTextureData()
	{
		myRenderToTextureData.myEffect = EffectContainer::GetInstance()->GetEffect(
			"Data/Resource/Shader/S_effect_render_to_texture.fx");

		myRenderToTextureData.mySource
			= myRenderToTextureData.myEffect->GetEffect()->GetVariableByName("DiffuseTexture")->AsShaderResource();
	}

	void FullScreenHelper::CreateBloomData()
	{
		myBloomData.myFinalTexture = new Texture();
		myBloomData.myFinalTexture->Init(Engine::GetInstance()->GetWindowSize().x / 4.f
			, Engine::GetInstance()->GetWindowSize().y / 4.f
			, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL
			, DXGI_FORMAT_R8G8B8A8_UNORM);

		myBloomData.myMiddleMan = new Texture();
		myBloomData.myMiddleMan->Init(Engine::GetInstance()->GetWindowSize().x / 4.f
			, Engine::GetInstance()->GetWindowSize().y / 4.f
			, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL
			, DXGI_FORMAT_R8G8B8A8_UNORM);

		myBloomData.myBloomEffect
			= EffectContainer::GetInstance()->GetEffect("Data/Resource/Shader/S_effect_bloom.fx");
		myBloomData.myBloomVariable
			= myBloomData.myBloomEffect->GetEffect()->GetVariableByName("DiffuseTexture")->AsShaderResource();
		myBloomData.myTexelWidthVariable
			= myBloomData.myBloomEffect->GetEffect()->GetVariableByName("TexelWidth")->AsScalar();
		myBloomData.myTexelHeightVariable
			= myBloomData.myBloomEffect->GetEffect()->GetVariableByName("TexelHeight")->AsScalar();

		myBloomData.myTexelWidthVariable->SetFloat(1.f / (Engine::GetInstance()->GetWindowSize().x / 4));
		myBloomData.myTexelHeightVariable->SetFloat(1.f / (Engine::GetInstance()->GetWindowSize().y / 4));


		myBloomData.myDownSampleEffect
			= EffectContainer::GetInstance()->GetEffect("Data/Resource/Shader/S_effect_down_sample.fx");
		myBloomData.myDownSampleVariable
			= myBloomData.myDownSampleEffect->GetEffect()->GetVariableByName("DiffuseTexture")->AsShaderResource();

		myBloomData.myDownSampleTextures[0] = new Texture();
		myBloomData.myDownSampleTextures[0]->Init(Engine::GetInstance()->GetWindowSize().x / 2.f
			, Engine::GetInstance()->GetWindowSize().y / 2.f
			, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL
			, DXGI_FORMAT_R8G8B8A8_UNORM);

		myBloomData.myDownSampleTextures[1] = new Texture();
		myBloomData.myDownSampleTextures[1]->Init(Engine::GetInstance()->GetWindowSize().x / 4.f
			, Engine::GetInstance()->GetWindowSize().y / 4.f
			, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL
			, DXGI_FORMAT_R8G8B8A8_UNORM);
	}

	void FullScreenHelper::CreateVertices()
	{
		CU::GrowingArray<VertexPosUV> vertices;
		vertices.Init(4);
		vertices.Add({ { -1.f, -1.f, 0.f }, { 0.f, 1.f } }); //topleft
		vertices.Add({ { 1.f, -1.f, 0.f }, { 1.f, 1.f } }); //topright
		vertices.Add({ { -1.f, 1.f, 0.f }, { 0.f, 0.f } }); //bottomleft
		vertices.Add({ { 1.f, 1.f, 0.f }, { 1.f, 0.f } }); //bottomright

		CU::GrowingArray<int> indices;
		indices.Init(6);

		indices.Add(0);
		indices.Add(2);
		indices.Add(1);

		indices.Add(1);
		indices.Add(2);
		indices.Add(3);

		SetupVertexBuffer(vertices.Size(), sizeof(VertexPosUV), reinterpret_cast<char*>(&vertices[0])
			, "FullScreenHelper::VertexBuffer");
		SetupIndexBuffer(indices.Size(), reinterpret_cast<char*>(&indices[0])
			, "FullScreenHelper::IndexBuffer");
	}

	void FullScreenHelper::ActivateBuffers()
	{
		Engine::GetInstance()->GetContex()->IASetInputLayout(myVertexLayout);
		Engine::GetInstance()->GetContex()->IASetVertexBuffers(myVertexBuffer->myStartSlot,
			myVertexBuffer->myNumberOfBuffers, &myVertexBuffer->myVertexBuffer,
			&myVertexBuffer->myStride, &myVertexBuffer->myByteOffset);
		Engine::GetInstance()->GetContex()->IASetIndexBuffer(myIndexBuffer->myIndexBuffer,
			myIndexBuffer->myIndexBufferFormat, myIndexBuffer->myByteOffset);
		Engine::GetInstance()->GetContex()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	void FullScreenHelper::Render(Effect* aEffect)
	{
		D3DX11_TECHNIQUE_DESC techDesc;
		aEffect->GetTechnique()->GetDesc(&techDesc);
		for (UINT i = 0; i < techDesc.Passes; ++i)
		{
			aEffect->GetTechnique()->GetPassByIndex(i)->Apply(0, Engine::GetInstance()->GetContex());
			Engine::GetInstance()->GetContex()->DrawIndexed(6, 0, 0);
		}
	}

	void FullScreenHelper::Render(Effect* aEffect, const std::string& aTechnique)
	{
		ID3DX11EffectTechnique* tech = aEffect->GetEffect()->GetTechniqueByName(aTechnique.c_str());
		DL_ASSERT_EXP(tech->IsValid() != 0, "[FullScreenHelper]: Tried to use invalid EffectTechnique");

		D3DX11_TECHNIQUE_DESC techDesc;
		tech->GetDesc(&techDesc);
		for (UINT i = 0; i < techDesc.Passes; ++i)
		{
			tech->GetPassByIndex(i)->Apply(0, Engine::GetInstance()->GetContex());
			Engine::GetInstance()->GetContex()->DrawIndexed(6, 0, 0);
		}
	}


	void FullScreenHelper::DownSample(Texture* aTarget, Texture* aSource, float aWidth, float aHeight)
	{
		myViewPort.Width = aWidth;
		myViewPort.Height = aHeight;
		Engine::GetInstance()->GetContex()->RSSetViewports(1, &myViewPort);

		ID3D11RenderTargetView* target = aTarget->GetRenderTargetView();
		ID3D11DepthStencilView* depth = aTarget->GetDepthStencilView();
		
		Engine::GetInstance()->GetContex()->OMSetRenderTargets(1, &target, depth);

		myBloomData.myDownSampleVariable->SetResource(aSource->GetShaderView());

		Render(myBloomData.myDownSampleEffect, "BLOOM_DOWNSAMPLE");

		myBloomData.myDownSampleVariable->SetResource(NULL);
	}

	void FullScreenHelper::BloomEffect(Texture* aSource)
	{
		float width = static_cast<FLOAT>(Engine::GetInstance()->GetWindowSize().x / 2);
		float height = static_cast<FLOAT>(Engine::GetInstance()->GetWindowSize().y / 2);

		DownSample(myBloomData.myDownSampleTextures[0], aSource, width, height);

		width /= 2.f;
		height /= 2.f;

		DownSample(myBloomData.myDownSampleTextures[1]
			, myBloomData.myDownSampleTextures[0], width, height);


		DoBloom(myBloomData.myDownSampleTextures[1], myBloomData.myFinalTexture);
	}

	void FullScreenHelper::DoBloom(Texture* aSource, Texture* aTarget)
	{
		ID3D11RenderTargetView* target = myBloomData.myMiddleMan->GetRenderTargetView();
		ID3D11DepthStencilView* depth = myBloomData.myMiddleMan->GetDepthStencilView();
		Engine::GetInstance()->GetContex()->ClearRenderTargetView(target, myClearColor);
		Engine::GetInstance()->GetContex()->OMSetRenderTargets(1, &target, depth);
		myBloomData.myBloomVariable->SetResource(aSource->GetShaderView());

		Render(myBloomData.myBloomEffect, "BLOOM_X");

		target = aTarget->GetRenderTargetView();
		depth = aTarget->GetDepthStencilView();
		Engine::GetInstance()->GetContex()->ClearRenderTargetView(target, myClearColor);
		Engine::GetInstance()->GetContex()->OMSetRenderTargets(1, &target, depth);
		myBloomData.myBloomVariable->SetResource(myBloomData.myMiddleMan->GetShaderView());

		Render(myBloomData.myBloomEffect, "BLOOM_Y");
	}

	void FullScreenHelper::OnResize(float aWidth, float aHeight)
	{
		myBloomData.myFinalTexture->Resize(aWidth/4.f, aHeight/4.f);
		myBloomData.myMiddleMan->Resize(aWidth/4.f, aHeight/4.f);
		myBloomData.myDownSampleTextures[0]->Resize(aWidth/2.f, aHeight/2.f);
		myBloomData.myDownSampleTextures[1]->Resize(aWidth/4.f, aHeight/4.f);

		myProcessingTexture->Resize(aWidth, aHeight);
	}
}
