#pragma once
#include "CLmathlib.hpp"
#include "CLshared_structs.hpp"
#include <CL/cl2.hpp>

namespace Glaze3D
{
    struct CLBVHBuildNode
    {
        void InitLeaf(int first, int n, const CLBounds3& b)
        {
            firstPrimOffset = first;
            nPrimitives = n;
            bounds = b;
            children[0] = children[1] = nullptr;
        }

        void InitInterior(int axis, CLBVHBuildNode* c0, CLBVHBuildNode* c1)
        {
            children[0] = c0;
            children[1] = c1;
            bounds = Union(c0->bounds, c1->bounds);
            splitAxis = axis;
            nPrimitives = 0;
        }

        CLBounds3 bounds;
        CLBVHBuildNode* children[2];
        int splitAxis, firstPrimOffset, nPrimitives;
    };

    struct CLBVHPrimitiveInfo
    {
        CLBVHPrimitiveInfo() {}
        CLBVHPrimitiveInfo(unsigned int primitiveNumber, const CLBounds3& bounds)
            : primitiveNumber(primitiveNumber), bounds(bounds),
            centroid(bounds.min * 0.5f + bounds.max * 0.5f)
        {}

        unsigned int primitiveNumber;
        CLBounds3 bounds;
        float3 centroid;
    };

    struct CLBucketInfo
    {
        int count = 0;
        CLBounds3 bounds;
    };

    class CLBVHScene
    {
    public:
        std::vector<CLTriangle> m_Triangles;
        std::vector<CLMaterial> m_Materials;
        std::vector<std::string> m_MaterialNames;
        unsigned int m_MaxPrimitivesInNode = 0;

        CLBVHScene() {}
        void CreateBVHTrees(unsigned int maxPrimitivesInNode);

    private:
        cl::Buffer m_TriangleBuffer;
        cl::Buffer m_MaterialBuffer;

        CLBVHBuildNode* RecursiveBuild(
            std::vector<CLBVHPrimitiveInfo>& primitiveInfo,
            unsigned int start,
            unsigned int end, unsigned int* totalNodes,
            std::vector<CLTriangle>& orderedTriangles);

        unsigned int FlattenBVHTree(CLBVHBuildNode* node, unsigned int* offset);

        // Set uniforms for triangles, materials, and BVH nodes in the CL kernel code.
        void SetupBuffers();

        std::vector<CLLinearBVHNode> m_Nodes;
        cl::Buffer m_NodeBuffer;
        CLBVHBuildNode* m_Root = nullptr;
    };
}