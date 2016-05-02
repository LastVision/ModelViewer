#include "stdafx.h"

#include "Animation.h"
#include "AnimationNode.h"
#include "BaseModel.h"
#include <CommonHelper.h>
#include "DGFXLoader.h"
#include "Model.h"
#include "ModelAnimated.h"
#include "HierarchyBone.h"
#include "VertexDataWrapper.h"
#include "VertexIndexWrapper.h"
#include "Surface.h"
#include <TimerManager.h>

namespace Prism
{
	DGFXLoader::DGFXLoader()
	{
	}


	DGFXLoader::~DGFXLoader()
	{
		for (auto it = myModels.begin(); it != myModels.end(); ++it)
		{
			SAFE_DELETE(it->second);
		}
		myModels.clear();

		for (auto it = myModelsAnimated.begin(); it != myModelsAnimated.end(); ++it)
		{
			SAFE_DELETE(it->second);
		}
		myModelsAnimated.clear();
	}

	Model* DGFXLoader::LoadModel(const std::string& aFilePath, Effect* aEffect)
	{
#ifndef DLL_EXPORT
		if (myModels.find(aFilePath) != myModels.end())
		{
			return myModels[aFilePath];
		}
#endif

		std::string dgfxFile = CU::GetGeneratedDataFolderFilePath(aFilePath, "dgfx");

#ifndef RELEASE_BUILD
		if (CheckIfFbxIsNewer(dgfxFile) == true)
		{
			DL_MESSAGE_BOX("Found a FBX-File thats newer than the DGFX-File, did you forget to run the tool?", "Old DGFX", MB_ICONQUESTION);
		}
#endif


		CU::TimerManager::GetInstance()->StartTimer("LoadDGFX");

		std::fstream file;
		file.open(dgfxFile.c_str(), std::ios::in | std::ios::binary);
		DL_ASSERT_EXP(file.fail() == false, CU::Concatenate("Failed to open %s, did you forget to run the tool?", dgfxFile.c_str()));
		if (file.fail() == true)
		{
			assert(false && "FAILED TO OPEN DGFX-FILE");
			return nullptr;
		}

		Model* newModel = CreateModel(aEffect, file);

		file.close();

		newModel->Init();
		int elapsed = static_cast<int>(
			CU::TimerManager::GetInstance()->StopTimer("LoadDGFX").GetMilliseconds());
		RESOURCE_LOG("DGFX-Model \"%s\" took %d ms to load", dgfxFile.c_str(), elapsed);

		myModels[aFilePath] = newModel;

		return newModel;
	}

	ModelAnimated* DGFXLoader::LoadAnimatedModel(const std::string& aFilePath, Effect* aEffect)
	{
		if (myModelsAnimated.find(aFilePath) != myModelsAnimated.end())
		{
			return myModelsAnimated[aFilePath];
		}

		std::string dgfxFile = CU::GetGeneratedDataFolderFilePath(aFilePath, "dgfx");

#ifndef RELEASE_BUILD
		if (CheckIfFbxIsNewer(dgfxFile) == true)
		{
			DL_MESSAGE_BOX("Found a FBX-File thats newer than the DGFX-File, did you forget to run the tool?", "Old DGFX", MB_ICONQUESTION);
		}
#endif

		CU::TimerManager::GetInstance()->StartTimer("LoadDGFXAnimated");

		std::fstream file;
		file.open(dgfxFile.c_str(), std::ios::in | std::ios::binary);
		DL_ASSERT_EXP(file.fail() == false, CU::Concatenate("Failed to open %s, did you forget to run the tool?", dgfxFile.c_str()));
		if (file.fail() == true)
		{
			assert(false && "FAILED TO OPEN DGFX-FILE");
			return nullptr;
		}

		ModelAnimated* newModel = CreateModelAnimated(aEffect, file);

		file.close();

		newModel->Init();

		int elapsed = static_cast<int>(
			CU::TimerManager::GetInstance()->StopTimer("LoadDGFXAnimated").GetMilliseconds());
		RESOURCE_LOG("Animated DGFX-Model \"%s\" took %d ms to load", dgfxFile.c_str(), elapsed);

		myModelsAnimated[aFilePath] = newModel;

		return newModel;
	}

