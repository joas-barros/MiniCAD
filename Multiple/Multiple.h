/**********************************************************************************
// Multiple (Arquivo de Cabeçalho)
//
// Criação:     27 Abr 2016
// Atualização: 22 Jun 2025
// Compilador:  Visual C++ 2022
//
// Descrição:   Cena 3D usando múltiplos buffers, um por objeto
//
**********************************************************************************/

#include "DXUT.h"

// ------------------------------------------------------------------------------

enum ShapeType { SHAPE_BOX, SHAPE_SPHERE, SHAPE_CYLINDER, SHAPE_GEOSPHERE, SHAPE_GRID, SHAPE_QUAD, SHAPE_OBJ };

struct Constants
{
    XMFLOAT4X4 WorldViewProj =
    { 1.0f, 0.0f, 0.0f, 0.0f,
      0.0f, 1.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f };
};

struct Object
{
    XMFLOAT4X4 world = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f };

    Mesh* mesh = nullptr;
    VertexBuffer<Vertex>* vbuffer = nullptr;
    IndexBuffer<uint>* ibuffer = nullptr;
    ConstantBuffer<Constants>* cbuffer = nullptr;

    ShapeType type;
    bool selected = false;

    float scale = 1.0f;
    XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
    XMFLOAT3 rotation = { 0.0f, 0.0f, 0.0f };

    Geometry* baseGeo = nullptr;
};

// ------------------------------------------------------------------------------

class Multiple : public App
{
private:
    ID3D12RootSignature* rootSignature = nullptr;
    ID3D12PipelineState* pipelineState = nullptr;
    
    OrbitCamera camera;
    XMFLOAT4X4 Proj;
    static Timer timer;
    bool spinning = true;
    
    vector<Object> scene;

    Box* baseBox = nullptr;
    Cylinder* baseCylinder = nullptr;
    Sphere* baseSphere = nullptr;
    GeoSphere* baseGeoSphere = nullptr  ;
    Grid* baseGrid = nullptr;
    Quad* baseQuad = nullptr;

    ModelOBJ* baseModels[5] = { nullptr };

	// Metodos de criação e manipulação de objetos
    uint selectedIndex = -1;
    void CreateObject(ShapeType type, float x, float y, float z, Geometry* customBase = nullptr);
    void ChangeObjectColor(Object& obj, XMFLOAT4 color);
	void SelectObject(uint index);

	// Metodos de ataulização de cena
    void HandleSelectionAndDeletion();
    void HandleInsertion();
    void HandleTransformations();
    void UpdateSceneBuffers(XMMATRIX view, XMMATRIX proj);

    bool showFourViews = false;
    void HandleViewToggle();

    // linas divisorias
    VertexBuffer<Vertex>* linesVBuffer = nullptr;
    ConstantBuffer<Constants>* linesCBuffer = nullptr;

public:
    void Init();
    void Update();
    void Draw();
    void Finalize();

    static void Pause()  { timer.Stop();  }
    static void Resume() { timer.Start(); }

    void BuildRootSignature();
    void BuildPipelineState();
};

// ------------------------------------------------------------------------------
