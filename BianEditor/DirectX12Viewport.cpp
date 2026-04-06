#include "DirectX12Viewport.h"
#include <QDebug>

DirectX12Viewport::DirectX12Viewport(QWidget* parent) : QWidget(parent), m_Width(420), m_Height(228), m_EngineWindow(nullptr), m_renderTimer(new QTimer(this))
{
    qDebug() << "viewport initializing";

    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_NativeWindow);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAutoFillBackground(false);

    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    connect(m_renderTimer, &QTimer::timeout, [this]()
        {
            if (!m_EngineWindow) return;

            MSG msg;
            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) 
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }

            UpdateEventArgs updateEventArgs(0.0f, 0.0f);
            m_EngineWindow->OnUpdate(updateEventArgs);

            RenderEventArgs renderEventArgs(0.0f, 0.0f);
            m_EngineWindow->OnRender(renderEventArgs);
        });

    m_renderTimer->start(0.01);
}

DirectX12Viewport::~DirectX12Viewport()
{
    qDebug() << "viewport destroying";

    m_EngineWindow.reset();
    m_EngineWindow = nullptr;
}

HWND DirectX12Viewport::GetHWND() const
{
    return reinterpret_cast<HWND>(winId());
}

void DirectX12Viewport::SetEngineWindow(std::shared_ptr<Window> window)
{
    m_EngineWindow = window;
}

void DirectX12Viewport::resizeEvent(QResizeEvent* event)
{
    const QSize oldSize = event->oldSize();
    const QSize size = event->size();
    qDebug() << "resizing viewport from " << oldSize.width() << "x" << oldSize.height() << " to " << size.width() << "x" << size.height();

    m_Width = size.width();
    m_Height = size.height();

    if (!m_EngineWindow) return;

    ResizeEventArgs resizeEventArgs(m_Width, m_Height);
    m_EngineWindow->OnResize(resizeEventArgs);
}

void DirectX12Viewport::keyPressEvent(QKeyEvent* event)
{
    bool shift = event->modifiers() == Qt::KeyboardModifier::ShiftModifier;
    bool control = event->modifiers() == Qt::KeyboardModifier::ControlModifier;
    bool alt = event->modifiers() == Qt::KeyboardModifier::AltModifier;

    quint32 code = event->nativeVirtualKey();

    KeyCode::Key key = (KeyCode::Key)code;
    KeyEventArgs keyEventArgs(key, code, KeyEventArgs::Pressed, shift, control, alt);

    m_EngineWindow->OnKeyPressed(keyEventArgs);
}

void DirectX12Viewport::keyReleaseEvent(QKeyEvent* event)
{
    bool shift = event->modifiers() == Qt::KeyboardModifier::ShiftModifier;
    bool control = event->modifiers() == Qt::KeyboardModifier::ControlModifier;
    bool alt = event->modifiers() == Qt::KeyboardModifier::AltModifier;

    quint32 code = event->nativeVirtualKey();

    KeyCode::Key key = (KeyCode::Key)code;
    KeyEventArgs keyEventArgs(key, code, KeyEventArgs::Released, shift, control, alt);

    m_EngineWindow->OnKeyReleased(keyEventArgs);
}

static MouseButtonEventArgs::MouseButton DecodeMouseButton(bool lButton, bool mButton, bool rButton)
{
    if (lButton) return MouseButtonEventArgs::MouseButton::Left;
    if (rButton) return MouseButtonEventArgs::MouseButton::Right;
    if (mButton) return MouseButtonEventArgs::MouseButton::Middel;
    return MouseButtonEventArgs::MouseButton::None;
}

void DirectX12Viewport::mouseMoveEvent(QMouseEvent* event)
{
    if (!m_EngineWindow) return;

    bool lButton = event->button() == Qt::MouseButton::LeftButton;
    bool rButton = event->button() == Qt::MouseButton::RightButton;
    bool mButton = event->button() == Qt::MouseButton::MiddleButton;
    bool shift = event->modifiers() == Qt::KeyboardModifier::ShiftModifier;
    bool control = event->modifiers() == Qt::KeyboardModifier::ControlModifier;

    auto pos = event->position();
    int x = pos.x();
    int y = pos.y();

    MouseMotionEventArgs mouseMotionEventArgs(lButton, mButton, rButton, control, shift, x, y);
    m_EngineWindow->OnMouseMoved(mouseMotionEventArgs);
}

void DirectX12Viewport::mousePressEvent(QMouseEvent* event)
{
    if (!m_EngineWindow) return;

    bool lButton = event->button() == Qt::MouseButton::LeftButton;
    bool rButton = event->button() == Qt::MouseButton::RightButton;
    bool mButton = event->button() == Qt::MouseButton::MiddleButton;
    bool shift = event->modifiers() == Qt::KeyboardModifier::ShiftModifier;
    bool control = event->modifiers() == Qt::KeyboardModifier::ControlModifier;

    auto pos = event->position();
    int x = pos.x();
    int y = pos.y();

    MouseButtonEventArgs::MouseButton button = DecodeMouseButton(lButton, mButton, rButton);

    MouseButtonEventArgs mouseButtonEventArgs(button, MouseButtonEventArgs::Pressed, lButton, mButton, rButton, control, shift, x, y);
    m_EngineWindow->OnMouseButtonPressed(mouseButtonEventArgs);
}

void DirectX12Viewport::mouseReleaseEvent(QMouseEvent* event)
{
    if (!m_EngineWindow) return;

    bool lButton = event->button() == Qt::MouseButton::LeftButton;
    bool rButton = event->button() == Qt::MouseButton::RightButton;
    bool mButton = event->button() == Qt::MouseButton::MiddleButton;
    bool shift = event->modifiers() == Qt::KeyboardModifier::ShiftModifier;
    bool control = event->modifiers() == Qt::KeyboardModifier::ControlModifier;

    auto pos = event->position();
    int x = pos.x();
    int y = pos.y();

    MouseButtonEventArgs::MouseButton button = DecodeMouseButton(lButton, mButton, rButton);

    MouseButtonEventArgs mouseButtonEventArgs(button, MouseButtonEventArgs::Released, lButton, mButton, rButton, control, shift, x, y);
    m_EngineWindow->OnMouseButtonReleased(mouseButtonEventArgs);
}

void DirectX12Viewport::wheelEvent(QWheelEvent* event)
{
    if (!m_EngineWindow) return;

    float zDelta = event->angleDelta().y() / (float)WHEEL_DELTA;

    bool lButton = event->button() == Qt::MouseButton::LeftButton;
    bool rButton = event->button() == Qt::MouseButton::RightButton;
    bool mButton = event->button() == Qt::MouseButton::MiddleButton;
    bool shift = event->modifiers() == Qt::KeyboardModifier::ShiftModifier;
    bool control = event->modifiers() == Qt::KeyboardModifier::ControlModifier;

    auto pos = event->position();
    int x = pos.x();
    int y = pos.y();

    MouseWheelEventArgs mouseWheelEventArgs(zDelta, lButton, mButton, rButton, control, shift, x, y);
    m_EngineWindow->OnMouseWheel(mouseWheelEventArgs);
}
