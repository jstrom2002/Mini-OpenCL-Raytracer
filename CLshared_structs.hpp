#pragma once
#ifdef __cplusplus
#include "CLmathlib.hpp"
#endif

#ifndef __cplusplus
typedef struct
{
    float3 pos[2];
} Bounds3;
#endif

typedef struct Material
{
#ifdef __cplusplus
    Material() : diffuse(float3(0.2)), specular(float3(1)), roughness(9999),ior(0) {}
#endif
    // Basic material properties.
    float3 diffuse;
    float3 specular;
    float3 emission;
    unsigned int type;
    float roughness;
    float ior;
    int padding;
} Material;

typedef struct Vertex
{
#ifdef __cplusplus
    Vertex() {}
    Vertex(const float3& position, const float2& texcoord, const float3& normal)
        : position(position), uv(texcoord.x, texcoord.y, 0),
        normal(normal)
    {}
#endif
    float3 position;
    float3 uv;
    float3 normal;
    float3 tangent_s;
    float3 tangent_t;
} Vertex;

typedef struct Triangle
{
#ifdef __cplusplus
    Triangle(Vertex v1, Vertex v2, Vertex v3, unsigned int mtlIndex)
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

    Bounds3 GetBounds() const
    {
        return Union(Bounds3(v1.position, v2.position), v3.position);
    }
#endif
    Vertex v1, v2, v3;
    unsigned int mtlIndex;
    unsigned int padding[3];
} Triangle;

typedef struct Light
{
#ifdef __cplusplus
    Light() {}
#endif 
    float3 position;
    float3 direction;
    int type;
    float intensity;
    float attenuation;
}Light;

typedef struct LinearBVHNode
{
#ifdef __cplusplus
    LinearBVHNode() {}
#endif    
    // NOTE: ensure that this struct is 48 byte total size.
    Bounds3 bounds;
    unsigned int offset; // primitives (leaf) or second child (interior) offset
    unsigned short nPrimitives;  // 0 -> interior node
    unsigned char axis;          // interior node: xyz
    unsigned char pad[9];        

} LinearBVHNode;