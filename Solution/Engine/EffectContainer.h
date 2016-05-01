#pragma once
#include <string>
#include <unordered_map>

namespace Prism
{
	class Effect;
	class EffectContainer
	{
	public:
		static EffectContainer* GetInstance();
		static void Destroy();

		Effect* GetEffect(const std::string& aFilePath);

		void Update(const float aDeltaTime);
		void SetCubeMap(const std::string& aFilePath);	

	private:
		EffectContainer(){};
		~EffectContainer();

		void LoadEffect(const std::string& aFilePath);
		void ReloadEffect(const std::string& aFilePath);
		void VerifyShader(const std::string& aFilePath);

		std::string GetCSOPath(const std::string& aFXPath);

		std::unordered_map<std::string, Effect*> myEffects;
		std::string myCubeMap;

		static EffectContainer* myInstance;
	};
}