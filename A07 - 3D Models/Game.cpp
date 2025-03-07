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

// Namespace to store variables
namespace {
	bool showDemo = 0;	// Whether or not to show demo window
	const char* starterNames[] = { "Bulbasaur", "Charmander", "Squirtle", "Pikachu" }; // Popup list options
	int selectedStarter = -1; // Index of chosen name
	ImVec4 windowColor(1.0f, 0.0f, 0.0f, 1.0f); // Color Vector
	bool stopConfirmation = 0;

	// Cameras
	std::shared_ptr<Camera> activeCamera;
	int cameraChoice = 0;
	const char* cameraNames[] = { "Default", "Side", "Top" };
	std::vector<std::shared_ptr<Camera>> cameras = { std::make_shared<Camera>(Camera({0.0f, 0.0f, -2.0f}, Window::AspectRatio(), 45.0f)), 
													 std::make_shared<Camera>(Camera({-1.0f, 0.0f, -1.0f}, Window::AspectRatio(), 60.0f)), 
													 std::make_shared<Camera>(Camera({3.0f, 2.0f, -2.0f}, Window::AspectRatio(), 90.0f)) };
}

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

	// Dark Color Style
	ImGui::StyleColorsDark();

	// Initialize Active Camera
	activeCamera = cameras.at(0);
	
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

		// Ensure the pipeline knows how to interpret all the numbers stored in
		// the vertex buffer. For this course, all of your vertices will probably
		// have the same layout, so we can just set this once at startup.
		// Graphics::Context->IASetInputLayout(inputLayout.Get());

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
	XMFLOAT4 red = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 green = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 blue = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	XMFLOAT4 yellow = XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 purple = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 white = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 black = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	// Set the active vertex and pixel shaders via Materials
		//  - Once you start applying different shaders to different objects,
		//    these calls will need to happen multiple times per frame
	Material redMaterial = Material(red,
		std::make_shared<SimpleVertexShader>(Graphics::Device, Graphics::Context, FixPath(L"VertexShader.cso").c_str()),
		std::make_shared<SimplePixelShader>(Graphics::Device, Graphics::Context, FixPath(L"PixelShader.cso").c_str()));

	Material greenMaterial = Material(green,
		std::make_shared<SimpleVertexShader>(Graphics::Device, Graphics::Context, FixPath(L"VertexShader.cso").c_str()),
		std::make_shared<SimplePixelShader>(Graphics::Device, Graphics::Context, FixPath(L"PixelShader.cso").c_str()));

	Material blueMaterial = Material(blue,
		std::make_shared<SimpleVertexShader>(Graphics::Device, Graphics::Context, FixPath(L"VertexShader.cso").c_str()),
		std::make_shared<SimplePixelShader>(Graphics::Device, Graphics::Context, FixPath(L"PixelShader.cso").c_str()));

	// Set up the vertices of the triangle we would like to draw
	// - We're going to copy this array, exactly as it exists in CPU memory
	//    over to a Direct3D-controlled data structure on the GPU (the vertex buffer)
	// - Note: Since we don't have a camera or really any concept of
	//    a "3d world" yet, we're simply describing positions within the
	//    bounds of how the rasterizer sees our screen: [-1 to +1] on X and Y
	// - This means (0,0) is at the very center of the screen.
	// - These are known as "Normalized Device Coordinates" or "Homogeneous 
	//    Screen Coords", which are ways to describe a position without
	//    knowing the exact size (in pixels) of the image/window/etc.  
	// - Long story short: Resizing the window also resizes the triangle,
	//    since we're describing the triangle in terms of the window itself
	Vertex verticesTri[] =
	{
		{ XMFLOAT3(+0.0f, +0.1f, +0.0f), red },
		{ XMFLOAT3(+0.1f, -0.1f, +0.0f), green },
		{ XMFLOAT3(-0.1f, -0.1f, +0.0f), yellow },
	};

	Vertex verticesBox[] =
	{
		{ XMFLOAT3(-0.2f, +0.2f, +0.0f), black },
		{ XMFLOAT3(-0.2f, -0.2f, +0.0f), blue },
		{ XMFLOAT3(+0.2f, +0.2f, +0.0f), white },
		{ XMFLOAT3(+0.2f, -0.2f, +0.0f), red },
	};

	Vertex verticesCrown[] =
	{
		{ XMFLOAT3(+0.0f, +0.5f, +0.0f), red },
		{ XMFLOAT3(+0.5f, -0.5f, +0.0f), blue },
		{ XMFLOAT3(-0.5f, -0.5f, +0.0f), green },
		{ XMFLOAT3(+0.5f, +0.5f, +0.0f), yellow },
		{ XMFLOAT3(-0.5f, +0.5f, +0.0f), purple },
	};

	// Set up indices, which tell us which vertices to use and in which order
	// - This is redundant for just 3 vertices, but will be more useful later
	// - Indices are technically not required if the vertices are in the buffer 
	//    in the correct order and each one will be used exactly once
	// - But just to see how it's done...
	unsigned int indicesTri[] = { 0, 1, 2 };
	unsigned int indicesBox[] = { 0, 2, 1, 3, 1, 2};
	unsigned int indicesCrown[] = { 0, 1, 2, 3, 1, 2, 4, 1, 2};

	triangle = std::make_shared<Mesh>("Triangle", (unsigned int)std::size(verticesTri), (unsigned int)std::size(indicesTri),
		verticesTri, indicesTri);
	box = std::make_shared<Mesh>("Box", (unsigned int)std::size(verticesBox), (unsigned int)std::size(indicesBox),
		verticesBox, indicesBox);
	crown = std::make_shared<Mesh>("Crown", (unsigned int)std::size(verticesCrown), (unsigned int)std::size(indicesCrown),
		verticesCrown, indicesCrown);

	meshes.push_back(GameEntity(*triangle, redMaterial));
	meshes.push_back(GameEntity(*triangle, greenMaterial));
	meshes.push_back(GameEntity(*box, blueMaterial));
	meshes.push_back(GameEntity(*box,blueMaterial));
	meshes.push_back(GameEntity(*crown, redMaterial));

	// Move some meshes
	meshes[0].GetTransform()->SetPosition(XMFLOAT3(-0.3f, 0.5f, 0.0f));
	meshes[1].GetTransform()->SetPosition(XMFLOAT3(-0.8f, -0.8f, 0.0f));
	meshes[2].GetTransform()->SetPosition(XMFLOAT3(0.3f, 0.5f, 0.0f));
	meshes[3].GetTransform()->SetPosition(XMFLOAT3(0.6f, -0.5f, 0.0f));
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

	// Update transformations of objects
	{	
		meshes[0].GetTransform()->Rotate(0.0f, 0.0f, 6.0f * deltaTime);
		meshes[1].GetTransform()->MoveAbsolute(0.0f, 0.4f * deltaTime, 0.0f);
		meshes[2].GetTransform()->MoveAbsolute(-0.1f * deltaTime, 0.0f, 0.0f);
		meshes[3].GetTransform()->Rotate(0.0f, 0.0f, -2.0f * deltaTime);
		meshes[4].GetTransform()->Scale(1.0001f, 1.0001f, 0.0f);
	}

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
		const float color[4] = { 0.4f, 0.6f, 0.75f, 0.0f };
		Graphics::Context->ClearRenderTargetView(Graphics::BackBufferRTV.Get(),	color);
		Graphics::Context->ClearDepthStencilView(Graphics::DepthBufferDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

	// Draw Meshes
	{
		for (int i = 0; i < meshes.size(); i++) {
			meshes[i].Draw(activeCamera);
		}
	}

	// Draw ImGui
	{
		ImGui::Render(); // Turns this frame’s UI into renderable triangles
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData()); // Draws it to the screen
	}

	// Frame END
	// - These should happen exactly ONCE PER FRAME
	// - At the very end of the frame (after drawing *everything*)
	{
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

		// Button to set background color
		if (ImGui::Button("Set Background Color")) {
			ImGui::GetStyle().Colors[ImGuiCol_WindowBg] = windowColor;
		}
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
	if (ImGui::CollapsingHeader("Meshes", 1)) {
		std::vector<ImGuiID> meshIds;

		for (int i = 0; i < std::size(meshes); i++) {
			std::shared_ptr<Mesh> object = meshes.at(i).GetMesh();

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
	if (ImGui::CollapsingHeader("GameEntities", 1)) {
		std::vector<ImGuiID> meshIds;

		for (int i = 0; i < std::size(meshes); i++) {
			std::shared_ptr<Mesh> object = meshes.at(i).GetMesh();
			std::shared_ptr<Transform> transform = meshes.at(i).GetTransform();

			// Get current buffer data of mesh
			XMFLOAT3 position = transform->GetPosition();
			XMFLOAT3 rotation = transform->GetRotation();
			XMFLOAT3 scale = transform->GetScale();

			// Ensure that ID is not the same between objects
			ImGui::PushID(i);
			if (ImGui::TreeNode(object->GetName())) {
				ImGui::SliderFloat3("Position", (float*) &position, -1.0f, 1.0f);
				ImGui::SliderFloat3("Rotation (Radians)", (float*) &rotation, -1.0f, 1.0f);
				ImGui::SliderFloat3("Scale", (float*) &scale, 0.0f, 2.0f);
				ImGui::Text("Mesh Index Count: %i", object->GetIndexCount());
				ImGui::TreePop();
			}

			// Set new data
			transform->SetPosition(position);
			transform->SetRotation(rotation);
			transform->SetScale(scale);

			ImGui::PopID();
		}
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