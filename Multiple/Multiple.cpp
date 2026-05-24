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

void Multiple::ChangeObjectColor(Object& obj, XMFLOAT4 color)
{
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
}

// ------------------------------------------------------------------------------

Object Multiple::CreateObject(ShapeType type, float x, float y, float z)
{
    Object obj;
    obj.type = type;
    obj.selected = false;

    // Define a posição inicial do objeto
    XMStoreFloat4x4(&obj.world, XMMatrixTranslation(x, y, z));

    // Todo objeto precisa do seu próprio buffer de constantes para matrizes
    obj.cbuffer = new ConstantBuffer<Constants>();

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
    }

    return obj;
}

// ------------------------------------------------------------------------------

void Multiple::Init()
{
    // --------------------------------------
    // Transformação, Visualização e Projeção
    // --------------------------------------

    // posição inicial da câmera
    camera = { XM_PIDIV2, XM_PIDIV4, 5.0f };

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

    // Cena inicial com 3 objetos
    scene.push_back(CreateObject(SHAPE_BOX, -2.5f, 0.5f, 0.0f));
    scene.push_back(CreateObject(SHAPE_SPHERE, 0.0f, 0.5f, 0.0f));
    scene.push_back(CreateObject(SHAPE_CYLINDER, 2.5f, 0.5f, 0.0f));

    if (!scene.empty())
    {
        selectedIndex = 0;
        scene[selectedIndex].selected = true;
        ChangeObjectColor(scene[selectedIndex], Crimson);
    }
 
    // ---------------------

    BuildRootSignature();
    BuildPipelineState();    

    // ----------------------

    timer.Start();
}

// ------------------------------------------------------------------------------

void Multiple::Update()
{
    // sai com o pressionamento da tecla ESC
    if (input->KeyPress(VK_ESCAPE))
        window->Close();

    // ---------------------------------------------------------
    // LÓGICA DE SELEÇÃO (TAB)
    // ---------------------------------------------------------
    if (input->KeyPress(VK_TAB))
    {
        if (!scene.empty())
        {
            // Volta a cor do atual para BRANCO
            if (selectedIndex >= 0 && selectedIndex < scene.size())
            {
                scene[selectedIndex].selected = false;
                ChangeObjectColor(scene[selectedIndex], XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
            }

            // Avança o índice para o próximo objeto da lista
            selectedIndex = (selectedIndex + 1) % scene.size();

            // Pinta o novo objeto selecionado de VERMELHO
            scene[selectedIndex].selected = true;
            ChangeObjectColor(scene[selectedIndex], XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f));
        }
    }

    // ---------------------------------------------------------
    // ATUALIZAÇÃO DA CÂMERA
    // ---------------------------------------------------------
    camera.Update();

    XMVECTOR pos = XMVectorSet(camera.x, camera.y, camera.z, 1.0f);
    XMVECTOR target = XMVectorZero();
    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    XMMATRIX view = XMMatrixLookAtLH(pos, target, up);

    XMMATRIX proj = XMLoadFloat4x4(&Proj);

    // ---------------------------------------------------------
    // INSERÇÃO DE OBJETOS COM POSIÇÕES FIXAS
    // ---------------------------------------------------------

    // Espaçamento dinâmico para os novos objetos não nascerem um dentro do outro
    float spawnX = -4.0f + (scene.size() % 5) * 2.0f;
    float spawnZ = 2.0f + (scene.size() / 5) * 2.0f;

    if (input->KeyPress('B')) scene.push_back(CreateObject(SHAPE_BOX, spawnX, 0.5f, spawnZ));
    if (input->KeyPress('C')) scene.push_back(CreateObject(SHAPE_CYLINDER, spawnX, 0.5f, spawnZ));
    if (input->KeyPress('S')) scene.push_back(CreateObject(SHAPE_SPHERE, spawnX, 0.5f, spawnZ));
    if (input->KeyPress('G')) scene.push_back(CreateObject(SHAPE_GEOSPHERE, spawnX, 0.5f, spawnZ));
    if (input->KeyPress('I')) scene.push_back(CreateObject(SHAPE_GRID, spawnX, 0.5f, spawnZ));
    if (input->KeyPress('Q')) scene.push_back(CreateObject(SHAPE_QUAD, spawnX, 0.5f, spawnZ));

    // ---------------------------------------------------------
    // REMOÇÃO DO OBJETO SELECIONADO (Tecla DEL)
    // ---------------------------------------------------------
    if (input->KeyPress(VK_DELETE))
    {
        if (selectedIndex >= 0 && selectedIndex < scene.size())
        {
            // Remove o objeto do vetor
            scene.erase(scene.begin() + selectedIndex);

            // Reajusta a seleção para o próximo elemento válido (se a cena não ficou vazia)
            if (!scene.empty())
            {
                // Garante que o índice não passe do tamanho atual do vetor
                selectedIndex = selectedIndex % scene.size();

                // Pinta o novo item selecionado de vermelho
                scene[selectedIndex].selected = true;
                ChangeObjectColor(scene[selectedIndex], Crimson);
            }
            else
            {
                selectedIndex = -1; // Cena ficou completamente vazia
            }
        }
    }

    // ---------------------------------------------------------
    // ATUALIZAÇÃO DOS BUFFERS CONSTANTES
    // ---------------------------------------------------------
    for (size_t i = 0; i < scene.size(); i++)
    {
        XMMATRIX world = XMLoadFloat4x4(&scene[i].world);
        XMMATRIX WorldViewProj = world * view * proj;

        Constants constants;
        XMStoreFloat4x4(&constants.WorldViewProj, XMMatrixTranspose(WorldViewProj));
        scene[i].cbuffer->Copy(&constants);
    }
}

// ------------------------------------------------------------------------------

void Multiple::Draw()
{
    // limpa o backbuffer
    graphics->Clear();

    // comandos de configuração comuns a todos os objetos
    graphics->CommandList()->SetPipelineState(pipelineState);
    graphics->CommandList()->SetGraphicsRootSignature(rootSignature);
    graphics->CommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    
    // desenha objetos da cena
    for (auto& obj : scene)
    {
        // comandos de configuração específicos a cada objeto
        graphics->CommandList()->SetGraphicsRootConstantBufferView(0, obj.cbuffer->View());
        graphics->CommandList()->IASetVertexBuffers(0, 1, obj.vbuffer->View());
        graphics->CommandList()->IASetIndexBuffer(obj.ibuffer->View());

        // desenha objeto
        graphics->CommandList()->DrawIndexedInstanced(
            obj.mesh->indexCount, 1,
            obj.mesh->startIndex,
            obj.mesh->baseVertex,
            0);
    }
 
    // apresenta o backbuffer na tela
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
