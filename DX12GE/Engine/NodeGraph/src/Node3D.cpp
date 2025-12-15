#include "../../Base/Singleton.h"

Node3D::Node3D() : m_Parrent(nullptr), m_ComponentId(-1)
{
    m_WorldMatrixCache = DirectX::XMMATRIX();
    Transform.SetDefault();
    Rename("Node3D");
}

const std::string Node3D::GetNodePath()
{
    std::string nodePath = m_Name;
    Node3D* parrent = m_Parrent;
    while (parrent)
    {
        nodePath = parrent->GetName() + "/" + nodePath;
        parrent = parrent->GetParrent();
    }
    return nodePath;
}

void SetTransformCacheStatus(TransformComponent& transform, bool cacheIsDirty)
{
    transform.m_CacheIsDirty = cacheIsDirty;
}

void Node3D::OnUpdate(const double& deltaTime)
{
    if (Transform.IsCacheDirty())
    {
        m_WorldMatrixCache = Transform.GetLocalMatrix();

        if (m_Parrent)
        { 
            m_WorldMatrixCache = m_WorldMatrixCache * m_Parrent->GetWorldMatrix();
        }

        for (auto child : m_Children)
        {
            child.second->NotifyParrentChanged();
        }

        SetTransformCacheStatus(Transform, false);
    }

    for (auto child : m_Children)
    {
        child.second->OnUpdate(deltaTime);
    }
}

void Node3D::Destroy(bool keepComponent)
{
    auto children = GetChildren();

    for (auto child : children)
    {
        child->Destroy(keepComponent);
        delete child;
    }

    if (m_Parrent)
    {
        m_Parrent->RemoveChild(m_Name);
        m_Parrent = nullptr;
    }

    m_ComponentId = -1;
}

const DirectX::XMMATRIX& Node3D::GetWorldMatrix()
{
    return m_WorldMatrixCache;
}

void Node3D::NotifyParrentChanged()
{
    SetTransformCacheStatus(Transform, true);
}

void Node3D::Rename(const std::string& name)
{
    if (name.find('/') != std::string::npos || name.find('%') != std::string::npos) throw;
    
    std::string newName = name;
    if (m_Parrent)
    {
        int number = 2;
        while (m_Parrent->HasChild(newName))
        {
            newName = name + " " + std::to_string(number);
            number++;
        }

        if (m_Parrent->GetChild(m_Name) == this)
        {
            m_Parrent->RenameChild(m_Name, newName);
        }        
    } 
    m_Name = newName;
}

void Node3D::RenameChild(const std::string& oldName, const std::string& newName)
{
    auto pair = m_Children.find(oldName);
    if (pair == m_Children.end())
        return;

    m_Children[newName] = pair->second;
    m_Children.erase(pair);
}

bool Node3D::IsInsideTree()
{
    if (!m_Parrent)
    {
        return this == Singleton::GetNodeGraph()->GetRoot();
    }
    return m_Parrent->IsInsideTree();
}

Node3D* Node3D::GetChild(const std::string& name)
{
    auto pair = m_Children.find(name);
    return pair != m_Children.end() ? pair->second : nullptr;
}

const std::vector<Node3D*> Node3D::GetChildren()
{
    std::vector<Node3D*> nodes;
    for (auto child : m_Children)
    {
        nodes.emplace_back(child.second);
    }
    return nodes;
}

const std::vector<std::string> Node3D::GetChildrenNames()
{
    std::vector<std::string> names;
    for (auto child : m_Children)
    {
        names.emplace_back(child.first);
    }
    return names;
}

Node3D* Node3D::GetParrent()
{
    return m_Parrent;
}

Node3D* Node3D::FindNodeRecursive(const std::string& name)
{
    if (m_Name == name)
    {
        return this;
    }

    for (auto child : m_Children)
    {
        Node3D* found = child.second->FindNodeRecursive(name);
        if (found)
        {
            return found;
        }
    }
    return nullptr;
}

