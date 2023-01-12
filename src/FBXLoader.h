//This is a modification of a source from https://inf.run/Cq6i
#pragma once

#include "common.h"

struct FbxMaterialInfo {
    DirectX::SimpleMath::Vector4 diffuse;
    DirectX::SimpleMath::Vector4 ambient;
    DirectX::SimpleMath::Vector4 specular;
    std::wstring name;
    std::wstring diffuseTexName;
    std::wstring normalTexName;
    std::wstring specularTexName;
};

struct BoneWeight {
    using Pair = std::pair<int32_t, double>;
    std::vector<Pair> boneWeights;

    void AddWeights(uint32_t index, double weight) {
        if (weight <= 0.f)
            return;

        auto findIt = std::find_if(boneWeights.begin(), boneWeights.end(),
                                   [=](const Pair &p) {
                                       return p.second < weight;
                                   });

        if (findIt != boneWeights.end())
            boneWeights.insert(findIt, Pair(index, weight));
        else
            boneWeights.push_back(Pair(index, weight));

        // ����ġ�� �ִ� 4��
        if (boneWeights.size() > 4)
            boneWeights.pop_back();
    }

    void Normalize() {
        double sum = 0.f;
        std::for_each(boneWeights.begin(), boneWeights.end(),
                      [&](Pair &p) { sum += p.second; });
        std::for_each(boneWeights.begin(), boneWeights.end(),
                      [=](Pair &p) { p.second = p.second / sum; });
    }
};

struct FbxMeshInfo {
    std::wstring name;
    std::vector<fuse::directx::vertex> vertices;
    std::vector<std::vector<uint32_t>> indices;
    std::vector<FbxMaterialInfo> materials;
    std::vector<BoneWeight> boneWeights; // �� ����ġ
    bool hasAnimation;
};

struct FbxKeyFrameInfo {
    FbxAMatrix matTransform;
    double time;
};

struct FbxBoneInfo {
    std::wstring boneName;
    int32_t parentIndex;
    FbxAMatrix matOffset;
};

struct FbxAnimClipInfo {
    std::wstring name;
    FbxTime startTime;
    FbxTime endTime;
    FbxTime::EMode mode;
    std::vector<std::vector<FbxKeyFrameInfo>> keyFrames;
};

class FBXLoader {
public:
    FBXLoader();
    ~FBXLoader();

public:
    void LoadFbx(const std::wstring &path);
    std::vector<fuse::directx::geometry<fuse::directx::vertex>> geometries();

public:
    int32_t GetMeshCount() { return static_cast<int32_t>(_meshes.size()); }
    const FbxMeshInfo &GetMesh(int32_t idx) { return _meshes[idx]; }
    std::vector<std::shared_ptr<FbxBoneInfo>> &GetBones() { return _bones; }
    std::vector<std::shared_ptr<FbxAnimClipInfo>> &
    GetAnimClip() { return _animClips; }
private:
    void Import(const std::wstring &path);

    void ParseNode(FbxNode *root);
    void LoadMesh(FbxMesh *mesh);
    void LoadMaterial(FbxSurfaceMaterial *surfaceMaterial);

    void GetNormal(FbxMesh *mesh, FbxMeshInfo *container, int32_t idx,
                   int32_t vertexCounter);
    void GetTangent(FbxMesh *mesh, FbxMeshInfo *container, int32_t idx,
                    int32_t vertexCounter);
    void GetUV(FbxMesh *mesh, FbxMeshInfo *container, int32_t idx,
               int32_t vertexCounter);
    DirectX::SimpleMath::Vector4
    GetMaterialData(FbxSurfaceMaterial *surface, const char *materialName,
                    const char *factorName);
    std::wstring GetTextureRelativeName(FbxSurfaceMaterial *surface,
                                        const char *materialProperty);

    void CreateTextures();
    void CreateMaterials();

    // Animation
    void LoadBones(FbxNode *node) { LoadBones(node, 0, -1); }
    void LoadBones(FbxNode *node, int32_t idx, int32_t parentIdx);
    void LoadAnimationInfo();

    void LoadAnimationData(FbxMesh *mesh, FbxMeshInfo *meshInfo);
    void
    LoadBoneWeight(FbxCluster *cluster, int32_t boneIdx, FbxMeshInfo *meshInfo);
    void
    LoadOffsetMatrix(FbxCluster *cluster, const FbxAMatrix &matNodeTransform,
                     int32_t boneIdx, FbxMeshInfo *meshInfo);
    void LoadKeyframe(int32_t animIndex, FbxNode *node, FbxCluster *cluster,
                      const FbxAMatrix &matNodeTransform, int32_t boneIdx,
                      FbxMeshInfo *container);

    int32_t FindBoneIndex(std::string name);
    FbxAMatrix GetTransform(FbxNode *node);

    void FillBoneWeight(FbxMesh *mesh, FbxMeshInfo *meshInfo);

private:
    FbxManager *_manager = nullptr;
    FbxScene *_scene = nullptr;
    FbxImporter *_importer = nullptr;
    std::wstring _resourceDirectory;

    std::vector<FbxMeshInfo> _meshes;
    std::vector<std::shared_ptr<FbxBoneInfo>> _bones;
    std::vector<std::shared_ptr<FbxAnimClipInfo>> _animClips;
    FbxArray<FbxString *> _animNames;
};
