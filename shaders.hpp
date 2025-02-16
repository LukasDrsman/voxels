#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <GL/glew.h>

namespace ctx
{

    class Shader
    {
        GLuint shader_id = 0;
    public:
        Shader(const std::string& shader_path, GLenum shader_type);
        ~Shader();
        [[nodiscard]] GLuint get_id() const;
    };

    inline Shader::Shader(const std::string& shader_path, const GLenum shader_type)
    {
        std::ifstream shader_file;
        shader_file.open(shader_path);

        std::stringstream buffer;
        buffer << shader_file.rdbuf();
        std::string content = buffer.str();
        std::cout << content << std::endl;
        shader_id = glCreateShader(shader_type);
        glShaderSource(shader_id, 1, reinterpret_cast<GLchar * const *>(&content), nullptr);
        glCompileShader(shader_id);

        GLint log_size = 0;
        glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &log_size);

        char log[log_size] = {'\0'};
        glGetShaderInfoLog(shader_id, log_size, nullptr, log);
        std::cerr << "[OpenGL shader log] " << log << std::endl;

        if (shader_id == GL_INVALID_VALUE)
            throw std::runtime_error("[OpenGL] Panic on shader compilation.");


        shader_file.close();
    }

    inline Shader::~Shader()
    {
        glDeleteShader(shader_id);
    }

    inline GLuint Shader::get_id() const
    {
        return shader_id;
    }

    class Program
    {
        GLuint program_id = 0;
        bool linked = false;

    public:
        explicit Program();
        ~Program();
        void attach(Shader shader) const;
        void link();
        void use() const;
        [[nodiscard]] GLuint get_id() const;
    };

    inline Program::Program()
    {
        program_id = glCreateProgram();
    }

    inline Program::~Program()
    {
        if (!linked)
        {
            GLuint shader_ids[8] = {};
            GLsizei shader_count;

            glGetAttachedShaders(program_id, 8, &shader_count, shader_ids);

            for (GLsizei i = 0; i < shader_count; ++i)
                glDetachShader(program_id, shader_ids[i]);
        }

        glDeleteProgram(program_id);
    }

    inline GLuint Program::get_id() const
    {
        return program_id;
    }


    inline void Program::attach(Shader shader) const
    {
        if (!linked)
        {
            glAttachShader(program_id, shader.get_id());
        }
    }

    inline void Program::link()
    {
        glLinkProgram(program_id);
        linked = true;
    }

    inline void Program::use() const
    {
        glUseProgram(program_id);
    }
}
