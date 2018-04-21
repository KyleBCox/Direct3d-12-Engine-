#include "stdafx.h"
#include "CameraComponent.h"


CameraComponent::CameraComponent() :
m_pitch(0),
m_yaw(0),
m_camTarget({}),
m_position({ 0,0,0,0 }),
m_direction({}),
isJumping(false),
m_isUnderGround(false)
{
	// Creates shared_ptr of \class TerrainCollisionHelper
	m_terrainCollider = std::make_unique<TerrainCollisionHelper>();
}

CameraComponent::~CameraComponent()
{
}

int CameraComponent::InitRootSignatureParameters(int indexOffset)
{
	return 0;
}

void CameraComponent::Init(std::shared_ptr<CommandListManager>* commandListManager, std::shared_ptr<DescriptorHeapManager> descriptorHeapManager, UINT * descOffset, std::shared_ptr<PSOManager>* pso)
{
}

void CameraComponent::Update()
{
	// The vector that is facing up
	DirectX::XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

	// Gets the rotation matrix from \property m_pitch (pitch from mouse movement) and m_yaw (yaw from mouse movenment)
	m_camRotationMatrix = XMMatrixRotationRollPitchYaw(m_pitch, m_yaw, 0);
	// Transforms the forward facing vector with the rotation matrix
	m_camTarget = XMVector3TransformCoord(DefaultForward, m_camRotationMatrix);
	// Normalises the vector which has just been rotated
	m_camTarget = XMVector3Normalize(m_camTarget);
	// Gets the yaw rotation matrix from \property m_yaw
	const auto RotateYTempMatrix = XMMatrixRotationY(m_yaw);

	// Rotates the \property DefaultRight vector with the yaw rotation matrix \var RotateYTempMatrix
	const auto camRight = XMVector3TransformCoord(DefaultRight, RotateYTempMatrix);
	// Rotates the \property DefaultForward vector with the yaw rotation matrix \var RotateYTempMatrix
	const auto camForward = XMVector3TransformCoord(DefaultForward, RotateYTempMatrix);
	// Rotates the \var up vector with the yaw rotation matrix \var RotateYTempMatrix
	up.v = XMVector3TransformCoord(up, RotateYTempMatrix);

	// The float value to multiply the \property m_direction vector coords X and Z
	const auto multiplyerXZ = -0.3f;
	// The float value to multiply the \property m_direction vector coord Y
	const auto multiplyerY = 0.3f;

	// Multiplies the \property m_direction vector with the multiplyers \var multiplyerXZ and \var multiplyerY
	const auto x = XMVectorGetX(m_direction) * multiplyerXZ;
	const auto z = XMVectorGetZ(m_direction) * multiplyerXZ;
	const auto y = XMVectorGetY(m_direction) * multiplyerY;

	// Creates XMVECTOR from \var x and multiplies it by \var camRight to get the x movement of the camera
	const auto xAxisMovement = XMVectorMultiply(XMVectorSet(x, x, x, x), camRight);
	// Creates XMVECTOR from \var z and multiplies it by \var camForward to get the z movement of the camera
	const auto zAxisMovement = XMVectorMultiply(XMVectorSet(z, z, z, z), camForward);
	// Creates XMVECTOR from \var y and multiplies it by the \var up vector to get the y movement of the camera
	const auto yAxisMovement = XMVectorMultiply(XMVectorSet(y, y, y, y), up);

	// Add the movement vectors (\var xAxisMovement \var zAxisMovement \var yAxisMovement) to the position (\var pos) of the camera
	m_transform.position = XMVectorAdd(m_transform.position, xAxisMovement);
	m_transform.position = XMVectorAdd(m_transform.position, zAxisMovement);
	m_transform.position = XMVectorAdd(m_transform.position, yAxisMovement);

	// Creates the variable which will be passed to the XMStoreFloat3 function to store the final vector as a XMFLOAT3 structure
	XMFLOAT3 fullNewPosFloat3;
	// Creates the multiplyer / offset for the camera orbitting the player.
	const auto camMultiplyer = 10;
	// Orbits the camera around the player (research sin waves and cos waves to understand more)
	XMStoreFloat3(&fullNewPosFloat3, XMVectorSet(sin(m_yaw) * camMultiplyer, sin(m_pitch) * camMultiplyer, cos(m_yaw) * camMultiplyer, 1) + m_transform.position);
	// the new position of Y for the cameras position (to be passed into the \fn bool TerrainCollisionHelper::GetNewYPos(XMFLOAT3 position, float* yOut) )
	float newYPos;

	// Gets the new Y location of the cameras position (in case of colliding with terrain)
	m_terrainCollider->GetNewYPos(fullNewPosFloat3, &newYPos);

	// sets the \var newYPos if it is above terrain and the camera is not (\property c_cameraGroundOffset is the constant offset of how much the camera should be above ground)
	if (newYPos > fullNewPosFloat3.y - c_cameraGroundOffset) {
		fullNewPosFloat3.y = newYPos + c_cameraGroundOffset;
		m_isUnderGround = true;
	}
	else {
		m_isUnderGround = false;
	}

	// Puts the new pos (after orbitting and terrain collision) in to an XMVECTOR structure
	auto newPos = XMLoadFloat3(&fullNewPosFloat3);
	// Creates the View Matrix and adds it to the \class ModelViewProjectionManager helper class
	XMStoreFloat4x4(&m_view, XMMatrixTranspose(XMMatrixLookAtRH(newPos, m_transform.position, up.v)));
}

