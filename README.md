# DirectX 12 Game Engine

# Nau Engine Test
В качестве основы для прохождения тестового задания я использую собственный движок на DX12.  
[1. Система компонентов и управление ресурсами](#first-part)  
[2. Сериализация и десериализация сцены](#second-part)  
[3. Интеграция скриптового языка](#third-part)  

<a id="first-part"></a>
## Часть 1: Система компонентов и управление ресурсами
Для хранения и управления в менеджере ресурсов я выбрал самые тяжелые и долго-загружаемые ресурсы в движке: геометрия (вершины и индексы) и текстуры.
Загрузка этих ресурсов занимает большую часть времени при запуске движка. 
Был реализован класс `ResourceStorage`, который хранит `Object3DComponent` с данными о вершинах и `TextureComponent` с буфером текстуры.
А `Object3DEntity` и `MaterialEntity` хранят идентификаторы компонентов из хранилища ресурсов. Теперь ресурсы не будут повторно загружаться из файлов, 
и каждый компонент может быть использован несколькими сущностями.
### 1. Реализуйте базовую систему Entity-Component-System
В качестве Entity будут выступать классы Object3DEntity и MaterialEntity. 
Первый хранит идентификатор Object3DComponent в хранилище ресурсов, а также информацию об изменении объекта (позиция, вращение, масштаб).  
```c++ 
class Object3DEntity
{
public:
    void OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList, const std::string& filePath);
    void OnUpdate(double deltaTime);
    void OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewProjMatrix);
    TransformComponent Transform;
private:
    void SetConstBuffers(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewProjMatrix);
    uint32_t ObjectId;
};
```
А MaterialEntity хранит идентификаторы на ресурсы текстур в хранилище. 
```c++
class MaterialEntity
{
private:
    uint32_t m_DiffuseTextureId;
    uint32_t m_EmissiveTextureId;
    uint32_t m_NormalTextureId;
    uint32_t m_MetallicTextureId;
    uint32_t m_RoughnessTextureId;
    uint32_t m_GltfMetallicRoughnessTextureId;
    uint32_t m_AOTextureId;

    bool DrawIt = true;

    DirectX::SimpleMath::Vector4 HasDiffuseNormalEmissive = DirectX::SimpleMath::Vector4(0, 0, 0, 0);
    DirectX::SimpleMath::Vector4 HasOcclusionRoughnessMetallicCombined = DirectX::SimpleMath::Vector4(0, 0, 0, 0);

    uint32_t AddTexture(ComPtr<ID3D12GraphicsCommandList2> commandList, std::string &path);

public:
    map<TextureType, string> m_ImagePaths;

    void Load(ComPtr<ID3D12GraphicsCommandList2> commandList);
    void RenderTexture(ComPtr<ID3D12GraphicsCommandList2> commandList, int slot, uint32_t textureId, float mask);
    void Render(ComPtr<ID3D12GraphicsCommandList2> commandList);

    bool CanDrawIt() { return DrawIt; }
};
```
Выполняем загрузку текстуры только в том случае, если она ещё не инициализирована.  
```c++
uint32_t MaterialEntity::AddTexture(ComPtr<ID3D12GraphicsCommandList2> commandList, std::string& path)
{
    if (NotFoundFile(path.c_str())) return -1;

    int id = ResourceStorage::AddTexture(path);
    auto texture = ResourceStorage::GetTexture(id);

    if (!texture->IsInitialized())
        texture->OnLoad(commandList, path);
    
    return id;
}
```
Компонент объекта `Object3DComponent` хранит массив мэшей. 
```c++
class Object3DComponent
{
private:
	std::vector<Mesh3DComponent*> m_Meshes;
	bool m_Initialized = false;
public:
	void OnLoad(std::vector<Mesh3DComponent*>& meshes);
	void OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList);
	bool IsInitialized();
    void Unload();
};
```
А `TextureComponent` хранит буфер текстуры, а также дескрипторы памяти в CPU и GPU.
```c++
class TextureComponent
{
public:
    ComPtr<ID3D12Resource> m_Resource;
    int m_SRVHeapIndex;

private:
    ComPtr<ID3D12Resource> m_UploadBuffer;
    bool m_Initialized = false;

public:
    void OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList, string path);
    void OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, int slot);
    void Release();

    D3D12_CPU_DESCRIPTOR_HANDLE GetCpuDescHandle(D3D12_DESCRIPTOR_HEAP_TYPE heapType);
    D3D12_GPU_DESCRIPTOR_HANDLE GetGpuDescHandle(D3D12_DESCRIPTOR_HEAP_TYPE heapType);

    bool IsInitialized();
};
```
В качестве сцены выступает класс игры `KatamariGame`. Он хранит объекты сцены и игрока.
```c++
class KatamariGame
{
private:
	map<string, Object3DEntity> m_objects;

public:
	Player player;

	void OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList);

	void OnUpdate(float deltaTime);
	void OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewProjMatrix);
private:
	void Add(ComPtr<ID3D12GraphicsCommandList2> commandList, string name, string path);
};
```
В нем создаются, трансформируются, и отрисовываются все объекты.
```c++
void KatamariGame::OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
	player.OnLoad(commandList);

	Add(commandList, "scene", "../../DX12GE/Resources/Models/gaz/scene.gltf");
	m_objects["scene"].Transform.SetScale(3, 3, 3);
	m_objects["scene"].Transform.SetPosition(0, 0, 0);

	Add(commandList, "scene1", "../../DX12GE/Resources/Models/gaz/scene.gltf");
	m_objects["scene1"].Transform.SetScale(3, 3, 3);
	m_objects["scene1"].Transform.SetPosition(-75, 0, 0);

	Add(commandList, "scene2", "../../DX12GE/Resources/Models/gaz/scene.gltf");
	m_objects["scene2"].Transform.SetScale(3, 3, 3);
	m_objects["scene2"].Transform.SetPosition(-150, 0, 0);

	Add(commandList, "scene3", "../../DX12GE/Resources/Models/gaz/scene.gltf");
	m_objects["scene3"].Transform.SetScale(3, 3, 3);
	m_objects["scene3"].Transform.SetPosition(0, 0, 75);

	Add(commandList, "scene4", "../../DX12GE/Resources/Models/gaz/scene.gltf");
	m_objects["scene4"].Transform.SetScale(3, 3, 3);
	m_objects["scene4"].Transform.SetPosition(-75, 0, 75);

	Add(commandList, "scene5", "../../DX12GE/Resources/Models/gaz/scene.gltf");
	m_objects["scene5"].Transform.SetScale(3, 3, 3);
	m_objects["scene5"].Transform.SetPosition(-150, 0, 75);
}

void KatamariGame::OnUpdate(float deltaTime)
{
	player.OnUpdate(deltaTime);

	static float counter = 0.0f;
	counter += deltaTime;

	if (counter >= 2 * PI) counter -= 2 * PI;

	Vector3 sc1Pos = m_objects["scene1"].Transform.GetPosition();
	sc1Pos.y = sin(counter) * 10;
	m_objects["scene1"].Transform.SetPosition(sc1Pos);
	m_objects["scene4"].Transform.SetRotationY(-counter);

	for (auto obj : m_objects)
	{
		obj.second.OnUpdate(deltaTime);
	}
}

void KatamariGame::OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewProjMatrix)
{
	player.OnRender(commandList, viewProjMatrix);

	for (auto obj : m_objects)
	{
		obj.second.OnRender(commandList, viewProjMatrix);
	}
}

void KatamariGame::Add(ComPtr<ID3D12GraphicsCommandList2> commandList, string name, string path)
{
	if (m_objects.find(name) != m_objects.end()) { return; }

	m_objects.insert({ name, Object3DEntity() });
	m_objects[name].OnLoad(commandList, path);
}

```
Метод `OnUpdate` отвечает за изменение объектов на сцене. А `OnRender` - за их отрисовку.
### 2. Реализуйте простой менеджер ресурсов
Класс `ResourceStorage` отвечает за хранение и менеджмент компонентов. 
```c++
class ResourceStorage
{
public:
	static uint32_t Find(const std::string& name);
	static uint32_t AddObject3D(const std::string& name);
	static uint32_t AddTexture(const std::string& name);
	
	static std::shared_ptr<Object3DComponent> GetObject3D(uint32_t id);
	static std::shared_ptr<TextureComponent> GetTexture(uint32_t id);
	static std::shared_ptr<Object3DComponent> GetObject3DByName(const std::string& name);
	static std::shared_ptr<TextureComponent> GetTextureByName(const std::string& name);
};
```
При добавлении нового компонента в хранилище выполняется проверка, есть ли уже в словаре имён `m_Names` путь к файлу этого компонента (будь то obj или png).
Если есть, возвращаем идентификатор ранее загруженного компонента. Нет - создаем новый shared_ptr компонента. `shared_ptr` был выбран вместо `unique_ptr`,
потому что указатель компонента используется не только в `ResourceStorage`. Через методы `GetObject3D` и `GetTexture` можно получить компоненты, чтобы,
наример, отрендерить или изменить их. А unique_ptr может владеть объектом только один (если не использовать std::move).
```c++
static std::map<std::string, uint32_t> m_Names;
static std::vector<std::shared_ptr<Object3DComponent>> m_Objects;
static std::vector<std::shared_ptr<TextureComponent>> m_Textures;

uint32_t ResourceStorage::Find(const std::string& name)
{
    auto pair = m_Names.find(name);
    return pair != m_Names.end() ? pair->second : -1;
}

uint32_t ResourceStorage::AddObject3D(const std::string& name)
{
    uint32_t foundId = Find(name);
    if (foundId != -1) return foundId;

    uint32_t id = m_Objects.size();
    m_Objects.push_back(std::make_shared<Object3DComponent>());
    m_Names.emplace(name, id);

    return id;
}

uint32_t ResourceStorage::AddTexture(const std::string& name)
{
    uint32_t foundId = Find(name);
    if (foundId != -1) 
        return foundId;

    uint32_t id = m_Textures.size();
    m_Textures.push_back(std::make_shared<TextureComponent>());
    m_Names.emplace(name, id);

	return id;
}

std::shared_ptr<Object3DComponent> ResourceStorage::GetObject3D(uint32_t id)
{
    return m_Objects[id];
}

std::shared_ptr<TextureComponent> ResourceStorage::GetTexture(uint32_t id)
{
    return m_Textures[id];
}

std::shared_ptr<Object3DComponent> ResourceStorage::GetObject3DByName(const std::string& name)
{
    uint32_t foundId = Find(name);
    if (foundId != -1) 
        return m_Objects[foundId];
    return nullptr;
}

std::shared_ptr<TextureComponent> ResourceStorage::GetTextureByName(const std::string& name)
{
    uint32_t foundId = Find(name);
    if (foundId != -1) 
        return m_Textures[foundId];
    return nullptr;
}

void ResourceStorage::Unload()
{
    for (auto obj : m_Objects)
    {
        obj->Unload();
        obj = nullptr;
    }
    m_Objects.clear();

    for (auto tex : m_Textures)
    {
        tex->Release();
        tex = nullptr;
    }
    m_Textures.clear();

    m_Names.clear();
}
```
Загрузка объектов из файлов производится с помощью библиотеки Assimp в классе `AssimpModelLoader`. После загрузки компонента из файла или нахождения 
его в хранилище ресурсов сущность получает Id компонента.
```c++
void Object3DEntity::OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList, const std::string& filePath)
{
    AssimpModelLoader modelLoader;

    float yOffset;
    ObjectId = modelLoader.LoadModelData(commandList, filePath, yOffset);
    Transform.SetDefault(yOffset);
}
```
Так выглядит загрузка моделей из файлов. Сначала проверяется существование файла, далее нахождение его в хранилище ресурсов. Если он не загружался 
ранее, то выполняется загрузка. Сначала получаются данные о материалах, а затем о мэшах. И те, и те инициализируются сразу. В самом конце 
компонент объекта также инициализируется, и идентификатор нового компонента возвращается в сущность.
```c++
uint32_t AssimpModelLoader::LoadModelData(ComPtr<ID3D12GraphicsCommandList2> commandList, const string& filePath, float& OutYOffset)
{
    if (NotFoundFile(filePath.c_str())) return -1;

    int id = ResourceStorage::AddObject3D(filePath);
    auto object = ResourceStorage::GetObject3D(id);

    if (object->IsInitialized())
        return id;

    Assimp::Importer importer;
    
    const aiScene* pScene = importer.ReadFile(filePath.c_str(),
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_CalcTangentSpace |
        aiProcess_FlipUVs |
        aiProcess_PreTransformVertices |
        aiProcess_SortByPType |
        aiProcess_JoinIdenticalVertices
    );

    if (!pScene)
    {
        printf("Error parsing '%s': '%s'\n", filePath.c_str(), importer.GetErrorString());
        return false;
    }

    int meshesCount = pScene->mNumMeshes;
    int materialsCount = pScene->mNumMaterials;

    const size_t last_slash_idx = filePath.rfind('/');
    string directory;
    if (string::npos != last_slash_idx)
    {
        directory = filePath.substr(0, last_slash_idx);
    }

    if(!(meshesCount > 0 && materialsCount > 0)) 
    {
        printf("Error parsing '%s': '%s'\n", filePath.c_str(), importer.GetErrorString());
        return false;
    }

    std::vector<MaterialEntity*> materials;

    for (unsigned int i = 0; i < materialsCount; i++)
    {
        materials.push_back(new MaterialEntity());

        for (int tt = 0; tt <= 27; tt++)
        {
            aiString path;
            aiTextureType texType = (aiTextureType)tt;
            pScene->mMaterials[i]->GetTexture(texType, 0, &path, NULL, NULL, NULL, NULL, NULL);

            string p = path.C_Str();

            if (p != "")
            {
                materials[i]->m_ImagePaths[(TextureType)tt] = directory + "/" + p;
            }
        }

        materials[i]->Load(commandList);
    }

    std::vector<Mesh3DComponent*> meshes;

    float yOffset = 0.0f;

    for (unsigned int i = 0; i < meshesCount; i++)
    {
        meshes.push_back(new Mesh3DComponent());

        const aiMesh* paiMesh = pScene->mMeshes[i];

        vector<VertexStruct> Vertices;
        vector<WORD> Indices;

        const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);
        for (unsigned int i = 0; i < paiMesh->mNumVertices; i++)
        {
            const aiVector3D* pPos = &(paiMesh->mVertices[i]);
            const aiVector3D* pNormal = paiMesh->HasNormals() ? &(paiMesh->mNormals[i]) : &Zero3D;
            const aiVector3D* pTexCoord = paiMesh->HasTextureCoords(0) ? &(paiMesh->mTextureCoords[0][i]) : &Zero3D;
            const aiVector3D* pTangent = &Zero3D;
            const aiVector3D* pBitangent = &Zero3D;
            if (paiMesh->HasTangentsAndBitangents())
            {
                pTangent = &(paiMesh->mTangents[i]);
                pBitangent = &(paiMesh->mBitangents[i]);
            }

            VertexStruct v({
                XMFLOAT3(pPos->x, pPos->y, pPos->z),
                XMFLOAT3(pNormal->x, pNormal->y, pNormal->z),
                XMFLOAT2(pTexCoord->x, pTexCoord->y),
                XMFLOAT3(pTangent->x, pTangent->y, pTangent->z),
                XMFLOAT3(pBitangent->x, pBitangent->y, pBitangent->z)});

            yOffset = pPos->y < yOffset ? pPos->y : yOffset;

            Vertices.push_back(v);
        }

        meshes[i]->Material = materials[paiMesh->mMaterialIndex];

        for (unsigned int i = 0; i < paiMesh->mNumFaces; i++)
        {
            const aiFace& Face = paiMesh->mFaces[i];

            Indices.push_back(Face.mIndices[0]);
            Indices.push_back(Face.mIndices[1]);
            Indices.push_back(Face.mIndices[2]);
        }

        meshes[i]->OnLoad<VertexStruct>(commandList, Vertices, Indices);
    }

    OutYOffset = yOffset;

    object->OnLoad(meshes);

    return id;
}
```
Результат:
![ECS](https://github.com/galeevlxix/DX12GE/blob/master/Progress%20Screenshots/ecs.gif)

<a id="second-part"></a>
## Часть 2: Сериализация и десериализация сцены


<a id="third-part"></a>
## Часть 3: Интеграция скриптового языка




## Last update
Improved SSR  
![SSR](https://github.com/galeevlxix/DX12GE/blob/master/Progress%20Screenshots/ssr%20improved.png)

## Previous updates
Simple SSR + Normal Maps + Emissive Maps  
![simple SSR](https://github.com/galeevlxix/DX12GE/blob/master/Progress%20Screenshots/ssr.png)  

Particle System + Sorting for transparent particles  
![particles](https://github.com/galeevlxix/DX12GE/blob/master/Progress%20Screenshots/particles.png)

Deferred Rendering  
![deferred](https://github.com/galeevlxix/DX12GE/blob/master/Progress%20Screenshots/deferred.png)  

Cascaded Shadow Maps  
![shadows](https://github.com/galeevlxix/DX12GE/blob/master/Progress%20Screenshots/cascade%20shadow%20maps.jpg)  
