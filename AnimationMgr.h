#pragma once
#include <QVector4D>
#include <QVector3D>
#include <QVector>
#include <QString>
#include <QMap>
#include "QMatrix4x4"
#include "QQuaternion"

struct aiScene;
struct aiNode;
class AnimationMgr
{
	struct NodeAnim
	{
		QString					name;
		NodeAnim				*parent;
		QVector<NodeAnim*>		childs;
		QMatrix4x4				localTransform;
		QMatrix4x4				globalTransform;
		int						channelId;

		NodeAnim()
			:channelId(-1), parent(nullptr)
		{}
	};
	struct BoneInfo
	{
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
		unsigned int			numRotationKeys;
		QVector<QQuaternion>	rotationKeys;
		unsigned int			numScalingKeys;
		QVector<QVector3D>		scalingKeys;
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

		AnimInfo()
			:id(0), duration(0), tickPerSecond(25), totalTime(0), numChannels(0)
			, animRootNode(nullptr)
		{}
	};
public:
	~AnimationMgr();

	static AnimationMgr& Instance();
	static unsigned int GenAnimID();

	unsigned int CreateAnimFromAiScene(const aiScene *scene);

	void ReadNodeHeirarchy(AnimInfo &info, float frameRate, NodeAnim *node, QMatrix4x4 &matParent);
	void UpdateAnimation(unsigned int animId, float second);
	void CalcInterpolatedScaling(AnimInfo &info, QVector3D &scaling, float frameRate, NodeAnim *node);
	void CalcInterpolatedRotation(AnimInfo &info, QQuaternion &q, float frameRate, NodeAnim *node);
	void CalcInterpolatedPosition(AnimInfo &info, QVector3D &translation, float frameRate, NodeAnim *node);

protected:
	void CreateAnimNodesFromScene(AnimInfo &info, const aiScene *scene, NodeAnim **root);
	NodeAnim* CreateAnimNodes(AnimInfo &info, const aiNode *node, NodeAnim *parent, const aiScene *scene);
	void GetGlobalTransform(NodeAnim *node);
	void GetAllBonesInfo(const aiScene *scene, AnimInfo &info);
	NodeAnim* FindNodeAnimByName(AnimInfo &info, const char *name);
	void FindNodeAnimRecursive(NodeAnim *node, const char *name, NodeAnim **outNode);

	// for test
	void GetAnimNodesCount(NodeAnim *root, unsigned int &count);

private:
	AnimationMgr();

	typedef QMap<unsigned int, AnimInfo> AnimInfoMap;
	AnimInfoMap						m_animInfoMap;

	QVector<QMatrix4x4>				m_boneTransforms;
};