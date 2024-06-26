#include <Core/src/log/SeverityLevelPolicy.h>
#include <Core/src/gfx/d11/RendererD11.h>
#include <Core/src/gfx/d12/Renderer.h>
#include <Core/src/gfx/d12/IRenderer.h>
#include <Core/src/win/CopperWin.h>
#include <Core/src/win/boot.h>
#include <Core/src/log/Log.h>
#include <Core/src/utl/Assert.h>
#include <Core/src/win/IWindow.h>
#include <Core/src/ioc/Container.h>
#include <Core/src/ioc/Singletons.h>
#include <Core/src/ecs/Ecs.h>

#include <Core/thirdParty/ImGUI/ImguiIncludes.h>
#include <filesystem>
#include <DirectXMath.h>
#include <SimpleMath.h>
#include <stdexcept>
#include <format>
#include <ranges>
#include <string>
#include <vector>
#include <chrono>

#define STB_IMAGE_IMPLEMENTATION
#pragma warning (push)
#pragma warning (disable : 26451 26819 6262)
#include "../3rd/stb_image.h"
#pragma warning (pop)
#include "App.h"
#include "AppConfig.h"
#include "pcg/Tile.h"
#include "pcg/GridManager.h"
#include "EcsComponents.h"

using namespace CPR;
using namespace CPR::GFX;
using namespace CPR::GFX::D11;
using namespace std::string_literals;
using namespace std::chrono_literals;
using namespace DirectX;
namespace rn = std::ranges;
namespace vi = rn::views;

namespace CPR::APP
{
    static constexpr u32 NR_OF_TILE_TEXTURES = 9;
    static constexpr u32 INVALID_INDEX = u32(-1);
    struct Inputs
    {
        bool moveLeftPushed = false;
        bool moveRightPushed = false;
        bool moveForwardPushed = false;
        bool moveBackwardsPushed = false;
        bool moveUpPushed = false;
        bool moveDownPushed = false;

        bool turnLeftPushed = false;
        bool turnRightPushed = false;

        bool quitKey = false;
    }keyboardInputs;

    struct SimpleVertex
    {
        float position[3] = { 0.0f, 0.0f, 0.0f };
        float uv[2] = { 0.0f, 0.0f };
        float normal[3] = { 0.0f, 0.0f, 0.0f };
    };

    struct PointLight
    {
        float position[3] = { 0.0f, 0.0f, 0.0f };
        float colour[3] = { 1.0f, 1.0f, 1.0f };
    };

    struct ImguiVariables
    {
        f32 a[4] = { 0.f, 0.f, 0.f, 1.f };
        f32 b[4] = { 0.f, 0.65882f, 0.95294f, 1.f };
        f32 c[4] = { 0.0549f, 0.81961f, 0.27059f, 1.f };
    };

