#include <enpch.hpp>
#include "RenderCommand.hpp"

#include "Engine/Platform/OpenGL/OpenGLRendererAPI.hpp"

namespace rh {
    RendererAPI* RenderCommand::s_RendererAPI = new OpenGLRendererAPI();
}