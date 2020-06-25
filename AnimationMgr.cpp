#include "AnimationMgr.h"
#include "assimp/scene.h"
#include "assimp/ai_assert.h"
#include "assimp/anim.h"
#include "ShaderHelper.h"

AnimationMgr::AnimationMgr()
{
}

AnimationMgr::~AnimationMgr()
{
}

AnimationMgr& AnimationMgr::Instance()
{
	static AnimationMgr ins;
	return ins;
}

unsigned int AnimationMgr::GenAnimID()
{
	static unsigned int id = 1;
	return id++;
}

unsigned int AnimationMgr::CreateAnimFromAiScene(const aiScene *scene)
{
	if (0 == scene->mNumAnimations) {
		return 0;
	}

	auto animId = GenAnimID();

	ai_assert(scene->mNumAnimations == 1);
	aiAnimation *anim = scene->mAnimations[0];

	AnimInfo info;
	info.id = animId;
	info.name = anim->mName.C_Str();
	info.duration = anim->mDuration;
	info.tickPerSecond = anim->mTicksPerSecond;

	// ------Get channels info
	info.numChannels = anim->mNumChannels;
	for (unsigned int i = 0; i < info.numChannels; ++i)
	{
		ChannelInfo cInfo;
		aiNodeAnim *node = anim->mChannels[i];
		cInfo.name = node->mNodeName.C_Str();

		cInfo.numPositionKeys = node->mNumPositionKeys;
		for (unsigned int j = 0; j < cInfo.numPositionKeys; ++j)
		{
			cInfo.positionKeys.push_back(QVector3D(node->mPositionKeys[j].mValue.x, node->mPositionKeys[j].mValue.y, node->mPositionKeys[j].mValue.z));
			cInfo.positionKeysTime.push_back(node->mPositionKeys[j].mTime);
		}
		cInfo.numRotationKeys = node->mNumRotationKeys;
		for (unsigned int j = 0; j < cInfo.numRotationKeys; ++j)
		{
			cInfo.rotationKeys.push_back(QQuaternion(node->mRotationKeys[j].mValue.w, node->mRotationKeys[j].mValue.x, node->mRotationKeys[j].mValue.y, node->mRotationKeys[j].mValue.z));
			cInfo.rotationKeysTime.push_back(node->mRotationKeys[j].mTime);
		}
		cInfo.numScalingKeys = node->mNumScalingKeys;
		for (unsigned int j = 0; j < cInfo.numScalingKeys; ++j)
		{
			cInfo.scalingKeys.push_back(QVector3D(node->mScalingKeys[j].mValue.x, node->mScalingKeys[j].mValue.y, node->mScalingKeys[j].mValue.z));
			cInfo.scalingKeysTime.push_back(node->mScalingKeys[j].mTime);
		}

		info.channels.push_back(cInfo);
	}

	// ------get all anim nodes
	CreateAnimNodesFromScene(info, scene, &info.animRootNode);

	unsigned int iTest = 0;
	GetAnimNodesCount(info.animRootNode, iTest);

	// ------Get all bones info
	GetAllBonesInfo(scene, info);

	m_animInfoMap.insert(animId, info);
	return animId;
}

static bool bDebug = true;
void AnimationMgr::UpdateAllChannels(AnimInfo &info, float frameRate)
{
	m_allChannelsInterpValueVec.clear();

	bDebug = true;
	for (unsigned int i = 0; i < info.numChannels; ++i)
	{
		auto channel = info.channels[i];
		if (channel.name.compare("hair_l_02") == 0) {
			int i = 5;
			i += 2;
		}

		// Interpolate translation and generate translation transformation matrix
		QVector3D Translation;
		CalcInterpolatedPosition(info, Translation, frameRate, channel);
		QMatrix4x4 TranslationM;
		TranslationM.translate(Translation);

		// Interpolate rotation and generate rotation transformation matrix
		QQuaternion RotationQ;
		CalcInterpolatedRotation(info, RotationQ, frameRate, channel);
		QMatrix4x4 RotationM;
		RotationM = QMatrix4x4(RotationQ.toRotationMatrix());

		// Interpolate scaling and generate scaling transformation matrix
		QVector3D Scaling;
		CalcInterpolatedScaling(info, Scaling, frameRate, channel);
		QMatrix4x4 ScalingM;
		ScalingM.scale(Scaling);

		// Combine the above transformations
		m_allChannelsInterpValueVec.push_back(TranslationM * RotationM * ScalingM);

		//every frame only output debug info once
		bDebug = false;
	}
}

