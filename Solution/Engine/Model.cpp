#include "stdafx.h"

#include <CommonHelper.h>
#include <d3dx11effect.h>
#include "Effect.h"
#include "EffectContainer.h"
#include "Engine.h"
#include "IndexBufferWrapper.h"
#include "Model.h"
#include "Surface.h"
#include "VertexBufferWrapper.h"
#include "VertexDataWrapper.h"
#include "VertexIndexWrapper.h"

namespace Prism
{

	Model::Model()
		: BaseModel()
		, myChildren(32)
		, myChildTransforms(32)
		, myVertexFormat(8)
		, myIsNULLObject(true)
		, myVertexBaseData(nullptr)
		, myIndexBaseData(nullptr)
		, myIsLodGroup(false)
		, myLodGroup(nullptr)
		, myInited(false)
		, myParent(nullptr)
		, myVertexCount(0)
	{
	}

	Model::~Model()
	{
		myChildren.DeleteAll();
		myVertexFormat.DeleteAll();
		delete myVertexBaseData;
		delete myIndexBaseData;
	}

	void Model::Init()
	{
		DL_ASSERT_EXP(myInited == false, "Tried to Init a model twice");

		if (myIsNULLObject == false)
		{
			const int size = myVertexFormat.Size();
			D3D11_INPUT_ELEMENT_DESC* vertexDesc = new D3D11_INPUT_ELEMENT_DESC[size];
			for (int i = 0; i < myVertexFormat.Size(); ++i)
			{
				vertexDesc[i] = *myVertexFormat[i];
			}

			EvaluateEffectTechnique();
			InitInputLayout(vertexDesc, size, "Model::InputLayout");
			delete[] vertexDesc;
			InitVertexBuffer(myVertexBaseData->myStride, D3D11_USAGE_IMMUTABLE, 0);
			InitIndexBuffer();

			SetupVertexBuffer(myVertexBaseData->myNumberOfVertices
				, myVertexBaseData->myStride, myVertexBaseData->myVertexData, "Model::VertexBuffer");

			SetupIndexBuffer(myIndexBaseData->myNumberOfIndices, myIndexBaseData->myIndexData, "Model::IndexBuffer");

			myVertexCount = myVertexBaseData->myNumberOfVertices;
		}

		for (int i = 0; i < myChildren.Size(); ++i)
		{
			myChildren[i]->myFileName = myFileName;
			myChildren[i]->Init();
		}


		myInited = true;

		if (myEffect->GetFileName() == "Data/Resource/Shader/S_effect_pbl.fx")
		{
			for (int i = 0; i < mySurfaces.Size(); ++i)
			{
				if (mySurfaces[i]->VerifyTextures(myFileName) == false)
				{
					DL_ASSERT(CU::Concatenate("Missing PBL-texture from model: %s", myFileName.c_str()));
				}
			}
		}
	}

