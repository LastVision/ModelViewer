#include <string>
#include <Camera.h>
#include <DirectionalLight.h>
#include <DebugDrawer.h>
#include <DL_Debug.h>
#include "DLLApp.h"
#include "DLLCamera.h"
#include "DLLModel.h"
#include <EffectContainer.h>
#include <Engine.h>
#include <EngineEnums.h>
#include <FileWatcher.h>
#include <Instance.h>
#include <InputWrapper.h>
#include <Matrix.h>
#include <Scene.h>
#include <SetupInfo.h>
#include <TimerManager.h>
#include <CommonHelper.h>
#include <TextureContainer.h>
#include <EffectContainer.h>

#define EngineInstance Prism::Engine::GetInstance()
#define InputInstance CU::InputWrapper::GetInstance()

DLLApp::DLLApp(int* aHwnd, Prism::SetupInfo& aWindowSetup, WNDPROC aWindowProc, CU::FileWatcher& aFileWatcher)
	: myFileWatcher(aFileWatcher)
{
	DL_Debug::Debug::Create();;
	CU::TimerManager::Create();
	DL_DEBUG("DLL APP 0");
	myPanelWindowHandler = (HWND)aHwnd;
	Prism::EffectContainer::GetInstance()->SetFileWatcher(&myFileWatcher);
	Prism::TextureContainer::GetInstance()->SetFileWatcher(&myFileWatcher);
	Prism::Engine::Create(myEngineWindowHandler, aWindowProc, aWindowSetup);
	DL_DEBUG("DLL APP 1");

	SetupLight();
	SetupInput();
	SetParentWindow(aWindowSetup);
	SetupCubeMap();
	DL_DEBUG("DLL APP 2");

	CU::Vector2<float> windowSize(aWindowSetup.myScreenWidth, aWindowSetup.myScreenHeight);
	myCamera = new DLLCamera(windowSize, 1.0f, 1.0f, 1.0f);
	myModel = new DLLModel();
	DL_DEBUG("DLL APP 3");

	DL_DEBUG("DLL APP 4");

}

DLLApp::~DLLApp()
{
	delete myCamera;
	myCamera = nullptr;

	delete myModel;
	myModel = nullptr;
}

void DLLApp::Render()
{
	
	Prism::Engine::GetInstance()->Render();

	myDirectionalLight->Update();
	myDirectionalLightData[0].myDirection = myDirectionalLight->GetCurrentDir();
	myDirectionalLightData[0].myColor = myDirectionalLight->GetColor();

	if (myModel->GetInstance() != nullptr)
	{
		myModel->GetInstance()->UpdateDirectionalLights(myDirectionalLightData);
		myModel->GetInstance()->Render(*myCamera->GetCamera());
	}

	Prism::DebugDrawer::GetInstance()->Render(*myCamera->GetCamera());
}

void DLLApp::Update()
{
	CU::TimerManager::GetInstance()->Update();
	float deltaTime = CU::TimerManager::GetInstance()->GetMasterTimer().GetTime().GetFrameTime();
	CU::InputWrapper::GetInstance()->Update();
	LogicUpdate(deltaTime);
	myCamera->GetCamera()->Update(deltaTime);
}

void DLLApp::LoadModel(const std::string& aModelFile, const std::string& aShaderFile)
{
	std::string shaderFile = CU::GetRealDataFolderFilePath(aShaderFile, "fx");

	myModel->LoadModel(aModelFile.c_str(), shaderFile.c_str());
	myModelFile = aModelFile;
	myShaderFile = shaderFile;

	myFileWatcher.WatchFileChange(myModelFile, std::bind(&DLLApp::LoadModel, this, aModelFile, aShaderFile));
}

void DLLApp::SetClearColor(CU::Vector4f& aClearColor)
{
	Prism::Engine::GetInstance()->SetClearColor({ aClearColor.x, aClearColor.y, aClearColor.z, aClearColor.w });
}

void DLLApp::SetCubeMap(const char* aCubeMapFile)
{
	Prism::EffectContainer::GetInstance()->SetCubeMap(aCubeMapFile);
	LoadModel(myModelFile.c_str(), myShaderFile.c_str());
}

void DLLApp::LogicUpdate(float aDeltaTime)
{
	if (GetActiveWindow()) {
		if (InputInstance->KeyIsPressed(DIK_LALT) && InputInstance->MouseIsPressed(0))
		{
			myCamera->Zoom(aDeltaTime, myMouseSensitivty);
		}
		if (InputInstance->KeyIsPressed(DIK_LALT) && InputInstance->MouseIsPressed(2))
		{
			myCamera->Pan(aDeltaTime, myMouseSensitivty);
		}
		if (InputInstance->KeyIsPressed(DIK_LALT) && InputInstance->MouseIsPressed(1))
		{
			myCamera->Rotate(aDeltaTime, myMouseSensitivty);
		}
		if (InputInstance->KeyDown(DIK_SPACE))
		{
			myCamera->ResetCamera();
		}
	}
	myModel->Update(aDeltaTime);
}

void DLLApp::SetupCubeMap()
{
	Prism::EffectContainer::GetInstance()->SetCubeMap("Data/Resource/Texture/CubeMap/T_cubemap_level_01.dds");
	EngineInstance->SetClearColor(CU::Vector4f(0.3f, 0.3f, 0.3f, 1.f));
}

void DLLApp::SetupInput()
{
	CU::InputWrapper::GetInstance()->Create(myPanelWindowHandler, GetModuleHandle(NULL),
		DISCL_NONEXCLUSIVE | DISCL_FOREGROUND, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
}

void DLLApp::SetParentWindow(Prism::SetupInfo &aWindowSetup)
{
	SetParent(myEngineWindowHandler, myPanelWindowHandler);
	SetWindowLong(myEngineWindowHandler, GWL_STYLE, WS_POPUP);
	SetWindowPos(myEngineWindowHandler, HWND_TOP, 0, 0,
		aWindowSetup.myScreenWidth, aWindowSetup.myScreenHeight, SWP_SHOWWINDOW);
}

void DLLApp::SetupLight()
{
	myDirectionalLight = new Prism::DirectionalLight();
	myDirectionalLight->SetDir(CU::Vector3f( 0.f, 0.f, -1.f ));
	myDirectionalLight->SetColor(CU::Vector4f(1.f, 1.f, 1.f, 1.f));

	memset(&myDirectionalLightData[0], 0, sizeof(Prism::DirectionalLightData) * NUMBER_OF_DIRECTIONAL_LIGHTS);
}

void DLLApp::SetDirectionalLightRotation(CU::Vector3f aRotation)
{
	myDirectionalLightRotation = aRotation;
	myDirectionalLight->SetDir(myDirectionalLightRotation);
}
