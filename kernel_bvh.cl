#include "CLshared_structs.hpp"

#define PI 3.14159265359f
#define TWO_PI 6.28318530718f
#define INV_PI 0.31830988618f
#define INV_TWO_PI 0.15915494309f
#define MAX_RENDER_DIST 100000.0f


typedef struct
{
    float3 origin;
    float3 dir;
    float3 invDir;
    int sign[3];
} Ray;

typedef struct
{
    bool hit;
    Ray ray;
    float t;
    float3 pos;
    float3 uv;
    float3 normal;
    const __global CLTriangle* object;
} IntersectData;

typedef struct
{
    __global CLTriangle* triangles;
    __global CLLinearBVHNode* nodes;
    __global CLMaterial* materials;
    unsigned int lightBounces;
    int lightType;
    float skyboxIntensity;
    float3 cameraPos;
    float3 cameraFront;
    float3 cameraUp;
} Scene;

Ray InitRay(float3 origin, float3 dir)
{
    dir = normalize(dir);

    Ray r;
    r.origin = origin;
    r.dir = dir;
    r.invDir = 1.0f / dir;
    r.sign[0] = r.invDir.x < 0;
    r.sign[1] = r.invDir.y < 0;
    r.sign[2] = r.invDir.z < 0;

    return r;
}

unsigned int HashUInt32(unsigned int x) {
    return 1103515245 * x + 12345;
}
unsigned int hash(unsigned int* x) {
    *x ^= *x >> 16;
    *x *= 0x7feb352dU;
    *x ^= *x >> 15;
    *x *= 0x846ca68bU;
    *x ^= *x >> 16;
    return *x;
}
float GetRandomFloat(unsigned int* seed)
{
    return (float)(hash(seed)) / (float)(0xffffffffU);
}


float3 reflect(float3 v, float3 n)
{
    return -v + 2.0f * dot(v, n) * n;
}

float3 SampleHemisphereCosine(float3 n, unsigned int* seed)
{
    float phi = TWO_PI * GetRandomFloat(seed);
    float sinThetaSqr = GetRandomFloat(seed);
    float sinTheta = sqrt(sinThetaSqr);

    float3 axis = fabs(n.x) > 0.001f ? (float3)(0.0f, 1.0f, 0.0f) : (float3)(1.0f, 0.0f, 0.0f);
    float3 t = normalize(cross(axis, n));
    float3 s = cross(n, t);

    return normalize(s*cos(phi)*sinTheta + t*sin(phi)*sinTheta + n*sqrt(1.0f - sinThetaSqr));
}

float3 SampleSky(float3 pos, float skyboxIntensity)
{
    float3 bkgCol = (float3)(0.5f, 0.5f, 0.5f);
    return bkgCol * skyboxIntensity;
}

bool RayTriangle(const Ray* r, const __global CLTriangle* tri, IntersectData* isect)
{
    bool CULL_BACKFACE = false;
    float HIT_EPSILON = 1.0e-8f;
    for (int i = 0; i < 2; ++i)
    {
        float3 t1, t2, t3;
        t1 = tri->v1.position;
        t2 = tri->v2.position;
        t3 = tri->v3.position;        

        float3 e1 = t2 - t1;
        float3 e2 = t3 - t1;
        // Calculate planes normal vector
        float3 pvec = cross(r->dir, e2);
        float det = dot(e1, pvec);

        // Cull back face (optional), check if ray is too close to being parallel to plane for a ray hit.
        if (det < 0 && CULL_BACKFACE || det < HIT_EPSILON || -det > HIT_EPSILON)
        {
            continue;
        }

        // Inside-out test. Check edge 1.
        float inv_det = 1.0f / det;
        float3 tvec = r->origin - t1;
        float u = dot(tvec, pvec) * inv_det;
        if (u < 0.0f || u > 1.0f)
        {
            continue;
        }
        // Check edge 2.
        float3 qvec = cross(tvec, e1);
        float v = dot(r->dir, qvec) * inv_det;
        if (v < 0.0f || u + v > 1.0f)
        {
            continue;
        }

        // Get parameter t.
        float t = dot(e2, qvec) * inv_det;

        if (t < isect->t)
        {
            isect->hit = true;
            isect->t = t;
            isect->pos = isect->ray.origin + isect->ray.dir * t;
            isect->object = tri;
            isect->normal = normalize(u * tri->v2.normal + v * tri->v3.normal + (1.0f - u - v) * tri->v1.normal);
            isect->uv = u * tri->v2.uv + v * tri->v3.uv + (1.0f - u - v) * tri->v1.uv;
            return true;
        }

        return false;
    }
}


