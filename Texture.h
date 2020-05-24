#pragma once
#include <QString>
#include <QOpenGLFunctions_4_5_Core>

class Texture : public QOpenGLFunctions_4_5_Core
{
public:
	Texture();
	~Texture();

	void LoadTexture(QString path);
	void LoadSkyboxTexture(QVector<QString> paths);
	GLuint GetId() const;

private:
	QString						m_path;
	GLuint						m_texId;
};