void CameraComponent::Render()
{
}

void CameraComponent::OnKeyDown(UINT key)
{
	if (key == 0x57)
	{
		m_direction.v = XMVectorSetZ(m_direction, 1);
	}
	// Set \property m_direction to left on "A" pressed
	else if (key == 0x41)
	{
		m_direction.v = XMVectorSetX(m_direction, 1);
	}
	// Set \property m_direction to backward on "S" pressed
	else if (key == 0x53)
	{
		m_direction.v = XMVectorSetZ(m_direction, -1);
	}
	// Set \property m_direction to right on "D" pressed
	else if (key == 0x44)
	{
		m_direction.v = XMVectorSetX(m_direction, -1);
	}
	// Set \property m_direction to up on "SpaceBar" pressed
	else if (key == VK_SPACE)
	{
		isJumping = true;
		m_direction.v = XMVectorSetY(m_direction, 1);
	}
	// Set \property m_direction to down on "Shift" pressed
	else if (key == VK_SHIFT)
	{
		m_direction.v = XMVectorSetY(m_direction, -1);
	}
}

void CameraComponent::OnKeyUp(UINT key)
{
	// Set \property m_direction to forward on "W" pressed
	if (key == 0x57)
	{
		m_direction.v = XMVectorSetZ(m_direction, 0);
	}
	// Set \property m_direction to not left on "A" pressed
	else if (key == 0x41)
	{
		m_direction.v = XMVectorSetX(m_direction, 0);
	}
	// Set \property m_direction to not backward on "S" pressed
	else if (key == 0x53)
	{
		m_direction.v = XMVectorSetZ(m_direction, 0);
	}
	// Set \property m_direction to not right on "D" pressed
	else if (key == 0x44)
	{
		m_direction.v = XMVectorSetX(m_direction, 0);
	}
	// Set \property m_direction to not up on "SpaceBar" pressed
	else if (key == VK_SPACE)
	{
		isJumping = false;
		m_direction.v = XMVectorSetY(m_direction, 0);
	}
	// Set \property m_direction to not down on "Shift" pressed
	else if (key == VK_SHIFT)
	{
		m_direction.v = XMVectorSetY(m_direction, 0);
	}
}

void CameraComponent::OnMouseMoved(float x, float y)
{
	// Multiplyer of the relative \param x  \param y;
	const auto multiplyer = 0.003f;
	// is touching or under the terrain (see \fn void CameraHelper::Update(XMVECTOR* pos) )
	if ((y < 0 && m_isUnderGround) || !m_isUnderGround) {
		// subtracts \param y multiplied by \var multiplyer to get the new \property m_pitch
		m_pitch -= y * multiplyer;
	}

	// subtracts negative \param x multiplied by \var multiplyer to get the new \property m_yaw
	m_yaw -= -x * multiplyer;

	// Stops the rotations from going past half a circle
	m_pitch = (float)__max(-DirectX::XM_PI / 2.0f - 0.00003f, m_pitch);
	m_pitch = (float)__min(+DirectX::XM_PI / 2.0f - 0.00003f, m_pitch);
}

void CameraComponent::OnDeviceRemoved()
{
}

void CameraComponent::CreateWindowSizeDependentResources()
{
}

void CameraComponent::CreateDeviceDependentResoures()
{
}