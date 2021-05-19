#pragma once

namespace Glaze3D
{

	// Helper for example code, delete in final release.
	class CLOBJloader
	{
	public:
		CLOBJloader() {}
		void Load(const char* filename, unsigned int maxPrimitivesInNode);
	private:
		void LoadTriangles(const char* filename);
		void LoadMaterials(const char* filename);
	};
}