    GfxRenderPassD11* CreateStandardRenderPass(IRendererD11* renderer)
    {
        RenderPassInfo info;
        info.vsPath = "StandardVS.cso";
        info.psPath = "StandardPS.cso";

        PipelineBinding vertexBinding;
        vertexBinding.dataType = PipelineDataType::VERTEX;
        vertexBinding.bindingType = PipelineBindingType::SHADER_RESOURCE;
        vertexBinding.shaderStage = PipelineShaderStage::VS;
        vertexBinding.slotToBindTo = 0;
        info.objectBindings.push_back(vertexBinding);

        PipelineBinding indicesBinding;
        indicesBinding.dataType = PipelineDataType::INDEX;
        indicesBinding.bindingType = PipelineBindingType::SHADER_RESOURCE;
        indicesBinding.shaderStage = PipelineShaderStage::VS;
        indicesBinding.slotToBindTo = 1;
        info.objectBindings.push_back(indicesBinding);

        PipelineBinding transformBinding;
        transformBinding.dataType = PipelineDataType::TRANSFORM;
        transformBinding.bindingType = PipelineBindingType::CONSTANT_BUFFER;
        transformBinding.shaderStage = PipelineShaderStage::VS;
        transformBinding.slotToBindTo = 0;
        info.objectBindings.push_back(transformBinding);

        PipelineBinding vpBinding;
        vpBinding.dataType = PipelineDataType::VIEW_PROJECTION;
        vpBinding.bindingType = PipelineBindingType::CONSTANT_BUFFER;
        vpBinding.shaderStage = PipelineShaderStage::VS;
        vpBinding.slotToBindTo = 1;
        info.globalBindings.push_back(vpBinding);

        PipelineBinding diffuseTextureBinding;
        diffuseTextureBinding.dataType = PipelineDataType::DIFFUSE;
        diffuseTextureBinding.bindingType = PipelineBindingType::SHADER_RESOURCE;
        diffuseTextureBinding.shaderStage = PipelineShaderStage::PS;
        diffuseTextureBinding.slotToBindTo = 0;
        info.objectBindings.push_back(diffuseTextureBinding);

        PipelineBinding specularTextureBinding;
        specularTextureBinding.dataType = PipelineDataType::SPECULAR;
        specularTextureBinding.bindingType = PipelineBindingType::SHADER_RESOURCE;
        specularTextureBinding.shaderStage = PipelineShaderStage::PS;
        specularTextureBinding.slotToBindTo = 1;
        info.objectBindings.push_back(specularTextureBinding);

        PipelineBinding lightBufferBinding;
        lightBufferBinding.dataType = PipelineDataType::LIGHT;
        lightBufferBinding.bindingType = PipelineBindingType::SHADER_RESOURCE;
        lightBufferBinding.shaderStage = PipelineShaderStage::PS;
        lightBufferBinding.slotToBindTo = 2;
        info.globalBindings.push_back(lightBufferBinding);

        PipelineBinding cameraPosBinding;
        cameraPosBinding.dataType = PipelineDataType::CAMERA_POS;
        cameraPosBinding.bindingType = PipelineBindingType::CONSTANT_BUFFER;
        cameraPosBinding.shaderStage = PipelineShaderStage::PS;
        cameraPosBinding.slotToBindTo = 0;
        info.globalBindings.push_back(cameraPosBinding);

        PipelineBinding testBufferBinding;
        testBufferBinding.dataType = PipelineDataType::IMGUI;
        testBufferBinding.bindingType = PipelineBindingType::CONSTANT_BUFFER;
        testBufferBinding.shaderStage = PipelineShaderStage::PS;
        testBufferBinding.slotToBindTo = 1;
        info.globalBindings.push_back(testBufferBinding);


        ResourceIndex samplerIndex = renderer->CreateSampler(
            SamplerType::ANISOTROPIC, AddressMode::CLAMP);

        if (samplerIndex == ResourceIndex(-1))
            throw std::runtime_error("Could not create clamp sampler");

        PipelineBinding clampSamplerBinding;
        clampSamplerBinding.dataType = PipelineDataType::SAMPLER;
        clampSamplerBinding.bindingType = PipelineBindingType::NONE;
        clampSamplerBinding.shaderStage = PipelineShaderStage::PS;
        clampSamplerBinding.slotToBindTo = 0;
        info.globalBindings.push_back(clampSamplerBinding);

        GfxRenderPassD11* toReturn = renderer->CreateRenderPass(info);
        toReturn->SetGlobalSampler(PipelineShaderStage::PS, 0, samplerIndex);

        return toReturn;
    }

