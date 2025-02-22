#include "Pong.h"

bool Racket::CheckXBorder(float dx, bool isLeft)
{
    if (isLeft)
    {
        return !((dx < 0 && GetPosition().X == -borderX) || (dx > 0 && GetPosition().X == -1));
    }
    return !((dx > 0 && GetPosition().X == borderX) || (dx < 0 && GetPosition().X == 1));
}

bool Racket::CheckYBorder(float dy)
{
    return !((dy > 0 && cubes[length - 1].GetPosition().Y == borderY) || (dy < 0 && cubes[0].GetPosition().Y == -borderY));
}

void Racket::OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList, bool left)
{
    float xPos = left ? -borderX : borderX;
    float yPos = 0;

    for (int i = 0; i < length; i++)
    {
        cubes[i].OnLoad(commandList, Vector3(xPos, yPos + i - 2, 10), Vector3(0, 0, 0), Vector3(0.5, 0.5, 0.5), left ? Vector3(1, 0, 0) : Vector3(0, 0, 1));
    }
}

void Racket::OnUpdate(double deltaTime)
{
    for (int i = 0; i < length; i++)
    {
        cubes[i].OnUpdate(deltaTime);
    }
}

void Racket::OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewProjMatrix)
{
    for (int i = 0; i < length; i++)
    {
        cubes[i].OnRender(commandList, viewProjMatrix);
    }
}

void Racket::Move(float dx, float dy)
{
    if (dy != 0 && CheckYBorder(dy))
    {
        for (int i = 0; i < length; i++)
        {
            cubes[i].Move(0, dy, 0);
        }
    }

    if (dx != 0 && CheckXBorder(dx, GetPosition().X < 0))
    {
        for (int i = 0; i < length; i++)
        {
            cubes[i].Move(dx, 0, 0);
        }
    }
}

void Racket::SetStartPosition()
{
    for (int i = 0; i < length; i++)
    {
        cubes[i].SetPosition(GetPosition().X < 0 ? -borderX : borderX, i - 2, cubes[i].GetPosition().Z);
    }
}

Vector3 Racket::GetNewDirection(Vector3 ballPos, Vector3 direction)
{
    float coef = ballPos.Y - GetPosition().Y;
    Vector3 newDir;
    newDir.Set(-direction.X, coef / 2.5f, direction.Z);
    return newDir;
}

Vector3 Racket::GetPosition()
{
    return cubes[2].GetPosition();
}

void Ball::CheckYBorder()
{
    if ((direction.Y > 0 && cube.GetPosition().Y >= borderY) || (direction.Y < 0 && cube.GetPosition().Y <= -borderY))
    {
        direction.Y = -direction.Y;
    }
}

void Ball::CheckRackets(Racket* left, Racket* right)
{
    if (direction.X < 0 &&
        cube.GetPosition().X <= left->GetPosition().X + 1 &&
        cube.GetPosition().X >= left->GetPosition().X - 1 &&
        cube.GetPosition().Y <= left->GetPosition().Y + 2.5f &&
        cube.GetPosition().Y >= left->GetPosition().Y - 2.5f)
    {
        direction = left->GetNewDirection(cube.GetPosition(), direction);
    }
    else if (direction.X > 0 &&
        cube.GetPosition().X >= right->GetPosition().X - 1 &&
        cube.GetPosition().X <= right->GetPosition().X + 1 &&
        cube.GetPosition().Y <= right->GetPosition().Y + 2.5f &&
        cube.GetPosition().Y >= right->GetPosition().Y - 2.5f)
    {
        direction = right->GetNewDirection(cube.GetPosition(), direction);
    }
    else return;

    speed += SpeedIcrease;
}

int GetRandomNumber(int start, int end)
{
    return rand() % (end - start + 1) + start;
}

Vector3 GetRandomDirection()
{
    Vector3 out;
    float randNum = (float)GetRandomNumber(-100, 100);
    randNum /= 100.0;
    bool randNum2 = GetRandomNumber(-100, 100) < 0 ? false : true;
    out.Set(randNum2 ? 1 : -1, randNum, 0);
    return out;
}

void Ball::CheckXBorder(Racket* left, Racket* right)
{
    static int leftScore = 0;
    static int rightScore = 0;

    if (direction.X < 0 && cube.GetPosition().X <= -borderX)
    {
        rightScore++;
    }
    else if (direction.X > 0 && cube.GetPosition().X >= borderX)
    {
        leftScore++;
    }
    else return;

    char buffer[512];
    sprintf_s(buffer, "яв╗р %d:%d\n", leftScore, rightScore);
    OutputDebugStringA(buffer);

    left->SetStartPosition();
    right->SetStartPosition();

    cube.SetPosition(0, 0, 10);
    Vector3 newDir = GetRandomDirection();
    direction.Set(newDir.X, newDir.Y, newDir.Z);

    speed = InitialSpeed;
}

