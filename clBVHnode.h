#pragma once
#include "CLmathlib.hpp"
#include "CLshared_structs.hpp"
#include <CL/cl2.hpp>

namespace Glaze3D
{
    struct BVHBuildNode
    {
        void InitLeaf(int first, int n, const Bounds3& b)
        {
            firstPrimOffset = first;
            nPrimitives = n;
            bounds = b;
            children[0] = children[1] = nullptr;
        }

        void InitInterior(int axis, BVHBuildNode* c0, BVHBuildNode* c1)
        {
            children[0] = c0;
            children[1] = c1;
            bounds = Union(c0->bounds, c1->bounds);
            splitAxis = axis;
            nPrimitives = 0;
        }

        Bounds3 bounds;
        BVHBuildNode* children[2];
        int splitAxis, firstPrimOffset, nPrimitives;
    };

    struct BVHPrimitiveInfo
    {
        BVHPrimitiveInfo() {}
        BVHPrimitiveInfo(unsigned int primitiveNumber, const Bounds3& bounds)
            : primitiveNumber(primitiveNumber), bounds(bounds),
            centroid(bounds.min * 0.5f + bounds.max * 0.5f)
        {}

        unsigned int primitiveNumber;
        Bounds3 bounds;
        float3 centroid;
    };

    struct BucketInfo
    {
        int count = 0;
        Bounds3 bounds;
    };

    class BVHScene
    {
    public:
        std::vector<Triangle> m_Triangles;
        std::vector<Material> m_Materials;
        std::vector<Light> m_Lights;
        std::vector<std::string> m_MaterialNames;

        BVHScene() {}
        void Load(const char* filename, unsigned int maxPrimitivesInNode);
        void SetupBuffers();

    private:
        cl::Buffer m_TriangleBuffer;
        cl::Buffer m_MaterialBuffer;

        BVHBuildNode* RecursiveBuild(
            std::vector<BVHPrimitiveInfo>& primitiveInfo,
            unsigned int start,
            unsigned int end, unsigned int* totalNodes,
            std::vector<Triangle>& orderedTriangles);

        unsigned int FlattenBVHTree(BVHBuildNode* node, unsigned int* offset);

        std::vector<LinearBVHNode> m_Nodes;
        unsigned int m_MaxPrimitivesInNode = 0;
        cl::Buffer m_NodeBuffer;
        BVHBuildNode* m_Root = nullptr;

        void LoadTriangles(const char* filename);
        void LoadMaterials(const char* filename);
    };
}