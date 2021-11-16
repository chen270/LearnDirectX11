#include "skinned_box.h"
#include "bindableBase.h"
#include "misc/DXTrace.h"
#include <memory>

SkinnedBox::SkinnedBox(D3dClass& d3d,
	std::mt19937& rng,
	std::uniform_real_distribution<float>& adist,
	std::uniform_real_distribution<float>& ddist,
	std::uniform_real_distribution<float>& odist,
	std::uniform_real_distribution<float>& rdist)
	:
	r(rdist(rng)),
	droll(ddist(rng)),
	dpitch(ddist(rng)),
	dyaw(ddist(rng)),
	dphi(odist(rng)),
	dtheta(odist(rng)),
	dchi(odist(rng)),
	chi(adist(rng)),
	theta(adist(rng)),
	phi(adist(rng))
{
	if (!IsStaticInitialized())
	{
		struct Vertex
		{
			DirectX::XMFLOAT3 pos;
			struct
			{
				float u;
				float v;
			} tex;
		};
		std::vector<Vertex> vertices;
		std::vector<unsigned short> indices;
		this->MakeSkinned(vertices, indices);

		AddStaticBind(std::make_unique<VertexBuffer>(d3d, vertices));

		AddStaticBind(std::make_unique<Texture>(d3d, L"../data/cube.png"));

		auto pvs = std::make_unique<VertexShader>(d3d, L"../bin/tex_vs.cso");
		auto pvsbc = pvs->GetBytecode();
		AddStaticBind(std::move(pvs));

		AddStaticBind(std::make_unique<PixelShader>(d3d, L"../bin/tex_ps.cso"));

		AddStaticIndexBuffer(std::make_unique<IndexBuffer>(d3d, indices));

		struct ConstantBuffer2
		{
			struct
			{
				float r;
				float g;
				float b;
				float a;
			} face_colors[6];
		};
		const ConstantBuffer2 cb2 =
		{
			{
				{ 1.0f,0.0f,1.0f },
				{ 1.0f,0.0f,0.0f },
				{ 0.0f,1.0f,0.0f },
				{ 0.0f,0.0f,1.0f },
				{ 1.0f,1.0f,0.0f },
				{ 0.0f,1.0f,1.0f },
			}
		};
		AddStaticBind(std::make_unique<PixelConstantBuffer<ConstantBuffer2>>(d3d, cb2));

		const std::vector<D3D11_INPUT_ELEMENT_DESC> ied =
		{
			{ "Position",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0 },
			{ "TexCoord",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0 },
		};
		AddStaticBind(std::make_unique<InputLayout>(d3d, ied, pvsbc));

		AddStaticBind(std::make_unique<Topology>(d3d, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
	}
	else
	{
		SetIndexFromStatic();//取出静态的 IndexBuffer,然后做相应的变化即可
	}

	AddBind(std::make_unique<TransformCbuf>(d3d, *this));
}

void SkinnedBox::Update(float dt) noexcept
{
	roll += droll * dt;
	pitch += dpitch * dt;
	yaw += dyaw * dt;
	theta += dtheta * dt;
	phi += dphi * dt;
	chi += dchi * dt;
}

DirectX::XMMATRIX SkinnedBox::GetTransformXM() const noexcept
{
	return DirectX::XMMatrixRotationRollPitchYaw(pitch, yaw, roll) *
		DirectX::XMMatrixTranslation(r, 0.0f, 0.0f) *
		DirectX::XMMatrixRotationRollPitchYaw(theta, phi, chi) *
		DirectX::XMMatrixTranslation(0.0f, 0.0f, 20.0f);
}

template<typename V>
void SkinnedBox::MakeSkinned(std::vector<V>& vertices, std::vector<unsigned short>& indices)
{
	namespace dx = DirectX;

	constexpr float side = 1.0f / 2.0f;

	//std::vector<V> vertices(14);
	vertices.resize(14);

	vertices[0].pos = { -side,-side,-side };
	vertices[0].tex = { 2.0f / 3.0f,0.0f / 4.0f };
	vertices[1].pos = { side,-side,-side };
	vertices[1].tex = { 1.0f / 3.0f,0.0f / 4.0f };
	vertices[2].pos = { -side,side,-side };
	vertices[2].tex = { 2.0f / 3.0f,1.0f / 4.0f };
	vertices[3].pos = { side,side,-side };
	vertices[3].tex = { 1.0f / 3.0f,1.0f / 4.0f };
	vertices[4].pos = { -side,-side,side };
	vertices[4].tex = { 2.0f / 3.0f,3.0f / 4.0f };
	vertices[5].pos = { side,-side,side };
	vertices[5].tex = { 1.0f / 3.0f,3.0f / 4.0f };
	vertices[6].pos = { -side,side,side };
	vertices[6].tex = { 2.0f / 3.0f,2.0f / 4.0f };
	vertices[7].pos = { side,side,side };
	vertices[7].tex = { 1.0f / 3.0f,2.0f / 4.0f };
	vertices[8].pos = { -side,-side,-side };
	vertices[8].tex = { 2.0f / 3.0f,4.0f / 4.0f };
	vertices[9].pos = { side,-side,-side };
	vertices[9].tex = { 1.0f / 3.0f,4.0f / 4.0f };
	vertices[10].pos = { -side,-side,-side };
	vertices[10].tex = { 3.0f / 3.0f,1.0f / 4.0f };
	vertices[11].pos = { -side,-side,side };
	vertices[11].tex = { 3.0f / 3.0f,2.0f / 4.0f };
	vertices[12].pos = { side,-side,-side };
	vertices[12].tex = { 0.0f / 3.0f,1.0f / 4.0f };
	vertices[13].pos = { side,-side,side };
	vertices[13].tex = { 0.0f / 3.0f,2.0f / 4.0f };


	std::vector<unsigned short> ans = {
			0,2,1,   2,3,1,
			4,8,5,   5,8,9,
			2,6,3,   3,6,7,
			4,5,7,   4,7,6,
			2,10,11, 2,11,6,
			12,3,7,  12,7,13
	};
	ans.swap(indices);
	return;
}