	Animation* DGFXLoader::LoadAnimation(const std::string& aFilePath)
	{
		if (myAnimations.find(aFilePath) != myAnimations.end())
		{
			return myAnimations[aFilePath];
		}

		std::string dgfxFile = CU::GetGeneratedDataFolderFilePath(aFilePath, "dgfx");

#ifndef RELEASE_BUILD
		if (CheckIfFbxIsNewer(dgfxFile) == true)
		{
			DL_MESSAGE_BOX("Found a FBX-File thats newer than the DGFX-File, did you forget to run the tool?", "Old DGFX", MB_ICONQUESTION);
		}
#endif

		CU::TimerManager::GetInstance()->StartTimer("LoadAnimationDGFX");


		std::fstream stream;
		stream.open(dgfxFile.c_str(), std::ios::in | std::ios::binary);
		DL_ASSERT_EXP(stream.fail() == false, CU::Concatenate("Failed to open %s, did you forget to run the tool?", dgfxFile.c_str()));
		if (stream.fail() == true)
		{
			_wassert(L"FAILED TO OPEN DGFX-FILE", L"DGFXLoader.cpp", 159);
			return nullptr;
		}

		int fileVersion = -1;
		stream.read((char*)&fileVersion, sizeof(int));
		DL_ASSERT_EXP(fileVersion == DGFX_VERSION, "Found a old DGFX-Animation, try running the Converter again");
		if (fileVersion != DGFX_VERSION)
		{
			_wassert(L"FOUND OLD ANIMATION_DGFX", L"DGFXLoader.cpp", 173);
			return nullptr;
		}

		int isNullObject = -1;
		stream.read((char*)&isNullObject, sizeof(int));

		int isLodGroup = -1;
		stream.read((char*)&isLodGroup, sizeof(int));
		DL_ASSERT_EXP(isLodGroup == 0, "AnimatedModel cant be LOD'ed, not supported yet");

		int isAnimated = -1;
		stream.read((char*)&isAnimated, sizeof(int));

		if (isNullObject == 0)
		{
			_wassert(L"ANIMATION NEEDS TO BE A NULLOBJECT, BUT IT WASNT", L"DGFXLoader.cpp", 189);
		}

		Animation* animation = LoadAnimation(nullptr, stream);

		stream.close();

		int elapsed = static_cast<int>(
			CU::TimerManager::GetInstance()->StopTimer("LoadAnimationDGFX").GetMilliseconds());
		RESOURCE_LOG("DGFX-Animation \"%s\" took %d ms to load", dgfxFile.c_str(), elapsed);

		myAnimations[aFilePath] = animation;
		return animation;
	}

	Model* DGFXLoader::CreateModel(Effect* aEffect, std::fstream& aStream)
	{
		Model* tempModel = new Model();
		tempModel->SetEffect(aEffect);

		int fileVersion = -1;
		aStream.read((char*)&fileVersion, sizeof(int));
		DL_ASSERT_EXP(fileVersion == DGFX_VERSION, "Found a old DGFX-Model, try running the Converter again");
		if (aStream.fail() == true)
		{
			assert(false && "Found a old DGFX-Model, RELEASE-ASSERT");
			return nullptr;
		}

		int isNullObject = -1;
		aStream.read((char*)&isNullObject, sizeof(int));

		int isLodGroup = -1;
		aStream.read((char*)&isLodGroup, sizeof(int));

		int isAnimated = -1;
		aStream.read((char*)&isAnimated, sizeof(int));

		if (isNullObject == 0)
		{
			tempModel->myIsNULLObject = false;

			LoadModelData(tempModel, aEffect, aStream);
			CU::Matrix44<float> matrix;
			aStream.read((char*)&tempModel->myOrientation.myMatrix[0], sizeof(float) * 16);
		}

		if (isLodGroup == 1)
		{
			LoadLodGroup(tempModel, aStream);
		}

		int childCount = 0;
		aStream.read((char*)&childCount, sizeof(int));
		for (int i = 0; i < childCount; ++i)
		{
			tempModel->AddChild(CreateModel(aEffect, aStream));
		}

		return tempModel;
	}