bool RayBounds(const __global CLBounds3* bounds, const Ray* ray, float t)
{
    float t0 = max(0.0f, (bounds->pos[ray->sign[0]].x - ray->origin.x) * ray->invDir.x);
    float t1 = min(t, (bounds->pos[1 - ray->sign[0]].x - ray->origin.x) * ray->invDir.x);

    t0 = max(t0, (bounds->pos[ray->sign[1]].y - ray->origin.y) * ray->invDir.y);
    t1 = min(t1, (bounds->pos[1 - ray->sign[1]].y - ray->origin.y) * ray->invDir.y);

    t0 = max(t0, (bounds->pos[ray->sign[2]].z - ray->origin.z) * ray->invDir.z);
    t1 = min(t1, (bounds->pos[1 - ray->sign[2]].z - ray->origin.z) * ray->invDir.z);

    return (t1 >= t0);

}

IntersectData Intersect(Ray *ray, const Scene* scene)
{
    IntersectData isect;
    isect.hit = false;
    isect.ray = *ray;
    isect.t = MAX_RENDER_DIST;
    
    float t;
    // Follow ray through BVH nodes to find primitive intersections
    int toVisitOffset = 0, currentNodeIndex = 0;
    int nodesToVisit[64];
    while (true)
    {
        __global CLLinearBVHNode* node = &scene->nodes[currentNodeIndex];

        if (RayBounds(&node->bounds, ray, isect.t)){
            // Leaf node
            if (node->nPrimitives > 0)
            {
                // Intersect ray with primitives in leaf BVH node
                for (int i = 0; i < node->nPrimitives; ++i)                
                    RayTriangle(ray, &scene->triangles[node->offset + i], &isect);                
                if (toVisitOffset == 0) 
                    break;
                currentNodeIndex = nodesToVisit[--toVisitOffset];
            }
            else
            {
                // Put far BVH node on _nodesToVisit_ stack, advance to near node
                if (ray->sign[node->axis]){
                    nodesToVisit[toVisitOffset++] = currentNodeIndex + 1;
                    currentNodeIndex = node->offset;
                }
                else{
                    nodesToVisit[toVisitOffset++] = node->offset;
                    currentNodeIndex = currentNodeIndex + 1;
                }
            }
        }
        else
        {
            if (toVisitOffset == 0) 
                break;
            currentNodeIndex = nodesToVisit[--toVisitOffset];
        }
    }

    return isect;
}

float DistributionGGX(float cosTheta, float alpha)
{
    float alpha2 = alpha*alpha;
    return alpha2 * INV_PI / pow(cosTheta * cosTheta * (alpha2 - 1.0f) + 1.0f, 2.0f);
}

float3 SampleGGX(float3 n, float alpha, float* cosTheta, unsigned int* seed)
{
    float phi = TWO_PI * GetRandomFloat(seed);
    float xi = GetRandomFloat(seed);
    *cosTheta = pow(GetRandomFloat(seed), 1.0f / (alpha + 1.0f));//<-- temporary hack, this is actually the same as Blinn sampling
    float sinTheta = sqrt(max(0.0f, 1.0f - (*cosTheta) * (*cosTheta)));

    float3 axis = fabs(n.x) > 0.001f ? (float3)(0.0f, 1.0f, 0.0f) : (float3)(1.0f, 0.0f, 0.0f);
    float3 t = normalize(cross(axis, n));
    float3 s = cross(n, t);

    return normalize(s*cos(phi)*sinTheta + t*sin(phi)*sinTheta + n*(*cosTheta));
}

