#include <Core/src/log/SeverityLevelPolicy.h>
#include <Core/src/gfx/d11/RendererD11.h>
#include <Core/src/gfx/d12/Renderer.h>
#include <Core/src/gfx/d12/IRenderer.h>
#include <Core/src/win/CopperWin.h>
#include <Core/src/win/boot.h>
#include <Core/src/log/Log.h>
#include <Core/src/win/IWindow.h>
#include <Core/src/ioc/Container.h>
#include <Core/src/ioc/Singletons.h>

#include <DirectXMath.h>
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

using namespace CPR;
using namespace CPR::GFX;
using namespace CPR::GFX::D11;
using namespace std::string_literals;
using namespace std::chrono_literals;
namespace rn = std::ranges;
namespace vi = rn::views;

namespace CPR::APP
{
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
    }gInputs;

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

    bool CreateTriangleMesh(Mesh& mesh, IRendererD11* renderer)
    {
        SimpleVertex vertices[] =
        {
            {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},
            {{0.0f, 0.5f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},
            {{0.5f, -0.5f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},
        };

        ResourceIndex verticesIndex = renderer->SubmitBuffer(
            vertices, sizeof(SimpleVertex), ARRAYSIZE(vertices),
            PerFrameUsage::STATIC, BufferBinding::STRUCTURED_BUFFER);

        if (verticesIndex == ResourceIndex(-1))
            return false;

        unsigned int indices[] = { 0, 1, 2 };

        ResourceIndex indicesIndex = renderer->SubmitBuffer(
            indices, sizeof(unsigned int), ARRAYSIZE(indices),
            PerFrameUsage::STATIC, BufferBinding::STRUCTURED_BUFFER);

        if (indicesIndex == ResourceIndex(-1))
            return false;

        mesh.vertexBuffer = verticesIndex;
        mesh.indexBuffer = indicesIndex;

        return true;
    }

    bool CreateCubeMesh(Mesh& mesh, IRendererD11* renderer)
    {
        // Order per face is top left, top right, bottom left, bottom right
        SimpleVertex vertices[] =
        {
            {{-0.5f, 0.5f, 0.5f}, {1.0f / 3, 0.0f}, {0.0f, 1.0f, 0.0f}}, // top
            {{0.5f, 0.5f, 0.5f}, {2.0f / 3, 0.0f}, {0.0f, 1.0f, 0.0f}},
            {{-0.5f, 0.5f, -0.5f}, {1.0f / 3, 0.5f}, {0.0f, 1.0f, 0.0f}},
            {{0.5f, 0.5f, -0.5f}, {2.0f / 3, 0.5f}, {0.0f, 1.0f, 0.0f}},

            {{0.5f, 0.5f, -0.5f}, {2.0f / 3, 0.0f}, {1.0f, 0.0f, 0.0f}}, // right
            {{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, -0.5f, -0.5f}, {2.0f / 3, 0.5f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, -0.5f, 0.5f}, {1.0f, 0.5f}, {1.0f, 0.0f, 0.0f}},

            {{-0.5f, 0.5f, -0.5f}, {1.0f / 3, 0.5f}, {0.0f, 0.0f, -1.0f}}, // back
            {{0.5f, 0.5f, -0.5f}, {2.0f / 3, 0.5f}, {0.0f, 0.0f, -1.0f}},
            {{-0.5f, -0.5f, -0.5f}, {1.0f / 3, 1.0f}, {0.0f, 0.0f, -1.0f}},
            {{0.5f, -0.5f, -0.5f}, {2.0f / 3, 1.0f}, {0.0f, 0.0f, -1.0f}},

            {{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}}, // left
            {{-0.5f, 0.5f, -0.5f}, {1.0f / 3, 0.0f}, {-1.0f, 0.0f, 0.0f}},
            {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.5f}, {-1.0f, 0.0f, 0.0f}},
            {{-0.5f, -0.5f, -0.5f}, {1.0f / 3, 0.5f}, {-1.0f, 0.0f, 0.0f}},

            {{0.5f, 0.5f, 0.5f}, {0.0f, 0.5f}, {0.0f, 0.0f, 1.0f}}, // front
            {{-0.5f, 0.5f, 0.5f}, {1.0f / 3, 0.5f}, {0.0f, 0.0f, 1.0f}},
            {{0.5f, -0.5f, 0.5f}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
            {{-0.5f, -0.5f, 0.5f}, {1.0f / 3, 1.0f}, {0.0f, 0.0f, 1.0f}},

            {{-0.5f, -0.5f, -0.5f}, {2.0f / 3, 0.5f}, {0.0f, 0.0f, -1.0f}}, // bottom
            {{0.5f, -0.5f, -0.5f}, {1.0f, 0.5f}, {0.0f, 0.0f, -1.0f}},
            {{-0.5f, -0.5f, 0.5f}, {2.0f / 3, 1.0f}, {0.0f, 0.0f, -1.0f}},
            {{0.5f, -0.5f, 0.5f}, {1.0f, 1.0f}, {0.0f, 0.0f, -1.0f}}
        };

        ResourceIndex verticesIndex = renderer->SubmitBuffer(
            vertices, sizeof(SimpleVertex), ARRAYSIZE(vertices),
            PerFrameUsage::STATIC, BufferBinding::STRUCTURED_BUFFER);

        if (verticesIndex == ResourceIndex(-1))
            return false;

        const unsigned int NR_OF_INDICES = 36;
        unsigned int indices[NR_OF_INDICES];
        for (unsigned int i = 0; i < NR_OF_INDICES / 6; ++i)
        {
            unsigned int baseBufferIndex = i * 6;
            unsigned int baseVertexIndex = i * 4;
            indices[baseBufferIndex + 0] = baseVertexIndex + 0;
            indices[baseBufferIndex + 1] = baseVertexIndex + 1;
            indices[baseBufferIndex + 2] = baseVertexIndex + 2;
            indices[baseBufferIndex + 3] = baseVertexIndex + 2;
            indices[baseBufferIndex + 4] = baseVertexIndex + 1;
            indices[baseBufferIndex + 5] = baseVertexIndex + 3;
        }

        ResourceIndex indicesIndex = renderer->SubmitBuffer(
            indices, sizeof(unsigned int), NR_OF_INDICES, PerFrameUsage::STATIC,
            BufferBinding::STRUCTURED_BUFFER);

        if (indicesIndex == ResourceIndex(-1))
            return false;

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

    bool CreateTransformBuffer(ResourceIndex& toSet,
        IRendererD11* renderer, float xPos, float yPos, float zPos)
    {
        float matrix[16] =
        {
            1.0f, 0.0f, 0.0f, xPos,
            0.0f, 1.0f, 0.0f, yPos,
            0.0f, 0.0f, 1.0f, zPos,
            0.0f, 0.0f, 0.0f, 1.0f
        };

        toSet = renderer->SubmitBuffer(matrix,
            sizeof(float) * 16, 1, PerFrameUsage::DYNAMIC,
            BufferBinding::CONSTANT_BUFFER);

        return toSet != ResourceIndex(-1);
    }

    void TransformCamera(CameraD11* camera, float moveSpeed,
        float turnSpeed, float deltaTime)
    {
        if (gInputs.moveRightPushed)
            camera->MoveX(moveSpeed * deltaTime);
        else if (gInputs.moveLeftPushed)
            camera->MoveX(-moveSpeed * deltaTime);

        if (gInputs.moveForwardPushed)
            camera->MoveZ(moveSpeed * deltaTime);
        else if (gInputs.moveBackwardsPushed)
            camera->MoveZ(-moveSpeed * deltaTime);

        if (gInputs.moveUpPushed)
            camera->MoveY(moveSpeed * deltaTime);
        else if (gInputs.moveDownPushed)
            camera->MoveY(-moveSpeed * deltaTime);

        if (gInputs.turnLeftPushed)
            camera->RotateY(-turnSpeed * deltaTime);
        else if (gInputs.turnRightPushed)
            camera->RotateY(turnSpeed * deltaTime);
    }

    bool LoadSurfacePropertyFiles(SurfaceProperty& surfaceProperties,
        IRendererD11* renderer, const std::string& prefix)
    {
        ResourceIndex diffuseTextureIndex;
        if (!LoadTexture(diffuseTextureIndex, renderer, prefix + "Diffuse.png", 4))
            return false;

        ResourceIndex specularTextureIndex;
        if (!LoadTexture(specularTextureIndex, renderer, prefix + "Specular.png", 4))
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
            sizeof(PointLight), 4, PerFrameUsage::STATIC,
            BufferBinding::STRUCTURED_BUFFER);

        return toSet != ResourceIndex(-1);
    }

    bool PlacePyramid(const Mesh& cubeMesh, const SurfaceProperty& stoneProperties,
        std::vector<RenderObject>& toStoreIn, IRendererD11* renderer, int height)
    {
        int base = (height - 1) * 2 + 1;

        for (int level = 0; level < height - 1; ++level)
        {
            int topLeftX = -(height - level - 1);
            int topLeftZ = (height - level - 1);
            for (int row = 0; row < base - level * 2; ++row)
            {
                ResourceIndex transformBuffer;
                bool result = CreateTransformBuffer(transformBuffer, renderer,
                    static_cast<float>(topLeftX + row),
                    static_cast<float>(level + 1), static_cast<float>(topLeftZ));

                if (result == false)
                    return false;

                RenderObject toStore;
                toStore.transformBuffer = transformBuffer;
                toStore.surfaceProperty = stoneProperties;
                toStore.mesh = cubeMesh;
                toStoreIn.push_back(toStore);

                result = CreateTransformBuffer(transformBuffer, renderer,
                    static_cast<float>(topLeftX + row),
                    static_cast<float>(level + 1), static_cast<float>(-topLeftZ));

                if (result == false)
                    return false;

                toStore.transformBuffer = transformBuffer;
                toStore.surfaceProperty = stoneProperties;
                toStore.mesh = cubeMesh;
                toStoreIn.push_back(toStore);
            }

            for (int column = 1; column < base - level * 2 - 1; ++column)
            {
                ResourceIndex transformBuffer;
                bool result = CreateTransformBuffer(transformBuffer, renderer,
                    static_cast<float>(topLeftX), static_cast<float>(level + 1),
                    static_cast<float>(topLeftZ - column));

                if (result == false)
                    return false;

                RenderObject toStore;
                toStore.transformBuffer = transformBuffer;
                toStore.surfaceProperty = stoneProperties;
                toStore.mesh = cubeMesh;
                toStoreIn.push_back(toStore);

                result = CreateTransformBuffer(transformBuffer, renderer,
                    static_cast<float>(-topLeftX), static_cast<float>(level + 1),
                    static_cast<float>(topLeftZ - column));

                if (result == false)
                    return false;

                toStore.transformBuffer = transformBuffer;
                toStore.surfaceProperty = stoneProperties;
                toStore.mesh = cubeMesh;
                toStoreIn.push_back(toStore);
            }
        }

        ResourceIndex transformBuffer;
        bool result = CreateTransformBuffer(transformBuffer, renderer,
            0.0f, static_cast<float>(height), 0.0f);

        if (result == false)
            return false;

        RenderObject toStore;
        toStore.transformBuffer = transformBuffer;
        toStore.surfaceProperty = stoneProperties;
        toStore.mesh = cubeMesh;
        toStoreIn.push_back(toStore);

        return true;
    }

    bool PlaceGround(const Mesh& cubeMesh, const SurfaceProperty& grassProperties,
        std::vector<RenderObject>& toStoreIn, IRendererD11* renderer, int height)
    {
        height += 2;
        int base = (height - 1) * 2 + 1;

        for (int level = 0; level < height - 1; ++level)
        {
            int topLeftX = -(height - level - 1);
            int topLeftZ = (height - level - 1);

            for (int column = 0; column < base - level * 2; ++column)
            {
                for (int row = 0; row < base - level * 2; ++row)
                {
                    ResourceIndex transformBuffer;
                    bool result = CreateTransformBuffer(transformBuffer, renderer,
                        static_cast<float>(topLeftX + column), 0.0f,
                        static_cast<float>(topLeftZ - row));

                    if (result == false)
                        return false;

                    RenderObject toStore;
                    toStore.transformBuffer = transformBuffer;
                    toStore.surfaceProperty = grassProperties;
                    toStore.mesh = cubeMesh;
                    toStoreIn.push_back(toStore);
                }
            }
        }

        return true;
    }

    bool PlaceCrystal(const Mesh& cubeMesh, const SurfaceProperty& crystalProperties,
        std::vector<RenderObject>& toStoreIn, IRendererD11* renderer, int height)
    {
        ResourceIndex transformBuffer;
        bool result = CreateTransformBuffer(transformBuffer, renderer,
            0.0f, static_cast<float>(height + 2), 0.0f);

        if (result == false)
            return false;

        RenderObject toStore;
        toStore.transformBuffer = transformBuffer;
        toStore.surfaceProperty = crystalProperties;
        toStore.mesh = cubeMesh;
        toStoreIn.push_back(toStore);

        return true;
    }

    bool PlaceBlocks(std::vector<RenderObject>& toStoreIn, IRendererD11* renderer, int height)
    {
        Mesh cubeMesh;
        if (!CreateCubeMesh(cubeMesh, renderer))
            return false;

        const std::string path = "../../WindowApp/Assets/Textures/";

        SurfaceProperty stoneProperties;
        if (!LoadSurfacePropertyFiles(stoneProperties, renderer, path + "Stone"))
            return false;

        SurfaceProperty grassProperties;
        if (!LoadSurfacePropertyFiles(grassProperties, renderer, path + "Grass"))
            return false;

        SurfaceProperty crystalProperties;
        if (!LoadSurfacePropertyFiles(crystalProperties, renderer, path + "Crystal"))
            return false;

        return PlacePyramid(cubeMesh, stoneProperties, toStoreIn, renderer, height)
            && PlaceGround(cubeMesh, grassProperties, toStoreIn, renderer, height)
            && PlaceCrystal(cubeMesh, crystalProperties, toStoreIn, renderer, height);
    }

    void RotateCrystal(RenderObject& crystal, float deltaTime, int height,
        IRendererD11* renderer)
    {
        static float rotationAmount = 0.0f;
        static float heightOffset = 0.0f;
        static float offsetSpeed = 0.5f;

        DirectX::XMMATRIX rotMatrix = DirectX::XMMatrixRotationY(rotationAmount);
        DirectX::XMMATRIX translationMatrix = DirectX::XMMatrixTranslation(0.0f,
            static_cast<float>(height + 2 + heightOffset), 0.0f);
        DirectX::XMFLOAT4X4 toUpload;
        DirectX::XMStoreFloat4x4(&toUpload,
            DirectX::XMMatrixMultiplyTranspose(rotMatrix, translationMatrix));
        renderer->UpdateBuffer(crystal.transformBuffer, &toUpload);

        rotationAmount += deltaTime * DirectX::XM_PI;
        if (rotationAmount >= DirectX::XM_2PI)
            rotationAmount -= DirectX::XM_2PI;

        heightOffset += offsetSpeed * deltaTime;
        if (heightOffset >= 0.5f)
        {
            heightOffset = 0.5f;
            offsetSpeed *= -1;
        }
        else if (heightOffset <= -0.5f)
        {
            heightOffset = -0.5f;
            offsetSpeed *= -1;
        }
    }
    void InterpretKeyboardInput(unsigned long long input)
    {
        gInputs.moveLeftPushed = input & 1;
        gInputs.moveRightPushed = input & 2;
        gInputs.moveForwardPushed = input & 4;
        gInputs.moveBackwardsPushed = input & 8;
        gInputs.moveUpPushed = input & 16;
        gInputs.moveDownPushed = input & 32;

        gInputs.turnLeftPushed = input & 64;
        gInputs.turnRightPushed = input & 128;

        gInputs.quitKey = input & (1ULL << 63);
    }


    int Run(WIN::IWindow* window, HINSTANCE hInstance, GFX::D11::IRendererD11* rendaerer)
    {
        const unsigned int WINDOW_WIDTH = 1280;
        const unsigned int WINDOW_HEIGHT = 642;
        HWND windowHandle = window->GetHandle();
        //renderer->Initialize(windowHandle);
        RendererD11* renderer = new D11::RendererD11(windowHandle);
        GfxRenderPassD11* standardPass = CreateStandardRenderPass(renderer);
        //globalInputs = reinterpret_cast<Inputs*>(&window.GetInputs());

        const int DIMENSION = 5;
        std::vector<RenderObject> renderObjects;
        if (!PlaceBlocks(renderObjects, renderer, DIMENSION))
            return -1;

        CameraD11* camera = renderer->CreateCamera(0.1f, 20.0f,
            static_cast<float>(WINDOW_WIDTH) / WINDOW_HEIGHT);
        camera->MoveZ(-DIMENSION);
        camera->MoveY(1);

        ResourceIndex lightBufferIndex;
        if (!CreateLights(lightBufferIndex, renderer, DIMENSION * 2.5f))
            return -1;

        renderer->SetLightBuffer(lightBufferIndex);

        MSG msg = { };

        float deltaTime = 0.0f;
        float moveSpeed = 2.0f;
        float turnSpeed = 3.14f / 2;
        auto lastFrameEnd = std::chrono::system_clock::now();

        while (!window->IsClosing())
        {
            if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            else
            {
                //InterpretKeyboardInput(window.GetKeyboardInputs());
                TransformCamera(camera, moveSpeed, turnSpeed, deltaTime);
                RotateCrystal(renderObjects.back(), deltaTime,
                    DIMENSION, renderer);

                renderer->PreRender();

                //renderer->SetCamera(camera);
                //renderer->SetRenderPass(standardPass);
                renderer->Render(renderObjects);

                renderer->Present();
                auto currentFrameEnd = std::chrono::system_clock::now();
                auto elapsed = std::chrono::duration_cast<
                    std::chrono::microseconds>(currentFrameEnd - lastFrameEnd).count();
                deltaTime = elapsed / 1000000.0f;
                lastFrameEnd = currentFrameEnd;
            }

        }

        //renderer->DestroyGraphicsRenderPass(standardPass);
        //renderer->DestroyCamera(camera);
        //delete(standardPass);
        delete(renderer);
        return 0;
        /*auto x = 0;
        while (!window.IsClosing())
        {
            window.SetTitle(std::format(L"Animated Window Title [{:*<{}}]", L'*', x + 1));
            x = (x + 1) % 20;
            std::this_thread::sleep_for(50ms);
        }
        return 1;*/
    }

}