	ModelAnimated* DGFXLoader::CreateModelAnimated(Effect* aEffect, std::fstream& aStream)
	{
		ModelAnimated* tempModel = new ModelAnimated();
		tempModel->SetEffect(aEffect);

		int fileVersion = -1;
		aStream.read((char*)&fileVersion, sizeof(int));
		DL_ASSERT_EXP(fileVersion == DGFX_VERSION, "Found a old Animated DGFX-Model, try running the Converter again");
		if (aStream.fail() == true)
		{
			assert(false && "Found a old Animated DGFX-Model, RELEASE-ASSERT");
			return nullptr;
		}

		int isNullObject = -1;
		aStream.read((char*)&isNullObject, sizeof(int));

		int isLodGroup = 0;
		aStream.read((char*)&isLodGroup, sizeof(int));
		DL_ASSERT_EXP(isLodGroup == 0, "AnimatedModel cant be LOD'ed, not supported yet");

		int isAnimated = 0;
		aStream.read((char*)&isAnimated, sizeof(int));

		if (isNullObject == 0)
		{
			tempModel->myIsNULLObject = false;
			LoadModelAnimatedData(tempModel, aEffect, aStream);
			CU::Matrix44<float> matrix;
			aStream.read((char*)&tempModel->myOrientation.myMatrix[0], sizeof(float) * 16);
		}

		if (isAnimated == 1)
		{
			LoadAnimation(tempModel, aStream);
		}

		int childCount = 0;
		aStream.read((char*)&childCount, sizeof(int));
		for (int i = 0; i < childCount; ++i)
		{
			tempModel->AddChild(CreateModelAnimated(aEffect, aStream));
		}

		return tempModel;
	}

	void DGFXLoader::LoadModelData(Model* aOutData, Effect* aEffect, std::fstream& aStream)
	{
		int indexCount = 0;
		aStream.read((char*)&indexCount, sizeof(int)); //Index count

		unsigned int* indexData = new unsigned int[indexCount];
		aStream.read((char*)(indexData), sizeof(int) * indexCount); //All index data

		VertexIndexWrapper* indexWrapper = new VertexIndexWrapper();
		indexWrapper->myFormat = DXGI_FORMAT_R32_UINT;
		indexWrapper->myIndexData = (char*)indexData;
		indexWrapper->mySize = indexCount * sizeof(unsigned int);
		indexWrapper->myNumberOfIndices = indexCount;
		aOutData->myIndexBaseData = indexWrapper;


		int vertexCount = 0;
		aStream.read((char*)&vertexCount, sizeof(int)); //Vertex count
		int stride = 0;
		aStream.read((char*)&stride, sizeof(int)); //Vertex stride

		int sizeOfBuffer = vertexCount * stride * sizeof(float);
		char* vertexRawData = new char[sizeOfBuffer];
		aStream.read(vertexRawData, sizeOfBuffer); //All vertex data

		VertexDataWrapper* vertexData = new VertexDataWrapper();
		vertexData->myVertexData = vertexRawData;
		vertexData->myNumberOfVertices = vertexCount;
		vertexData->mySize = sizeOfBuffer;
		vertexData->myStride = stride*sizeof(float);
		aOutData->myVertexBaseData = vertexData;


		int layoutCount = 0;
		aStream.read((char*)&layoutCount, sizeof(int)); //Inputlayout element count

		for (int i = 0; i < layoutCount; ++i)
		{
			int byteOffset = 0;
			aStream.read((char*)&byteOffset, sizeof(int)); //Inputlayout element count

			int semanticIndex = 0;
			aStream.read((char*)&semanticIndex, sizeof(int)); //Inputlayout semantic index


			D3D11_INPUT_ELEMENT_DESC* desc = new D3D11_INPUT_ELEMENT_DESC();
			desc->SemanticIndex = semanticIndex;
			desc->AlignedByteOffset = byteOffset;
			desc->InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			desc->InputSlot = 0;
			desc->InstanceDataStepRate = 0;

			int type = -1;
			aStream.read((char*)&type, sizeof(int)); //element type

			if (type == eVertexLayout::VERTEX_POS)
			{
				desc->SemanticName = "POSITION";
				desc->Format = DXGI_FORMAT_R32G32B32_FLOAT;
			}
			else if (type == eVertexLayout::VERTEX_NORMAL)
			{
				desc->SemanticName = "NORMAL";
				desc->Format = DXGI_FORMAT_R32G32B32_FLOAT;
			}
			else if (type == eVertexLayout::VERTEX_UV)
			{
				desc->SemanticName = "TEXCOORD";
				desc->Format = DXGI_FORMAT_R32G32_FLOAT;
			}
			else if (type == eVertexLayout::VERTEX_BINORMAL)
			{
				desc->SemanticName = "BINORMAL";
				desc->Format = DXGI_FORMAT_R32G32B32_FLOAT;
			}
			else if (type == eVertexLayout::VERTEX_TANGENT)
			{
				desc->SemanticName = "TANGENT";
				desc->Format = DXGI_FORMAT_R32G32B32_FLOAT;
			}
			else if (type == eVertexLayout::VERTEX_SKINWEIGHTS)
			{
				desc->SemanticName = "WEIGHTS";
				desc->Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			}
			else if (type == eVertexLayout::VERTEX_BONEID)
			{
				desc->SemanticName = "BONES";
				desc->Format = DXGI_FORMAT_R32G32B32A32_SINT;
			}
			else if (type == eVertexLayout::VERTEX_COLOR)
			{
				desc->SemanticName = "COLOR";
				desc->Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			}
			else
			{
				DL_ASSERT("Found a invalid InputElement while loading DGFX");
				assert(false && "Found a invalid InputElement while loading DGFX, RELEASE-ASSERT");
			}

			aOutData->myVertexFormat.Add(desc);
		}

		Surface surface;
		surface.SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		surface.SetIndexCount(indexCount);
		surface.SetVertexStart(0);
		surface.SetIndexStart(0);
		surface.SetVertexCount(vertexCount);
		surface.SetEffect(aEffect);

		int textureCount;
		aStream.read((char*)&textureCount, sizeof(int)); //numberOfTextures
		for (int i = 0; i < textureCount; ++i)
		{
			int textureType;
			aStream.read((char*)&textureType, sizeof(int)); //textureType

			std::string resourceName;
			if (textureType == eTextureType::ALBEDO)
			{
				resourceName = "AlbedoTexture";
			}
			else if (textureType == eTextureType::NORMAL)
			{
				resourceName = "NormalTexture";
			}
			else if (textureType == eTextureType::ROUGHNESS)
			{
				resourceName = "RoughnessTexture";
			}
			else if (textureType == eTextureType::METALNESS)
			{
				resourceName = "MetalnessTexture";
			}
			else if (textureType == eTextureType::AMBIENT)
			{
				resourceName = "AOTexture";
			}
			else if (textureType == eTextureType::EMISSIVE)
			{
				resourceName = "EmissiveTexture";
			}
			else
			{
				DL_ASSERT("Found a invalid TextureType while loading DGFX");
				assert(false && "Found a invalid TextureType while loading DGFX, RELEASE-ASSERT");
			}


			int textureLenght = 0;
			aStream.read((char*)&textureLenght, sizeof(int)); //currentTexture.myFileName lenght
			char* texture = new char[textureLenght+1];
			aStream.read(texture, sizeof(char) * textureLenght); //currentTexture.myFileName
			texture[textureLenght] = '\0';

			surface.SetTexture(resourceName, texture, false);
			delete texture;
		}

		aOutData->mySurfaces.Add(new Surface(surface));
	}