	void Model::InitCube(const float aWidth, const float aHeight, const float aDepth, CU::Vector4f aColour)
	{
		DL_ASSERT_EXP(myInited == false, "Tried to Init a model twice");
		myEffect = EffectContainer::GetInstance()->GetEffect("Data/Resource/Shader/S_effect_cube_colored.fx");


		if (myEffect == nullptr)
		{
			DL_MESSAGE_BOX("Failed to GetEffect", "InitLightCube", MB_ICONWARNING);
		}

		D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		InitInputLayout(vertexDesc, ARRAYSIZE(vertexDesc), "Model::InputLayout");


#pragma region Vertices
		CU::GrowingArray<VertexPosColor> vertices;
		vertices.Init(24);

		float halfWidth = aWidth / 2.f;
		float halfHeight = aHeight / 2.f;
		float halfDepth = aDepth / 2.f;

		//0 - 3 (Top)
		vertices.Add({ { -halfWidth, halfHeight, -halfDepth }, aColour });
		vertices.Add({ { halfWidth, halfHeight, -halfDepth }, aColour });
		vertices.Add({ { halfWidth, halfHeight, halfDepth }, aColour });
		vertices.Add({ { -halfWidth, halfHeight, halfDepth }, aColour });

		//4 - 7 (Bottom)
		vertices.Add({ { -halfWidth, -halfHeight, -halfDepth }, aColour });
		vertices.Add({ { halfWidth, -halfHeight, -halfDepth }, aColour });
		vertices.Add({ { halfWidth, -halfHeight, halfDepth }, aColour });
		vertices.Add({ { -halfWidth, -halfHeight, halfDepth }, aColour });

		//8 - 11 (Left)
		vertices.Add({ { -halfWidth, -halfHeight, halfDepth }, aColour });
		vertices.Add({ { -halfWidth, -halfHeight, -halfDepth }, aColour });
		vertices.Add({ { -halfWidth, halfHeight, -halfDepth }, aColour });
		vertices.Add({ { -halfWidth, halfHeight, halfDepth }, aColour });

		//12 - 15 (Right)
		vertices.Add({ { halfWidth, -halfHeight, halfDepth }, aColour });
		vertices.Add({ { halfWidth, -halfHeight, -halfDepth }, aColour });
		vertices.Add({ { halfWidth, halfHeight, -halfDepth }, aColour });
		vertices.Add({ { halfWidth, halfHeight, halfDepth }, aColour });

		//16 - 19 (Front)
		vertices.Add({ { -halfWidth, -halfHeight, -halfDepth }, aColour });
		vertices.Add({ { halfWidth, -halfHeight, -halfDepth }, aColour });
		vertices.Add({ { halfWidth, halfHeight, -halfDepth }, aColour });
		vertices.Add({ { -halfWidth, halfHeight, -halfDepth }, aColour });

		//20 - 23 (Back)
		vertices.Add({ { -halfWidth, -halfHeight, halfDepth }, aColour });
		vertices.Add({ { halfWidth, -halfHeight, halfDepth }, aColour });
		vertices.Add({ { halfWidth, halfHeight, halfDepth }, aColour });
		vertices.Add({ { -halfWidth, halfHeight, halfDepth }, aColour });
#pragma endregion

#pragma region Indices

		CU::GrowingArray<int> indices(24);
		//Top
		indices.Add(3);
		indices.Add(1);
		indices.Add(0);

		indices.Add(2);
		indices.Add(1);
		indices.Add(3);

		//Bottom
		indices.Add(6);
		indices.Add(4);
		indices.Add(5);

		indices.Add(7);
		indices.Add(4);
		indices.Add(6);

		//Left
		indices.Add(11);
		indices.Add(9);
		indices.Add(8);

		indices.Add(10);
		indices.Add(9);
		indices.Add(11);

		//Right
		indices.Add(14);
		indices.Add(12);
		indices.Add(13);

		indices.Add(15);
		indices.Add(12);
		indices.Add(14);

		//Front
		indices.Add(19);
		indices.Add(17);
		indices.Add(16);

		indices.Add(18);
		indices.Add(17);
		indices.Add(19);

		//Back
		indices.Add(22);
		indices.Add(20);
		indices.Add(21);

		indices.Add(23);
		indices.Add(20);
		indices.Add(22);

#pragma endregion


		InitVertexBuffer(sizeof(VertexPosColor), D3D11_USAGE_IMMUTABLE, 0);
		InitIndexBuffer();

		SetupVertexBuffer(vertices.Size(), sizeof(VertexPosColor), reinterpret_cast<char*>(&vertices[0]), "Model::VertexBuffer");
		SetupIndexBuffer(indices.Size(), reinterpret_cast<char*>(&indices[0]), "Model::IndexBuffer");

		Surface surf;
		surf.SetEffect(myEffect);
		surf.SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		surf.SetVertexStart(0);
		surf.SetVertexCount(vertices.Size());
		surf.SetIndexStart(0);
		surf.SetIndexCount(indices.Size());
		surf.SetTexture("DiffuseTexture", "Data/resources/texture/seafloor.dds", true);

		mySurfaces.Add(new Surface(surf));

		myIsNULLObject = false;
		myInited = true;
	}

