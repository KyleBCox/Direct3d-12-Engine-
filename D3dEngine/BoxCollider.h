#pragma once
#include "Component.h"
class D3DENGINE_API BoxCollider : public Component
{
public:
	BoxCollider();
	~BoxCollider();

	// Inherited via Component
	virtual int InitRootSignatureParameters(int indexOffset) override;
	virtual void Init() override;
	virtual void Update() override;
	virtual void Render() override;
	virtual void OnKeyDown(UINT key) override;
	virtual void OnKeyUp(UINT key) override;
	virtual void OnMouseMoved(float x, float y) override;
	virtual void OnDeviceRemoved() override;
	virtual void CreateWindowSizeDependentResources() override;
	virtual void CreateDeviceDependentResources() override;
	void InitCollider(BoundingBox collider);
	BoundingBox GetCollider() const { return m_collider; };
private:
	BoundingBox m_collider; /// The collider
};

