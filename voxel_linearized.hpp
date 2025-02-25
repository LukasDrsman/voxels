#pragma once
#include <vector>
#include <iostream>
#include <glm/vec3.hpp>
#include "voxel.hpp"
#include "interfaces.hpp"

namespace lin
{
    class LinearizedVoxel final : public ctx::IRenderable
    {
        std::vector<std::pair<glm::vec3, std::vector<int>>> layout;
        ctx::Program *glsl_program = nullptr;
        GLuint model_vao = 0;
        float radians = 0;
        float model_scale = 1.f;
        glm::vec3 model_offset = glm::vec3{0, 0, 0};

    public:
        explicit LinearizedVoxel(vox::Octree *_layout);
        static std::vector<std::pair<glm::vec3, std::vector<int>>> linearize(vox::Octree *_layout, std::vector<int> octants);
        void render() override;

        void print_layout() const;

        void use_shader() const;
        void pre_render() override;
        void pre_render_cleanup() override;
    };

    inline LinearizedVoxel::LinearizedVoxel(vox::Octree *_layout)
    {
        layout = linearize(_layout, std::vector<int>{{}});
    }

    inline std::vector<std::pair<glm::vec3, std::vector<int>>> LinearizedVoxel::linearize(vox::Octree *_layout, std::vector<int> octants)
    {
        if (_layout->empty()) return {};
        if (_layout->leaf()) return std::vector{{std::pair(
            _layout->get_color(),
            octants
        )}};

        std::vector<std::pair<glm::vec3, std::vector<int>>> new_linearized{};
        for (int i = 0; i < 8; i++)
        {
            std::vector new_octants(octants);
            new_octants.push_back(i);
            auto linearized = linearize(_layout->get_children()[i], new_octants);
            if (!linearized.empty())
                new_linearized.insert(std::end(new_linearized), std::begin(linearized), std::end(linearized));
        }

        return new_linearized;
    }

    inline void LinearizedVoxel::pre_render_cleanup()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.18039,0.00784,0.16470,1.0);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glFrontFace(GL_CW);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
    }

    inline void LinearizedVoxel::render()
    {
        radians += 0.01;
        if (radians >= glm::two_pi<float>()) radians = 0;

        glProgramUniform1f(glsl_program->get_id(), 1, radians);
        glProgramUniform1f(glsl_program->get_id(), 3, model_scale);
        glProgramUniform3fv(glsl_program->get_id(), 4, 1, glm::value_ptr(model_offset));

        for (auto voxel: layout)
        {
            vox::drawVoxCube(
                model_vao, std::get<1>(voxel).data(),
                static_cast<int>(std::get<1>(voxel).size()),
                model_scale * static_cast<float>(glm::pow(0.5f, std::get<1>(voxel).size())),
                std::get<0>(voxel), glsl_program
            );
        }
    }

    inline void LinearizedVoxel::print_layout() const
    {
        for (auto voxel: layout)
        {
            glm::vec3 color = std::get<0>(voxel);
            std::vector octants = std::get<1>(voxel);

            std::cout << "RGB::" << color.r << "," << color.g << "," << color.b << "OCT::";
            for (int octant : octants) std::cout << octant << ",";
            std::cout << std::endl;
        }
    }

    inline void LinearizedVoxel::use_shader() const
    {
        glsl_program->use();
    }

    inline void LinearizedVoxel::pre_render()
    {
        glsl_program = new ctx::Program();
        const ctx::Shader glsl_vertex_s(
            "/home/lukas/projects/voxels/shaders/voxel.vert",
            GL_VERTEX_SHADER
        );
        const ctx::Shader glsl_fragment_s(
            "/home/lukas/projects/voxels/shaders/voxel.frag",
            GL_FRAGMENT_SHADER
        );

        glsl_program->attach(glsl_vertex_s);
        glsl_program->attach(glsl_fragment_s);
        glsl_program->link();
        glsl_program->use();

        model_vao = vox::preDrawCube();
    }
}
