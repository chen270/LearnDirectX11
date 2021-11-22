#ifndef __SKINNED_BOX_H__
#define __SKINNED_BOX_H__

#include "drawableBase.h"
#include <random>

class SkinnedBox : public DrawableBase<SkinnedBox>
{
public:
	SkinnedBox(D3dClass& d3d, std::mt19937& rng,
		std::uniform_real_distribution<float>& adist,
		std::uniform_real_distribution<float>& ddist,
		std::uniform_real_distribution<float>& odist,
		std::uniform_real_distribution<float>& rdist);
	void Update(float dt) noexcept override;
	DirectX::XMMATRIX GetTransformXM() const noexcept override;
private:
	// positional
	float r;
	float roll = 0.0f;
	float pitch = 0.0f;
	float yaw = 0.0f;
	float theta;
	float phi;
	float chi;
	// speed (delta/s)
	float droll;
	float dpitch;
	float dyaw;
	float dtheta;
	float dphi;
	float dchi;

private:
	template<typename V>
	void MakeSkinned(std::vector<V>& vertices, std::vector<unsigned short>& indices);
};

#endif // __SKINNED_BOX_H__
