/**********************************************************************************
// Multiple (Código Fonte)
//
// Criação:     27 Abr 2016
// Atualização: 22 Jun 2025
// Compilador:  Visual C++ 2022
//
// Descrição:   Cena 3D usando múltiplos buffers, um por objeto
//
**********************************************************************************/

#include "Multiple.h"
#include "ShapeConstants.h"

// ------------------------------------------------------------------------------

Timer Multiple::timer;

// ------------------------------------------------------------------------------

void Multiple::SelectObject(uint index)
{
    // Limpa a seleção anterior
    if (selectedIndex >= 0 && selectedIndex < (int)scene.size()) {
        scene[selectedIndex].selected = false;
        ChangeObjectColor(scene[selectedIndex], White);
    }

    selectedIndex = index;

    // Aplica a nova seleção
    if (selectedIndex >= 0 && selectedIndex < (int)scene.size()) {
        scene[selectedIndex].selected = true;
        ChangeObjectColor(scene[selectedIndex], Crimson);
    }
}

// ------------------------------------------------------------------------------

void Multiple::ChangeObjectColor(Object& obj, XMFLOAT4 color)
{

    graphics->WaitForGpu();

    // Deleta o buffer de vértices antigo
    if (obj.vbuffer) delete obj.vbuffer;

    // Recria a geometria com a nova cor baseada no tipo do objeto
    if (obj.type == SHAPE_BOX) {
        Box shape(BOX_SIZE, BOX_SIZE, BOX_SIZE, color);
        obj.vbuffer = new VertexBuffer<Vertex>(shape);
    }
    else if (obj.type == SHAPE_SPHERE) {
        Sphere shape(SPHERE_RADIUS, SPHERE_SLICES, SPHERE_STACKS, color);
        obj.vbuffer = new VertexBuffer<Vertex>(shape);
    }
    else if (obj.type == SHAPE_CYLINDER) {
        Cylinder shape(CYL_BOTTOM_RADIUS, CYL_TOP_RADIUS, CYL_HEIGHT, CYL_SLICES, CYL_STACKS, color);
        obj.vbuffer = new VertexBuffer<Vertex>(shape);
    }
    else if (obj.type == SHAPE_GEOSPHERE) {
        GeoSphere shape(GEOSPHERE_RADIUS, GEOSPHERE_SUBDIVISIONS, color);
        obj.vbuffer = new VertexBuffer<Vertex>(shape);
    }
    else if (obj.type == SHAPE_GRID) {
        Grid shape(PLANE_WIDTH, PLANE_DEPTH, GRID_M, GRID_N, color);
        obj.vbuffer = new VertexBuffer<Vertex>(shape);
    }
    else if (obj.type == SHAPE_QUAD) {
        Quad shape(QUAD_SIZE, QUAD_SIZE, color);
        obj.vbuffer = new VertexBuffer<Vertex>(shape);
    }
    else if (obj.type == SHAPE_OBJ && obj.baseGeo != nullptr)
    {
        Geometry tempGeo = *obj.baseGeo;

        for (size_t i = 0; i < tempGeo.vertices.size(); i++)
        {
            tempGeo.vertices[i].color = color;
        }

        obj.vbuffer = new VertexBuffer<Vertex>(tempGeo);
    }
}

// ------------------------------------------------------------------------------

