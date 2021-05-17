#include "CLOBJloader.h"
#include "CLBVHnode.h"
#include "CLEngineBase.h"
#include "CLRaytracer.h"
#include <vector>


namespace Glaze3D
{
	void CLOBJloader::Load(const char* filename, unsigned int maxPrimitivesInNode)
	{
		eng->render->m_Scene->m_MaxPrimitivesInNode = maxPrimitivesInNode;
		LoadTriangles(filename);
	}

	void CLOBJloader::LoadTriangles(const char* filename)
	{
        char mtlname[80];
        memset(mtlname, 0, 80);
        strncpy(mtlname, filename, strlen(filename) - 4);
        strcat(mtlname, ".mtl");

        LoadMaterials(mtlname);

        std::vector<float3> positions;
        std::vector<float3> normals;
        std::vector<float2> texcoords;

        CLMaterial currentMaterial;
        FILE* file = fopen(filename, "r");
        if (!file)
        {
            throw std::exception();//String argument is an MSVC only extension
            //throw std::exception("Failed to open scene file!");
        }

        unsigned int materialIndex = -1;

        while (true)
        {
            char lineHeader[128];
            int res = fscanf(file, "%s", lineHeader);
            if (res == EOF)
            {
                break;
            }
            if (strcmp(lineHeader, "v") == 0)
            {
                float3 vertex;
                fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
                positions.push_back(vertex);
            }
            else if (strcmp(lineHeader, "vt") == 0)
            {
                float2 uv;
                fscanf(file, "%f %f\n", &uv.x, &uv.y);
                texcoords.push_back(uv);
            }
            else if (strcmp(lineHeader, "vn") == 0)
            {
                float3 normal;
                fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
                normals.push_back(normal);
            }
            else if (strcmp(lineHeader, "usemtl") == 0)
            {
                char str[80];
                fscanf(file, "%s\n", str);
                for (unsigned int i = 0; i < eng->render->m_Scene->m_MaterialNames.size(); ++i)
                {
                    if (strcmp(str, eng->render->m_Scene->m_MaterialNames[i].c_str()) == 0)
                    {
                        materialIndex = i;
                        break;
                    }
                }

            }
            else if (strcmp(lineHeader, "f") == 0)
            {
                // Count spaces in line, which delimit values.
                int numberOfEntries = 0;//assume at least one value in line.
                char linestr[128];
                fgets(linestr, sizeof(linestr), file);

                // tokenize and handle values
                char* tokens = strtok(linestr, " ");
                std::vector<unsigned int> iv, it, in;
                while (tokens != NULL)
                {
                    if (strlen(tokens) > 1)
                    {
                        iv.push_back(0);
                        it.push_back(0);
                        in.push_back(0);
                        sscanf(tokens, "%d/%d/%d", &iv[iv.size() - 1], &it[it.size() - 1], &in[in.size() - 1]);
                    }
                    tokens = strtok(NULL, " ");
                }

                // Add all triangles to the array.
                for (int i = 0; i < iv.size() - 2; ++i)
                {
                    int idx1 = i + 0;
                    int idx2 = i + 1;
                    int idx3 = i + 2;
                    eng->render->m_Scene->m_Triangles.push_back(CLTriangle(
                        CLVertex(positions[iv[idx1] - 1], texcoords[it[idx1] - 1], normals[in[idx1] - 1]),
                        CLVertex(positions[iv[idx2] - 1], texcoords[it[idx2] - 1], normals[in[idx2] - 1]),
                        CLVertex(positions[iv[idx3] - 1], texcoords[it[idx3] - 1], normals[in[idx3] - 1]),
                        materialIndex
                    ));
                }
                // Add final triangle, connecting face back to the first vertex.
                if (iv.size() > 1)
                {
                    int idx1 = iv.size() - 2;
                    int idx2 = iv.size() - 1;
                    int idx3 = 0;
                    eng->render->m_Scene->m_Triangles.push_back(CLTriangle(
                        CLVertex(positions[iv[idx1] - 1], texcoords[it[idx1] - 1], normals[in[idx1] - 1]),
                        CLVertex(positions[iv[idx2] - 1], texcoords[it[idx2] - 1], normals[in[idx2] - 1]),
                        CLVertex(positions[iv[idx3] - 1], texcoords[it[idx3] - 1], normals[in[idx3] - 1]),
                        materialIndex
                    ));
                }
            }
        }
	}

	void CLOBJloader::LoadMaterials(const char* filename)
	{
        FILE* file = fopen(filename, "r");
        if (!file)
        {
            throw std::exception();//String argument is an MSVC only extension
            //throw std::exception("Failed to open material file!");
        }

        while (true)
        {
            char buf[128];
            int res = fscanf(file, "%s", buf);
            if (res == EOF)
            {
                break;
            }
            if (strcmp(buf, "newmtl") == 0)
            {
                char str[80];
                fscanf(file, "%s\n", str);
                eng->render->m_Scene->m_MaterialNames.push_back(str);
                eng->render->m_Scene->m_Materials.push_back(CLMaterial());
            }
            else if (strcmp(buf, "Kd") == 0)
            {
                fscanf(file, "%f %f %f\n", &eng->render->m_Scene->m_Materials.back().diffuse.x, &eng->render->m_Scene->m_Materials.back().diffuse.y, &eng->render->m_Scene->m_Materials.back().diffuse.z);
            }
            else if (strcmp(buf, "Ks") == 0)
            {
                fscanf(file, "%f %f %f\n", &eng->render->m_Scene->m_Materials.back().specular.x, &eng->render->m_Scene->m_Materials.back().specular.y, &eng->render->m_Scene->m_Materials.back().specular.z);
            }
            else if (strcmp(buf, "Ke") == 0)
            {
                fscanf(file, "%f %f %f\n", &eng->render->m_Scene->m_Materials.back().emission.x, &eng->render->m_Scene->m_Materials.back().emission.y, &eng->render->m_Scene->m_Materials.back().emission.z);
            }
            else if (strcmp(buf, "Ns") == 0)
            {
                fscanf(file, "%f\n", &eng->render->m_Scene->m_Materials.back().roughness);
            }
            else if (strcmp(buf, "Ni") == 0)
            {
                fscanf(file, "%f\n", &eng->render->m_Scene->m_Materials.back().ior);
            }
        }
    }
}