    bool CreateTileMesh(Mesh& mesh, IRendererD11* renderer)
    {
        SimpleVertex vertices[] =
        {
            {{-0.5f, 0.5f, -0.5f}, {0.f, 0.f}, {0.0f, 0.0f, -1.0f}}, // back
            {{0.5f, 0.5f, -0.5f}, {1.f, 0.f}, {0.0f, 0.0f, -1.0f}},
            {{-0.5f, -0.5f, -0.5f}, {0.f, 1.0f}, {0.0f, 0.0f, -1.0f}},
            {{0.5f, -0.5f, -0.5f}, {1.f, 1.0f}, {0.0f, 0.0f, -1.0f}},
        };

        ResourceIndex verticesIndex = renderer->SubmitBuffer(
            vertices,
            BufferInfo{
                .elementSize = sizeof(SimpleVertex),
                .nrOfElements = ARRAYSIZE(vertices),
                .rwPattern = PerFrameUsage::STATIC,
                .bindingFlags = BufferBinding::STRUCTURED_BUFFER
            }
        );

        const u32 NR_OF_INDICES = 6;
        u32 indices[NR_OF_INDICES];
        for (u32 i = 0; i < NR_OF_INDICES / 6; ++i)
        {
            u32 baseBufferIndex = i * 6;
            u32 baseVertexIndex = i * 4;
            indices[baseBufferIndex + 0] = baseVertexIndex + 0;
            indices[baseBufferIndex + 1] = baseVertexIndex + 1;
            indices[baseBufferIndex + 2] = baseVertexIndex + 2;
            indices[baseBufferIndex + 3] = baseVertexIndex + 2;
            indices[baseBufferIndex + 4] = baseVertexIndex + 1;
            indices[baseBufferIndex + 5] = baseVertexIndex + 3;
        }

        ResourceIndex indicesIndex = renderer->SubmitBuffer(
            indices,
            BufferInfo{
                .elementSize = sizeof(u32),
                .nrOfElements = NR_OF_INDICES,
                .rwPattern = PerFrameUsage::STATIC,
                .bindingFlags = BufferBinding::STRUCTURED_BUFFER,
            }
        );

        if (indicesIndex == ResourceIndex(-1) || verticesIndex == ResourceIndex(-1))
            return false;

        mesh.vertexBuffer = verticesIndex;
        mesh.indexBuffer = indicesIndex;

        return true;
    }

    bool CreateCircleMesh(Mesh& mesh, IRendererD11* renderer, f32 radius, u32 segments)
    {
        // Calculate the total number of vertices (1 for the center + one for each segment point)
        const unsigned int totalVertices = segments + 1;

        // Allocate memory for vertices
        SimpleVertex* vertices = new SimpleVertex[totalVertices];

        // Center vertex
        vertices[0] = { {0.0f, 0.0f, 0.0f}, {0.5f, 0.5f}, {0.0f, 0.0f, -1.0f} };

        // Vertex positions around the circle
        for (unsigned int i = 1; i <= segments; ++i)
        {
            f32 angle = 2 * DirectX::XM_PI * i / segments;
            f32 x = radius * cos(angle);
            f32 y = radius * sin(angle);

            vertices[i] = { {x, y, 0.0f}, {cos(angle), sin(angle)}, {0.0f, 0.0f, -1.0f} };
        }

        // Submit vertex buffer
        ResourceIndex verticesIndex = renderer->SubmitBuffer(
            vertices,
            BufferInfo{
                .elementSize = sizeof(SimpleVertex),
                .nrOfElements = totalVertices,
                .rwPattern = PerFrameUsage::STATIC,
                .bindingFlags = BufferBinding::STRUCTURED_BUFFER
            }
        );

        // Calculate the total number of indices
        const u32 totalIndices = segments * 3;

        // Allocate memory for indices
        u32* indices = new u32[totalIndices];

        // Create the indices
        for (u32 i = 0; i < segments; ++i)
        {
            indices[i * 3] = 0;  // Center vertex
            indices[i * 3 + 1] = (i + 1) % segments + 1;
            indices[i * 3 + 2] = i + 1;
        }

        // Submit index buffer
        ResourceIndex indicesIndex = renderer->SubmitBuffer(
            indices,
            BufferInfo{
                .elementSize = sizeof(u32),
                .nrOfElements = totalIndices,
                .rwPattern = PerFrameUsage::STATIC,
                .bindingFlags = BufferBinding::STRUCTURED_BUFFER
            }
        );

        // Clean up allocated memory
        delete[] vertices;
        delete[] indices;

        // Check for errors
        if (indicesIndex == ResourceIndex(-1) || verticesIndex == ResourceIndex(-1))
            return false;

        // Set up the mesh
        mesh.vertexBuffer = verticesIndex;
        mesh.indexBuffer = indicesIndex;

        return true;
    }

