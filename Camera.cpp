#include "Camera.h"
#include "QtMath"
#include "QQuaternion"
#include "LightMgr.h"

Camera::Camera()
	:m_nearClip(2.03f), m_farClip(10000.0f)
	, m_fov(60.0f), m_aspectRatio(1.0f), m_orthoSize(155.5f)
	, m_camPos(0, 50, -200, 1), m_lookAtPos(0, 0, 0, 1), m_camUpDir(0, 1, 0)
	, m_camMoveSpeed(0.05f), m_camRotateSpeed(0.05f), m_camRotateEnable(false), m_speedBoost(false)
{
}

Camera::~Camera()
{
}

QMatrix4x4 Camera::GetLightViewMatrix() const
{
	QMatrix4x4 matView;
	QVector3D dirLightPos = LightMgr::Instance().GetDirLightPos();
	QVector4D lightPos(dirLightPos.x(), dirLightPos.y(), dirLightPos.z(), 1);

	QVector3D axisZ = QVector3D(1, 1, 1).normalized();
	QVector3D axisX = (QVector3D::crossProduct(m_camUpDir, axisZ)).normalized();
	QVector3D axisY = QVector3D::crossProduct(axisZ, axisX);

	// 必须用C到P的列主序形式实现，因为该矩阵不能直接转置成P到C
	matView.setColumn(0, QVector4D(axisX, 0));
	matView.setColumn(1, QVector4D(axisY, 0));
	matView.setColumn(2, QVector4D(axisZ, 0));
	matView.setColumn(3, lightPos);

	static float rotate = 0.01f, rotate2 = 0.0001f;
// 	matView.translate(QVector3D(qCos(rotate2), 0, qSin(M_PI - rotate2)) * 200);
// 	matView.rotate(rotate, QVector3D(0, 1, 0));
	rotate += 0.01f;
	rotate2 += 0.0001f;

	return matView.inverted();
}

QMatrix4x4 Camera::GetViewMatrix() const
{
	QMatrix4x4 matView;

	// 方法1：使用lookat方式，这种方式可以锁定看的地方
	QVector3D axisZ = (m_camPos - m_lookAtPos).toVector3D().normalized();
	QVector3D axisX = (QVector3D::crossProduct(m_camUpDir, axisZ)).normalized();
	QVector3D axisY = QVector3D::crossProduct(axisZ, axisX);

	// 方式2：只记录摄像机本体的一个旋转（也就是世界姿态，有了姿态和摄像机的偏移组合一下就可以得到观察矩阵了）
// 	QVector3D axisX = matPose.row(0).toVector3D();
// 	QVector3D axisY = matPose.row(1).toVector3D();
// 	QVector3D axisZ = matPose.row(2).toVector3D();

	// 必须用C到P的列主序形式实现，因为该矩阵不能直接转置成P到C
	matView.setColumn(0, QVector4D(axisX, 0));
	matView.setColumn(1, QVector4D(axisY, 0));
	matView.setColumn(2, QVector4D(axisZ, 0));
	matView.setColumn(3, m_camPos);

	// 这里为什么不能直接用转置到行的表示，是因为，观察矩阵不是一个正交矩阵
	// 如果需要用行表示的话，其中的xyz是需要加负号的，这是我通过上面列排列的
	// 逆矩阵看到后，才明白为什么直接转置过来填写不行，因为其实不能直接转置，而是需要求逆
	// 所以以后最好还是不要尝试用下面的写法了，虽然可以实现
// 	matView.setRow(0, QVector4D(axisX, -m_camPos.x()));
// 	matView.setRow(1, QVector4D(axisY, -m_camPos.y()));
// 	matView.setRow(2, QVector4D(axisZ, -m_camPos.z()));
// 	matView.setRow(3, QVector4D(0, 0, 0, 1.0f));

	return matView.inverted();
}

QMatrix4x4 Camera::GetProjectionMatrix() const
{
	QMatrix4x4 matProjection;

	matProjection.setColumn(0, QVector4D(1.0f / qTan(m_fov / 2.0f * M_PI / 180.0f) / m_aspectRatio, 0, 0, 0));
	matProjection.setColumn(1, QVector4D(0, 1.0f / qTan(m_fov / 2.0f * M_PI / 180.0f), 0, 0));
	matProjection.setColumn(2, QVector4D(0, 0, -1 * (m_farClip + m_nearClip) / (m_farClip - m_nearClip), -1));
	matProjection.setColumn(3, QVector4D(0, 0, -2 * m_farClip * m_nearClip / (m_farClip - m_nearClip), 0));

	// 神奇的错用了公式，居然摄像头基本是正确的（只是上下左右移动都反了，当然其实视觉效果还是有点问题的，比如天空盒的接缝就很明显）
// 	matProjection.setRow(0, QVector4D(qTan(m_fov / 2.0f) * m_nearClip / m_aspectRatio, 0, 0, 0));
// 	matProjection.setRow(1, QVector4D(0, qTan(m_fov / 2.0f) * m_nearClip, 0, 0));
// 	matProjection.setRow(2, QVector4D(0, 0, -1 * (m_farClip + m_nearClip) / (m_farClip - m_nearClip), -1.0f));
// 	matProjection.setRow(3, QVector4D(0, 0, -2 * m_farClip * m_nearClip / (m_farClip - m_nearClip), 0));

	// 测试上面的错误公式的效果，其实上面算出来就是一个定值r或者t，就是一个正数（因为我当时n为1，所有rt都应该小于1）
// 	matProjection.setRow(0, QVector4D(2.5f / m_aspectRatio, 0, 0, 0));
// 	matProjection.setRow(1, QVector4D(0, 2.5f, 0, 0));
// 	matProjection.setRow(2, QVector4D(0, 0, -1 * (m_farClip + m_nearClip) / (m_farClip - m_nearClip), -1.0f));
// 	matProjection.setRow(3, QVector4D(0, 0, -2 * m_farClip * m_nearClip / (m_farClip - m_nearClip), 0));

	return matProjection;
}	