float GeometrySchlickGGX(float NdotV, float roughness){
    float r = (roughness + 1.0f);
    float k = (r * r) / 8.0f;
    float nom = NdotV;
    float denom = NdotV * (1.0f - k) + k;
    return nom / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0f);
    float NdotL = max(dot(N, L), 0.0f);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

float3 FresnelSchlick(float3 f0, float nDotWi)
{
    return f0 + (1.0f - f0) * pow(1.0f - nDotWi, 5.0f);
}

float3 SampleDiffuse(float3 wo, float3* wi, float* pdf, float3 texcoord, float3 normal, const __global CLMaterial* material, unsigned int* seed)
{
    *wi = SampleHemisphereCosine(normal, seed);
    *pdf = dot(*wi, normal) * INV_PI;
    return material->diffuse * INV_PI;
}

float3 SampleSpecular(float3 wo, float3* wi, float* pdf, float3 normal, 
    const __global CLMaterial* material, unsigned int* seed)
{
    float cosTheta=1;
    float alpha = 2.0f / pow(material->roughness, 2.0f) - 2.0f;
    float3 wh = SampleGGX(normal, alpha, &cosTheta, seed);
    *wi = reflect(wo, wh);
    if (dot(*wi, normal) * dot(wo, normal) < 0.000001f) 
        return 0.0f;

    float D = DistributionGGX(cosTheta, alpha);
    float G = GeometrySmith(normal, wo, *wi, material->roughness);
    float3 F0 = 0.04f;
    float3 F = FresnelSchlick(F0, max(dot(wh, wo), 0.0f));

    *pdf = D * cosTheta / (4.0f * max(dot(wo, wh), 0.0f));

    // Actually, _material->ior_ isn't ior value, this is f0 value for now
    float3 colorOut = D / ((4.0f * max(dot(*wi, normal),0.0f) * max(dot(wo, normal),0.0f)) + 0.001f) 
        * material->specular;
    return colorOut;
}

float3 SampleBrdf(float3 wo, float3* wi, float* pdf, float3 texcoord, float3 normal, 
    const __global CLMaterial* material, unsigned int* seed)
{
    // Alternate between specular and diffuse rays
    if (GetRandomFloat(seed) > 0.5f)    
        return SampleSpecular(wo, wi, pdf, normal, material, seed) * 1.0f;    
    else    
        return SampleDiffuse(wo, wi, pdf, texcoord, normal, material, seed) * 1.0f;    
}

float3 lightPixel(Ray* ray, const Scene* scene, const IntersectData* isect)
{
    // Light properties.
    float3 lightPosition = (float3)(0.0f, -10.0f, 16.0f);
    float3 lightDirection = (float3)(-0.5f, 0.4f, -0.1f);
    float lightIntensity = 1.0f;
    
    // Lighting calc vals.
    float NdotL = 1.0f;
    float attn = 1.0f;
    float3 L, X;

    if (scene->lightType <= 0)// Directional light.
    {
        // Calculate N*L.
        L = -lightDirection;
        NdotL = max(dot(isect->normal, L), 0.0f);
    }
    else if (scene->lightType == 1)// Point light.
    {
        lightIntensity = 16.0f;
        float lightFalloff = 0.8f;

        // Calculate N*L.
        X = ray->origin + ray->dir * isect->t;
        L = lightPosition - X;
        NdotL = max(dot(isect->normal, L), 0.0f);

        // Calculate attenuation.
        float3 eye = L - X;
        float d = sqrt(dot(eye, eye));
        attn = 1.0 / (lightFalloff * (d * d));//quadratic attenuation.
    }
    else if (scene->lightType >= 2)// Spot light.
    {
        X = ray->origin + ray->dir * isect->t;
        L = lightPosition - X;
        NdotL = max(dot(isect->normal, L), 0.0f);

        // ADD MORE LATER
    }

    return attn * lightIntensity * NdotL;
}

