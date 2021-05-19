#include "CLBVHnode.h"
#include "CLRaytracer.h"
#include "CLEngineBase.h"

namespace Glaze3D
{
    CLBVHBuildNode* CLBVHScene::RecursiveBuild(
        std::vector<CLBVHPrimitiveInfo>& primitiveInfo,
        unsigned int start,
        unsigned int end, unsigned int* totalNodes,
        std::vector<CLTriangle>& orderedTriangles)
    {
        assert(start <= end);

        CLBVHBuildNode* node = new CLBVHBuildNode;
        (*totalNodes)++;

        // Compute bounds of all primitives in BVH node
        CLBounds3 bounds;
        for (unsigned int i = start; i < end; ++i)
        {
            bounds = Union(bounds, primitiveInfo[i].bounds);
        }

        unsigned int nPrimitives = end - start;
        if (nPrimitives == 1)
        {
            // Create leaf
            int firstPrimOffset = orderedTriangles.size();
            for (unsigned int i = start; i < end; ++i)
            {
                int primNum = primitiveInfo[i].primitiveNumber;
                orderedTriangles.push_back(m_Triangles[primNum]);
            }
            node->InitLeaf(firstPrimOffset, nPrimitives, bounds);
            return node;
        }
        else
        {
            // Compute bound of primitive centroids, choose split dimension
            CLBounds3 centroidBounds;
            for (unsigned int i = start; i < end; ++i)
            {
                centroidBounds = Union(centroidBounds, primitiveInfo[i].centroid);
            }
            unsigned int dim = centroidBounds.MaximumExtent();

            // Partition primitives into two sets and build children
            unsigned int mid = (start + end) / 2;
            if (centroidBounds.max[dim] == centroidBounds.min[dim])
            {
                // Create leaf
                unsigned int firstPrimOffset = orderedTriangles.size();
                for (unsigned int i = start; i < end; ++i)
                {
                    unsigned int primNum = primitiveInfo[i].primitiveNumber;
                    orderedTriangles.push_back(m_Triangles[primNum]);
                }
                node->InitLeaf(firstPrimOffset, nPrimitives, bounds);
                return node;
            }
            else
            {
                if (nPrimitives <= 2)
                {
                    // Partition primitives into equally-sized subsets
                    mid = (start + end) / 2;
                    std::nth_element(&primitiveInfo[start], &primitiveInfo[mid], &primitiveInfo[end - 1] + 1,
                        [dim](const CLBVHPrimitiveInfo& a, const CLBVHPrimitiveInfo& b)
                        {
                            return a.centroid[dim] < b.centroid[dim];
                        });
                }
                else
                {
                    // Partition primitives using approximate SAH
                    const unsigned int nBuckets = 12;
                    CLBucketInfo buckets[nBuckets];

                    // Initialize _BucketInfo_ for SAH partition buckets
                    for (unsigned int i = start; i < end; ++i)
                    {
                        int b = nBuckets * centroidBounds.Offset(primitiveInfo[i].centroid)[dim];
                        if (b == nBuckets) b = nBuckets - 1;
                        assert(b >= 0 && b < nBuckets);
                        buckets[b].count++;
                        buckets[b].bounds = Union(buckets[b].bounds, primitiveInfo[i].bounds);
                    }

                    // Compute costs for splitting after each bucket
                    float cost[nBuckets - 1];
                    for (unsigned int i = 0; i < nBuckets - 1; ++i)
                    {
                        CLBounds3 b0, b1;
                        int count0 = 0, count1 = 0;
                        for (unsigned int j = 0; j <= i; ++j)
                        {
                            b0 = Union(b0, buckets[j].bounds);
                            count0 += buckets[j].count;
                        }
                        for (unsigned int j = i + 1; j < nBuckets; ++j)
                        {
                            b1 = Union(b1, buckets[j].bounds);
                            count1 += buckets[j].count;
                        }
                        cost[i] = 1.0f + (count0 * b0.SurfaceArea() + count1 * b1.SurfaceArea()) / bounds.SurfaceArea();
                    }

                    // Find bucket to split at that minimizes SAH metric
                    float minCost = cost[0];
                    unsigned int minCostSplitBucket = 0;
                    for (unsigned int i = 1; i < nBuckets - 1; ++i)
                    {
                        if (cost[i] < minCost)
                        {
                            minCost = cost[i];
                            minCostSplitBucket = i;
                        }
                    }

                    // Either create leaf or split primitives at selected SAH bucket
                    float leafCost = float(nPrimitives);
                    if (nPrimitives > m_MaxPrimitivesInNode || minCost < leafCost)
                    {
                        CLBVHPrimitiveInfo* pmid = std::partition(&primitiveInfo[start], &primitiveInfo[end - 1] + 1,
                            [=](const CLBVHPrimitiveInfo& pi)
                            {
                                int b = nBuckets * centroidBounds.Offset(pi.centroid)[dim];
                                if (b == nBuckets) b = nBuckets - 1;
                                assert(b >= 0 && b < nBuckets);
                                return b <= minCostSplitBucket;
                            });
                        mid = pmid - &primitiveInfo[0];
                    }
                    else
                    {
                        // Create leaf
                        unsigned int firstPrimOffset = orderedTriangles.size();
                        for (unsigned int i = start; i < end; ++i)
                        {
                            unsigned int primNum = primitiveInfo[i].primitiveNumber;
                            orderedTriangles.push_back(m_Triangles[primNum]);
                        }
                        node->InitLeaf(firstPrimOffset, nPrimitives, bounds);

                        return node;
                    }
                }

                node->InitInterior(dim,
                    RecursiveBuild(primitiveInfo, start, mid,
                        totalNodes, orderedTriangles),
                    RecursiveBuild(primitiveInfo, mid, end,
                        totalNodes, orderedTriangles));
            }
        }

        return node;
    }

