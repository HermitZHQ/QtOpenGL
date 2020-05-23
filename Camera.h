#pragma once
#include "QMatrix4x4"

class Camera
{
public:
	Camera();
	~Camera();

	void SetNearClip(float n) {
		m_nearClip = n;
	}
	void SetFarClip(float f) {
		m_farClip = f;
	}
	void SetFOV(float fov) {
		m_fov = fov;
	}
	void SetAspectRatio(float ratio) {
		m_aspectRatio = ratio;
	}
	void SetMouseMoveSpeed(int value) {
		m_camMoveSpeed = value / 10.0f;
	}

	void SetCamPos(QVector3D pos) {
		m_camPos = pos;
	}
	QVector4D GetCamPos() const {
		return m_camPos;
	}
	void SetLookAtPos(QVector3D pos) {
		m_lookAtPos = pos;
	}
	void SetCamUpDir(QVector3D upDir) {
		m_camUpDir = upDir;
	}

	QMatrix4x4 GetViewMatrix() const;
	QMatrix4x4 GetProjectionMatrix() const;
	QMatrix4x4 GetOrthographicMatrix() const;
	QMatrix4x4 GetVPMatrix() const;

	// camera move
	void MoveForward();
	void MoveBackward();
	void MoveLeft();
	void MoveRight();
	void MoveUp();
	void MoveDown();

	void SetCamRotateEnable(bool bEnable) {
		m_camRotateEnable = bEnable;
	}
	bool GetCamRotateEnable() const {
		return m_camRotateEnable;
	}
	void SetRotateStartPos(QPoint pos) {
		m_rotateStartPos = pos;
	}
	void SetRotateCurPos(QPoint pos) {
		m_rotateCurPos = pos;
	}
	void UpdateRotateInfo();

private:
	unsigned int			m_width;
	unsigned int			m_height;
	float					m_orthoSize;
	float					m_nearClip;
	float					m_farClip;
	float					m_fov;
	float					m_aspectRatio;
	QVector4D				m_camPos;
	QVector4D				m_lookAtPos;
	QVector3D				m_camUpDir;

	QMatrix4x4				m_viewMat;
	QMatrix4x4				m_projectionMat;

	QQuaternion				m_camWorldPose;
	float					m_camMoveSpeed;
	float					m_camRotateSpeed;
	bool					m_camRotateEnable;
	QPoint					m_rotateStartPos;
	QPoint					m_rotateCurPos;
};

