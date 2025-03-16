#pragma once
#include <vector>
#include <iostream>
#include <glm/vec3.hpp>
#include "voxel.hpp"
#include "interfaces.hpp"

namespace lin
{
    inline GLint pow8(const GLint n)
    {
        GLint ret = 1;
        for (int i = 0; i < n; i++) ret <<= 3;
        return ret;
    }

    inline void drawInstancedVoxCubes(GLuint cube_vao, const ctx::Program *program, GLint count)
    {
        glBindVertexArray(cube_vao);
        glDrawElementsInstanced(
            GL_TRIANGLE_FAN,
            8, GL_UNSIGNED_INT,
            nullptr, count
        );
        glDrawElementsInstanced(
            GL_TRIANGLE_FAN,
            8, GL_UNSIGNED_INT,
            reinterpret_cast<const void *>(8 * sizeof(float)), count
        );
    }

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

    class LinVox final : public ctx::IRenderable
    {
        std::vector<glm::vec3> color;
        std::vector<GLint> location;
        std::vector<GLint> depth;
        ctx::Program *glsl_program = nullptr;
        GLuint model_vao = 0;
        float radians = 0;
        float model_scale = 1.f;
        glm::vec3 model_offset = glm::vec3{0, 0, 0};

    public:
        explicit LinVox(vox::Octree *layout);
        void linearize(vox::Octree *node, GLint d, GLint l, GLint d8);
        void render() override;
        void use_shader() override;
        void pre_render() override;
        void pre_render_cleanup() override;
        void print();
    };

    inline LinVox::LinVox(vox::Octree *layout)
    {
        linearize(layout, 0, 0, 1);
    }

    inline void LinVox::print()
    {
        for (int i = 0; i < color.size(); i++)
        {
            std::cout << color[i].r << " " << color[i].g << " " << color[i].b << "; "
                      <<  std::oct << location[i] << " " << depth[i] << std::endl;
        }
    }


    inline void LinVox::linearize(vox::Octree *node, GLint d, GLint l, GLint d8)
    {
        if (node->empty()) return;
        if (node->leaf())
        {
            color.push_back(node->get_color());
            location.push_back(l);
            depth.push_back(d);
            return;
        }

        for (int i = 0; i < 8; i++)
        {
            linearize(
                node->get_children()[i],
                d + 1, l + i * d8, d8 * 8
            );
        }
    }

    inline void LinVox::use_shader()
    {
        glsl_program->use();
    }

    inline void LinVox::pre_render()
    {
        glsl_program = new ctx::Program();
        const ctx::Shader glsl_vertex_s(
            "/home/lukas/projects/voxels/shaders/linear.vert",
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

        GLuint colorVBO, locationVBO, depthVBO;
        glGenBuffers(1, &colorVBO);
        glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
        glBufferData(GL_ARRAY_BUFFER, color.size() * sizeof(glm::vec3), &color[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glEnableVertexAttribArray(5);
        glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
        glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glVertexAttribDivisor(5, 1);

        glGenBuffers(1, &locationVBO);
        glBindBuffer(GL_ARRAY_BUFFER, locationVBO);
        glBufferData(GL_ARRAY_BUFFER,  location.size() * sizeof(GLint), &location[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glEnableVertexAttribArray(6);
        glBindBuffer(GL_ARRAY_BUFFER, locationVBO);
        glVertexAttribIPointer(6, 1, GL_INT, sizeof(GLint), nullptr);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glVertexAttribDivisor(6, 1);

        glGenBuffers(1, &depthVBO);
        glBindBuffer(GL_ARRAY_BUFFER, depthVBO);
        glBufferData(GL_ARRAY_BUFFER, depth.size() * sizeof(GLint), &depth[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glEnableVertexAttribArray(7);
        glBindBuffer(GL_ARRAY_BUFFER, depthVBO);
        glVertexAttribIPointer(7, 1, GL_INT, sizeof(GLint), nullptr);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glVertexAttribDivisor(7, 1);
    }

    inline void LinVox::render()
    {
        radians += 0.01;
        if (radians >= glm::two_pi<float>()) radians = 0;

        glProgramUniform1f(glsl_program->get_id(), 1, radians);
        glProgramUniform1f(glsl_program->get_id(), 3, model_scale);
        glProgramUniform3fv(glsl_program->get_id(), 4, 1, glm::value_ptr(model_offset));

        glBindVertexArray(model_vao);
        glDrawElementsInstanced(
            GL_TRIANGLE_FAN,
            8, GL_UNSIGNED_INT,
            nullptr, location.size()
        );
        glDrawElementsInstanced(
            GL_TRIANGLE_FAN,
            8, GL_UNSIGNED_INT,
            reinterpret_cast<const void *>(8 * sizeof(float)), location.size()
        );
    }

    inline void LinVox::pre_render_cleanup()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(1.0,1.0,1.0,1.0);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glFrontFace(GL_CW);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
    }
}
