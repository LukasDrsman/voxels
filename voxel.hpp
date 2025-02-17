#pragma once

#include <array>
#include <glm/vec3.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "irenderable.hpp"


namespace vox
{
	static float CUBE_VERTICES[24] = {
		1.f, 1.f, 1.f,
		-1.f, 1.f, 1.f,
		-1.f, -1.f, 1.f,
		1.f, -1.f, 1.f,		// front face

		1.f, 1.f, -1.f,
		1.f, -1.f, -1.f,
		-1.f, -1.f, -1.f,
		-1.f, 1.f, -1.f		// back face
	};

	static unsigned int CUBE_INDICES[16] = {
		0, 4, 7, 1, 2, 3, 5, 4, // upper-right
		6, 5, 3, 2, 1, 7, 4, 5
	};

	static glm::vec3 DCENTERS[8] = {
		glm::vec3{1, 1, 1},
		glm::vec3{-1, 1, 1},
		glm::vec3{-1, -1, 1},
		glm::vec3{1, -1, 1}, // upper

		glm::vec3{1, 1, -1},
		glm::vec3{-1, 1, -1},
		glm::vec3{-1, -1, -1},
		glm::vec3{1, -1, -1}
	};

	inline GLuint preDrawCube()
	{
		GLuint vao;
		glCreateVertexArrays(1, &vao);
		glBindVertexArray(vao);

		GLuint vbo_pos;
		glCreateBuffers(1, &vbo_pos);
		glNamedBufferStorage(vbo_pos, sizeof(float) * 24, CUBE_VERTICES, 0);

		glEnableVertexArrayAttrib(vao, 0);
		glVertexArrayVertexBuffer(vao, 0, vbo_pos, 0, 3 * sizeof(float));
		glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, 0);

		GLuint vbo_idx;
		glCreateBuffers(1, &vbo_idx);
		glNamedBufferStorage(vbo_idx, sizeof(unsigned int) * 16, CUBE_INDICES, 0);
		glVertexArrayElementBuffer(vao, vbo_idx);

