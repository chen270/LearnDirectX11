#include "graphics2D.h"
#include "bindableBase.h"

Graphics2D::Graphics2D(int w, int h): m_screenWidth(w), m_screenHeight(h)
{
	ratio_hw = (float)m_screenHeight / (float)m_screenWidth;
}

void Graphics2D::InitTriangle(D3dClass& d3d)
{
	std::vector<VertexWithColor> vertices =			// 顶点数组
	{
		{DirectX::XMFLOAT3(0.0f, 0.3f, 0.3f),{255, 0, 0,  1} },
		{DirectX::XMFLOAT3(0.3f, -0.3f, 0.3f),{0, 255, 0, 1} },
		{DirectX::XMFLOAT3(-0.3f, -0.3f, 0.3f),{0, 0, 255, 1} },
	};

	AddBind(std::make_unique<VertexBuffer>(d3d, vertices));//顶点缓冲区

	//5.创建 shader
	auto pvs = std::make_unique<VertexShader>(d3d, L"../bin/vs.cso");
	auto pvsbc = pvs->GetBytecode();

	AddBind(std::move(pvs));//顶点着色器

	AddBind(std::make_unique<PixelShader>(d3d, L"../bin/ps.cso"));//像素着色器

	const std::vector<unsigned short> indices =
	{
		0,1,2,
	};
	AddIndexBuffer(std::make_unique<IndexBuffer>(d3d, indices));//索引缓冲区


	//7.告诉CPU如何从shader中读取数据
	//在 DirectX 代码中创建一个 InputLayout 来描述 input-assembler 阶段的数据。
	const std::vector<D3D11_INPUT_ELEMENT_DESC> pInputLayout = {
		{"POSITIONT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, sizeof(float) * 3, D3D11_INPUT_PER_VERTEX_DATA, 0},//UNORM归一化
	};
	AddBind(std::make_unique<InputLayout>(d3d, pInputLayout, pvsbc));//绑定layout

	AddBind(std::make_unique<Topology>(d3d, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));

	AddBind(std::make_unique<TransformCbuf>(d3d, *this));

	graph2DTransform = DirectX::XMMatrixScaling(ratio_hw, 1.0f, 1.0f)* DirectX::XMMatrixTranslation(0, 0.0f, 0 + 1.0f);
}

void Graphics2D::Update(float dt) noexcept
{
	angle = dt;
}

DirectX::XMMATRIX Graphics2D::GetTransformXM() const noexcept
{
	return 	DirectX::XMMatrixRotationZ(angle) * DirectX::XMMatrixRotationX(angle) * graph2DTransform;
}

void Graphics2D::InitCube(D3dClass& d3d)
{
	std::vector<VertexWithColor> vertices  =			// 顶点数组
	{
		{DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f),{255, 0, 0,  1} },
		{DirectX::XMFLOAT3(1.0f, -1.0f, -1.0f),{0, 255, 0,  1} },
		{DirectX::XMFLOAT3(-1.0f,  1.0f, -1.0f),{0, 0, 255,  1} },
		{DirectX::XMFLOAT3(1.0f,  1.0f, -1.0f),{255, 255, 0,  1} },
		{DirectX::XMFLOAT3(-1.0f, -1.0f,  1.0f),{255, 0, 255,  1} },
		{DirectX::XMFLOAT3(1.0f, -1.0f,  1.0f),{0, 255, 255,  1} },
		{DirectX::XMFLOAT3(-1.0f,  1.0f,  1.0f),{0, 0, 0,  1} },
		{DirectX::XMFLOAT3(1.0f,  1.0f,  1.0f),{255, 255, 255,  1} },
	};

	AddBind(std::make_unique<VertexBuffer>(d3d, vertices));//顶点缓冲区

	//5.创建 shader
	auto pvs = std::make_unique<VertexShader>(d3d, L"../bin/vs.cso");
	auto pvsbc = pvs->GetBytecode();

	AddBind(std::move(pvs));//顶点着色器

	AddBind(std::make_unique<PixelShader>(d3d, L"../bin/ps.cso"));//像素着色器

	const std::vector<unsigned short> indices =
	{
		0,2,1, 2,3,1,
		1,3,5, 3,7,5,
		2,6,3, 3,6,7,
		4,5,7, 4,7,6,
		0,4,2, 2,4,6,
		0,1,4, 1,5,4
	};
	AddIndexBuffer(std::make_unique<IndexBuffer>(d3d, indices));//索引缓冲区


	//7.告诉CPU如何从shader中读取数据
	//在 DirectX 代码中创建一个 InputLayout 来描述 input-assembler 阶段的数据。
	const std::vector<D3D11_INPUT_ELEMENT_DESC> pInputLayout = {
		{"POSITIONT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, sizeof(float) * 3, D3D11_INPUT_PER_VERTEX_DATA, 0},//UNORM归一化
	};
	AddBind(std::make_unique<InputLayout>(d3d, pInputLayout, pvsbc));//绑定layout

	AddBind(std::make_unique<Topology>(d3d, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));

	AddBind(std::make_unique<TransformCbuf>(d3d, *this));

	graph2DTransform = DirectX::XMMatrixTranslation(0, 0.0f, 0 + 4.0f);
}