QMatrix4x4 Camera::GetOrthographicMatrix() const
{
	QMatrix4x4 matOrtho;

	matOrtho.setColumn(0, QVector4D(1.0f / m_orthoSize, 0, 0, 0));
	matOrtho.setColumn(1, QVector4D(0, 1.0f / m_orthoSize, 0, 0));
	matOrtho.setColumn(2, QVector4D(0, 0, -2 / (m_farClip - m_nearClip), 0));
	matOrtho.setColumn(3, QVector4D(0, 0, 0, 1));

	return matOrtho;
}

QMatrix4x4 Camera::GetVPMatrix() const
{
	return (GetProjectionMatrix() * GetViewMatrix());
}

QVector3D Camera::GetViewDir() const
{
	QVector3D axisZ = (m_camPos - m_lookAtPos).toVector3D().normalized();

	return axisZ;
}

void Camera::MoveForward()
{
	QVector3D axisZFroward = (m_lookAtPos - m_camPos).toVector3D().normalized();
// 	QVector3D axisX = QVector3D::crossProduct(m_camUpDir, axisZFroward).normalized();
// 	QVector3D axisY = QVector3D::crossProduct(axisZFroward, axisX);

	m_camPos += (axisZFroward * m_camMoveSpeed);
	m_lookAtPos += axisZFroward * m_camMoveSpeed;
}

void Camera::MoveBackward()
{
	QVector3D axisZFroward = (m_lookAtPos - m_camPos).toVector3D().normalized();
	// 	QVector3D axisX = QVector3D::crossProduct(m_camUpDir, axisZFroward).normalized();
	// 	QVector3D axisY = QVector3D::crossProduct(axisZFroward, axisX);

	m_camPos -= (axisZFroward * m_camMoveSpeed);
	m_lookAtPos -= axisZFroward * m_camMoveSpeed;
}

void Camera::MoveLeft()
{
	QVector3D axisZFroward = (m_lookAtPos - m_camPos).toVector3D().normalized();
	QVector3D axisX = QVector3D::crossProduct(m_camUpDir, axisZFroward).normalized();

	m_camPos += axisX * m_camMoveSpeed;
	m_lookAtPos += axisX * m_camMoveSpeed;
}

void Camera::MoveRight()
{
	QVector3D axisZFroward = (m_lookAtPos - m_camPos).toVector3D().normalized();
	QVector3D axisX = QVector3D::crossProduct(m_camUpDir, axisZFroward).normalized();

	m_camPos -= axisX * m_camMoveSpeed;
	m_lookAtPos -= axisX * m_camMoveSpeed;
}

void Camera::MoveUp()
{
	m_camPos += m_camUpDir * m_camMoveSpeed;
	m_lookAtPos += m_camUpDir * m_camMoveSpeed;
}

void Camera::MoveDown()
{
	m_camPos -= m_camUpDir * m_camMoveSpeed;
	m_lookAtPos -= m_camUpDir * m_camMoveSpeed;
}

void Camera::UpdateRotateInfo()
{
	if (m_rotateStartPos == m_rotateCurPos) {
		return;
	}

	QPoint ptDiff = m_rotateCurPos - m_rotateStartPos;
	float xAngle = ptDiff.x() * -1.0f * m_camRotateSpeed;
	float yAngle = ptDiff.y() * -1.0f * m_camRotateSpeed;

	QQuaternion qDiff = QQuaternion::fromEulerAngles(yAngle, xAngle, 0);
	QMatrix4x4 matR;
	matR.rotate(qDiff);
	// 这个地方特别注意，为什么要先保存变量，而不能在下面调用matView的地方使用函数，因为函数中也在使用lookat的值，会有冲突
	QMatrix4x4 matView = GetViewMatrix();
	m_lookAtPos = matView * m_lookAtPos;
	m_lookAtPos = matR * m_lookAtPos;
	m_lookAtPos = matView.inverted() * m_lookAtPos;

// 	qDebug() << QString("LookAt[%1][%2][%3]").arg(m_lookAtPos.x()).arg(m_lookAtPos.y()).arg(m_lookAtPos.z());

	m_rotateStartPos = m_rotateCurPos;
}
