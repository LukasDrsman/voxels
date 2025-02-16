#pragma once

#include <GL/glew.h>
#include <iostream>
#include "shaders.hpp"

namespace ctx
{
	class IRenderable
	{
	public:
		virtual ~IRenderable() = default;
		virtual void render() = 0;
		virtual void pre_render() = 0;
		virtual void pre_render_cleanup() = 0;
	};
}
