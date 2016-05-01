#pragma once
#include <vector>
#include <unordered_map>
#include <GrowingArray.h>
#include <Vector.h>
#include <Matrix.h>

struct AnimationData;
struct Bone;
struct ModelData;

class FBXLoader;
class FbxModelData;

namespace Prism
{
	class Animation;
	class Model;
	class ModelAnimated;
	class Effect;
	class HierarchyBone;
	struct LodGroup;

	class FBXFactory
	{
	public:
		FBXFactory();
		~FBXFactory();

		Model* LoadModel(const char* aFilePath, Effect* aEffect);
		ModelAnimated* LoadModelAnimated(const char* aFilePath, Effect* aEffect);
		Animation* LoadAnimation(const char* aFilePath);
		void LoadModelForRadiusCalc(const char* aFilePath, CU::GrowingArray<CU::Vector3<float>>& someVerticesOut);

		void ConvertToDGFX(const char* aInputPath, const char* aOutputPath);
		void ConvertToDGFX(const char* aInputPath, const char* aOutputPath, CU::GrowingArray<std::string>& someOutErrors);
		
	private:
		void FillData(ModelData* someData, Model* outData, Effect* aEffect);
		void FillData(ModelData* someData, ModelAnimated* outData, Effect* aEffect);
		void FillAnimationData(FbxModelData* someData, ModelAnimated* outData);
		Animation* FillBoneAnimationData(FbxModelData* someData, ModelAnimated* aOutData);
		void BuildBoneHierarchy(Bone& aBone, AnimationData* aAnimationData, HierarchyBone& aOutBone);

		Model* CreateModel(FbxModelData* someModelData, Effect* aEffect);
		ModelAnimated* CreateModelAnimated(FbxModelData* someModelData, Effect* aEffect);

		void CreateModelForRadiusCalc(FbxModelData* someModelData, CU::GrowingArray<CU::Vector3<float>>& someVerticesOut
			, const CU::Matrix44<float>& aParentOrientation = CU::Matrix44<float>());
		void FillDataForRadiusCalc(ModelData* aModelData, CU::GrowingArray<CU::Vector3<float>>& someVerticesOut
			, const CU::Matrix44<float>& aOrientation);


		void SaveModelToFile(FbxModelData* aModelData, std::fstream& aStream);
		void SaveModelDataToFile(ModelData* aData, std::fstream& aStream);
		void SaveLodGroupToFile(Prism::LodGroup* aGroup, std::fstream& aStream);
		void SaveAnimationToFile(FbxModelData* aModeldata, std::fstream& aStream);
		void SaveBoneHierarchyToFile(Bone& aBone, AnimationData* aAnimationData, std::fstream& aStream);

		FBXLoader *myLoader;

		struct FBXData
		{
			FBXData() :myData(nullptr){}
			~FBXData();
			FbxModelData* myData;
			std::string myPath;
		};
		std::vector<FBXData*> myFBXData;
		std::unordered_map<std::string, Model*> myModels;
		std::unordered_map<std::string, ModelAnimated*> myModelsAnimated;
	};
}