static int iTest = 0;
void AnimationMgr::ReadNodeHeirarchy(AnimInfo &info, float frameRate, NodeAnim *node, QMatrix4x4 &matParent)
{
	QString nodeName(node->name);
	if (nodeName.compare("hair_l_02") == 0) {
		int i = 5;
		i += 2;
	}

	if (node->channelId != -1) {
		node->localTransform = m_allChannelsInterpValueVec[node->channelId];
	}
	GetGlobalTransform(node);

	for (uint i = 0; i < node->childs.size(); i++) {
		ReadNodeHeirarchy(info, frameRate, node->childs[i], node->globalTransform);
	}
}

float time1 = 0.001f;
static float frame = 0.0f;
void AnimationMgr::UpdateAnimation(unsigned int animId, float second)
{
	second = time1;// debug the 0-1 frame
	auto anim = m_animInfoMap.find(animId);
	if (anim == m_animInfoMap.end()) {
		return;
	}

	// begin to update the valid animation......
	QString animName =  anim.value().name;
	float duration = anim.value().duration;
	float tickPerSec = anim.value().tickPerSecond == 0 ? 25.0f : anim.value().tickPerSecond;
	float totalSeconds = duration / tickPerSec;
	float totalTime = anim.value().totalTime + second;
	anim.value().totalTime = totalTime;

	// reset the anim total time
	if (anim.value().totalTime > totalSeconds) {
		anim.value().totalTime = 0;
	}

	float frameRate = totalTime / totalSeconds; // change frameRate to the time(per sec) * tickPerSec
	frameRate = frameRate > 1.0f ? 0.0f : frameRate;
	frameRate = 21.2f;// for test frame 1-2
	frame += 0.3f;
// 	if (frame > 61.0f) {
// 		frame = 0.0f;
// 	}
// 	frame = 19.6f;


	UpdateAllChannels(anim.value(), frame);
	QMatrix4x4 identity;
	identity.setToIdentity();
	ReadNodeHeirarchy(anim.value(), frame, anim.value().animRootNode, identity);


	for (auto &bi : anim.value().bonesInfoVec)
	{
		// ------ Get all bones final transform
// 		if (nullptr == bi.nodeAnim) {
// 			continue;
// 		}

		if (bi.nodeAnim->name.compare("cape_l_03") == 0) {
			int i = 5;
			i += 2;
		}
			
			bi.finalMat = anim.value().globalInverseTransform * bi.nodeAnim->globalTransform * bi.offset;
	}

	m_boneTransforms.clear();
	auto boneInfoSize = anim.value().bonesInfoVec.size();
	m_boneTransforms.resize(boneInfoSize);

	// ------Update all the bones info to shader(with gBones)
	// First, we need to convert all child node's transform to the root transform
	for (int i = 0; i < boneInfoSize; ++i)
	{
		auto boneInfo = anim.value().bonesInfoVec[i];

// 		NodeAnim *parent = nullptr;
// 		if (boneInfo.nodeAnim) {
// 			parent = boneInfo.nodeAnim->parent;
// 		}
// 		while (parent) {
// 			boneInfo.finalMat = parent->transform * boneInfo.finalMat;
// 
// 			parent = parent->parent;
// 		}
		// test identity
// 		QMatrix4x4 mat;
		m_boneTransforms[i] = boneInfo.finalMat;
	}
	ShaderHelper::Instance().SetBonesInfo(m_boneTransforms);
}

void AnimationMgr::CalcInterpolatedScaling(AnimInfo &info, QVector3D &scaling, float frameRate, ChannelInfo &cInfo)
{
	if (1 == cInfo.numScalingKeys) {
		scaling = QVector3D(cInfo.scalingKeys[0].x(), cInfo.scalingKeys[0].y(), cInfo.scalingKeys[0].z());
		return;
	}

	frameRate = fmod(frameRate, info.duration);

	auto numScalingKeys = cInfo.numScalingKeys;
	unsigned int frame = 0;
	for (int i = 0; i < numScalingKeys; ++i)
	{
		if (frameRate < cInfo.scalingKeysTime[i]) {
			frame = i - 1;
			break;
		}
	}
	unsigned int frameNext = (frame + 1) % cInfo.numScalingKeys;

	double diffTime = cInfo.scalingKeysTime[frameNext] - cInfo.scalingKeysTime[frame];
	float factor = float((frameRate - cInfo.scalingKeysTime[frame]) / diffTime);

	QVector3D scale = cInfo.scalingKeys[frame];
	QVector3D scaleNext = cInfo.scalingKeys[frameNext];

	scaling = scale + (scaleNext - scale) * factor;
	scaling = scale;
}