Node3D* Node3D::FindParrentRecursive(const std::string& name)
{
    if (m_Parrent)
    {
        if (m_Parrent->GetName() == name)
        {
            return m_Parrent;
        }
        return m_Parrent->FindParrentRecursive(name);
    }

    return nullptr;
}

bool Node3D::AddChild(Node3D* node)
{
    if (!node || node == this || node->m_Parrent) return false;

	const std::string name = node->GetName();

    node->m_Parrent = this;
    if (HasChild(name))
    {
        node->Rename(name);
    }
    m_Children[node->GetName()] = node;

    bool result = Singleton::GetNodeGraph()->OnNodeAdded(node);
    if (!result)
    {
        m_Children.erase(node->GetName());
        node->m_Parrent = nullptr;
		node->Rename(name);
        return false;
	}

    return true;
}

bool Node3D::RemoveChild(const std::string& name)
{
    if (HasChild(name))
    {
        Singleton::GetNodeGraph()->OnNodeRemoved(m_Children[name]);
        m_Children.erase(name);
        return true;
    }
    return false;
}

bool Node3D::RemoveChild(Node3D* node)
{
    if (!node) return false;
    return RemoveChild(node->GetName());
}

bool Node3D::HasChild(const std::string& name)
{
    auto pair = m_Children.find(name);
    return pair != m_Children.end();
}

bool Node3D::Move(Node3D* newParrent)
{
    if (!newParrent || newParrent == this || newParrent == m_Parrent || Singleton::GetNodeGraph()->GetRoot() == this) return false;

    if (m_Parrent)
    {
        m_Parrent->RemoveChild(m_Name);
        m_Parrent = nullptr;
    }
    return newParrent->AddChild(this);
}

void Node3D::Clone(Node3D* cloneNode, Node3D* parrent, bool cloneChildrenRecursive)
{
    if (!cloneNode)
    {
        cloneNode = new Node3D();
    }

    cloneNode->Rename(m_Name);

    cloneNode->Transform.SetPosition(Transform.GetPosition());
    cloneNode->Transform.SetRotation(Transform.GetRotation());
    cloneNode->Transform.SetScale(Transform.GetScale());

    cloneNode->m_ComponentId = m_ComponentId;

    if (parrent)
    {
        if (!parrent->AddChild(cloneNode))
        {
            cloneNode = nullptr;
            return;
        }
    }

    if (cloneChildrenRecursive)
    {
        for (auto child : m_Children)
        {
            Node3D* cloneChild = nullptr;
            child.second->Clone(cloneChild, cloneNode, cloneChildrenRecursive);
        }
    }
}

void Node3D::OnKeyPressed(KeyEventArgs& e)
{
    for (auto child : m_Children)
    {
        child.second->OnKeyPressed(e);
    }
}

void Node3D::OnKeyReleased(KeyEventArgs& e)
{
    for (auto child : m_Children)
    {
        child.second->OnKeyReleased(e);
    }
}

void Node3D::OnMouseWheel(MouseWheelEventArgs& e)
{
    for (auto child : m_Children)
    {
        child.second->OnMouseWheel(e);
    }
}

void Node3D::OnMouseMoved(MouseMotionEventArgs& e)
{
    for (auto child : m_Children)
    {
        child.second->OnMouseMoved(e);
    }
}

void Node3D::OnMouseButtonPressed(MouseButtonEventArgs& e)
{
    for (auto child : m_Children)
    {
        child.second->OnMouseButtonPressed(e);
    }
}

void Node3D::OnMouseButtonReleased(MouseButtonEventArgs& e)
{
    for (auto child : m_Children)
    {
        child.second->OnMouseButtonReleased(e);
    }
}

void Node3D::OnWindowResize(ResizeEventArgs& e)
{
    for (auto child : m_Children)
    {
        child.second->OnWindowResize(e);
    }
}