    bool LoadTexture(ResourceIndex& toSet,
        IRendererD11* renderer, std::string filePath, unsigned int components)
    {
        int width, height;
        unsigned char* imageData = stbi_load(filePath.c_str(),
            &width, &height, nullptr, components);

        TextureInfo textureInfo;
        textureInfo.baseTextureWidth = width;
        textureInfo.baseTextureHeight = height;
        textureInfo.format.componentCount = components == 4 ?
            TexelComponentCount::QUAD : TexelComponentCount::SINGLE;
        textureInfo.format.componentSize = TexelComponentSize::BYTE;
        textureInfo.format.componentType = TexelComponentType::UNORM;
        textureInfo.mipLevels = 1;
        textureInfo.bindingFlags = TextureBinding::SHADER_RESOURCE;

        toSet = renderer->SubmitTexture(imageData, textureInfo);

        return toSet != ResourceIndex(-1);
    }

    XMFLOAT4X4 CreateTransformBuffer(ResourceIndex& toSet,
        IRendererD11* renderer, f32 xPos, f32 yPos, f32 zPos, f32 rad)
    {
        XMMATRIX rotationMatrix = XMMatrixRotationZ(rad);
        XMMATRIX translationMatrix = XMMatrixTranslation(xPos, yPos, zPos);
        XMMATRIX transposedMatrix = XMMatrixTranspose(rotationMatrix * translationMatrix);
        XMFLOAT4X4 matrix, toUpload;
        XMStoreFloat4x4(&matrix, translationMatrix);
        XMStoreFloat4x4(&toUpload, transposedMatrix);
        
        toSet = renderer->SubmitBuffer(&toUpload,
            BufferInfo{
                .elementSize = sizeof(XMFLOAT4X4),
                .nrOfElements = 1,
                .rwPattern = PerFrameUsage::DYNAMIC,
                .bindingFlags = BufferBinding::CONSTANT_BUFFER
            }
        );
        
        cpr_assert(toSet != INVALID_INDEX);
        
        return matrix;
    }

    void TransformCamera(CameraD11* camera, float moveSpeed,
        float turnSpeed, float deltaTime)
    {
        if (keyboardInputs.moveRightPushed)
            camera->MoveX(moveSpeed * deltaTime);
        else if (keyboardInputs.moveLeftPushed)
            camera->MoveX(-moveSpeed * deltaTime);

        if (keyboardInputs.moveForwardPushed)
            camera->MoveZ(moveSpeed * deltaTime);
        else if (keyboardInputs.moveBackwardsPushed)
            camera->MoveZ(-moveSpeed * deltaTime);

        if (keyboardInputs.moveUpPushed)
            camera->MoveY(moveSpeed * deltaTime);
        else if (keyboardInputs.moveDownPushed)
            camera->MoveY(-moveSpeed * deltaTime);

        if (keyboardInputs.turnLeftPushed)
            camera->RotateY(-turnSpeed * deltaTime);
        else if (keyboardInputs.turnRightPushed)
            camera->RotateY(turnSpeed * deltaTime);
    }

    bool LoadSurfacePropertyFiles(SurfaceProperty& surfaceProperties,
        IRendererD11* renderer, const std::string& prefix, u32 tileNr)
    {
        auto diffusePath = prefix +"t"+ std::to_string(tileNr) + ".jpg";
        ResourceIndex diffuseTextureIndex;
        if (!LoadTexture(diffuseTextureIndex, renderer, diffusePath, 4))
            return false;

        ResourceIndex specularTextureIndex;
        if (!LoadTexture(specularTextureIndex, renderer,  + "../../WindowApp/Assets/Textures/StoneSpecular.png", 4))
            return false;

        surfaceProperties.diffuseTexture = diffuseTextureIndex;
        surfaceProperties.specularTexture = specularTextureIndex;

        return true;
    }