void Ball::OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    srand(time(0));
    cube.OnLoad(commandList, Vector3(0, 0, 10), Vector3(0, 0, 0), Vector3(0.5, 0.5, 0.5), Vector3::One());
    Vector3 newDir = GetRandomDirection();
    direction.Set(newDir.X, newDir.Y, newDir.Z);
}

void Ball::OnUpdate(double deltaTime, Racket* left, Racket* right)
{
    CheckYBorder();
    CheckRackets(left, right);
    CheckXBorder(left, right);
    Vector3 moveVec = direction * speed * deltaTime;
    cube.Move(moveVec.X, moveVec.Y, moveVec.Z);
    cube.OnUpdate(deltaTime);
}

void Ball::OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewProjMatrix)
{
    cube.OnRender(commandList, viewProjMatrix);
}

void Wall::OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    float centerX = 0;
    float centerY = 0;

    float colorStep = 0.5 / ((borderX + 1) * 2 + 1);

    for (int i = 0; i < ((borderX + 1) * 2 + 1) * 2; i += 2)
    {
        cubes[i].OnLoad(commandList, Vector3(centerX - i / 2 + (borderX + 1), borderY + 1, 10), Vector3(0, 0, 0), Vector3(0.25, 0.25, 0.25), Vector3(0 + colorStep * i / 2, 0, 0.5 - colorStep * i / 2));
        cubes[i + 1].OnLoad(commandList, Vector3(centerX - i / 2 + (borderX + 1), -borderY - 1, 10), Vector3(0, 0, 0), Vector3(0.25, 0.25, 0.25), Vector3(0 + colorStep * i / 2, 0, 0.5 - colorStep * i / 2));
    }

    for (int i = 0; i < (borderY * 2 + 1) * 2; i += 2)
    {
        cubes[78 + i].OnLoad(commandList, Vector3(-borderX - 1, centerY - i / 2 + borderY, 10), Vector3(0, 0, 0), Vector3(0.25, 0.25, 0.25), Vector3(0.5, 0, 0));
        cubes[78 + i + 1].OnLoad(commandList, Vector3(borderX + 1, centerY - i / 2 + borderY, 10), Vector3(0, 0, 0), Vector3(0.25, 0.25, 0.25), Vector3(0, 0, 0.5));
    }
}

void Wall::OnUpdate(double deltaTime)
{
    for (int i = 0; i < length; i++)
    {
        cubes[i].OnUpdate(deltaTime);
    }
}

void Wall::OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewProjMatrix)
{
    for (int i = 0; i < length; i++)
    {
        cubes[i].OnRender(commandList, viewProjMatrix);
    }
}

void PongGame::OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    lRacket.OnLoad(commandList, true);
    rRacket.OnLoad(commandList, false);
    ball.OnLoad(commandList);
    wall.OnLoad(commandList);
}

void PongGame::OnUpdate(double deltaTime)
{
    lRacket.OnUpdate(deltaTime);
    rRacket.OnUpdate(deltaTime);
    ball.OnUpdate(deltaTime, &lRacket, &rRacket);
    wall.OnUpdate(deltaTime);
}

void PongGame::OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewProjMatrix)
{
    lRacket.OnRender(commandList, viewProjMatrix);
    rRacket.OnRender(commandList, viewProjMatrix);
    ball.OnRender(commandList, viewProjMatrix);
    wall.OnRender(commandList, viewProjMatrix);
}

void PongGame::OnKeyPressed(KeyEventArgs& e)
{
    switch (e.Key)
    {
    case KeyCode::W:
        lRacket.Move(0, 1);
        break;
    case KeyCode::S:
        lRacket.Move(0, -1);
        break;
    case KeyCode::A:
        lRacket.Move(-1, 0);
        break;
    case KeyCode::D:
        lRacket.Move(1, 0);
        break;

    case KeyCode::Up:
        rRacket.Move(0, 1);
        break;
    case KeyCode::Down:
        rRacket.Move(0, -1);
        break;
    case KeyCode::Left:
        rRacket.Move(-1, 0);
        break;
    case KeyCode::Right:
        rRacket.Move(1, 0);
        break;
    }
}
