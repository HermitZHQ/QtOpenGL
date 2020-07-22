#pragma once
#include <QVector4D>
#include <QVector3D>
#include <QVector>
#include <QString>
#include <QMap>
#include "QOpenGLFunctions_4_5_Core"
#include "QMatrix4x4"
#include "QQuaternion"

struct aiScene;
struct aiNode;
struct aiMesh;
class AnimationMgr : QOpenGLFunctions_4_5_Core
{
	struct NodeAnim
	{
		QString					name;
		NodeAnim				*parent;
		QVector<NodeAnim*>		childs;
		QMatrix4x4				localTransform;
		QMatrix4x4				globalTransform;
		int						channelId;
		bool					isBone;

		NodeAnim()
			:channelId(-1), parent(nullptr), isBone(false)
		{}
	};
	struct BoneInfo
	{
		QString					name;
		QMatrix4x4				offset;
		QMatrix4x4				finalMat;
		NodeAnim				*nodeAnim;

		BoneInfo()
			:nodeAnim(nullptr)
		{}
	};
	struct ChannelInfo
	{
		QString					name;
		unsigned int			numPositionKeys;
		QVector<QVector3D>		positionKeys;
		QVector<double>			positionKeysTime;
		unsigned int			numRotationKeys;
		QVector<QQuaternion>	rotationKeys;
		QVector<double>			rotationKeysTime;
		unsigned int			numScalingKeys;
		QVector<QVector3D>		scalingKeys;
		QVector<double>			scalingKeysTime;
		bool					isBone;

		ChannelInfo()
			:isBone(false) 
		{}
	};
	struct AnimInfo
	{
		unsigned int			id;
		QString					name;
		float					duration;
		float					tickPerSecond;
		float					totalTime;
		unsigned int			numChannels;
		QVector<ChannelInfo>	channels;
		NodeAnim*				animRootNode;
		typedef QMap<QString, unsigned int> BoneMap;
		BoneMap					bonesMap;
		QVector<BoneInfo>		bonesInfoVec;
		QMatrix4x4				globalInverseTransform;

		GLuint					vao_skin;
		GLuint					vbo_skin;
		QVector<float>			skin_vertices;
		QVector<unsigned int>	skin_indices;

		AnimInfo()
			:id(0), duration(0), tickPerSecond(25), totalTime(0), numChannels(0)
			, animRootNode(nullptr), vao_skin(0), vbo_skin(0)
		{}
	};
public:
	~AnimationMgr();

	static AnimationMgr& Instance();
	static unsigned int GenAnimID();

	unsigned int CreateAnimFromAiScene(const aiScene *scene, const aiMesh *mesh);
	void UpdateAnimation(unsigned int animId, float second);
	void DrawSkeleton(unsigned int animId);
	void RenderSkeleton(AnimInfo &info, NodeAnim *node, QMatrix4x4 mat, QMatrix4x4 parentMat);
	void RenderSkeletonWithBones(AnimInfo &info);

protected:
	void UpdateAllChannels(AnimInfo &info, float frameRate);
	void ReadNodeHeirarchy(AnimInfo &info, NodeAnim *node);
	ChannelInfo* FindChannelTransformByName(AnimInfo &info, QString name);
	void CalcInterpolatedScaling(AnimInfo &info, QVector3D &scaling, float frameRate, ChannelInfo &cInfo);
	void CalcInterpolatedRotation(AnimInfo &info, QQuaternion &q, float frameRate, ChannelInfo &cInfo);
	void CalcInterpolatedPosition(AnimInfo &info, QVector3D &translation, float frameRate, ChannelInfo &cInfo);

	void CreateAnimNodesFromScene(AnimInfo &info, const aiScene *scene, NodeAnim **root);
	NodeAnim* CreateAnimNodes(AnimInfo &info, const aiNode *node, NodeAnim *parent, const aiScene *scene);
	void GetGlobalTransform(NodeAnim *node);
	void GetAllBonesInfo(const aiMesh *mesh, AnimInfo &info);
	void GetAllBonesAnimNode(AnimInfo &info);
	bool CheckNodeIsBoneByName(AnimInfo &info, QString &name);
	QMatrix4x4 GetBoneOffsetByName(AnimInfo &info, QString &name);
	void GetAllChannelsBoneInfo(AnimInfo &info);

	NodeAnim* FindNodeAnimByName(AnimInfo &info, const char *name);
	void FindNodeAnimRecursive(NodeAnim *node, const char *name, NodeAnim **outNode);

	// for test
	void GetAnimNodesCount(NodeAnim *root, unsigned int &count);

private:
	AnimationMgr();

	typedef QMap<unsigned int, AnimInfo> AnimInfoMap;
	AnimInfoMap						m_animInfoMap;

	QVector<QMatrix4x4>				m_boneTransforms;
	QVector<QMatrix4x4>				m_allChannelsInterpValueVec;
};