	void DGFXLoader::LoadModelAnimatedData(ModelAnimated* aOutData, Effect* aEffect, std::fstream& aStream)
	{
		int indexCount = 0;
		aStream.read((char*)&indexCount, sizeof(int)); //Index count

		unsigned int* indexData = new unsigned int[indexCount];
		aStream.read((char*)(indexData), sizeof(int) * indexCount); //All index data

		VertexIndexWrapper* indexWrapper = new VertexIndexWrapper();
		indexWrapper->myFormat = DXGI_FORMAT_R32_UINT;
		indexWrapper->myIndexData = (char*)indexData;
		indexWrapper->mySize = indexCount * sizeof(unsigned int);
		indexWrapper->myNumberOfIndices = indexCount;
		aOutData->myIndexBaseData = indexWrapper;


		int vertexCount = 0;
		aStream.read((char*)&vertexCount, sizeof(int)); //Vertex count
		int stride = 0;
		aStream.read((char*)&stride, sizeof(int)); //Vertex stride

		int sizeOfBuffer = vertexCount * stride * sizeof(float);
		char* vertexRawData = new char[sizeOfBuffer];
		aStream.read(vertexRawData, sizeOfBuffer); //All vertex data

		VertexDataWrapper* vertexData = new VertexDataWrapper();
		vertexData->myVertexData = vertexRawData;
		vertexData->myNumberOfVertices = vertexCount;
		vertexData->mySize = sizeOfBuffer;
		vertexData->myStride = stride*sizeof(float);
		aOutData->myVertexBaseData = vertexData;


		int layoutCount = 0;
		aStream.read((char*)&layoutCount, sizeof(int)); //Inputlayout element count

		for (int i = 0; i < layoutCount; ++i)
		{
			int byteOffset = -1;
			aStream.read((char*)&byteOffset, sizeof(int)); //Inputlayout element count

			int semanticIndex = -1;
			aStream.read((char*)&semanticIndex, sizeof(int)); //Inputlayout semantic index

			D3D11_INPUT_ELEMENT_DESC* desc = new D3D11_INPUT_ELEMENT_DESC();
			desc->SemanticIndex = semanticIndex;
			desc->AlignedByteOffset = byteOffset;
			desc->InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			desc->InputSlot = 0;
			desc->InstanceDataStepRate = 0;

			int type = -1;
			aStream.read((char*)&type, sizeof(int)); //semanticName lenght

			if (type == eVertexLayout::VERTEX_POS)
			{
				desc->SemanticName = "POSITION";
				desc->Format = DXGI_FORMAT_R32G32B32_FLOAT;
			}
			else if (type == eVertexLayout::VERTEX_NORMAL)
			{
				desc->SemanticName = "NORMAL";
				desc->Format = DXGI_FORMAT_R32G32B32_FLOAT;
			}
			else if (type == eVertexLayout::VERTEX_UV)
			{
				desc->SemanticName = "TEXCOORD";
				desc->Format = DXGI_FORMAT_R32G32_FLOAT;
			}
			else if (type == eVertexLayout::VERTEX_BINORMAL)
			{
				desc->SemanticName = "BINORMAL";
				desc->Format = DXGI_FORMAT_R32G32B32_FLOAT;
			}
			else if (type == eVertexLayout::VERTEX_TANGENT)
			{
				desc->SemanticName = "TANGENT";
				desc->Format = DXGI_FORMAT_R32G32B32_FLOAT;
			}
			else if (type == eVertexLayout::VERTEX_SKINWEIGHTS)
			{
				desc->SemanticName = "WEIGHTS";
				desc->Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			}
			else if (type == eVertexLayout::VERTEX_BONEID)
			{
				desc->SemanticName = "BONES";
				desc->Format = DXGI_FORMAT_R32G32B32A32_SINT;
			}
			else
			{
				DL_ASSERT("Found a invalid InputElement while loading DGFX");
				assert(false && "Found a invalid InputElement while loading DGFX, RELEASE-ASSERT");
			}

			aOutData->myVertexFormat.Add(desc);
		}

		Surface surface;
		surface.SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		surface.SetIndexCount(indexCount);
		surface.SetVertexStart(0);
		surface.SetIndexStart(0);
		surface.SetVertexCount(vertexCount);
		surface.SetEffect(aEffect);

		int textureCount = 0;
		aStream.read((char*)&textureCount, sizeof(int)); //numberOfTextures
		for (int i = 0; i < textureCount; ++i)
		{
			int textureType = -1;
			aStream.read((char*)&textureType, sizeof(int)); //textureType

			std::string resourceName;
			if (textureType == eTextureType::ALBEDO)
			{
				resourceName = "AlbedoTexture";
			}
			else if (textureType == eTextureType::NORMAL)
			{
				resourceName = "NormalTexture";
			}
			else if (textureType == eTextureType::ROUGHNESS)
			{
				resourceName = "RoughnessTexture";
			}
			else if (textureType == eTextureType::METALNESS)
			{
				resourceName = "MetalnessTexture";
			}
			else if (textureType == eTextureType::AMBIENT)
			{
				resourceName = "AOTexture";
			}
			else if (textureType == eTextureType::EMISSIVE)
			{
				resourceName = "EmissiveTexture";
			}
			else
			{
				DL_ASSERT("Found a invalid TextureType while loading DGFX");
				assert(false && "Found a invalid TextureType while loading DGFX, RELEASE-ASSERT");
			}


			int textureLenght = 0;
			aStream.read((char*)&textureLenght, sizeof(int)); //currentTexture.myFileName lenght
			char* texture = new char[textureLenght+1];
			aStream.read(texture, sizeof(char) * textureLenght); //currentTexture.myFileName
			texture[textureLenght] = '\0';

			surface.SetTexture(resourceName, texture, false);
			delete texture;
		}

		aOutData->mySurfaces.Add(new Surface(surface));
	}

