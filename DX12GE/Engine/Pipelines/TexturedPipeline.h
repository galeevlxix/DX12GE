#pragma once
#include "../Pipeline.h"

class TexturedPipeline : public Pipeline
{
private:
	const static int InputLayoutCount = 2;
protected:
	D3D12_INPUT_ELEMENT_DESC m_InputLayout[InputLayoutCount];

	virtual void LoadVertexShader() override;
	virtual void LoadPixelShader() override;
	virtual void CreateVertexInputLayout() override;
	virtual void CreateRootSignatureBlob() override;
};