    bool CreateLights(ResourceIndex& toSet, IRendererD11* renderer, float offset)
    {
        float height = offset / 2.0f;
        PointLight lights[4] =
        {
            {{ -offset, height, 0.0f }, { 1.0f, 0.0f, 0.0f }},
            {{ offset, height, 0.0f }, { 0.0f, 0.0f, 1.0f }},
            {{ 0.0f, height, -offset }, { 1.0f, 1.0f, 1.0f }},
            {{ 0.0f, height, offset }, { 0.0f, 1.0f, 0.0f }}
        };

        toSet = renderer->SubmitBuffer(lights,
            BufferInfo{
                .elementSize = sizeof(PointLight),
                .nrOfElements = ARRAYSIZE(lights),
                .rwPattern = PerFrameUsage::STATIC,
                .bindingFlags = BufferBinding::STRUCTURED_BUFFER,
            }
        );

        return toSet != ResourceIndex(-1);
    }

    bool CreateTest(ResourceIndex& toSet, IRendererD11* renderer, float offset)
    {
        ImguiVariables data[1] = {};

        toSet = renderer->SubmitBuffer(data,
            BufferInfo{
                .elementSize = sizeof(ImguiVariables),
                .nrOfElements = ARRAYSIZE(data),
                .rwPattern = PerFrameUsage::DYNAMIC,
                .bindingFlags = BufferBinding::CONSTANT_BUFFER,
            }
        );

        return toSet != ResourceIndex(-1);
    }

    u32 PlaceGrid(std::vector<RenderObject>& toStoreIn, std::vector<Tile>& tiles, GridManager& gm, IRendererD11* renderer )
    {
        Mesh tileMesh;
        if (!CreateTileMesh(tileMesh, renderer))
            return false;

        const std::string path = "../../WindowApp/Assets/Textures/";

        const auto nrOfTileTextures = gm.GetTileCount();
        std::vector<SurfaceProperty> surfaceProperties(nrOfTileTextures);
        for (u32 i = 0; i < nrOfTileTextures; i++)
        {
            if (!LoadSurfacePropertyFiles(surfaceProperties[i], renderer, path, i))
                return false;
        }

        auto& ths = gm.GetTileHandles(true);
        u32 tileIndex = static_cast<u32>(toStoreIn.size());
        for (u32 x = 0; x < GRID_DIM; x++)
        {
            for (u32 y = 0; y < GRID_DIM; y++)
            {
                auto& th = ths[y * GRID_DIM + x];
                u32 tileNr = th.id == u32(-1) ? 1 : th.id;

                auto& ecs = ECS::Get();
                auto entity = ecs.createEntity();
                ecs.addComponent<CoordinateComponent>(entity, CoordinateComponent{ (int)x, (int)y });
                DirectX::SimpleMath::Vector3 pos = { static_cast<f32>(x + .5f), static_cast<f32>(y + .5f), 0.0f };
                DirectX::SimpleMath::Vector3 rot = { 0.0f, 0.0f, static_cast<f32>(-th.rotation * XM_PIDIV2) };
                DirectX::SimpleMath::Vector3 scale = { 1.0f, 1.0f, 1.0f };
                ecs.addComponent<TransformComponent>(entity, TransformComponent{pos, rot, scale});
                
                ResourceIndex transformBufferIdx;
                auto result = CreateTransformBuffer(transformBufferIdx, renderer,
                    static_cast<f32>(x+.5f),
                    static_cast<f32>(y+.5f), 0.f, static_cast<f32>(-th.rotation * XM_PIDIV2));

                ecs.addComponent<TransformBufferComponent>(entity, TransformBufferComponent{ transformBufferIdx });
                ecs.addComponent<MeshComponent>(entity, MeshComponent{ tileMesh.vertexBuffer, tileMesh.indexBuffer });
                ecs.addComponent<SurfacePropertyComponent>(entity,
                    SurfacePropertyComponent{ .diffuse = surfaceProperties[tileNr].diffuseTexture,
                                              .specular = surfaceProperties[tileNr].specularTexture,
                                              .sampler = surfaceProperties[tileNr].sampler
                    }
                );


                RenderObject toStore;
                toStore.transformBuffer = transformBufferIdx;
                toStore.surfaceProperty = surfaceProperties[tileNr];
                toStore.mesh = tileMesh;
                toStoreIn.push_back(toStore);

                Tile t;
                t.renderObjectIndex = tileIndex++;
                t.transformation = result;
                t.tileTexture = tileNr;
                t.x = x;
                t.y = y;
                tiles.push_back(t);
            }
        }
        
        return tileIndex;
    }