	void DGFXLoader::LoadLodGroup(Model* aOutData, std::fstream& aStream)
	{
		Prism::LodGroup* lodGroup = new Prism::LodGroup();

		int lodCount = 0;
		aStream.read((char*)&lodCount, sizeof(int));

		lodGroup->myLods.Reserve(lodCount);
		aStream.read((char*)&lodGroup->myLods[0], sizeof(Prism::Lod) * lodCount);


		int threshHoldCount = 0;
		aStream.read((char*)&threshHoldCount, sizeof(int));

		lodGroup->myThreshHolds.Reserve(threshHoldCount);
		aStream.read((char*)&lodGroup->myThreshHolds[0], sizeof(double) * threshHoldCount);

		aOutData->SetLodGroup(lodGroup);
	}

	Animation* DGFXLoader::LoadAnimation(ModelAnimated* aOutData, std::fstream& aStream)
	{
		CU::Matrix44<float> bindMatrix;
		aStream.read((char*)&bindMatrix.myMatrix[0], sizeof(float) * 16);

		HierarchyBone rootBone;
		LoadBoneHierarchy(rootBone, aStream);

		Animation* newAnimation = new Animation();


		int nrOfbones = 0;
		aStream.read((char*)&nrOfbones, sizeof(int));

		for (int i = 0; i < nrOfbones; ++i)
		{
			int boneNameLenght = 0;
			aStream.read((char*)&boneNameLenght, sizeof(int));

			char* boneName = new char[boneNameLenght+1];
			aStream.read(boneName, sizeof(char) * boneNameLenght);
			boneName[boneNameLenght] = '\0';

			CU::Matrix44<float> boneMatrix;
			aStream.read((char*)&boneMatrix.myMatrix[0], sizeof(float) * 16);

			CU::Matrix44<float> boneBindMatrix;
			aStream.read((char*)&boneBindMatrix.myMatrix[0], sizeof(float) * 16);


			int nrOfFrames = 0;
			aStream.read((char*)&nrOfFrames, sizeof(int));
			if (nrOfFrames > 0)
			{
				AnimationNode* newNode = new AnimationNode(nrOfFrames);
				newNode->myValues.Reserve(nrOfFrames);

				aStream.read((char*)&newNode->myValues[0], sizeof(AnimationNodeValue) * nrOfFrames);
				newNode->CalculateEndTime();

				newNode->myBoneName = boneName;
				newAnimation->AddAnimation(newNode);
			}

			newAnimation->SetBoneMatrix(boneMatrix, i);
			newAnimation->SetBoneBindPose(boneBindMatrix, i);
			newAnimation->AddBoneName(boneName);

			newAnimation->SetBindMatrix(bindMatrix);
			delete[] boneName;
		}

		newAnimation->SetHierarchy(rootBone);

		float animationLenght = 0.f;
		aStream.read((char*)&animationLenght, sizeof(float));

		newAnimation->SetAnimationLenght(animationLenght);

		if (aOutData != nullptr)
		{
			aOutData->myAnimation = newAnimation;
		}

		return newAnimation;
	}