void AnimationMgr::CalcInterpolatedRotation(AnimInfo &info, QQuaternion &q, float frameRate, ChannelInfo &cInfo)
{
	if (1 == cInfo.numRotationKeys) {
		q = cInfo.rotationKeys[0];
		return;
	}

	frameRate = fmod(frameRate, info.duration);

	auto numRotationKeys = cInfo.numRotationKeys;
	unsigned int frame = 0;
	for (int i = 0; i < numRotationKeys; ++i)
	{
		if (frameRate < cInfo.rotationKeysTime[i]) {
			frame = i - 1;
			break;
		}
	}
	unsigned int frameNext = (frame + 1) % cInfo.numRotationKeys;

	double diffTime = cInfo.rotationKeysTime[frameNext] - cInfo.rotationKeysTime[frame];
	float factor = float((frameRate - cInfo.rotationKeysTime[frame]) / diffTime);

	if (bDebug)
	{
		qDebug() << QString("frame:%1, next:%2, factor:%3").arg(frame).arg(frameNext).arg(factor);
	}
	QQuaternion quat = cInfo.rotationKeys[frame];
	QQuaternion quatNext = cInfo.rotationKeys[frameNext];

	q = quat.slerp(quat, quatNext, factor);
}

void AnimationMgr::CalcInterpolatedPosition(AnimInfo &info, QVector3D &translation, float frameRate, ChannelInfo &cInfo)
{
	if (1 == cInfo.numPositionKeys) {
		translation = QVector3D(cInfo.positionKeys[0].x(), cInfo.positionKeys[0].y(), cInfo.positionKeys[0].z());
		return;
	}

	frameRate = fmod(frameRate, info.duration);

	auto numPositionKeys = cInfo.numPositionKeys;
	unsigned int frame = 0;
	for (int i = 0; i < numPositionKeys; ++i)
	{
		if (frameRate < cInfo.positionKeysTime[i]) {
			frame = i - 1;
			break;
		}
	}
	unsigned int frameNext = (frame + 1) % cInfo.numPositionKeys;

	double diffTime = cInfo.positionKeysTime[frameNext] - cInfo.positionKeysTime[frame];
	float factor = float((frameRate - cInfo.positionKeysTime[frame]) / diffTime);

	QVector3D pos = cInfo.positionKeys[frame];
	QVector3D posNext = cInfo.positionKeys[frameNext];

	translation = pos + (posNext - pos) * factor;
}

void AnimationMgr::CreateAnimNodesFromScene(AnimInfo &info, const aiScene *scene, NodeAnim **root)
{
	*root = new NodeAnim;
	(*root)->name = scene->mRootNode->mName.data;
	(*root)->parent = nullptr;
	auto trans = scene->mRootNode->mTransformation;
	(*root)->localTransform.setRow(0, QVector4D(trans.a1, trans.a2, trans.a3, trans.a4));
	(*root)->localTransform.setRow(1, QVector4D(trans.b1, trans.b2, trans.b3, trans.b4));
	(*root)->localTransform.setRow(2, QVector4D(trans.c1, trans.c2, trans.c3, trans.c4));
	(*root)->localTransform.setRow(3, QVector4D(trans.d1, trans.d2, trans.d3, trans.d4));

	// find correspond channel id, default is -1(this means this node is not a anim node)
	auto numChannels = scene->mAnimations[0]->mNumChannels;
	auto animation = scene->mAnimations[0];
	for (int i = 0; i < numChannels; ++i)
	{
		if (animation->mChannels[i]->mNodeName.data == (*root)->name) {
			(*root)->channelId = i;
		}
	}

	for (int i = 0; i < scene->mRootNode->mNumChildren; ++i)
	{
		(*root)->childs.push_back(CreateAnimNodes(info, scene->mRootNode->mChildren[i], *root, scene));
	}
}