    u32 PlacePlayer(std::vector<RenderObject>& toStoreIn, IRendererD11* renderer)
    {
        Mesh playerMesh;
        /*if (!CreateTileMesh(playerMesh, renderer))
            return false;*/
        if (!CreateCircleMesh(playerMesh, renderer, 1.f, 10))
            return false;

        const std::string path = "../../WindowApp/Assets/Textures/";

        std::vector<SurfaceProperty> surfaceProperties(5);
        for (u32 i = 0; i < 5; i++)
        {
            if (!LoadSurfacePropertyFiles(surfaceProperties[i], renderer, path, i))
                return false;
        }

        u32 playerIndex = static_cast<u32>(toStoreIn.size());
        ResourceIndex transformBuffer;
        auto result = CreateTransformBuffer(transformBuffer, renderer,
            static_cast<f32>(.5f), static_cast<f32>(.5f), 0.f,
            0.f);

        RenderObject toStore;
        toStore.transformBuffer = transformBuffer;
        toStore.surfaceProperty = surfaceProperties[0];
        toStore.mesh = playerMesh;
        toStoreIn.push_back(toStore);

        return playerIndex;
    }

    void RotateTile(std::vector<RenderObject>& renderObjects, Tile& tile, f32 radians, IRendererD11* renderer)
    {
        auto& tileRenderObj = renderObjects[tile.renderObjectIndex];

        XMMATRIX tileTf = XMLoadFloat4x4(&tile.transformation);
        XMVECTOR scale, rotationQuat, translation;
        XMMatrixDecompose(&scale, &rotationQuat, &translation, tileTf);

        DirectX::XMMATRIX rotMatrix = DirectX::XMMatrixRotationZ(radians);
        DirectX::XMMATRIX translationMatrix = DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f);

        XMMATRIX newTransform = XMMatrixScalingFromVector(scale) *
            rotMatrix * XMMatrixRotationQuaternion(rotationQuat) *
            translationMatrix * XMMatrixTranslationFromVector(translation);

        DirectX::XMStoreFloat4x4(&tile.transformation, newTransform);

        XMFLOAT4X4 toUpload;
        auto transposed = XMMatrixTranspose(newTransform);
        XMStoreFloat4x4(&toUpload, transposed);

