#include "Game.h"
#include "Graphics.h"
#include "Vertex.h"
#include "Input.h"
#include "PathHelpers.h"
#include "Window.h"
#include <vector>
#include "Transform.h"
#include "Camera.h"
#include "SimpleShader.h"
#include "Material.h"
#include "Sky.h"

#include "WICTextureLoader.h"
#include <DirectXMath.h>

// Needed for a helper function to load pre-compiled shader files
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>


// For the DirectX Math library
using namespace DirectX;

// This code assumes files are in "ImGui" subfolder!
// Adjust as necessary for your own folder structure and project setup
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

// Local variables
bool showDemo = 0;	// Whether or not to show demo window
const char* starterNames[] = { "Bulbasaur", "Charmander", "Squirtle", "Pikachu" }; // Popup list options
int selectedStarter = -1; // Index of chosen name
float windowColor[4] = {0.4f, 0.6f, 0.75f, 0.0f}; // Color Vector
bool stopConfirmation = 0;
std::vector<float> shadowResolution = { 8192.0f, 8192.0f };

// Cameras
std::shared_ptr<Camera> activeCamera;
int cameraChoice = 0;
const char* cameraNames[] = { "Default", "Side", "Top" };
std::vector<std::shared_ptr<Camera>> cameras = { std::make_shared<Camera>(Camera({0.0f, 0.0f, -2.0f}, Window::AspectRatio(), 45.0f)),
											 std::make_shared<Camera>(Camera({-1.0f, 0.0f, -1.0f}, Window::AspectRatio(), 60.0f)),
											 std::make_shared<Camera>(Camera({3.0f, 2.0f, -2.0f}, Window::AspectRatio(), 90.0f)) };

// Textures
Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> iceSRV;
Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronzeSRV;
Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> normalSRV;
Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> roughSRV;
Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> metalSRV;
Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;
D3D11_SAMPLER_DESC samplerDesc;
std::vector<std::shared_ptr<Material>> materials;
std::vector<std::shared_ptr<Mesh>> meshes;
std::shared_ptr<Sky> skybox;
XMFLOAT3 ambientColor = { 0.5f, 0.5f, 0.5f };
std::shared_ptr<SimpleVertexShader> shadowVS;

