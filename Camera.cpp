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

	// ������C��P����������ʽʵ�֣���Ϊ�þ�����ֱ��ת�ó�P��C
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

	// ����1��ʹ��lookat��ʽ�����ַ�ʽ�����������ĵط�
	QVector3D axisZ = (m_camPos - m_lookAtPos).toVector3D().normalized();
	QVector3D axisX = (QVector3D::crossProduct(m_camUpDir, axisZ)).normalized();
	QVector3D axisY = QVector3D::crossProduct(axisZ, axisX);

	// ��ʽ2��ֻ��¼����������һ����ת��Ҳ����������̬��������̬���������ƫ�����һ�¾Ϳ��Եõ��۲�����ˣ�
// 	QVector3D axisX = matPose.row(0).toVector3D();
// 	QVector3D axisY = matPose.row(1).toVector3D();
// 	QVector3D axisZ = matPose.row(2).toVector3D();

	// ������C��P����������ʽʵ�֣���Ϊ�þ�����ֱ��ת�ó�P��C
	matView.setColumn(0, QVector4D(axisX, 0));
	matView.setColumn(1, QVector4D(axisY, 0));
	matView.setColumn(2, QVector4D(axisZ, 0));
	matView.setColumn(3, m_camPos);

	// ����Ϊʲô����ֱ����ת�õ��еı�ʾ������Ϊ���۲������һ����������
	// �����Ҫ���б�ʾ�Ļ������е�xyz����Ҫ�Ӹ��ŵģ�������ͨ�����������е�
	// ����󿴵��󣬲�����Ϊʲôֱ��ת�ù�����д���У���Ϊ��ʵ����ֱ��ת�ã�������Ҫ����
	// �����Ժ���û��ǲ�Ҫ�����������д���ˣ���Ȼ����ʵ��
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

	// ����Ĵ����˹�ʽ����Ȼ����ͷ��������ȷ�ģ�ֻ�����������ƶ������ˣ���Ȼ��ʵ�Ӿ�Ч�������е�����ģ�������պеĽӷ�ͺ����ԣ�
// 	matProjection.setRow(0, QVector4D(qTan(m_fov / 2.0f) * m_nearClip / m_aspectRatio, 0, 0, 0));
// 	matProjection.setRow(1, QVector4D(0, qTan(m_fov / 2.0f) * m_nearClip, 0, 0));
// 	matProjection.setRow(2, QVector4D(0, 0, -1 * (m_farClip + m_nearClip) / (m_farClip - m_nearClip), -1.0f));
// 	matProjection.setRow(3, QVector4D(0, 0, -2 * m_farClip * m_nearClip / (m_farClip - m_nearClip), 0));

	// ��������Ĵ���ʽ��Ч������ʵ�������������һ����ֵr����t������һ����������Ϊ�ҵ�ʱnΪ1������rt��Ӧ��С��1��
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
	// ����ط��ر�ע�⣬ΪʲôҪ�ȱ�����������������������matView�ĵط�ʹ�ú�������Ϊ������Ҳ��ʹ��lookat��ֵ�����г�ͻ
	QMatrix4x4 matView = GetViewMatrix();
	m_lookAtPos = matView * m_lookAtPos;
	m_lookAtPos = matR * m_lookAtPos;
	m_lookAtPos = matView.inverted() * m_lookAtPos;

// 	qDebug() << QString("LookAt[%1][%2][%3]").arg(m_lookAtPos.x()).arg(m_lookAtPos.y()).arg(m_lookAtPos.z());

	m_rotateStartPos = m_rotateCurPos;
}
