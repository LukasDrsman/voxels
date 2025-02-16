#include "window.hpp"
#include "irenderable.hpp"
#include <iostream>
#include <array>
#include <functional>
#include <memory>
#include <GLFW/glfw3.h>
#include <glm/vec3.hpp>

#include "voxel.hpp"
#include "voxel_scene.hpp"


inline vox::Octree *genericVolume(std::function<bool(glm::vec3)> enclosed, glm::vec3 center, float norm, int max_depth, glm::vec3 color)
{
	if (enclosed(center) && max_depth == 0) return new vox::Octree(color);
	if (max_depth == 0) return new vox::Octree();

	std::array<vox::Octree *, 8> children{};

	for (int i = 0; i < 8; i++)
	{
		children[i] = genericVolume(enclosed,
			vox::DCENTERS[i] * norm * 0.5f + center,
			norm * 0.5, max_depth - 1, color
		);
	}

	return new vox::Octree(children);
}

int main()
{
	const ctx::Window win(640, 640);

	const auto model_heart = genericVolume([](glm::vec3 c) -> bool
	{
		const float cubed = 6.f*c.x*c.x + 16.f*c.z*c.z + 7.f*c.y*c.y - 1.f;
		return (
			cubed*cubed*cubed
			- 113.0f*c.x*c.x*c.y*c.y*c.y
			- 0.005f*c.z*c.z*c.y*c.y*c.y < 0
		);
	}, glm::vec3{0, 0, 0}, 0.5, 4, glm::vec3{0.65882, 0.19607, 0.42745});

	auto model_sphere = genericVolume([](glm::vec3 c) -> bool
	{
		return (c.x*c.x + c.y*c.y + c.z*c.z) < 0.2;
	}, glm::vec3{0, 0, 0}, 0.5, 4, glm::vec3{0.0, 0.8, 0.5});

	// auto model_sine_shit = genericVolume([](glm::vec3 c) -> bool
	// {
	// 	return (glm::sin(64.f * c.x*c.x) + glm::cos(64.f * c.y*c.y) + 16.f * c.z*c.z) < 0;
	// }, glm::vec3{0, 0, 0}, 0.5, 7);

	model_heart->cull();
	model_sphere->cull();

	const vox::Voxel voxel_heart(model_heart, 0.8, glm::vec3{0, 0.5, 0});
	const vox::Voxel voxel_sphere(model_sphere, 0.8, glm::vec3{0, -0.5, 0});

	ctx::VoxelScene scene(glm::vec3{0.18039, 0.00784, 0.16470});
	scene.add_model(voxel_sphere);
	scene.add_model(voxel_heart);

	win.run(scene);
	// win.run(dummy);
}