// --------------------------------------------------------
// Called once per program, after the window and graphics API
// are initialized but before the game loop begins
// --------------------------------------------------------
void Game::Initialize()
{
	// Initialize ImGui itself & platform/renderer backends
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplWin32_Init(Window::Handle());
	ImGui_ImplDX11_Init(Graphics::Device.Get(), Graphics::Context.Get());

	// Load Textures and Sampler State
	CreateWICTextureFromFile(Graphics::Device.Get(),
		Graphics::Context.Get(),
		L"Assets/Textures/ice_color.jpg",
		nullptr,
		&iceSRV);

	// Bronze
	CreateWICTextureFromFile(Graphics::Device.Get(),
		Graphics::Context.Get(),
		L"Assets/Textures/bronze_albedo.png",
		nullptr,
		&bronzeSRV);
	CreateWICTextureFromFile(Graphics::Device.Get(),
		Graphics::Context.Get(),
		L"Assets/Textures/bronze_normals.png",
		nullptr,
		&normalSRV);
	CreateWICTextureFromFile(Graphics::Device.Get(),
		Graphics::Context.Get(),
		L"Assets/Textures/bronze_roughness.png",
		nullptr,
		&roughSRV);
	CreateWICTextureFromFile(Graphics::Device.Get(),
		Graphics::Context.Get(),
		L"Assets/Textures/bronze_metal.png",
		nullptr,
		&metalSRV);

	// Create Shadow Map Texture and Bind it to the Pipeline
	shadowVS = std::make_shared<SimpleVertexShader>(Graphics::Device, Graphics::Context, FixPath(L"ShadowMapVertexShader.cso").c_str());
	Game::CreateShadowMap();

	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	Graphics::Device.Get()->CreateSamplerState(&samplerDesc, &samplerState);

	// Dark Color Style
	ImGui::StyleColorsDark();

	// Load Meshes
	meshes.push_back(std::make_shared<Mesh>(Mesh("Cube", FixPath("../../Assets/Models/cube.obj").c_str())));
	meshes.push_back(std::make_shared<Mesh>(Mesh("Cylinder", FixPath("../../Assets/Models/cylinder.obj").c_str())));
	meshes.push_back(std::make_shared<Mesh>(Mesh("Helix", FixPath("../../Assets/Models/helix.obj").c_str())));
	meshes.push_back(std::make_shared<Mesh>(Mesh("Quad", FixPath("../../Assets/Models/quad.obj").c_str())));
	meshes.push_back(std::make_shared<Mesh>(Mesh("Double-Sided Quad", FixPath("../../Assets/Models/quad_double_sided.obj").c_str())));
	meshes.push_back(std::make_shared<Mesh>(Mesh("Sphere", FixPath("../../Assets/Models/sphere.obj").c_str())));
	meshes.push_back(std::make_shared<Mesh>(Mesh("Torus", FixPath("../../Assets/Models/torus.obj").c_str())));

	// Initialize Active Camera
	activeCamera = cameras.at(0);

	//Create lights
	lightProjectionSize = 0.0f;
	CreateLights();

	// Create Skybox
	skybox = std::make_shared<Sky>(Sky(std::make_shared<SimpleVertexShader>(Graphics::Device, Graphics::Context, FixPath(L"SkyVertexShader.cso").c_str()),
		std::make_shared<SimplePixelShader>(Graphics::Device, Graphics::Context, FixPath(L"SkyPixelShader.cso").c_str()),
		meshes[0], samplerState, 
		FixPath(L"../../Assets/Skyboxes/right.png").c_str(),
		FixPath(L"../../Assets/Skyboxes/left.png").c_str(),
		FixPath(L"../../Assets/Skyboxes/up.png").c_str(),
		FixPath(L"../../Assets/Skyboxes/down.png").c_str(),
		FixPath(L"../../Assets/Skyboxes/front.png").c_str(),
		FixPath(L"../../Assets/Skyboxes/back.png").c_str()));

	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	CreateGeometry();

	// Set initial graphics API state
	//  - These settings persist until we change them
	//  - Some of these, like the primitive topology & input layout, probably won't change
	//  - Others, like setting shaders, will need to be moved elsewhere later
	{
		// Tell the input assembler (IA) stage of the pipeline what kind of
		// geometric primitives (points, lines or triangles) we want to draw.  
		// Essentially: "What kind of shape should the GPU draw with our vertices?"
		Graphics::Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}
}


