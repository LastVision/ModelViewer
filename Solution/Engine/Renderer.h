#pragma once

#include "EngineEnums.h"

#define MAX_NUMBER_OF_SCENES 2

struct ID3D11RenderTargetView;
struct ID3D11DepthStencilView;
namespace Prism
{
	class FullScreenHelper;
	class Texture;

	struct SceneData;

	class Renderer
	{
	public:
		Renderer();
		~Renderer();

		void BeginScene();
		void EndScene(int aEffect);

		void FinalRender();

		void OnResize(float aWidth, float aHeight);

		Texture* GetWorldTexture();
		void SetRenderTargets(ID3D11RenderTargetView* aRenderTarget, ID3D11DepthStencilView* aDepthBuffer);

	private:
		struct SceneData
		{
			Texture* myScene;
			Texture* myFinished;
		};
		
		Engine* myEngine;
		SceneData mySceneData[MAX_NUMBER_OF_SCENES];

		FullScreenHelper* myFullScreenHelper;
		float myClearColor[4];
		int mySceneIndex;

		Texture* myFinalTexture;
		Texture* myCombineMiddleMan;
	};

	inline Texture* Renderer::GetWorldTexture()
	{
		return mySceneData[1].myScene;
	}
}