float3 Render(Ray* ray, const Scene* scene, unsigned int* seed)
{
    float3 radiance = 0.0f;
    float3 beta = 1.0f;
            
    for (int i = 0; i < scene->lightBounces; ++i)
    {
        IntersectData isect = Intersect(ray, scene);

        if (!isect.hit)
        {
            radiance += beta * SampleSky(isect.pos, scene->skyboxIntensity);
            break;
        }
        
        const __global CLMaterial* material = &scene->materials[isect.object->mtlIndex];
        radiance += beta * material->emission * 50.0f;

        float3 wi;
        float3 wo = -ray->dir;
        float pdf = 0.0f;
        float3 f = SampleBrdf(wo, &wi, &pdf, isect.uv, isect.normal, material, seed);
        if (pdf <= 0.0f || pdf != pdf) 
            break;

        float3 mul = f *dot(wi, isect.normal) / pdf;
        beta *= mul;

        // Calculate lighting for ray hit.
        radiance += lightPixel(ray,scene,&isect) * material->diffuse * beta;

        *ray = InitRay(isect.pos + wi * 0.01f, wi);
    }
    
    return max(radiance, 0.0f);
}

Ray CreateRay(unsigned int width, unsigned int height, float3 cameraPos, float3 cameraFront,
    float3 cameraUp, unsigned int* seed)
{
    float invWidth = 1.0f / (float)(width);
    float invHeight = 1.0f / (float)(height);
    float aspectratio = (float)(width) / (float)(height);
    float angle = tan(0.5f * (45.0f * 3.1415f / 180.0f));

    float x = (float)(get_global_id(0) % width) + GetRandomFloat(seed) - 0.5f;
    float y = (float)(get_global_id(0) / width) + GetRandomFloat(seed) - 0.5f;

    x = (2.0f * ((x + 0.5f) * invWidth) - 1) * angle * aspectratio;
    y = -(1.0f - 2.0f * ((y + 0.5f) * invHeight)) * angle;

    float3 dir = normalize(x * cross(cameraFront, cameraUp) + y * cameraUp + cameraFront);

    return InitRay(cameraPos, dir);
}

float3 ToGamma(float3 value)
{
    return pow(value, 0.454545f);
}

float3 FromGamma(float3 value)
{
    return pow(value, 2.2f);
}

__kernel void KernelEntry
(
    __global float3* result,//0
    __global CLTriangle* triangles,//1
    __global CLLinearBVHNode* nodes,//2
    __global CLMaterial* materials,//3
    unsigned int width,//4
    unsigned int height,//5
    unsigned int frameCount,//6
    unsigned int frameSeed,//7
    int lightBounces,//8
    int lightType,//9
    float skyboxIntensity,//10
    float3 cameraPos,//11
    float3 cameraFront,//12
    float3 cameraUp//13
)
{
    Scene scene = { 
        triangles, 
        nodes, 
        materials,
        lightBounces,
        lightType,
        skyboxIntensity,
        cameraPos,
        cameraFront,
        cameraUp
    };

    unsigned int seed = get_global_id(0) + HashUInt32(frameCount);
    Ray ray = CreateRay(width, height, cameraPos, cameraFront, cameraUp, &seed);
    float3 radiance = Render(&ray, &scene, &seed);

    if (frameCount == 0)
        result[get_global_id(0)] = pow(radiance, 0.45454545f);
    else
    {
        result[get_global_id(0)] = ToGamma(
            (FromGamma(result[get_global_id(0)]) * (frameCount - 1) + radiance) / frameCount);
    }
}