		return vao;
	}

	inline void drawVoxCube(GLuint cube_vao, int *octants, int depth, float norm, glm::vec3 colorRGB, const ctx::Program *program)
	{
		glProgramUniform1f(program->get_id(), 5, norm);
		glProgramUniform3fv(program->get_id(), 6, 1, glm::value_ptr(colorRGB));
		glProgramUniform1i(program->get_id(), 7, depth);
		glProgramUniform1iv(program->get_id(), 8, depth, octants);

		glBindVertexArray(cube_vao);
		glDrawElements(GL_TRIANGLE_FAN, 8, GL_UNSIGNED_INT, nullptr);
		glDrawElements(GL_TRIANGLE_FAN, 8, GL_UNSIGNED_INT, reinterpret_cast<const void *>(8 * sizeof(float)));
	}

	enum EOctant
	{
		OCTANT_LEAF,
		OCTANT_EMPTY,
		OCTANT_NODE
	};

	class Octree
	{
		glm::vec3 colorRGB{};
		EOctant octant;
		std::array<Octree*, 8> children{};

	public:
		Octree();
		~Octree();
		explicit Octree(glm::vec3 _colorRGB);
		Octree(const std::array<Octree*, 8> &_children);
		[[nodiscard]] bool leaf() const;
		[[nodiscard]] bool empty() const;
		[[nodiscard]] bool node() const;
		void draw(GLuint vao, int *octants, int depth, float norm, ctx::Program *program) const;
		void cull();
		void print() const;
		[[nodiscard]] int node_count() const;

		[[nodiscard]] std::array<Octree *, 8> get_children() const;
		[[nodiscard]] glm::vec3 get_color() const;
	};

	inline Octree::~Octree()
	{
		for (int i = 0; i < 8; i++)
		{
			delete children[i];
		}
		delete this;
	}

	inline Octree::Octree()
	{
		octant = OCTANT_EMPTY;
		children = {};
	}

	inline Octree::Octree(const glm::vec3 _colorRGB)
	{
		octant = OCTANT_LEAF;
		colorRGB = _colorRGB;
		children = {};
	}

	inline Octree::Octree(const std::array<Octree*, 8>& _children)
	{
		octant = OCTANT_NODE;
		children = _children;
	}

	inline std::array<Octree *, 8> Octree::get_children() const
	{
		return children;
	}

	inline glm::vec3 Octree::get_color() const
	{
		return colorRGB;
	}

	inline bool Octree::leaf() const
	{
		return octant == OCTANT_LEAF;
	}

	inline bool Octree::empty() const
	{
		return octant == OCTANT_EMPTY;
	}

	inline bool Octree::node() const
	{
		return octant == OCTANT_NODE;
	}

	inline void Octree::draw(GLuint vao, int *octants, int depth, float norm, ctx::Program *program) const
	{
		if (empty()) return;
		if (leaf()) drawVoxCube(vao, octants, depth + 1, norm, colorRGB, program);
		if (node()) for (int i = 0; i < 8; i++)
		{
			octants[depth + 1] = i;
			children[i]->draw(vao, octants, depth + 1, norm * 0.5, program);
		}
	}

	inline void Octree::cull()
	{
		if (octant == OCTANT_LEAF) return;
		if (octant == OCTANT_EMPTY) return;

		// cull children
		for (int i = 0; i < 8; i++) children[i]->cull();

		// skip un-cullable nodes
		for (int i = 0; i < 8; i++)
			if (!children[i]->leaf()) return;

		if (children[0]->empty())
		{
			for (int i = 1; i < 8; i++) if (!children[i]->empty()) return;

			octant = OCTANT_EMPTY;
			return;
		}

		const glm::vec3 color = children[0]->colorRGB;
		for (int i = 1; i < 8; i++)
			if (color != children[i]->colorRGB) return;

		octant = OCTANT_LEAF;
		colorRGB = color;
	}

	inline int Octree::node_count() const
	{
		int count = 1;
		switch (octant)
		{
			case OCTANT_NODE:
				for (int i = 0; i < 8; i++)
					count += children[i]->node_count();
				return count;
			default:
				return count;
		}
	}


	class Voxel final : public ctx::IRenderable
	{
		Octree *layout;
		ctx::Program *glsl_program = nullptr;
		GLuint model_vao = 0;
		float radians;
		float model_scale = 1.f;
		glm::vec3 model_offset = glm::vec3{0, 0, 0};

	public:
		// explicit Voxel(std::string filename);
		void render() override;
		void pre_render() override;
		explicit Voxel(Octree *_layout);
		Voxel(Octree *_layout, float _model_scale, glm::vec3 _model_offset);
		void pre_render_cleanup() override;
		void use_shader() const;
	};

	inline Voxel::Voxel(Octree *_layout)
	{
		layout = _layout;
		radians = 0.f;
	}

	inline Voxel::Voxel(Octree *_layout, float _model_scale, glm::vec3 _model_offset)
	{
		layout = _layout;
		radians = 0.f;
		model_offset = _model_offset;
		model_scale = _model_scale;
	}


	inline void Voxel::pre_render_cleanup()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.18039,0.00784,0.16470,1.0);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glFrontFace(GL_CW);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
	}


	inline void Voxel::render()
	{
		radians += 0.01;
		if (radians >= glm::two_pi<float>()) radians = 0;

		glProgramUniform1f(glsl_program->get_id(), 1, radians);
		glProgramUniform1f(glsl_program->get_id(), 3, model_scale);
		glProgramUniform3fv(glsl_program->get_id(), 4, 1, glm::value_ptr(model_offset));

		int octants[10] = {};
		layout->draw(model_vao, octants, 0, 0.5, glsl_program);
	}

	inline void Voxel::use_shader() const
	{
		glsl_program->use();
	}


	inline void Voxel::pre_render()
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

		model_vao = preDrawCube();
	}

}



