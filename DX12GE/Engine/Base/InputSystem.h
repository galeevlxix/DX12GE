#pragma once
#include "Events.h"

struct inputs
{
    bool W = false;
    bool S = false;
    bool A = false;
    bool D = false;
    bool Q = false;
    bool E = false;

    bool Shift = false;
    bool Ctrl = false;

    bool LBC = false;
    bool RBC = false;
    bool MBC = false;

    void OnKeyPressed(KeyEventArgs& e)
	{
        switch (e.Key)
        {
        case KeyCode::W:
            W = true;
            break;
        case KeyCode::S:
            S = true;
            break;
        case KeyCode::A:
            A = true;
            break;
        case KeyCode::D:
            D = true;
            break;
        case KeyCode::E:
            E = true;
            break;
        case KeyCode::Q:
            Q = true;
            break;
        case KeyCode::ShiftKey:
            Shift = true;
            break;
        case KeyCode::ControlKey:
            Ctrl = true;
            break;
        }
	}

    void OnKeyReleased(KeyEventArgs& e)
	{
        switch (e.Key)
        {
        case KeyCode::W:
            W = false;
            break;
        case KeyCode::S:
            S = false;
            break;
        case KeyCode::A:
            A = false;
            break;
        case KeyCode::D:
            D = false;
            break;
        case KeyCode::E:
            E = false;
            break;
        case KeyCode::Q:
            Q = false;
            break;
        case KeyCode::ShiftKey:
            Shift = false;
            break;
        case KeyCode::ControlKey:
            Ctrl = false;
            break;
        }
	}

    void OnMouseButtonPressed(MouseButtonEventArgs& e)
	{
        switch (e.Button)
        {
        case 1: //Left
            LBC = true;
            break;
        case 2: //Right
            RBC = true;
            break;
        case 3: //Middel
            MBC = true;
            break;
        }
	}

    void OnMouseButtonReleased(MouseButtonEventArgs& e)
	{
        switch (e.Button)
        {
        case 1: //Left
            LBC = false;
            break;
        case 2: //Right
            RBC = false;
            break;
        case 3: //Middel
            MBC = false;
            break;
        }
	}
};