void Multiple::CreateObject(ShapeType type, float x, float y, float z, Geometry* customBase)
{
    Object obj;
    obj.type = type;
    obj.selected = false;

    // Define a posição inicial do objeto
    XMStoreFloat4x4(&obj.world, XMMatrixTranslation(x, y, z));
	obj.position = { x, y, z };
    obj.rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);

    // Salva a referência da geometria original
    obj.baseGeo = customBase;

    // Todo objeto precisa do seu próprio buffer de constantes para matrizes
    obj.cbuffer = new ConstantBuffer<Constants>(4);

    // Aloca buffers de vértice e índice independentes baseados no modelo base
    switch (type)
    {
    case SHAPE_BOX:
        obj.mesh = new Mesh(*baseBox);
        obj.vbuffer = new VertexBuffer<Vertex>(*baseBox);
        obj.ibuffer = new IndexBuffer<uint>(*baseBox);
        break;
    case SHAPE_CYLINDER:
        obj.mesh = new Mesh(*baseCylinder);
        obj.vbuffer = new VertexBuffer<Vertex>(*baseCylinder);
        obj.ibuffer = new IndexBuffer<uint>(*baseCylinder);
        break;
    case SHAPE_SPHERE:
        obj.mesh = new Mesh(*baseSphere);
        obj.vbuffer = new VertexBuffer<Vertex>(*baseSphere);
        obj.ibuffer = new IndexBuffer<uint>(*baseSphere);
        break;
    case SHAPE_GEOSPHERE:
        obj.mesh = new Mesh(*baseGeoSphere);
        obj.vbuffer = new VertexBuffer<Vertex>(*baseGeoSphere);
        obj.ibuffer = new IndexBuffer<uint>(*baseGeoSphere);
        break;
    case SHAPE_GRID:
        obj.mesh = new Mesh(*baseGrid);
        obj.vbuffer = new VertexBuffer<Vertex>(*baseGrid);
        obj.ibuffer = new IndexBuffer<uint>(*baseGrid);
        break;
    case SHAPE_QUAD:
        obj.mesh = new Mesh(*baseQuad);
        obj.vbuffer = new VertexBuffer<Vertex>(*baseQuad);
        obj.ibuffer = new IndexBuffer<uint>(*baseQuad);
        break;
    case SHAPE_OBJ:
        obj.mesh = new Mesh(*customBase);
        obj.vbuffer = new VertexBuffer<Vertex>(*customBase);
        obj.ibuffer = new IndexBuffer<uint>(*customBase);
        break;
    }

    scene.push_back(obj);

    if (scene.size() == 1) SelectObject(0);
}

// ------------------------------------------------------------------------------

void Multiple::Init()
{
    // --------------------------------------
    // Transformação, Visualização e Projeção
    // --------------------------------------

    // posição inicial da câmera
    camera = { XM_PIDIV2, XM_PIDIV4, 8.0f };

    // inicializa a matriz de projeção
    XMStoreFloat4x4(&Proj, XMMatrixPerspectiveFovLH(
        XMConvertToRadians(45.0f), 
        window->AspectRatio(), 
        1.0f, 100.0f));

    // -------------------------------------------
    // Criação das Geometrias: Vértices e Índices
    // -------------------------------------------

    baseBox = new Box(BOX_SIZE, BOX_SIZE, BOX_SIZE, White);
    baseCylinder = new Cylinder(CYL_BOTTOM_RADIUS, CYL_TOP_RADIUS, CYL_HEIGHT, CYL_SLICES, CYL_STACKS, White);
    baseSphere = new Sphere(SPHERE_RADIUS, SPHERE_SLICES, SPHERE_STACKS, White);
    baseGeoSphere = new GeoSphere(GEOSPHERE_RADIUS, GEOSPHERE_SUBDIVISIONS, White);
    baseGrid = new Grid(PLANE_WIDTH, PLANE_DEPTH, GRID_M, GRID_N, White);
    baseQuad = new Quad(QUAD_SIZE, QUAD_SIZE, White);

    baseModels[0] = new ModelOBJ("Resources/ball.obj", White);
    baseModels[1] = new ModelOBJ("Resources/monkey.obj", White);
    baseModels[2] = new ModelOBJ("Resources/house.obj", White);
    baseModels[3] = new ModelOBJ("Resources/capsule.obj", White);
    baseModels[4] = new ModelOBJ("Resources/thorus.obj", White);
	baseModels[5] = new ModelOBJ("Resources/pegazuls.obj", White, 0.003f);

    // Cena inicial com 3 objetos
    CreateObject(SHAPE_BOX, -1.5f, 0.5f, 0.0f);
    CreateObject(SHAPE_SPHERE, 0.0f, 0.5f, 0.0f);
    CreateObject(SHAPE_CYLINDER, 1.5f, 0.5f, 0.0f);

    // -------------------------------------------
    // Vértices das Linhas Divisórias
    // -------------------------------------------
 
    Vertex lineVerts[4] = {
        { XMFLOAT3(0.0f,  1.0f, 0.0f), White }, // Topo
        { XMFLOAT3(0.0f, -1.0f, 0.0f), White }, // Base

        { XMFLOAT3(-1.0f, 0.0f, 0.0f), White }, // Esquerda
        { XMFLOAT3(1.0f, 0.0f, 0.0f), White }  // Direita
    };

    linesVBuffer = new VertexBuffer<Vertex>(lineVerts, 4);
    linesCBuffer = new ConstantBuffer<Constants>();

    Constants lineConstants;
    XMStoreFloat4x4(&lineConstants.WorldViewProj, XMMatrixIdentity());
    linesCBuffer->Copy(&lineConstants);

    // ---------------------

    BuildRootSignature();
    BuildPipelineState();    

    // ----------------------

    timer.Start();
}

