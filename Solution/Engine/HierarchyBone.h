#pragma once

namespace Prism
{
	class AnimationNode;

	class HierarchyBone
	{
		friend class Animation;
		friend class FBXFactory;
		friend class DGFXLoader;
	public:
		HierarchyBone();
		~HierarchyBone();

		void Update(const CU::Matrix44<float>& aParentMatrix, float aCurrentTime);

	private:
		CU::Matrix44<float>* myResultMatrix;
		CU::Matrix44<float>* myBoneMatrix;
		AnimationNode* myCurrentAnimation;
		CU::GrowingArray<HierarchyBone> myChildren;
		int myBoneID;
		std::string myBoneName;
	};
}