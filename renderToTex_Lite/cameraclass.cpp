////////////////////////////////////////////////////////////////////////////////
// Filename: cameraclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "cameraclass.h"


CameraClass::CameraClass()
{
	m_positionX = 0.0f;
	m_positionY = 0.0f;
	m_positionZ = 0.0f;

	m_rotationX = 0.0f;
	m_rotationY = 0.0f;
	m_rotationZ = 0.0f;
}


CameraClass::CameraClass(const CameraClass& other)
{
}


CameraClass::~CameraClass()
{
}


void CameraClass::SetPosition(float x, float y, float z)
{
	m_positionX = x;
	m_positionY = y;
	m_positionZ = z;
	return;
}


void CameraClass::SetRotation(float x, float y, float z)
{
	m_rotationX = x;
	m_rotationY = y;
	m_rotationZ = z;
	return;
}


void CameraClass::Render()
{
	DirectX::XMVECTOR up, position, lookAt;
	float radians;


	// Setup the vector that points upwards.
	//up.x = 0.0f;
	//up.y = 1.0f;
	//up.z = 0.0f;
	DirectX::XMFLOAT3 p1(0.0f, 1.0f, 0.0f);
	up = DirectX::XMLoadFloat3(&p1);

	// Setup the position of the camera in the world.
	//position.x = m_positionX;
	//position.y = m_positionY;
	//position.z = m_positionZ;
	DirectX::XMFLOAT3 p2(m_positionX, m_positionY, m_positionZ);
	position = DirectX::XMLoadFloat3(&p2);

	// Calculate the rotation in radians.
	radians = m_rotationY * 0.0174532925f;

	// Setup where the camera is looking.
	//lookAt.x = sinf(radians) + m_positionX;
	//lookAt.y = m_positionY;
	//lookAt.z = cosf(radians) + m_positionZ;
	DirectX::XMFLOAT3 p3(sinf(radians) + m_positionX, m_positionY, cosf(radians) + m_positionZ);
	lookAt = DirectX::XMLoadFloat3(&p3);

	// Create the view matrix from the three vectors.
	m_viewMatrix = DirectX::XMMatrixLookAtLH(position, lookAt, up);

	return;
}


void CameraClass::GetViewMatrix(DirectX::XMMATRIX& viewMatrix)
{
	viewMatrix = m_viewMatrix;
	return;
}