// ------------------------------------------------------------------------------

void Multiple::HandleViewToggle()
{
    if (input->KeyPress('V'))
    {
        showFourViews = !showFourViews;
    }
}

void Multiple::Update()
{
    // sai com o pressionamento da tecla ESC
    if (input->KeyPress(VK_ESCAPE))
        window->Close();

    // Processamento de Inputs do Usuário
    HandleSelectionAndDeletion();
    HandleInsertion();
    HandleTransformations();
    HandleViewToggle();

    // Atualização da Câmera
    camera.Update();

    XMVECTOR pos = XMVectorSet(camera.x, camera.y, camera.z, 1.0f);
    XMVECTOR target = XMVectorZero();
    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
    XMMATRIX proj = XMLoadFloat4x4(&Proj);

    // Envio dos dados para a GPU
    UpdateSceneBuffers(view, proj);
}

void Multiple::HandleSelectionAndDeletion()
{
    // LÓGICA DE SELEÇÃO (TAB)
    if (input->KeyPress(VK_TAB))
    {
        if (!scene.empty())
        {
            if (selectedIndex >= 0 && selectedIndex < scene.size())
            {
                scene[selectedIndex].selected = false;
                ChangeObjectColor(scene[selectedIndex], White);
            }

            selectedIndex = (selectedIndex + 1) % scene.size();

            scene[selectedIndex].selected = true;
            ChangeObjectColor(scene[selectedIndex], Crimson);
        }
    }

    // REMOÇÃO DO OBJETO SELECIONADO (Tecla DEL)
    if (input->KeyPress(VK_DELETE))
    {
        if (selectedIndex >= 0 && selectedIndex < scene.size())
        {
            scene.erase(scene.begin() + selectedIndex);

            if (!scene.empty())
            {
                selectedIndex = selectedIndex % scene.size();
                scene[selectedIndex].selected = true;
                ChangeObjectColor(scene[selectedIndex], Crimson);
            }
            else
            {
                selectedIndex = -1;
            }
        }
    }
}

void Multiple::HandleInsertion()
{
    float spacing = 1.2f;

    // Espaçamento dinâmico
    float spawnX = -2.4f + (scene.size() % 5) * spacing;
    float spawnZ = 1.0f + (scene.size() / 5) * spacing;

    if (input->KeyPress('B')) CreateObject(SHAPE_BOX, spawnX, 0.5f, spawnZ);
    if (input->KeyPress('C')) CreateObject(SHAPE_CYLINDER, spawnX, 0.5f, spawnZ);
    if (input->KeyPress('S')) CreateObject(SHAPE_SPHERE, spawnX, 0.5f, spawnZ);
    if (input->KeyPress('G')) CreateObject(SHAPE_GEOSPHERE, spawnX, 0.5f, spawnZ);
    if (input->KeyPress('I')) CreateObject(SHAPE_GRID, spawnX, 0.5f, spawnZ);
    if (input->KeyPress('Q')) CreateObject(SHAPE_QUAD, spawnX, 0.5f, spawnZ);

    if (input->KeyPress('1')) CreateObject(SHAPE_OBJ, spawnX, 0.5f, spawnZ, baseModels[0]);
    if (input->KeyPress('2')) CreateObject(SHAPE_OBJ, spawnX, 0.5f, spawnZ, baseModels[1]);
    if (input->KeyPress('3')) CreateObject(SHAPE_OBJ, spawnX, 0.5f, spawnZ, baseModels[2]);
    if (input->KeyPress('4')) CreateObject(SHAPE_OBJ, spawnX, 0.5f, spawnZ, baseModels[3]);
    if (input->KeyPress('5')) CreateObject(SHAPE_OBJ, spawnX, 0.5f, spawnZ, baseModels[4]);
    if (input->KeyPress('6')) CreateObject(SHAPE_OBJ, spawnX, 0.5f, spawnZ, baseModels[5]);
}