        renderer->UpdateBuffer(tileRenderObj.transformBuffer, &toUpload);
    }
   
    void HandleKeyboard(WIN::Keyboard* keyboard)
    {
        while (const auto e = keyboard->GetEvent()) {
            keyboardInputs.moveUpPushed = keyboard->KeyIsPressed('W');
            keyboardInputs.moveDownPushed = keyboard->KeyIsPressed('S');
            keyboardInputs.moveLeftPushed = keyboard->KeyIsPressed('A');
            keyboardInputs.moveRightPushed = keyboard->KeyIsPressed('D');
            //keyboardInputs.moveUpPushed = keyboard->KeyIsPressed(VK_SHIFT);
            //keyboardInputs.moveDownPushed = keyboard->KeyIsPressed(VK_CONTROL);
            keyboardInputs.quitKey = keyboard->KeyIsPressed(VK_ESCAPE);
        }
    }

    void Tmp()
    {
   
    }

    int Run(WIN::IWindow* window, WIN::Keyboard* keyboard, GFX::D11::IRendererD11* renderer, HINSTANCE hInstance)
    {
        HWND windowHandle = window->GetHandle();

        GfxRenderPassD11* standardPass = CreateStandardRenderPass(renderer);
        //globalInputs = reinterpret_cast<Inputs*>(&window.GetInputs());

        std::vector<Tile> tiles;
        GridManager gm = GridManager();
        std::vector<RenderObject> renderObjects;
        PlaceGrid(renderObjects, tiles, gm, renderer);
        PlacePlayer(renderObjects, renderer);
        CameraD11* camera = renderer->CreateCamera(static_cast<float>(SCREEN_NR_OF_TILES_WIDTH), static_cast<float>(SCREEN_NR_OF_TILES_HEIGHT));
        //CameraD11* camera = renderer->CreateCamera(0.1f, 20.0f, static_cast<float>(WINDOW_WIDTH) / WINDOW_HEIGHT);
        const int DIMENSION = 5;
        camera->MoveZ(-DIMENSION);
        ResourceIndex lightBufferIndex;
        if (!CreateLights(lightBufferIndex, renderer, DIMENSION * 2.5f))
            return -1;
        renderer->SetLightBuffer(lightBufferIndex);

        ResourceIndex imguiBufferIndex;
        if (!CreateTest(imguiBufferIndex, renderer, DIMENSION * 2.5f))
            return -1;
        renderer->SetImguiBuffer(imguiBufferIndex);

        MSG msg = { };
        Tmp();
        float deltaTime = 0.0f;
        float moveSpeed = 2.0f;
        float turnSpeed = 3.14f / 2;
        auto lastFrameEnd = std::chrono::system_clock::now();
        static int screenShot = 0;
        static bool captureScreen = false;
        static constexpr int NR_OF_CAPTURES = (GRID_DIM / SCREEN_NR_OF_TILES_HEIGHT) * (GRID_DIM / SCREEN_NR_OF_TILES_HEIGHT);
        while (!window->IsClosing() && !keyboardInputs.quitKey)
        {
            if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            else
            {
                HandleKeyboard(keyboard);
                //InterpretKeyboardInput(window.GetKeyboardInputs());
                TransformCamera(camera, moveSpeed, turnSpeed, deltaTime);
                //RotateTile(renderObjects, tiles[2], 0, renderer);

                if(!captureScreen) // only render imgui if not capturing screen
                {
                    ImGui_ImplDX11_NewFrame();
                    ImGui_ImplWin32_NewFrame();
                    ImGui::NewFrame();
                    ImGui::Begin("Hello, world!");
                    ImGui::Text("This is some useful text.");
                    static ImguiVariables data;
                    static i32 counter = 0;
                    if (ImGui::Button("Button"))
                        counter++;
                    ImGui::ColorEdit4("Background", data.a);
                    ImGui::ColorEdit4("Blue Ground", data.b);
                    ImGui::ColorEdit4("Green Ground", data.c);
                    if (ImGui::Button("Screenshot"))
                        captureScreen = true;

                    renderer->UpdateBuffer(imguiBufferIndex, &data);
                    ImGui::Text("counter = %d", counter);
                    ImGui::End();
                }

                renderer->PreRender();
                renderer->Render(renderObjects);

                if (!captureScreen) // only render imgui if not capturing screen
                {
                    ImGui::Render();
                    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
                }
                
                if (captureScreen && screenShot < NR_OF_CAPTURES) {
                    camera->ResetPosition();
                    auto camX = screenShot % (GRID_DIM / SCREEN_NR_OF_TILES_WIDTH);
                    auto camY = screenShot / (GRID_DIM / SCREEN_NR_OF_TILES_HEIGHT);
                    for (u32 i = 0; i < camX; i++)
                        camera->MoveX(SCREEN_NR_OF_TILES_WIDTH - 1.f);
                    for (u32 i = 0; i < camY; i++)
                        camera->MoveY(SCREEN_NR_OF_TILES_WIDTH - 1.f);

                    renderer->Present(screenShot++);
                }
                else
                    renderer->Present();

                auto currentFrameEnd = std::chrono::system_clock::now();
                auto elapsed = std::chrono::duration_cast<
                    std::chrono::microseconds>(currentFrameEnd - lastFrameEnd).count();
                deltaTime = elapsed / 1000000.0f;
                lastFrameEnd = currentFrameEnd;
            }
        }

        return 0;
    }

}