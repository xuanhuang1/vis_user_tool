#ifndef MARCHINGCUBE_H
#define MARCHINGCUBE_H
#include <vector>

void MarchingCube(const std::vector<char> &input,
			const int dim[3],
		  	std::vector<float> &output_vertices,
		  	std::vector<float> &output_normals, char threshold);

#endif