void Multiple::HandleTransformations()
{
    if (selectedIndex == -1) return; // Se não houver seleção, aborta imediatamente

    // LÓGICA DE ESCALA COM SCROLL + CTRL
    if (input->KeyDown(VK_CONTROL))
    {
        float wheelDelta = input->MouseWheel();
        if (wheelDelta != 0.0f)
        {
            float scaleSpeed = 0.0005f;
            scene[selectedIndex].scale += wheelDelta * scaleSpeed;

            if (scene[selectedIndex].scale < 0.1f) scene[selectedIndex].scale = 0.1f;
            if (scene[selectedIndex].scale > 4.0f) scene[selectedIndex].scale = 4.0f;
        }
    }

    // LÓGICA DA TRANSLACAO
    float moveSpeed = 0.05f;
    if (input->KeyDown(VK_LEFT))  scene[selectedIndex].position.x += moveSpeed;
    if (input->KeyDown(VK_RIGHT)) scene[selectedIndex].position.x -= moveSpeed;
    if (input->KeyDown(VK_UP))    scene[selectedIndex].position.z -= moveSpeed;
    if (input->KeyDown(VK_DOWN))  scene[selectedIndex].position.z += moveSpeed;
    if (input->KeyDown(VK_PRIOR)) scene[selectedIndex].position.y += moveSpeed;
    if (input->KeyDown(VK_NEXT))  scene[selectedIndex].position.y -= moveSpeed;

    // LÓGICA DA ROTACAO
    float rotSpeed = 0.05f;
    if (input->KeyDown('Y')) scene[selectedIndex].rotation.x += rotSpeed;
    if (input->KeyDown('H')) scene[selectedIndex].rotation.x -= rotSpeed;
    if (input->KeyDown('U')) scene[selectedIndex].rotation.y += rotSpeed;
    if (input->KeyDown('J')) scene[selectedIndex].rotation.y -= rotSpeed;
    if (input->KeyDown('O')) scene[selectedIndex].rotation.z += rotSpeed;
    if (input->KeyDown('L')) scene[selectedIndex].rotation.z -= rotSpeed;
}