    unsigned int CLBVHScene::FlattenBVHTree(CLBVHBuildNode* node, unsigned int* offset)
    {
        CLLinearBVHNode* linearNode = &m_Nodes[*offset];
        linearNode->bounds = node->bounds;
        unsigned int myOffset = (*offset)++;
        if (node->nPrimitives > 0)
        {
            assert(!node->children[0] && !node->children[1]);
            assert(node->nPrimitives < 65536);
            linearNode->offset = node->firstPrimOffset;
            linearNode->nPrimitives = node->nPrimitives;
        }
        else
        {
            // Create interior flattened BVH node
            linearNode->axis = node->splitAxis;
            linearNode->nPrimitives = 0;
            FlattenBVHTree(node->children[0], offset);
            linearNode->offset = FlattenBVHTree(node->children[1], offset);
        }

        return myOffset;
    }

    void CLBVHScene::CreateBVHTrees(unsigned int maxPrimitivesInNode)
    {
        m_MaxPrimitivesInNode = maxPrimitivesInNode;

        // Build BVH for scene.
        std::vector<CLBVHPrimitiveInfo> primitiveInfo(m_Triangles.size());
        for (unsigned int i = 0; i < m_Triangles.size(); ++i)
            primitiveInfo[i] = { i, m_Triangles[i].GetBounds() };

        unsigned int totalNodes = 0;
        std::vector<CLTriangle> orderedTriangles;
        m_Root = RecursiveBuild(primitiveInfo, 0, m_Triangles.size(), &totalNodes, orderedTriangles);
        m_Triangles.swap(orderedTriangles);

        // Compute representation of depth-first traversal of BVH tree.
        m_Nodes.resize(totalNodes);
        unsigned int offset = 0;
        FlattenBVHTree(m_Root, &offset);
        assert(totalNodes == offset);

        // Finally, setup buffers for output.
        SetupBuffers();
    }

    void CLBVHScene::SetupBuffers()
    {
        cl_int errCode;

        // Set GPU-side triangle array.
        m_TriangleBuffer = cl::Buffer(eng->render->m_CLContext->GetContext(),
            CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, m_Triangles.size() *
            sizeof(CLTriangle), m_Triangles.data(), &errCode);
        if (errCode)
            throw CLException("Failed to create scene buffer", errCode);
        eng->render->SetUniform<cl::Buffer>((int)RenderKernelArgument_t::BUFFER_SCENE, m_TriangleBuffer);

        // Set GPU-side BVH node array.
        m_NodeBuffer = cl::Buffer(eng->render->m_CLContext->GetContext(),
            CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, m_Nodes.size() * sizeof(CLLinearBVHNode),
            m_Nodes.data(), &errCode);
        if (errCode)
            throw CLException("Failed to create BVH node buffer", errCode);
        eng->render->SetUniform<cl::Buffer>((int)RenderKernelArgument_t::BUFFER_NODE, m_NodeBuffer);

        // Set GPU-side material array.
        m_MaterialBuffer = cl::Buffer(eng->render->m_CLContext->GetContext(),
            CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, m_Materials.size() *
            sizeof(CLMaterial), m_Materials.data(), &errCode);
        if (errCode)
            throw CLException("Failed to create material buffer", errCode);
        eng->render->SetUniform<cl::Buffer>((int)RenderKernelArgument_t::BUFFER_MATERIAL, m_MaterialBuffer);
    }

    void ComputeTangentSpace(CLVertex& v1, CLVertex& v2, CLVertex& v3)
    {
        const float3& v1p = v1.position;
        const float3& v2p = v2.position;
        const float3& v3p = v3.position;

        const float2& v1t = float2(v1.uv[0], v1.uv[1]);
        const float2& v2t = float2(v2.uv[0], v2.uv[1]);
        const float2& v3t = float2(v3.uv[0], v3.uv[1]);

        double x1 = v2p.x - v1p.x;
        double x2 = v3p.x - v1p.x;
        double y1 = v2p.y - v1p.y;
        double y2 = v3p.y - v1p.y;
        double z1 = v2p.z - v1p.z;
        double z2 = v3p.z - v1p.z;

        double s1 = v2t.x - v1t.x;
        double s2 = v3t.x - v1t.x;
        double t1 = v2t.y - v1t.y;
        double t2 = v3t.y - v1t.y;

        double r = 1.0 / (s1 * t2 - s2 * t1);
        float3 sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r);
        float3 tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r);

        v1.tangent_s += sdir;
        v2.tangent_s += sdir;
        v3.tangent_s += sdir;

        v1.tangent_t += tdir;
        v2.tangent_t += tdir;
        v3.tangent_t += tdir;

    }
}