	void Model::AddChild(Model* aChild)
	{
		aChild->myParent = this;
		myChildren.Add(aChild);
		myChildTransforms.Add(aChild->myOrientation);
	}

	void Prism::Model::SetLodGroup(LodGroup* aLodGroup)
	{
		myLodGroup = aLodGroup;
		if (myLodGroup != nullptr)
		{
			myIsLodGroup = true;
		}
	}

	void Model::SetEffect(Effect* aEffect)
	{
		myEffect = aEffect;
		myEffect->AddListener(this);

		for (int i = 0; i < mySurfaces.Size(); ++i)
		{
			mySurfaces[i]->SetEffect(myEffect);
			mySurfaces[i]->ReloadSurface();
		}

		for (int i = 0; i < myChildren.Size(); ++i)
		{
			myChildren[i]->SetEffect(aEffect);
		}
	}

	void Model::Render(const CU::Matrix44<float>& aOrientation, const CU::Vector3<float>& aCameraPosition)
	{
		if (myIsLodGroup == true)
		{
			float lengthBetweenCameraAndModel = CU::Length(aCameraPosition - aOrientation.GetPos());
			int level = 0;

			Model* toRender = nullptr;
			for (int i = myChildren.Size() - 1; i >= 0; i--)
			{
				LodGroup* group = myLodGroup;
				double threshold = group->myThreshHolds[i];
				threshold /= 100;
				if (threshold <= lengthBetweenCameraAndModel)
				{
					toRender = myChildren[i];
					level = i;
					break;
				}
			}

			if (toRender)
			{
				toRender->Render(aOrientation, aCameraPosition);
			}
		}
		else
		{
			if (myIsNULLObject == false)
			{
				float blendFactor[4];
				blendFactor[0] = 0.f;
				blendFactor[1] = 0.f;
				blendFactor[2] = 0.f;
				blendFactor[3] = 0.f;

				myEffect->SetBlendState(NULL, blendFactor);
				myEffect->SetWorldMatrix(aOrientation);

				BaseModel::Render();
			}

			for (int i = 0; i < myChildren.Size(); ++i)
			{
				myChildren[i]->Render(myChildTransforms[i] * aOrientation, aCameraPosition);
			}
		}
	}

	void Model::ActivateAlbedo(eOwnerType aOwner)
	{
		for (int i = 0; i < mySurfaces.Size(); ++i)
		{
			mySurfaces[i]->ActivateAlbedo(aOwner);
		}

		for (int i = 0; i < myChildren.Size(); ++i)
		{
			myChildren[i]->ActivateAlbedo(aOwner);
		}
	}

	void Model::EvaluateEffectTechnique()
	{
		int uvCount = 0;
		bool hasVertexColor = false;
		for (int i = 0; i < myVertexFormat.Size(); ++i)
		{
			std::string semanticName(myVertexFormat[i]->SemanticName);
			if (semanticName == "TEXCOORD")
			{
				++uvCount;
			}
			if (semanticName == "COLOR")
			{
				hasVertexColor = true;
			}
		}

		if (hasVertexColor == true)
		{
			if (uvCount == 2)
			{
				myTechniqueName = "Render_2UVSET_COLOR";

			}
			else if (uvCount == 1)
			{
				myTechniqueName = "Render_1UVSET_COLOR";
			}
			else
			{
				DL_ASSERT("Model EvaluateTechnique: invalid uv-set-count with vertexcolor.");
			}
		}
		else
		{
			if (uvCount == 2)
			{
				myTechniqueName = "Render_2UVSET";

			}
			else if (uvCount == 1)
			{
				myTechniqueName = "Render_1UVSET";
			}
			else
			{
				DL_ASSERT("Model EvaluateTechnique: invalid uv-set-count without vertexcolor.");
			}
		}

	}

}