#pragma once

#include <QWidget>
#include <QResizeEvent>
#include <Engine/Base/Window.h>
#include <QTimer>

class DirectX12Viewport : public QWidget
{
    int m_Width, m_Height;
    const std::wstring m_Name = L"EditorViewportRender";
    std::shared_ptr<Window> m_EngineWindow;

    QTimer* m_renderTimer;

public:
    DirectX12Viewport(QWidget* parent = nullptr);
    ~DirectX12Viewport();

    HWND GetHWND() const;

    int GetWidth() { return m_Width; }

    int GetHeight() { return m_Height; }

    const std::wstring& GetName() { return m_Name; }    

    void SetEngineWindow(std::shared_ptr<Window> window);

    QPaintEngine* paintEngine() const override { return nullptr; }

protected:
    void resizeEvent(QResizeEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
};