// --------------------------------------------------------
// Clean up memory or objects created by this class
// 
// Note: Using smart pointers means there probably won't
//       be much to manually clean up here!
// --------------------------------------------------------
Game::~Game()
{
	// ImGui clean up
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

// --------------------------------------------------------
// Creates the geometry we're going to draw
// --------------------------------------------------------
void Game::CreateGeometry()
{
	// Create some temporary variables to represent colors
	// - Not necessary, just makes things more readable
	XMFLOAT4 red = XMFLOAT4(0.5f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 green = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 blue = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	XMFLOAT4 yellow = XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 purple = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 white = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 black = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	// Set the active vertex and pixel shaders via Materials
	//  - Once you start applying different shaders to different objects,
	//    these calls will need to happen multiple times per frame
	materials.push_back(std::make_shared<Material>(Material(white,
		std::make_shared<SimpleVertexShader>(Graphics::Device, Graphics::Context, FixPath(L"VertexShader.cso").c_str()),
		std::make_shared<SimplePixelShader>(Graphics::Device, Graphics::Context, FixPath(L"PixelShader.cso").c_str()),
		0.5f)));
	materials.push_back(std::make_shared<Material>(Material(yellow,
		std::make_shared<SimpleVertexShader>(Graphics::Device, Graphics::Context, FixPath(L"VertexShader.cso").c_str()),
		std::make_shared<SimplePixelShader>(Graphics::Device, Graphics::Context, FixPath(L"UVPixelShader.cso").c_str()),
		0.5f)));
	materials.push_back(std::make_shared<Material>(Material(purple,
		std::make_shared<SimpleVertexShader>(Graphics::Device, Graphics::Context, FixPath(L"VertexShader.cso").c_str()),
		std::make_shared<SimplePixelShader>(Graphics::Device, Graphics::Context, FixPath(L"NormalPixelShader.cso").c_str()),
		1.0f)));
	materials.push_back(std::make_shared<Material>(Material(yellow,
		std::make_shared<SimpleVertexShader>(Graphics::Device, Graphics::Context, FixPath(L"VertexShader.cso").c_str()),
		std::make_shared<SimplePixelShader>(Graphics::Device, Graphics::Context, FixPath(L"PixelShader.cso").c_str()),
		1.0f)));

	materials[0].get()->AddTextureSRV("Albedo", bronzeSRV);
	materials[0].get()->AddTextureSRV("NormalMap", normalSRV);
	materials[0].get()->AddTextureSRV("RoughnessMap", roughSRV);
	materials[0].get()->AddTextureSRV("MetalnessMap", metalSRV);
	materials[0].get()->AddSampler("Sampler", samplerState);

	materials[3].get()->AddTextureSRV("Ice", iceSRV);
	materials[3].get()->AddSampler("Sample", samplerState);

	// Top Row with Color Tint
	//AddObjects(materials[1], 4.5);

	// Middle Row with UV Colors
	//AddObjects(materials[2], 1.5);

	// Second Middle Row with Shaders
	AddObjects(materials[0], -1.5);

	// Floor
	models->push_back(GameEntity(meshes[4], materials[3]));
	models->at((int)models->size() - 1).GetTransform()->SetPosition(XMFLOAT3(0.0, -3.0f, 0.0));
	models->at((int)models->size() - 1).GetTransform()->SetScale(XMFLOAT3(15.0f, 1.0f, 15.0f));


	// Bottom Row with Fancy Shader
	//AddObjects(materials[3], -4.5);
}


// --------------------------------------------------------
// Handle resizing to match the new window size
//  - Eventually, we'll want to update our 3D camera
// --------------------------------------------------------
void Game::OnResize()
{
	for (int i = 0; i < cameras.size(); i++) {
		if (cameras[i] != nullptr_t()) {
			cameras[i]->UpdateProjectionMatrix(Window::AspectRatio());
		}
	}
}


// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	ImGuiRefresh(deltaTime);

	// Update Camera
	activeCamera->Update(deltaTime);

	// Example input checking: Quit if the escape key is pressed
	if (Input::KeyDown(VK_ESCAPE))
		Window::Quit();
}


// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Frame START
	// - These things should happen ONCE PER FRAME
	// - At the beginning of Game::Draw() before drawing *anything*
	{
		// Clear the back buffer (erase what's on screen) and depth buffer
		Graphics::Context->ClearRenderTargetView(Graphics::BackBufferRTV.Get(),	windowColor);
		Graphics::Context->ClearDepthStencilView(Graphics::DepthBufferDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
		Graphics::Context->ClearDepthStencilView(shadowDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

	// Draw Shadows
	// Set output merger
	ID3D11RenderTargetView* nullRTV = {};
	Graphics::Context->OMSetRenderTargets(1, &nullRTV, shadowDSV.Get());

	// Set new viewport
	D3D11_VIEWPORT viewport = {};
	viewport.Width = (float)shadowResolution.at(0);
	viewport.Height = (float)shadowResolution.at(1);
	viewport.MaxDepth = 1.0f;
	Graphics::Context->RSSetViewports(1, &viewport);

	CreateLightViewMatrix(lightsData[0]);
	CreateLightProjectionMatrix(lightsData[0]);

	// Deactivate Pixel Shader
	Graphics::Context->PSSetShader(0, 0, 0);

	Graphics::Context->RSSetState(shadowRasterizer.Get());
	shadowVS->SetShader();
	shadowVS->SetMatrix4x4("view", lightViewMatrix);
	shadowVS->SetMatrix4x4("projection", lightProjectionMatrix);


	// Loop and draw all entities for the shadow map
	for (int i = 0; i < models->size() - 1; i++)
	{
		shadowVS->SetMatrix4x4("world", models->at(i).GetTransform()->GetWorldMatrix());
		shadowVS->CopyAllBufferData();
		// Draw the mesh directly to avoid the entity's material
		// Note: Your code may differ significantly here!
		//models->at(i).GetMaterial()->GetPS()->SetShaderResourceView("ShadowMap", shadowSRV.Get());
		models->at(i).GetMesh()->Draw();
	}

	// Reset Pipeline
	viewport.Width = (float)Window::Width();
	viewport.Height = (float)Window::Height();
	Graphics::Context->RSSetViewports(1, &viewport);
	Graphics::Context->OMSetRenderTargets(
		1,
		Graphics::BackBufferRTV.GetAddressOf(),
		Graphics::DepthBufferDSV.Get());
	Graphics::Context->RSSetState(0);

	// Draw Meshes
	for (int i = 0; i < models->size(); i++) {
		models->at(i).GetMaterial()->GetPS()->SetFloat3("ambient", ambientColor);
		models->at(i).GetMaterial()->GetPS()->SetData("lights", &lightsData[0], sizeof(Light) * (int)lightsData.size());
		models->at(i).GetMaterial()->GetVS()->SetMatrix4x4("lightView", lightViewMatrix);
		models->at(i).GetMaterial()->GetVS()->SetMatrix4x4("lightProjection", lightProjectionMatrix); 
		models->at(i).GetMaterial()->GetPS()->SetShaderResourceView("ShadowMap", shadowSRV);
		models->at(i).GetMaterial()->GetPS()->SetSamplerState("ShadowSampler", shadowSampler);
		models->at(i).Draw(activeCamera);
	}

	skybox->Draw(*activeCamera);

	// Draw ImGui
	{
		ImGui::Render(); // Turns this frame’s UI into renderable triangles
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData()); // Draws it to the screen
	}

	// Frame END
	// - These should happen exactly ONCE PER FRAME
	// - At the very end of the frame (after drawing *everything*)
	{
		ID3D11ShaderResourceView* nullSRVs[128] = {};
		Graphics::Context->PSSetShaderResources(0, 128, nullSRVs);
		
		// Present at the end of the frame
		bool vsync = Graphics::VsyncState();
		Graphics::SwapChain->Present(
			vsync ? 1 : 0,
			vsync ? 0 : DXGI_PRESENT_ALLOW_TEARING);

		// Re-bind back buffer and depth buffer after presenting
		Graphics::Context->OMSetRenderTargets(
			1,
			Graphics::BackBufferRTV.GetAddressOf(),
			Graphics::DepthBufferDSV.Get());
	}


}	

// --------------------------------------------------------------------
// Ensures ImGui has fresh data and resets its data from the last frame
// Parameters:
// float deltaTime: time since last frame
// --------------------------------------------------------------------
void Game::ImGuiRefresh(float deltaTime) {
	// Feed fresh data to ImGui
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = deltaTime;
	io.DisplaySize.x = (float)Window::Width();
	io.DisplaySize.y = (float)Window::Height();

	// Reset the frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// Determine new input capture
	Input::SetKeyboardCapture(io.WantCaptureKeyboard);
	Input::SetMouseCapture(io.WantCaptureMouse);

	Game::BuildUI();
}	

/// -----------------------------
/// Builds UI with Current Data
/// -----------------------------
void Game::BuildUI() {

	//Create new Window
	ImGui::Begin("DirectX 11 Inspector");

	// Window Details Menu
	if (ImGui::CollapsingHeader("Window Details", 1)) {
		// Replace the %f with the next parameter, and format as a float
		ImGui::Text("Framerate: %f fps", ImGui::GetIO().Framerate);

		// Replace each %d with the next parameter, and format as decimal integers
		// The "x" will be printed as-is between the numbers, like so: 800x600
		ImGui::Text("Window Resolution: %dx%d", Window::Width(), Window::Height());
	}

	// Camera
	if (ImGui::CollapsingHeader("Camera", 1)) {
		// Brings up list of Gen 1 starters
		if (ImGui::Button("Choose Camera")) {
			ImGui::OpenPopup("Cameras");
		}

		ImGui::SameLine();
		ImGui::TextUnformatted(cameraNames[cameraChoice]); // Camera selection display

		// Create popup menu
		if (ImGui::BeginPopup("Cameras")) {
			ImGui::SeparatorText("Cameras");
			for (int i = 0; i < cameras.size(); i++) {
				if (ImGui::Selectable(cameraNames[i])) {
					activeCamera = cameras.at(i);
					cameraChoice = i;
				}
			}

			ImGui::EndPopup();
		}

		// Camera Attributes
		// Position
		ImGui::Text("Position");
		ImGui::Text("x: %f  y: %f  z: %f", activeCamera->GetPosition().x, 
										   activeCamera->GetPosition().y, 
										   activeCamera->GetPosition().z);
		
		// Rotation
		ImGui::NewLine();
		ImGui::Text("Rotation");
		ImGui::Text("Pitch: %f  Yaw: %f  Roll: %f", activeCamera->GetRotation().x,
													activeCamera->GetRotation().y,
													activeCamera->GetRotation().z);

		// FOV
		ImGui::NewLine();
		ImGui::Text("Field of View: %i degrees", (int)activeCamera->GetFOV());
	}

	// Color Picker Menu
	if (ImGui::CollapsingHeader("Color Picker", 1)) {
		ImGui::ColorEdit4("Window Color Editor", (float*)&windowColor);	//Color Picker
	}

	// Popup Menu
	if (ImGui::CollapsingHeader("Popup Menu", 1)) {

		// Brings up list of Gen 1 starters
		if (ImGui::Button("Choose Starter...")) {
			ImGui::OpenPopup("starters");
		}

		ImGui::SameLine();
		ImGui::TextUnformatted(selectedStarter == -1 ? "None" : starterNames[selectedStarter]); // Starter selection display

		// Create popup menu
		if (ImGui::BeginPopup("starters")) {
			ImGui::SeparatorText("Starters");
			for (int i = 0; i < IM_ARRAYSIZE(starterNames); i++) {
				if (ImGui::Selectable(starterNames[i])) {
					selectedStarter = i;
				}
			}

			ImGui::EndPopup();
		}
	}

	// Mesh Data
	if (ImGui::CollapsingHeader("Models", 1)) {
		std::vector<ImGuiID> meshIds;

		for (int i = 0; i < models->size(); i++) {
			std::shared_ptr<Mesh> object = models->at(i).GetMesh();

			// Ensure that ID is not the same between objects
			ImGui::PushID(i);
			if (ImGui::TreeNode(object->GetName())) {
				ImGui::Text("Triangles: %i", object->GetIndexCount() / 3);
				ImGui::Text("Vertices: %i", object->GetVertexCount());
				ImGui::Text("Indices: %i", object->GetIndexCount());
				ImGui::TreePop();
				ImGui::NewLine();	// Separation buffer
			}
			ImGui::PopID();
		}
	}

	// GameEntity Data
	if (ImGui::CollapsingHeader("Objects", 1)) {
		std::vector<ImGuiID> meshIds;

		for (int i = 0; i < models->size(); i++) {
			std::shared_ptr<Mesh> object = models->at(i).GetMesh();
			std::shared_ptr<Transform> transform = models->at(i).GetTransform();
			std::shared_ptr<Material> material = models.get()->at(i).GetMaterial();

			// Get current buffer data of mesh
			XMFLOAT3 position = transform->GetPosition();
			XMFLOAT3 rotation = transform->GetRotation();
			XMFLOAT3 scale = transform->GetScale();

			XMFLOAT4 colorTint = material->GetTint();
			XMFLOAT2 uvScale = { material->GetUVScale().at(0), material->GetUVScale().at(1) };
			XMFLOAT2 offset = { material->GetUVOffset().at(0), material->GetUVOffset().at(1) };

			// Ensure that ID is not the same between objects
			ImGui::PushID(i);
			if (ImGui::TreeNode(object->GetName())) {
				ImGui::Text("Object Data");
				ImGui::SliderFloat3("Position", (float*)&position, -10.0f, 10.0f);
				ImGui::SliderFloat3("Rotation (Radians)", (float*)&rotation, -10.0f, 10.0f);
				ImGui::SliderFloat3("Scale", (float*)&scale, 0.0f, 3.0f);
				ImGui::Text("Mesh Index Count: %i", object->GetIndexCount());

				// Material Data
				ImGui::NewLine();
				ImGui::Text("Material Data");
				ImGui::ColorEdit4("Window Color Editor", (float*) &colorTint);	//Color Picker
				ImGui::SliderFloat2("UV Scale", (float*) &uvScale, 1.0f, 10.0f);
				ImGui::SliderFloat2("UV Offset", (float*) &offset, 1.0f, 10.0f);
				ImGui::NewLine();
				ImGui::Text("Texures");

				// Show Textures
				for (auto& t : material.get()->GetTextureSRVs()) {
					ImGui::Text(t.first.c_str());
					ImGui::Image((ImTextureID)t.second.Get(), ImVec2(100, 100));
				}

				ImGui::TreePop();
			}


			// Set new data
			transform->SetPosition(position);
			transform->SetRotation(rotation);
			transform->SetScale(scale);

			material->SetTint(colorTint);
			material->SetUVScale({ uvScale.x, uvScale.y });
			material->SetUVOffset({ offset.x, offset.y });

			ImGui::PopID();
		}
	}
	
	// Lights Data
	if (ImGui::CollapsingHeader("Lights", 1)) {

		for (int i = 0; i < lightsData.size(); i++) {
			Light currentLight = lightsData[i];
			ImGui::PushID(i);
			if (ImGui::TreeNode(GetLightType(currentLight.Type))){
				XMFLOAT3 direction = currentLight.Direction;
				float range = currentLight.Range;
				XMFLOAT3 position = currentLight.Position;
				float intensity = currentLight.Intensity;
				XMFLOAT3 color = currentLight.Color;
				float spotInnerAngle = XMConvertToDegrees(currentLight.SpotInnerAngle);
				float spotOuterAngle = XMConvertToDegrees(currentLight.SpotOuterAngle);

				// New options depending on Light type
				// Intensity and color is shared by all lights
				ImGui::SliderFloat("Intensity", &intensity, 0.0f, 10.0f);
				ImGui::ColorEdit3("Color", (float*)&color);

				switch (currentLight.Type)
				{
				case LIGHT_TYPE_DIRECTION:

					ImGui::SliderFloat3("Direction", (float*) & direction, -10.0f, 10.0f);
					break;

				case LIGHT_TYPE_POINT:
					ImGui::SliderFloat("Range", (float*) &range, 0.0f, 50.0f);
					ImGui::SliderFloat3("Position", (float*) &position, -20.0f, 20.0f);
					break;

				case LIGHT_TYPE_SPOT:
					ImGui::SliderFloat3("Direction", (float*) &direction, -10.0f, 10.0f);
					ImGui::SliderFloat("Range", (float*) &range, 0.0f, 50.0f);
					ImGui::SliderFloat3("Position", (float*) &position, -10.0f, 10.0f);
					ImGui::SliderFloat("Inner Angle (Degrees)", (float*) &spotInnerAngle, 0.0f, 90.0f);
					ImGui::SliderFloat("Outer Angle (Degrees)", (float*) &spotOuterAngle, 0.0f, 90.0f);
					break;
				}

				// Set new data
				lightsData[i].Direction = direction;
				lightsData[i].Range = range;
				lightsData[i].Position = position;
				lightsData[i].Intensity = intensity;
				lightsData[i].Color = color;
				lightsData[i].SpotInnerAngle = XMConvertToRadians(spotInnerAngle);
				lightsData[i].SpotOuterAngle = XMConvertToRadians(spotOuterAngle);

				ImGui::TreePop();
			}

			ImGui::PopID();
		}
	}

	if (ImGui::CollapsingHeader("Shadow Map", 1)) {
		ImGui::Image((ImTextureID)shadowSRV.Get(), ImVec2(512, 512));
	}

	ImGui::NewLine();	// Separation buffer

	// Changes whether or not demo window will be shown with a popup
	if(ImGui::Button("Toggle ImGui Demo Window")) {
		if (stopConfirmation == false) {
			ImGui::OpenPopup("Confirmation");
		}
		
		else {
			showDemo = !showDemo;
		}
	}

	// Center Popup
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	// Display confirmation popup if confirmation window is allowed
	if (ImGui::BeginPopupModal("Confirmation", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("Are you sure?");
		ImGui::Separator();

		ImGui::Checkbox("Don't ask again", &stopConfirmation); // Confirmation checkbox to show again

		// Toggle ImGui demo window
		if (ImGui::Button("Toggle Window")) {
			showDemo = !showDemo;
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();

		if (ImGui::Button("Don't Toggle")) {
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	// Shows or hides demo window
	if (showDemo) {
		ImGui::ShowDemoWindow();
	}

		// Reset Stop Confirmation to false
	if (ImGui::Button("Reset Confirmation")) {
		stopConfirmation = false;
	}

	//End current window
	ImGui::End();
}

// Create Row of Objects
void Game::AddObjects(std::shared_ptr<Material> material, float offset) {
	int modelsLength = (int)models->size();
	
	// Set Meshes, Materials
	for (int i = 0; i < 7; i++) {
		models->push_back(GameEntity(meshes.at(i), material));
	}

	// Model Transforms
	models->at(modelsLength).GetTransform()->SetPosition(XMFLOAT3(-2.5, offset, 0.0));
	models->at(modelsLength + 1).GetTransform()->SetPosition(XMFLOAT3(2.5, offset, 0.0));
	models->at(modelsLength + 2).GetTransform()->SetPosition(XMFLOAT3(0.0, offset - 1.0, 0.0));
	models->at(modelsLength + 3).GetTransform()->SetPosition(XMFLOAT3(7.5, offset, 0.0));
	models->at(modelsLength + 4).GetTransform()->SetPosition(XMFLOAT3(-7.5, offset, 0.0));
	models->at(modelsLength + 5).GetTransform()->SetPosition(XMFLOAT3(5.0, offset - 1.3, 0.0));
	models->at(modelsLength + 6).GetTransform()->SetPosition(XMFLOAT3(-5.0, offset, 0.0));
}

const char* Game::GetLightType(int Type) {
	const char* LightTypeNames[] = { "Directional", "Point", "Spot" };
	return LightTypeNames[Type];
}

void Game::CreateLights() {
	Light light1 = {};
	light1.Type = LIGHT_TYPE_DIRECTION;
	light1.Direction = XMFLOAT3(0.0f, -0.25f, 1.0f);
	light1.Color = XMFLOAT3(1.0f, 1.0f, 1.0f);
	light1.Intensity = 5.0f;

	Light light2 = {};
	light2.Type = LIGHT_TYPE_DIRECTION;
	light2.Direction = XMFLOAT3(-1.0f, 0.0f, 0.0f);
	light2.Color = XMFLOAT3(0.4f, 0.9f, 0.7f);
	light2.Intensity = 1.0f;

	Light light3 = {};
	light3.Type = LIGHT_TYPE_POINT;
	light3.Range = 100.0f;
	light3.Position = XMFLOAT3(3.0f, 3.0f, 0.0f);
	light3.Color = XMFLOAT3(0.0f, 1.0f, 0.0f);
	light3.Intensity = 1.0f;

	Light light4 = {};
	light4.Type = LIGHT_TYPE_DIRECTION;
	light4.Direction = XMFLOAT3(0.0f, 0.0f, -1.0f);
	light4.Intensity = 1.0f;
	light4.Color = XMFLOAT3(0.5f, 0.5f, 0.5f);

	Light light5 = {};
	light5.Type = LIGHT_TYPE_SPOT;
	light5.Direction = { 0.0f, -1.0f, 0.0f };
	light5.Range = 4.0f;
	light5.Position = { 7.5f, 1.0f, 0.0f };
	light5.Intensity = 1.0f;
	light5.Color = XMFLOAT3(1.0f, 1.0f, 1.0f);
	light5.SpotInnerAngle = XMConvertToRadians(10.0f);
	light5.SpotOuterAngle = XMConvertToRadians(30.0f);


	lightsData.push_back(light1);
	lightsData.push_back(light2);
	lightsData.push_back(light3);
	lightsData.push_back(light4);
	lightsData.push_back(light5);
}

void Game::CreateShadowMap() {
	// Create the actual texture that will be the shadow map
	D3D11_TEXTURE2D_DESC shadowDesc = {};
	shadowDesc.Width = (int)shadowResolution.at(0); // Ideally a power of 2 (like 1024)
	shadowDesc.Height = (int)shadowResolution.at(1); // Ideally a power of 2 (like 1024)
	shadowDesc.ArraySize = 1;
	shadowDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	shadowDesc.CPUAccessFlags = 0;
	shadowDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	shadowDesc.MipLevels = 1;
	shadowDesc.MiscFlags = 0;
	shadowDesc.SampleDesc.Count = 1;
	shadowDesc.SampleDesc.Quality = 0;
	shadowDesc.Usage = D3D11_USAGE_DEFAULT;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> shadowTexture;
	Graphics::Device->CreateTexture2D(&shadowDesc, 0, shadowTexture.GetAddressOf());

	// Create the depth/stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC shadowDSDesc = {};
	shadowDSDesc.Format = DXGI_FORMAT_D32_FLOAT;
	shadowDSDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	shadowDSDesc.Texture2D.MipSlice = 0;
	Graphics::Device->CreateDepthStencilView(
		shadowTexture.Get(),
		&shadowDSDesc,
		shadowDSV.GetAddressOf());

	// Create the SRV for the shadow map
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	Graphics::Device->CreateShaderResourceView(
		shadowTexture.Get(),
		&srvDesc,
		shadowSRV.GetAddressOf());

	// Rasterizer
	D3D11_RASTERIZER_DESC shadowRastDesc = {};
	shadowRastDesc.FillMode = D3D11_FILL_SOLID;
	shadowRastDesc.CullMode = D3D11_CULL_BACK;
	shadowRastDesc.DepthClipEnable = true;
	shadowRastDesc.DepthBias = 1000; // Min. precision units, not world units!
	shadowRastDesc.SlopeScaledDepthBias = 1.0f; // Bias more based on slope
	Graphics::Device->CreateRasterizerState(&shadowRastDesc, &shadowRasterizer);

	// Sampler
	D3D11_SAMPLER_DESC shadowSampDesc = {};
	shadowSampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	shadowSampDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
	shadowSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.BorderColor[0] = 1.0f; // Only need the first component
	Graphics::Device->CreateSamplerState(&shadowSampDesc, &shadowSampler);
}

void Game::CreateLightViewMatrix(Light light) {
	XMMATRIX lightView = XMMatrixLookToLH(
			-XMVECTOR({ light.Direction.x, light.Direction.y, light.Direction.z }) * 20, // Position: "Backing up" 20 units from origin
			XMVECTOR({ light.Direction.x, light.Direction.y, light.Direction.z }),		 // Direction: light's direction
			XMVectorSet(0, 1, 0, 0)); // Up: World up vector (Y axis)
	XMStoreFloat4x4(&lightViewMatrix, lightView);
}

void Game::CreateLightProjectionMatrix(Light light) {
	lightProjectionSize = 15.0f; // Tweak for your scene!
	XMMATRIX lightProjection;

	lightProjection = XMMatrixOrthographicLH(
		lightProjectionSize,
		lightProjectionSize,
		1.0f,
		100.0f);
	XMStoreFloat4x4(&lightProjectionMatrix, lightProjection);
}