AnimationMgr::NodeAnim* AnimationMgr::CreateAnimNodes(AnimInfo &info, const aiNode *node, NodeAnim *parent, const aiScene *scene)
{
	NodeAnim *pNode = new NodeAnim;
	pNode->name = node->mName.data;
	pNode->parent = parent;
	auto trans = node->mTransformation;
	pNode->localTransform.setRow(0, QVector4D(trans.a1, trans.a2, trans.a3, trans.a4));
	pNode->localTransform.setRow(1, QVector4D(trans.b1, trans.b2, trans.b3, trans.b4));
	pNode->localTransform.setRow(2, QVector4D(trans.c1, trans.c2, trans.c3, trans.c4));
	pNode->localTransform.setRow(3, QVector4D(trans.d1, trans.d2, trans.d3, trans.d4));

	// Get the global transform(this is very important!!!!)
	GetGlobalTransform(pNode);

	// test
	if (pNode->name.compare("cape_l_03") == 0) {
		int i = 5;
		i += 2;
	}
	
	// Very important!!! must get the correct global inverseTransform
	// otherwise the anim not works correctly
	if (node->mNumMeshes > 0) {
		info.globalInverseTransform = pNode->globalTransform;
		info.globalInverseTransform = info.globalInverseTransform.inverted();
	}

	// find correspond channel id, default is -1(this means this node is not a anim node)
	auto numChannels = scene->mAnimations[0]->mNumChannels;
	auto animation = scene->mAnimations[0];
	for (int i = 0; i < numChannels; ++i)
	{
		if (animation->mChannels[i]->mNodeName.data == pNode->name) {
			pNode->channelId = i;
			qDebug() << QString("find channel--[%1]").arg(i);
		}
	}

	for (int i = 0; i < node->mNumChildren; ++i)
	{
		pNode->childs.push_back(CreateAnimNodes(info, node->mChildren[i], pNode, scene));
	}

	return pNode;
}

void AnimationMgr::GetGlobalTransform(NodeAnim *node)
{
	node->globalTransform = node->localTransform;
	NodeAnim *parent = node->parent;
	while (nullptr != parent)
	{
		node->globalTransform = parent->localTransform * node->globalTransform;

		parent = parent->parent;
	}
}

void AnimationMgr::GetAllBonesInfo(const aiScene *scene, AnimInfo &info)
{
	// Get the total bones num
	unsigned int numBones = 0;
	unsigned int numMeshes = 1;
	numMeshes = scene->mNumMeshes;
	for (int i = 0; i < numMeshes; ++i)
	{
		numBones += scene->mMeshes[i]->mNumBones;
	}

	info.bonesMap.clear();
	info.bonesInfoVec.clear();
// 	info.bonesInfoVec.resize(numBones);

	unsigned int boneIndex = 0;
	for (int i = 0; i < numMeshes; ++i)
	{
		auto mesh = scene->mMeshes[i];
		auto numBones = mesh->mNumBones;
		for (int j = 0; j < numBones; ++j)
		{
			auto bone = mesh->mBones[j];
			if (info.bonesMap.find(bone->mName.data) == info.bonesMap.end()) {
				info.bonesMap[bone->mName.data] = boneIndex;

				BoneInfo bi;
				info.bonesInfoVec.push_back(bi);
				++boneIndex;
			}
			else {

			}

			auto &boneInfo = info.bonesInfoVec[info.bonesMap[bone->mName.data]];
			boneInfo.nodeAnim = FindNodeAnimByName(info, bone->mName.data);

// 			if (nullptr == boneInfo.nodeAnim) {
// 				int i = 0;
// 				i += 2;
// 			}
			if (bone->mName == aiString("cape_l_03")) {
				int i = 0;
				i += 2;
			}

			boneInfo.offset.setRow(0, QVector4D(bone->mOffsetMatrix.a1, bone->mOffsetMatrix.a2, bone->mOffsetMatrix.a3, bone->mOffsetMatrix.a4));
			boneInfo.offset.setRow(1, QVector4D(bone->mOffsetMatrix.b1, bone->mOffsetMatrix.b2, bone->mOffsetMatrix.b3, bone->mOffsetMatrix.b4));
			boneInfo.offset.setRow(2, QVector4D(bone->mOffsetMatrix.c1, bone->mOffsetMatrix.c2, bone->mOffsetMatrix.c3, bone->mOffsetMatrix.c4));
			boneInfo.offset.setRow(3, QVector4D(bone->mOffsetMatrix.d1, bone->mOffsetMatrix.d2, bone->mOffsetMatrix.d3, bone->mOffsetMatrix.d4));
		}
	}
}

AnimationMgr::NodeAnim* AnimationMgr::FindNodeAnimByName(AnimInfo &info, const char *name)
{
	NodeAnim *node = nullptr;
	FindNodeAnimRecursive(info.animRootNode, name, &node);
	return node;
}

void AnimationMgr::FindNodeAnimRecursive(NodeAnim *node, const char *name, NodeAnim **outNode)
{
	if (node->name == name) {
		*outNode = node;
	}
	else {
		for (int i = 0; i < node->childs.size(); ++i)
		{
			FindNodeAnimRecursive(node->childs[i], name, outNode);
		}
	}
}

void AnimationMgr::GetAnimNodesCount(NodeAnim *root, unsigned int &count)
{
	++count;
	if (root->childs.size() != 0) {

		for (int i = 0; i < root->childs.size(); ++i)
		{
			GetAnimNodesCount(root->childs[i], count);
		}
	}
}
