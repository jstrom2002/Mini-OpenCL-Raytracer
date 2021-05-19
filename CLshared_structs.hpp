#pragma once
#ifdef __cplusplus
#include "CLmathlib.hpp"
#endif

#ifndef __cplusplus
typedef struct
{
    float3 pos[2];
} CLBounds3;
#endif

typedef struct CLMaterial
{
#ifdef __cplusplus
    CLMaterial() : diffuse(float3(0.2)), specular(float3(1)), roughness(9999),ior(0) {}
#endif
    // Basic material properties.
    float3 diffuse;
    float3 specular;
    float3 emission;
    unsigned int type;
    float roughness;
    float ior;
    int padding;
} CLMaterial;

typedef struct CLVertex
{
#ifdef __cplusplus
    CLVertex() {}
    CLVertex(const float3& position, const float2& texcoord, const float3& normal)
        : position(position), uv(texcoord.x, texcoord.y, 0),
        normal(normal)
    {}
#endif
    float3 position;
    float3 uv;
    float3 normal;
    float3 tangent_s;
    float3 tangent_t;
} CLVertex;

typedef struct CLTriangle
{
#ifdef __cplusplus
    CLTriangle(CLVertex v1, CLVertex v2, CLVertex v3, unsigned int mtlIndex)
        : v1(v1), v2(v2), v3(v3), mtlIndex(mtlIndex)
    {}

    void Project(float3 axis, float& min, float& max) const
    {
        min = std::numeric_limits<float>::max();
        max = std::numeric_limits<float>::lowest();

        float3 points[3] = { v1.position, v2.position, v3.position };

        for (size_t i = 0; i < 3; ++i)
        {
            float val = Dot(points[i], axis);
            min = std::min(min, val);
            max = std::max(max, val);
        }
    }

    CLBounds3 GetBounds() const
    {
        return Union(CLBounds3(v1.position, v2.position), v3.position);
    }
#endif
    CLVertex v1, v2, v3;
    unsigned int mtlIndex;
    unsigned int padding[3];
} CLTriangle;

typedef struct CLLinearBVHNode
{
#ifdef __cplusplus
    CLLinearBVHNode() {}
#endif    
    // NOTE: ensure that this struct is 48 byte total size.
    CLBounds3 bounds;
    unsigned int offset; // primitives (leaf) or second child (interior) offset
    unsigned short nPrimitives;  // 0 -> interior node
    unsigned char axis;          // interior node: xyz
    unsigned char pad[9];        

} CLLinearBVHNode;