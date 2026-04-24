#pragma once

#include "../Engine/MultiGpuGame.h"
#include "../Engine/SingleGpuGame.h"

/*

SingleGPU 1920x1080:
Среднее время:      0,145039 sec
        Shadow:     0,018366 sec
        Geometry:   0,012104 sec
        Lighting:   0,089375 sec
        SSR:        0,024780 sec
        Merging:    0,000393 sec

MultiGPU 1920x1080:
Среднее время:      0,177181 sec
        Shadow:     0,014212 sec
        Geometry:   0,012249 sec
        Lighting:   0,088148 sec
        SSR:        0,073450 sec
        Merging:    0,000858 sec
        CopyPrimaryDeviceDataToSharedMemory:    0,001053 sec
        CopySharedMemoryDataToPrimaryDevice:    0,000002 sec
        CopySecondDeviceDataToSharedMemory:     0,000001 sec
        CopySharedMemoryDataToSecondDevice:     0,004115 sec

SingleGPU 500x500:
Среднее время: 0,040834 sec
        Shadow: 0,016910 sec
        Geometry: 0,009332 sec
        Lighting: 0,011270 sec
        SSR: 0,002995 sec
        Merging: 0,000312 sec

MultiGPU 500x500:
Среднее время: 0,037864 sec
        Shadow: 0,004530 sec
        Geometry: 0,009163 sec
        Lighting: 0,011254 sec
        SSR: 0,010536 sec
        Merging: 0,000348 sec
        CopyPrimaryDeviceDataToSharedMemory: 0,000218 sec
        CopySharedMemoryDataToPrimaryDevice: 0,000045 sec
        CopySecondDeviceDataToSharedMemory: 0,000065 sec
        CopySharedMemoryDataToSecondDevice: 0,000049 sec
        WaitPrimaryDeviceDataToSharedMemory: 0,000018 sec
        WaitSharedMemoryDataToPrimaryDevice: 0,000001 sec
        WaitSecondDeviceDataToSharedMemory: 0,000001 sec
        WaitSharedMemoryDataToSecondDevice: 0,001588 sec

Среднее время: 0,038599 sec
        Shadow: 0,004839 sec
        Geometry: 0,009269 sec
        Lighting: 0,011335 sec
        SSR: 0,010681 sec
        Merging: 0,000389 sec
        CopyPrimaryDeviceDataToSharedMemory: 0,000252 sec
        CopySharedMemoryDataToPrimaryDevice: 0,000044 sec
        CopySecondDeviceDataToSharedMemory: 0,000071 sec
        CopySharedMemoryDataToSecondDevice: 0,000056 sec
        WaitPrimaryDeviceDataToSharedMemory: 0,000034 sec
        WaitSharedMemoryDataToPrimaryDevice: 0,000001 sec
        WaitSecondDeviceDataToSharedMemory: 0,000001 sec
        WaitSharedMemoryDataToSecondDevice: 0,001620 sec
*/


class GameSample : public MultiGpuGame
{
public:
    using super = MultiGpuGame;

	GameSample(const wstring& name, int width, int height, bool vSync = false);
	~GameSample();

	virtual bool LoadContent() override;
	virtual void UnloadContent() override;
	virtual void Destroy() override;

protected:
    virtual void OnUpdate(UpdateEventArgs& e) override;

    virtual void OnKeyPressed(KeyEventArgs& e) override;
    virtual void OnKeyReleased(KeyEventArgs& e) override;
    virtual void OnMouseWheel(MouseWheelEventArgs& e) override;
    virtual void OnMouseMoved(MouseMotionEventArgs& e) override;
    virtual void OnMouseButtonPressed(MouseButtonEventArgs& e) override;
    virtual void OnMouseButtonReleased(MouseButtonEventArgs& e) override;
    virtual void OnResize(ResizeEventArgs& e) override;
};