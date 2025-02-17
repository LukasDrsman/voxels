#pragma once

#include <glm/vec3.hpp>
#include <vector>
#include "voxel.hpp"
#include "irenderable.hpp"

namespace ctx
{
    class VoxelScene final : public IRenderable
    {
        std::vector<vox::Voxel> models;
        glm::vec3 background_color{0, 0, 0};

    public:
        explicit VoxelScene(glm::vec3 _background_color);
        void render() override;
        void pre_render() override;
        void pre_render_cleanup() override;
        void add_model(vox::Voxel model);
    };

    inline VoxelScene::VoxelScene(glm::vec3 _background_color)
    {
        background_color = _background_color;
    }

    inline void VoxelScene::add_model(vox::Voxel model)
    {
        models.push_back(model);
    }


    inline void VoxelScene::pre_render_cleanup()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(
            background_color.r,
            background_color.g,
            background_color.b,
            1.0
        );
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glFrontFace(GL_CW);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
    }

    inline void VoxelScene::pre_render()
    {
        for (vox::Voxel& model : models) model.pre_render();
    }


    inline void VoxelScene::render()
    {
        for (vox::Voxel& model : models)
        {
            model.use_shader();
            model.render();
        }
    }
}