void Multiple::UpdateSceneBuffers(XMMATRIX view, XMMATRIX proj)
{
    float width = (float)window->Width();
    float height = (float)window->Height();
    float aspect = width / height;

    XMMATRIX views[4];
    XMMATRIX projs[4];

    if (showFourViews)
    {
        float orthoHeight = 10.0f;
        float orthoWidth = orthoHeight * aspect;
        XMMATRIX orthoProj = XMMatrixOrthographicLH(orthoWidth, orthoHeight, 1.0f, 100.0f);

        // Slot 0: Vista Frontal (Olhando de Z- para a origem)
        views[0] = XMMatrixLookAtLH(XMVectorSet(0.0f, 0.0f, -orthoHeight, 1.0f), XMVectorZero(), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
        projs[0] = orthoProj;

        // Slot 1: Vista Superior (Olhando de Y+ para a origem)
		views[1] = XMMatrixLookAtLH(XMVectorSet(0.0f, orthoHeight, 0.0f, 1.0f), XMVectorZero(), XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f));
        projs[1] = orthoProj;

        // Slot 2: Vista Lateral (Olhando de X+ para a origem)
        views[2] = XMMatrixLookAtLH(XMVectorSet(orthoHeight, 0.0f, 0.0f, 1.0f), XMVectorZero(), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
        projs[2] = orthoProj;

        // Slot 3: Vista em Perspectiva (Câmera Orbitando)
        views[3] = view;
        projs[3] = proj;
    }
    else
    {
        views[0] = view;
        projs[0] = proj;
    }

    for (auto& obj : scene)
    {
        XMMATRIX S = XMMatrixScaling(obj.scale, obj.scale, obj.scale);

        XMMATRIX Rx = XMMatrixRotationX(obj.rotation.x);
        XMMATRIX Ry = XMMatrixRotationY(obj.rotation.y);
        XMMATRIX Rz = XMMatrixRotationZ(obj.rotation.z);
        XMMATRIX R = Rz * Rx * Ry; // Ordem Roll, Pitch, Yaw

        XMMATRIX T = XMMatrixTranslation(obj.position.x, obj.position.y, obj.position.z);

        XMMATRIX W = S * R * T;

        uint viewsToUpdate = showFourViews ? 4 : 1;

        for (uint v = 0; v < viewsToUpdate; ++v)
        {
			XMMATRIX WorldViewProj = W * views[v] * projs[v];
			Constants constants;
			XMStoreFloat4x4(&constants.WorldViewProj, XMMatrixTranspose(WorldViewProj));
			obj.cbuffer->Copy(&constants, v);
        }
    }
}

// ------------------------------------------------------------------------------

void Multiple::Draw()
{
    graphics->Clear();

    graphics->CommandList()->SetPipelineState(pipelineState);
    graphics->CommandList()->SetGraphicsRootSignature(rootSignature);
    graphics->CommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    float w = (float)window->Width();
    float h = (float)window->Height();

    if (showFourViews)
    {
        float halfW = w / 2.0f;
        float halfH = h / 2.0f;

		D3D12_VIEWPORT viewFront = { 0.0f, 0.0f, halfW, halfH, 0.0f, 1.0f };
		D3D12_VIEWPORT viewTop = { halfW, 0.0f, halfW, halfH, 0.0f, 1.0f };
		D3D12_VIEWPORT viewRight = { 0.0f,  halfH, halfW, halfH, 0.0f, 1.0f };
		D3D12_VIEWPORT viewPersp = { halfW, halfH, halfW, halfH, 0.0f, 1.0f };

        D3D12_VIEWPORT viewports[4] = { viewFront, viewTop, viewRight, viewPersp };

        for (uint v = 0; v < 4; ++v)
        {
            graphics->CommandList()->RSSetViewports(1, &viewports[v]);

            for (auto& obj : scene)
            {
                // Vincula o slot do buffer constante correspondente a esta vista (0, 1, 2 ou 3)
                graphics->CommandList()->SetGraphicsRootConstantBufferView(0, obj.cbuffer->View(v));
                graphics->CommandList()->IASetVertexBuffers(0, 1, obj.vbuffer->View());
                graphics->CommandList()->IASetIndexBuffer(obj.ibuffer->View());
                graphics->CommandList()->DrawIndexedInstanced(obj.mesh->indexCount, 1, obj.mesh->startIndex, obj.mesh->baseVertex, 0);
            }
        }

        // DESENHO DAS LINHAS DIVISORIAS
        D3D12_VIEWPORT viewFull = { 0.0f, 0.0f, w, h, 0.0f, 1.0f };
        graphics->CommandList()->RSSetViewports(1, &viewFull);

        graphics->CommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
        graphics->CommandList()->SetGraphicsRootConstantBufferView(0, linesCBuffer->View());
        graphics->CommandList()->IASetVertexBuffers(0, 1, linesVBuffer->View());
        graphics->CommandList()->DrawInstanced(4, 1, 0, 0);

        //Devolve a configuração original para o próximo Frame não quebrar
        graphics->CommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    }
    else
    {
        D3D12_VIEWPORT viewFull = { 0.0f, 0.0f, w, h, 0.0f, 1.0f };
        graphics->CommandList()->RSSetViewports(1, &viewFull);

        for (auto& obj : scene)
        {
            // Usa apenas o slot 0 gerado no Update para tela cheia
            graphics->CommandList()->SetGraphicsRootConstantBufferView(0, obj.cbuffer->View(0));
            graphics->CommandList()->IASetVertexBuffers(0, 1, obj.vbuffer->View());
            graphics->CommandList()->IASetIndexBuffer(obj.ibuffer->View());
            graphics->CommandList()->DrawIndexedInstanced(obj.mesh->indexCount, 1, obj.mesh->startIndex, obj.mesh->baseVertex, 0);
        }
    }

    graphics->Present();
}

// ------------------------------------------------------------------------------

void Multiple::Finalize()
{
    // espera GPU finalizar comandos pendentes
    graphics->WaitForGpu();

    // libera memória alocada
    rootSignature->Release();
    pipelineState->Release();

    for (auto& obj : scene)
    {
        delete obj.mesh;
        delete obj.vbuffer;
        delete obj.ibuffer;
        delete obj.cbuffer;
    }

    delete baseBox;
    delete baseCylinder;
    delete baseSphere;
    delete baseGeoSphere;
    delete baseGrid;
    delete baseQuad;

    for (int i = 0; i < 6; i++)
    {
        if (baseModels[i] != nullptr)
        {
            delete baseModels[i];
            baseModels[i] = nullptr;
        }
    }

    delete linesVBuffer;
    delete linesCBuffer;
}

// ------------------------------------------------------------------------------
//                                  WinMain                                      
// ------------------------------------------------------------------------------

int APIENTRY WinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nCmdShow)
{
    try
    {
        // cria motor e configura a janela
        Engine* engine = new Engine();
        engine->window->Mode(ASPECTRATIO);
        engine->window->Size(1024, 720);
        engine->window->Color(25, 25, 25);
        engine->window->Title("Multiple");
        engine->window->Icon("Icon");
        engine->window->LostFocus(Multiple::Pause);
        engine->window->InFocus(Multiple::Resume);

        // cria e executa a aplicação
        engine->Start(new Multiple());

        // finaliza execução
        delete engine;
    }
    catch (Error & e)
    {
        // exibe mensagem em caso de erro
        MessageBox(nullptr, e.ToString().data(), "Multiple", MB_OK);
    }

    return 0;
}

// ----------------------------------------------------------------------------
