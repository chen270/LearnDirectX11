#include "camera.h"
#include "misc/imgui/imgui.h"

void Camera::SpawnControlWindow() noexcept     //使用 Imgui 控制
{
	if (ImGui::Begin("camera"))//定义控制参数范围
	{
		ImGui::Text("Position");
		ImGui::SliderFloat("R", &r, 0.0f, 80.0f, "%.1f");
		ImGui::SliderAngle("Theta", &theta, -180.0f, 180.0f);
		ImGui::SliderAngle("Phi", &phi, -89.0f, 89.0f);
		ImGui::Text("Orientation");

		ImGui::SliderAngle("Roll", &roll, -180.0f, 180.0f);
		ImGui::SliderAngle("Pithch", &pitch, -180.0f, 180.0f);
		ImGui::SliderAngle("Yaw", &yaw, -180.0f, 180.0f);
		
		if (ImGui::Button("Reset"))
		{
			Reset();
		}
	}

	ImGui::End();
}


DirectX::XMMATRIX Camera::GetMatrix() const noexcept //通过参数计算矩阵
{
	//首先计算摄像机位置
	const auto pos = DirectX::XMVector3Transform(DirectX::XMVectorSet(0.0f, 0.0f, -r, 0.0f),
		DirectX::XMMatrixRotationRollPitchYaw(phi, -theta, 0.0f));

	return DirectX::XMMatrixLookAtLH ( pos, DirectX::XMVectorZero(), DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f) )* 
		DirectX::XMMatrixRotationRollPitchYaw(pitch, -yaw, roll);
}


void Camera::Reset() noexcept
{
	r = 20.0f;
	theta = 0.0f;
	phi = 0.0f;
	pitch = 0.0f;
	yaw = 0.0f;
	roll = 0.0f;
}

