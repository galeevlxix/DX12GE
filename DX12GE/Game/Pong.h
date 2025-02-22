#pragma once
#include "BaseObject.h"
#include "../Engine/Events.h"

const static float borderX = 18.0f;
const static float borderY = 10.0f;

class Racket
{
private:
    const static int length = 5;
    BaseObject cubes[length];

    bool CheckXBorder(float dx, bool isLeft);
    bool CheckYBorder(float dy);
public:
    void OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList, bool left);
    void OnUpdate(double deltaTime);
    void OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewProjMatrix);
    void Move(float dx, float dy);
    void SetStartPosition();
    Vector3 GetNewDirection(Vector3 ballPos, Vector3 direction);
    Vector3 GetPosition();
};

class Ball
{
private:
    BaseObject cube;

    const float InitialSpeed = 8.0;
    const float SpeedIcrease = 1.0;

    float speed = InitialSpeed;
    Vector3 direction;

    void CheckYBorder();
    void CheckRackets(Racket* left, Racket* right);
    void CheckXBorder(Racket* left, Racket* right);
public:
    void OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList);
    void OnUpdate(double deltaTime, Racket* left, Racket* right);
    void OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewProjMatrix);
};

class Wall
{
private:
    const static int length = ((borderX + 1) * 2 + 1 + borderY * 2 + 1) * 2; //98
    BaseObject cubes[length];

public:
    void OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList);
    void OnUpdate(double deltaTime);
    void OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewProjMatrix);
};

class PongGame
{
private:
    Racket lRacket;
    Racket rRacket;
    Ball ball;
    Wall wall;
public:
    void OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList);
    void OnUpdate(double deltaTime);
    void OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewProjMatrix);
    void OnKeyPressed(KeyEventArgs& e);
};