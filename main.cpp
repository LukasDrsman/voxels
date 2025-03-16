#include "window.hpp"
#include "interfaces.hpp"
#include <iostream>
#include <array>
#include <functional>
#include <memory>
#include <GLFW/glfw3.h>
#include <glm/vec3.hpp>

#include "norm.hpp"
#include "voxel.hpp"
#include "voxel_linearized.hpp"
// #include "scene.hpp"
#include "generators.hpp"

int main()
{

	// const auto model_heart = vox::genericVolume([](glm::vec3 c) -> bool
	// {
	// 	const float cubed = 6.f*c.x*c.x + 16.f*c.z*c.z + 7.f*c.y*c.y - 1.f;
	// 	return (
	// 		cubed*cubed*cubed
	// 		- 113.0f*c.x*c.x*c.y*c.y*c.y
	// 		- 0.005f*c.z*c.z*c.y*c.y*c.y < 0
	// 	);
	// }, glm::vec3{0, 0, 0}, 0.5, 9, glm::vec3{0.65882, 0.19607, 0.42745});
	//
	// model_heart->cull();
	// vox::Voxel heart(model_heart);
	//
	// const ctx::Window win1(640, 640);
	// win1.run(heart);

	// const auto model_pc = vox::genericPointCloud(
	// 	vox::pcdToPointCloud(
	// 		"/home/lukas/projects/voxels/pcd/bunny.pcd",
	// 		glm::vec3{0.4, 0.2, 0.2}, 1.f
	// 	), glm::vec3{0,0,0}, 0.5, 8
	// );

	const auto model_pc = vox::genericPointCloud(
		vox::randomPointCloud(4500),
		glm::vec3{0,0,0}, 0.5, 5
	);
	//
	// std::cout << "node count: " << model_pc->node_count() << std::endl;
	model_pc->cull();
	// vox::Voxel point_cloud(model_pc);
	lin::LinVox point_cloud(model_pc);

	const ctx::Window win2(640, 640);
	win2.run(point_cloud);
}
