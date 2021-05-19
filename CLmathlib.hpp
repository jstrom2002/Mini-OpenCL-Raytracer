#pragma once
#include <cmath>
#include <algorithm>
#include <cassert>
#include <cstring>
#include <iostream>
#include <glm/glm.hpp>

const float MATH_PI = 3.141592654f;
const float MATH_2PI = 6.283185307f;
const float MATH_1DIVPI = 0.318309886f;
const float MATH_1DIV2PI = 0.159154943f;
const float MATH_PIDIV2 = 1.570796327f;
const float MATH_PIDIV4 = 0.785398163f;

struct CLTriangle;

class float3
{
public:
    float3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
    float3(float val) : x(val), y(val), z(val) {}
    float3() : x(0), y(0), z(0) {}
    float3(glm::vec3 v) : x(v.x), y(v.y), z(v.z){}
    //float3(glm::vec4 v) : x(v.x), y(v.y), z(v.z){}

    glm::vec3 toGLM() { return glm::vec3(x, y, z); }
    float  Length()    const { return sqrt(x*x + y*y + z*z); }
    float3 Normalize() const { return float3(x / Length(), y / Length(), z / Length()); }

    // Scalar operators
    float3 operator+ (float scalar) { return float3(x + scalar, y + scalar, z + scalar); }
    float3 operator- (float scalar) { return float3(x - scalar, y - scalar, z - scalar); }
    float3 operator* (float scalar) { return float3(x * scalar, y * scalar, z * scalar); }
    float3 operator/ (float scalar) { return float3(x / scalar, y / scalar, z / scalar); }
    friend float3 operator* (const float3& a, float b) { return float3(a.x * b, a.y * b, a.z * b); }

    // Vector operators
    friend float3 operator+ (const float3 &lhs, const float3 &rhs) { return float3(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z); }
    friend float3 operator- (const float3 &lhs, const float3 &rhs) { return float3(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z); }

    float3& operator+= (const float3 &other) { x += other.x; y += other.y; z += other.z; return *this; }
    float3& operator*= (const float  &other) { x *= other;   y *= other;   z *= other;   return *this; }
    float3& operator-= (const float3 &other) { x -= other.x; y -= other.y; z -= other.z; return *this; }
    friend float3 operator- (const float3& vec) { return float3(-vec.x, -vec.y, -vec.z); }

    // Access
    float&       operator[] (size_t i) { return (i == 0) ? x : (i == 1 ? y : z); }
    const float& operator[] (size_t i) const { return (i == 0) ? x : (i == 1 ? y : z); }    
public:
    float x, y, z;
private:    
    float w;// Used for align to 4 bytes
};
#define float3_aligned _declspec(align(16)) float3

class float2
{
public:
    float2(float x, float y) : x(x), y(y) {}
    float2(float val) : x(val), y(val) {}
    float2() : x(0), y(0) {}
    //float2(glm::vec2 v) : x(v.x), y(v.y) {}
    //float2(glm::vec3 v) : x(v.x), y(v.y) {}
    //float2(glm::vec4 v) : x(v.x), y(v.y) {}

    float Length() const { return sqrt(x*x + y*y); }
    float2 Normalize() const { return float2(x / Length(), y / Length()); }

    // Scalar operators
    float2 operator+ (float scalar) { return float2(x + scalar, y + scalar); }
    float2 operator- (float scalar) { return float2(x - scalar, y - scalar); }
    float2 operator* (float scalar) { return float2(x * scalar, y * scalar); }
    float2 operator/ (float scalar) { return float2(x / scalar, y / scalar); }

    // Vector operators
    float2 operator+ (const float2 &other) { return float2(x + other.x, y + other.y); }
    float2 operator- (const float2 &other) { return float2(x - other.x, y - other.y); }
    friend float2 operator+ (const float2 &lhs, const float2 &rhs) { return float2(lhs.x + rhs.x, lhs.y + rhs.y); }
    friend float2 operator- (const float2 &lhs, const float2 &rhs) { return float2(lhs.x - rhs.x, lhs.y - rhs.y); }

    float2 operator+= (const float2 &other) { x += other.x; y += other.y; return *this; }
    float2 operator*= (const float  &other) { x *= other;   y *= other;   return *this; }
    float2 operator-= (const float2 &other) { x -= other.x; y -= other.y; return *this; }
    float2 operator- () { return float2(-x, -y); }

    // Access
    float&       operator[] (size_t i) { return (i == 0) ? x : y; }
    const float& operator[] (size_t i) const { return (i == 0) ? x : y; }

public:
    float x, y;
};

inline float3 Cross(const float3& a, const float3& b)
{
    return float3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

inline float Dot(const float3& a, const float3& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline float Distance(const float3& a, const float3& b)
{
    return (b - a).Length();
}

inline float3 Min(const float3 &a, const float3 &b)
{
    return float3(std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z));
}

inline float3 Max(const float3 &a, const float3 &b)
{
    return float3(std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z));
}

struct CLBounds3
{
public:
    CLBounds3()
    {
        float minNum = std::numeric_limits<float>::lowest();
        float maxNum = std::numeric_limits<float>::max();
        min = float3(maxNum, maxNum, maxNum);
        max = float3(minNum, minNum, minNum);
    }

    CLBounds3(float3 p) :
        min(p), max(p)
    {}

    CLBounds3(float3 p1, float3 p2) :
        min(Min(p1, p2)),
        max(Max(p1, p2))
    {}

    const float3& operator[](int i) const
    {
        assert(i == 0 || i == 1);
        return (i == 0) ? min : max;
    }

    float3& operator[](int i)
    {
        assert(i == 0 || i == 1);
        return (i == 0) ? min : max;
    }

    float3 Corner(int corner) const
    {
        return float3((*this)[(corner & 1)].x,
            (*this)[(corner & 2) ? 1 : 0].y,
            (*this)[(corner & 4) ? 1 : 0].z);
    }

    float3 Diagonal() const { return max - min; }
    float SurfaceArea() const { float3 d = Diagonal(); return 2 * (d.x * d.y + d.x * d.z + d.y * d.z); }
    float Volume() const { float3 d = Diagonal(); return d.x * d.y * d.z; }

    unsigned int MaximumExtent() const
    {
        float3 d = Diagonal();
        if (d.x > d.y && d.x > d.z)
            return 0;
        else if (d.y > d.z)
            return 1;
        else
            return 2;
    }

    float3 Offset(const float3 &p) const {
        float3 o = p - min;
        if (max.x > min.x) o.x /= max.x - min.x;
        if (max.y > min.y) o.y /= max.y - min.y;
        if (max.z > min.z) o.z /= max.z - min.z;
        return o;
    }

    bool Intersects(const CLTriangle &triangle) const;
    void Project(float3 axis, float &mins, float &maxs) const;

public:
    float3 min;
    float3 max;
};

inline CLBounds3 Union(const CLBounds3 &b, const float3 &p)
{
    CLBounds3 ret;
    ret.min = Min(b.min, p);
    ret.max = Max(b.max, p);
    return ret;
}

inline CLBounds3 Union(const CLBounds3 &b1, const CLBounds3 &b2)
{
    CLBounds3 ret;
    ret.min = Min(b1.min, b2.min);
    ret.max = Max(b1.max, b2.max);
    return ret;
}