	void DGFXLoader::LoadBoneHierarchy(HierarchyBone& aOutBone, std::fstream& aStream)
	{
		aStream.read((char*)&aOutBone.myBoneID, sizeof(int));

		int boneNameLenght = 0;
		aStream.read((char*)&boneNameLenght, sizeof(int));

		char* boneName = new char[boneNameLenght+1];
		aStream.read(boneName, sizeof(char) * boneNameLenght);
		boneName[boneNameLenght] = '\0';
		aOutBone.myBoneName = boneName;
		
		int nrOfChildren = 0;
		aStream.read((char*)&nrOfChildren, sizeof(int));

		if (nrOfChildren > 0)
		{
			aOutBone.myChildren.Init(nrOfChildren);
			for (int i = 0; i < nrOfChildren; ++i)
			{
				HierarchyBone child;
				LoadBoneHierarchy(child, aStream);
				aOutBone.myChildren.Add(child);
			}
		}

		delete[] boneName;
	}

	bool DGFXLoader::CheckIfFbxIsNewer(const std::string& aDGFXPath)
	{
		std::string fbxPath = CU::GetRealDataFolderFilePath(aDGFXPath, "fbx");

		HANDLE dgfxHandle;
		HANDLE fbxHandle;

		dgfxHandle = CreateFile(aDGFXPath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL
			, OPEN_EXISTING, 0, NULL);

		fbxHandle = CreateFile(fbxPath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL
			, OPEN_EXISTING, 0, NULL);

		FILETIME dgfxTime;
		FILETIME fbxTime;

		GetFileTime(dgfxHandle, NULL, NULL, &dgfxTime);
		GetFileTime(fbxHandle, NULL, NULL, &fbxTime);

		if (CompareFileTime(&dgfxTime, &fbxTime) == -1)
		{
			return true;
		}

		return false;
	}
}