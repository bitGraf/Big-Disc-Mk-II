#include "OpenGL_API.h"

#include "Engine/Core/Logger.h"
#include "Engine/Core/Asserts.h"
#include "Engine/Memory/Memory.h"
#include "Engine/Platform/Platform.h"

#include "OpenGL_Types.h"
#include "OpenGL_Platform.h"

#include <stdarg.h>

internal_func GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType Type) {
    switch (Type) {
        case ShaderDataType::Float:  return GL_FLOAT;
        case ShaderDataType::Float2: return GL_FLOAT;
        case ShaderDataType::Float3: return GL_FLOAT;
        case ShaderDataType::Float4: return GL_FLOAT;
        case ShaderDataType::Mat3:   return GL_FLOAT;
        case ShaderDataType::Mat4:   return GL_FLOAT;

        case ShaderDataType::Int:    return GL_INT;
        case ShaderDataType::Int2:   return GL_INT;
        case ShaderDataType::Int3:   return GL_INT;
        case ShaderDataType::Int4:   return GL_INT;

        case ShaderDataType::Bool:   return GL_BOOL;
    }

    AssertMsg(false, "Invalid ShaderDataType");
    return 0;
}

internal_func bool IsIntegerType(ShaderDataType Type) {
    switch (Type) {
        case ShaderDataType::Int:    return true;
        case ShaderDataType::Int2:   return true;
        case ShaderDataType::Int3:   return true;
        case ShaderDataType::Int4:   return true;

        case ShaderDataType::Float:  return false;
        case ShaderDataType::Float2: return false;
        case ShaderDataType::Float3: return false;
        case ShaderDataType::Float4: return false;
        case ShaderDataType::Mat3:   return false;
        case ShaderDataType::Mat4:   return false;
        case ShaderDataType::Bool:   return false;
    }

    AssertMsg(false, "Invalid ShaderDataType");
    return 0;
}

internal_func uint32 GetComponentCount(ShaderDataType Type) {
    switch (Type) {
        case ShaderDataType::Float:  return 1;
        case ShaderDataType::Int:    return 1;
        case ShaderDataType::Bool:   return 1;

        case ShaderDataType::Float2: return 2;
        case ShaderDataType::Int2:   return 2;

        case ShaderDataType::Float3: return 3;
        case ShaderDataType::Int3:   return 3;

        case ShaderDataType::Float4: return 4;
        case ShaderDataType::Int4:   return 4;

        case ShaderDataType::Mat3:   return 3 * 3;
        case ShaderDataType::Mat4:   return 4 * 4;
    }

    AssertMsg(false, "Invalid ShaderDataType");
    return 0;
}

internal_func uint32 ShaderDataTypeSize(ShaderDataType Type) {
    switch (Type) {
        case ShaderDataType::Float:  return 4;
        case ShaderDataType::Int:    return 4;
        case ShaderDataType::Bool:   return 4;

        case ShaderDataType::Float2: return 4 * 2;
        case ShaderDataType::Int2:   return 4 * 2;

        case ShaderDataType::Float3: return 4 * 3;
        case ShaderDataType::Int3:   return 4 * 3;

        case ShaderDataType::Float4: return 4 * 4;
        case ShaderDataType::Int4:   return 4 * 4;

        case ShaderDataType::Mat3:   return 4 * 3 * 3;
        case ShaderDataType::Mat4:   return 4 * 4 * 4;
    }

    AssertMsg(false, "Invalid ShaderDataType");
    return 0;
}

bool32 OpenGL_api::initialize(const char* application_name, platform_state* plat_state) {
    if (!OpenGL_create_context()) {
        RH_FATAL("Could not create OpenGL Context!");
        return false;
    }

    OpenGL_set_swap_interval(1);

    //glEnable(GL_LINE_SMOOTH);
    //glLineWidth(2.0f);
    glPointSize(4.0f);

    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    return true;
}
void OpenGL_api::shutdown() {
    OpenGL_ImGui_Shutdown();
}

void OpenGL_api::resized(uint16 width, uint16 height) {

}

bool32 OpenGL_api::begin_frame(real32 delta_time) {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);

    return true;
}
bool32 OpenGL_api::end_frame(real32 delta_time) {
    return true;
}
bool32 OpenGL_api::present(uint32 sync_interval) {
    platform_swap_buffers();

    return true;
}

bool32 OpenGL_api::ImGui_Init() {
    if (!OpenGL_ImGui_Init()) {
        RH_FATAL("Could not create Init OpenGL ImGui Context!");
        return false;
    }

    return true;
}
bool32 OpenGL_api::ImGui_begin_frame() {
    OpenGL_ImGui_Begin_Frame();

    return true;
}
bool32 OpenGL_api::ImGui_end_frame() {
    OpenGL_ImGui_End_Frame();

    return true;
}
bool32 OpenGL_api::ImGui_Shutdown() {
    OpenGL_ImGui_Shutdown();

    return true;
}

void OpenGL_api::set_draw_mode(render_draw_mode mode) {
    switch (mode) {
        case render_draw_mode::Normal: {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glDisable(GL_POLYGON_OFFSET_LINE);
            glEnable(GL_CULL_FACE);
        }; break;
        case render_draw_mode::Wireframe: {
            glPolygonOffset(0, -1);
            glEnable(GL_POLYGON_OFFSET_LINE);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDisable(GL_CULL_FACE);
        }; break;
        case render_draw_mode::Points: {
            glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
        }; break;
    }
}

void OpenGL_api::set_highlight_mode(bool32 enabled) {
    if (enabled) {
        glPolygonOffset(0, -1);
        glEnable(GL_POLYGON_OFFSET_FILL);
        glEnable(GL_POLYGON_OFFSET_LINE);
        glDisable(GL_CULL_FACE);
    } else {
        glDisable(GL_POLYGON_OFFSET_FILL);
        glDisable(GL_POLYGON_OFFSET_LINE);
        glEnable(GL_CULL_FACE);
    }
}

void OpenGL_api::disable_depth_test() {
    glDisable(GL_DEPTH_TEST);
}
void OpenGL_api::enable_depth_test() {
    glEnable(GL_DEPTH_TEST);
}

void OpenGL_api::disable_depth_mask() {
    glDepthMask(GL_FALSE);
}
void OpenGL_api::enable_depth_mask() {
    glDepthMask(GL_TRUE);
}

/* Stencil Testing */
void OpenGL_api::enable_stencil_test() {
    glEnable(GL_STENCIL_TEST);
}
void OpenGL_api::disable_stencil_test() {
    glDisable(GL_STENCIL_TEST);
}
void OpenGL_api::set_stencil_mask(uint32 mask) {
    glStencilMask(mask); // each bit is written to the stencil buffer as is
}
void OpenGL_api::set_stencil_func(render_stencil_func func, uint32 ref, uint32 mask) {
    GLenum gl_func = 0;
    switch (func) {
        case render_stencil_func::Never:          { gl_func = GL_NEVER; }    break;
        case render_stencil_func::Less:           { gl_func = GL_LESS; }     break;
        case render_stencil_func::LessOrEqual:    { gl_func = GL_LEQUAL; }   break;
        case render_stencil_func::Greater:        { gl_func = GL_GREATER; }  break;
        case render_stencil_func::GreaterOrEqual: { gl_func = GL_GEQUAL; }   break;
        case render_stencil_func::Equal:          { gl_func = GL_EQUAL; }    break;
        case render_stencil_func::NotEqual:       { gl_func = GL_NOTEQUAL; } break;
        case render_stencil_func::Always:         { gl_func = GL_ALWAYS;  }  break;
    }

    glStencilFunc(gl_func, ref, mask);
}
void OpenGL_api::set_stencil_op(render_stencil_op sfail, render_stencil_op dpfail, render_stencil_op dppass) {
    GLenum gl_op1 = 0, gl_op2 = 0, gl_op3 = 0;
    switch (sfail) {
        case render_stencil_op::Keep:           { gl_op1 = GL_KEEP;      } break;
        case render_stencil_op::Zero:           { gl_op1 = GL_ZERO;      } break;
        case render_stencil_op::Replace:        { gl_op1 = GL_REPLACE;   } break;
        case render_stencil_op::Increment:      { gl_op1 = GL_INCR;      } break;
        case render_stencil_op::Increment_wrap: { gl_op1 = GL_INCR_WRAP; } break;
        case render_stencil_op::Decrement:      { gl_op1 = GL_DECR;      } break;
        case render_stencil_op::Decrement_wrap: { gl_op1 = GL_DECR_WRAP; } break;
        case render_stencil_op::Invert:         { gl_op1 = GL_INVERT;    } break;
    }
    switch (dpfail) {
        case render_stencil_op::Keep:           { gl_op2 = GL_KEEP;      } break;
        case render_stencil_op::Zero:           { gl_op2 = GL_ZERO;      } break;
        case render_stencil_op::Replace:        { gl_op2 = GL_REPLACE;   } break;
        case render_stencil_op::Increment:      { gl_op2 = GL_INCR;      } break;
        case render_stencil_op::Increment_wrap: { gl_op2 = GL_INCR_WRAP; } break;
        case render_stencil_op::Decrement:      { gl_op2 = GL_DECR;      } break;
        case render_stencil_op::Decrement_wrap: { gl_op2 = GL_DECR_WRAP; } break;
        case render_stencil_op::Invert:         { gl_op2 = GL_INVERT;    } break;
    }
    switch (dppass) {
        case render_stencil_op::Keep:           { gl_op3 = GL_KEEP;      } break;
        case render_stencil_op::Zero:           { gl_op3 = GL_ZERO;      } break;
        case render_stencil_op::Replace:        { gl_op3 = GL_REPLACE;   } break;
        case render_stencil_op::Increment:      { gl_op3 = GL_INCR;      } break;
        case render_stencil_op::Increment_wrap: { gl_op3 = GL_INCR_WRAP; } break;
        case render_stencil_op::Decrement:      { gl_op3 = GL_DECR;      } break;
        case render_stencil_op::Decrement_wrap: { gl_op3 = GL_DECR_WRAP; } break;
        case render_stencil_op::Invert:         { gl_op3 = GL_INVERT;    } break;
    }

    glStencilOp(gl_op1, gl_op2, gl_op3);
}

void OpenGL_api::push_debug_group(const char* label) {
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, label);
}
void OpenGL_api::pop_debug_group() {
    glPopDebugGroup();
}

void OpenGL_api::create_texture_2D(struct render_texture_2D* texture, 
                                   texture_creation_info_2D create_info, 
                                   const void* data, bool32 is_hdr) {
    glGenTextures(1, &texture->handle);
    glBindTexture(GL_TEXTURE_2D, texture->handle);

    GLenum InternalFormat = 0;
    GLenum Format = 0;
    GLenum Type = is_hdr ? GL_FLOAT : GL_UNSIGNED_BYTE;
    switch (create_info.num_channels) {
        case 1: {
            InternalFormat = is_hdr ? GL_R16F : GL_R8;
            Format = GL_RED;
        } break;
        case 2: {
            InternalFormat = is_hdr ? GL_RG16F : GL_RG8;
            Format = GL_RG;
        } break;
        case 3: {
            InternalFormat = is_hdr ? GL_RGB16F : GL_RGB8;
            Format = GL_RGB;
        } break;
        case 4: {
            InternalFormat = is_hdr ? GL_RGBA16F : GL_RGBA8;
            Format = GL_RGBA;
        } break;
        default:
            AssertMsg(false, "Invalid number of channels");
    }

    glTexImage2D(GL_TEXTURE_2D, 0, InternalFormat, create_info.width, create_info.height, 0, Format, Type, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    // This for font texture
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
    // This for everything else
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}
void OpenGL_api::create_texture_cube(struct render_texture_cube* texture,
                                     texture_creation_info_cube create_info, 
                                     const void*** data, bool32 is_hdr, uint32 mip_levels) {
    glGenTextures(1, &texture->handle);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture->handle);

    GLenum InternalFormat = 0;
    GLenum Format = 0;
    GLenum Type = is_hdr ? GL_FLOAT : GL_UNSIGNED_BYTE;
    switch (create_info.num_channels) {
        case 1: {
            InternalFormat = is_hdr ? GL_R16F : GL_R8;
            Format = GL_RED;
        } break;
        case 2: {
            InternalFormat = is_hdr ? GL_RG16F : GL_RG8;
            Format = GL_RG;
        } break;
        case 3: {
            InternalFormat = is_hdr ? GL_RGB16F : GL_RGB8;
            Format = GL_RGB;
        } break;
        case 4: {
            InternalFormat = is_hdr ? GL_RGBA16F : GL_RGBA8;
            Format = GL_RGBA;
        } break;
        default:
            AssertMsg(false, "Invalid number of channels");
    }

    bool32 has_mips = mip_levels > 1;

    if (has_mips) {
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL , mip_levels-1);
        for (uint32 i = 0; i < 6; i++) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, InternalFormat, create_info.width, create_info.height, 0, Format, Type, NULL);
        }
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    }

    for (uint32 i = 0; i < 6; i++) {
        for (uint32 mip = 0; mip < mip_levels; mip++) {
            int32 mip_width  = (int32)((real32)(create_info.width)  * std::pow(0.5, mip));
            int32 mip_height = (int32)((real32)(create_info.height) * std::pow(0.5, mip));

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, mip, InternalFormat, mip_width, mip_height, 0, Format, Type, data[i][mip]);
        }
    }

    if (!has_mips) {
        // todo: need an option in texture_create_ingo_ that specifies if we want mipmaps generated
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    }

    // This for font texture
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
    // This for everything else
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void OpenGL_api::create_texture_3D(struct render_texture_3D* texture, 
                                   texture_creation_info_3D create_info, 
                                   const void* data, bool32 is_hdr) {
    RH_ERROR("OpenGL::create_texture_3D() not yet implemented!");
}

void OpenGL_api::destroy_texture_2D(struct render_texture_2D* texture) {
    glDeleteTextures(1, &texture->handle);
}
void OpenGL_api::destroy_texture_3D(struct render_texture_3D* texture) {
    glDeleteTextures(1, &texture->handle);
}
void OpenGL_api::destroy_texture_cube(struct render_texture_cube* texture) {
    glDeleteTextures(1, &texture->handle);
}

#define MAX_VERTEX_ATTRIBUTES 16

struct vertex_layout {
    uint32 stride;
    uint32 num_attributes;
};

vertex_layout calculate_stride(const ShaderDataType* attributes) {
    vertex_layout layout = {};
    for (const ShaderDataType* scan = attributes; *scan != ShaderDataType::None; scan++) {
        layout.num_attributes++;
        AssertMsg(layout.num_attributes < MAX_VERTEX_ATTRIBUTES, "Too many vertex attributes!"); // just limit it to a reasonable amount now
        layout.stride += ShaderDataTypeSize(*scan);
    }
    AssertMsg(layout.num_attributes > 0, "Zero vertex attributes assigned!");
    return layout;
}


void OpenGL_api::create_mesh(render_geometry* mesh, 
                             uint32 num_verts, const void* vertices,
                             uint32 num_inds, const uint32* indices,
                             const ShaderDataType* attributes) {
    // to be in the triangle_mesh struct
    vertex_layout layout = calculate_stride(attributes);
    bool32 normalized = false;

    mesh->num_verts = num_verts;
    mesh->num_inds = num_inds;
    mesh->flag = 0;

    // first create the VBO
    uint32 vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, num_verts*layout.stride, vertices, GL_STATIC_DRAW);

    // create EBO
    uint32 ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_inds*sizeof(uint32), indices, GL_STATIC_DRAW);

    // create the VAO
    glGenVertexArrays(1, &mesh->handle);
    glBindVertexArray(mesh->handle);

    // assign vertex attributes
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    size_t offset = 0;
    for (uint32 n = 0; n < layout.num_attributes; n++) {
        ShaderDataType type = attributes[n];

        glEnableVertexAttribArray(n);

        if (IsIntegerType(type)) {
            glVertexAttribIPointer(n, 
                                   GetComponentCount(type), 
                                   ShaderDataTypeToOpenGLBaseType(type), 
                                   layout.stride, 
                                   (const void*)(offset));
        } else {
            glVertexAttribPointer(n,
                                  GetComponentCount(type),
                                  ShaderDataTypeToOpenGLBaseType(type),
                                  normalized ? GL_TRUE : GL_FALSE,
                                  layout.stride,
                                  (const void*)(offset));
        }

        offset += ShaderDataTypeSize(type);
    }

    // assign index buffer
    //RH_INFO("VAO: %d   VBO: %d  EBO: %d", mesh->handle, vbo, ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

    glBindVertexArray(0);
}
void OpenGL_api::destroy_mesh(render_geometry* mesh) {
    glDeleteVertexArrays(1, &mesh->handle);
}

// TODO: move these string-parsing functions to a better place!
#define CheckToken(Start, Token) CheckToken_(Start, Token, sizeof(Token))
internal_func bool32 
    CheckToken_(const uint8* Start, const char Token[], uint8 TokenLength) {
    for (uint8 TokenIndex = 0; TokenIndex < TokenLength; TokenIndex++) {
        if (Start[TokenIndex] != Token[TokenIndex]) {
            return false;
        }
    }

    return true;
}

#define FindAfterToken(Buffer, BufferLength, Token) FindAfterToken_(Buffer, BufferLength, Token, sizeof(Token))
internal_func const uint8* 
    FindAfterToken_(const uint8* Buffer, uint64 BufferLength, const char Token[], uint8 TokenLength) {
    const uint8* Result = nullptr;

    for (const uint8* Scan = Buffer; *Scan; Scan++) {
        if (*Scan == Token[0]) {
            if (CheckToken_(Scan, Token, TokenLength-1)) {
                Result = Scan + TokenLength;
                break;
            }
        }
    }

    return Result;
}

bool32 OpenGL_api::create_shader(shader* shader_prog, const uint8* shader_source, uint64 num_bytes) {
    const uint8* VertexStart = shader_source;
    VertexStart = FindAfterToken(shader_source, num_bytes, "#type vertex");
    AssertMsg(VertexStart, "Could not find '#type vertex' tag!");
    VertexStart++;

    const uint8* FragmentStart = shader_source;
    FragmentStart = FindAfterToken(shader_source, num_bytes, "#type fragment");
    AssertMsg(FragmentStart, "Could not find '#type fragment' tag!");
    FragmentStart++;

    uint32 VertexLength   = (uint32)((FragmentStart - VertexStart) - sizeof("#type fragment") - 1);
    uint32 FragmentLength = (uint32)(num_bytes - (FragmentStart - shader_source) + 1);

    Assert(VertexLength && VertexLength < num_bytes);
    Assert(FragmentLength && FragmentLength < num_bytes);

    GLuint programID = glCreateProgram();

    // Compile the vertex shader
    GLuint vertShaderID = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertShaderID, 1, (const GLchar**)(&VertexStart), (GLint*)(&VertexLength));
    glCompileShader(vertShaderID);

    GLint isCompiled = 0;
    glGetShaderiv(vertShaderID, GL_COMPILE_STATUS, &isCompiled);
    if (isCompiled == GL_FALSE) {
        GLint maxLength = 0;
        glGetShaderiv(vertShaderID, GL_INFO_LOG_LENGTH, &maxLength);

        //std::vector<GLchar> infoLog(maxLength);
        char ErrorBuffer[256];
        GLint actLength;
        glGetShaderInfoLog(vertShaderID, 256, &actLength, ErrorBuffer);

        glDeleteShader(vertShaderID);

        RH_ERROR("Vertex shader failed to compile:\n            %s", ErrorBuffer);

        return false;
    }
    glAttachShader(programID, vertShaderID);

    // Compile the fragment shader
    GLuint fragShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShaderID, 1, (const GLchar**)(&FragmentStart), (GLint*)(&FragmentLength));
    glCompileShader(fragShaderID);

    glGetShaderiv(fragShaderID, GL_COMPILE_STATUS, &isCompiled);
    if (isCompiled == GL_FALSE) {
        GLint maxLength = 0;
        glGetShaderiv(fragShaderID, GL_INFO_LOG_LENGTH, &maxLength);

        //std::vector<GLchar> infoLog(maxLength);
        char ErrorBuffer[256];
        GLint actLength;
        glGetShaderInfoLog(fragShaderID, 256, &actLength, ErrorBuffer);

        glDeleteShader(fragShaderID);

        RH_ERROR("Fragment shader failed to compile:\n            %s", ErrorBuffer);

        return false;
    }
    glAttachShader(programID, fragShaderID);

    // Link the two shaders into the program
    glLinkProgram(programID);
    GLint isLinked = 0;
    glGetProgramiv(programID, GL_LINK_STATUS, (int *)&isLinked);
    if (isLinked == GL_FALSE) {
        GLint maxLength = 0;
        glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &maxLength);

        //std::vector<GLchar> infoLog(maxLength);
        char ErrorBuffer[256];
        GLint actLength;
        glGetProgramInfoLog(programID, 256, &actLength, ErrorBuffer);

        glDeleteProgram(programID);
        glDeleteShader(vertShaderID);
        glDeleteShader(fragShaderID);

        RH_ERROR("Program failed to link:\n            %s", ErrorBuffer);

        return false;
    }

    glDetachShader(programID, vertShaderID);
    glDetachShader(programID, fragShaderID);

    // Assign the values in the Shader wrapper
    shader_prog->handle = programID;

    return true;
}
void OpenGL_api::destroy_shader(shader* shader_prog) {
    glDeleteShader(shader_prog->handle);
    shader_prog->handle = 0;
}

static GLenum DataType(GLenum format) {
    switch (format) {
        case GL_R8:
        case GL_RGB8:
        case GL_RGBA:
        case GL_RGBA8: return GL_UNSIGNED_BYTE;
        case GL_RG16F:
        case GL_RG32F:
        case GL_RGBA16F:
        case GL_RGB16F:
        case GL_RGB32F:
        case GL_R32F:
        case GL_RGBA32F: return GL_FLOAT;
        case GL_DEPTH24_STENCIL8: return GL_UNSIGNED_INT_24_8;
    }

    Assert(false);
    return 0;
}

static GLenum DataFormat(GLenum format) {
    switch (format) {
        case GL_R32F:
        case GL_R8: return GL_RED;
        case GL_RG16F: return GL_RG;
        case GL_RGB8:
        case GL_RGB16F:
        case GL_RGB32F: return GL_RGB;
        case GL_RGBA:
        case GL_RGBA8:
        case GL_RGBA16F:
        case GL_RGBA32F: return GL_RGBA;
    }

    Assert(false);
    return 0;
}

static GLenum WrapType(texture_wrap_type type) {
    switch (type) {
        case texture_wrap_type::clamp_to_edge:   return GL_CLAMP_TO_EDGE;
        case texture_wrap_type::clamp_to_border: return GL_CLAMP_TO_BORDER;
        case texture_wrap_type::mirrored_repeat: return GL_MIRRORED_REPEAT;
        case texture_wrap_type::repeat:          return GL_REPEAT;
    }

    Assert(false);
    return 0;
}
static GLenum FilterType(texture_filter_type type) {
    switch (type) {
        case texture_filter_type::linear:   return GL_LINEAR;
        case texture_filter_type::nearest:  return GL_NEAREST;

        case texture_filter_type::nearest_mipmap_nearest: return GL_NEAREST_MIPMAP_NEAREST;
        case texture_filter_type::nearest_mipmap_linear:  return GL_NEAREST_MIPMAP_LINEAR;
        case texture_filter_type::linear_mipmap_nearest:  return GL_LINEAR_MIPMAP_NEAREST;
        case texture_filter_type::linear_mipmap_linear:   return GL_LINEAR_MIPMAP_LINEAR;
    }

    Assert(false);
    return 0;
}

bool32 OpenGL_api::create_framebuffer(frame_buffer* fbo, int num_attachments, 
                                      const frame_buffer_attachment* attachments,
                                      frame_buffer_create_info info) {

    glGenFramebuffers(1, &fbo->handle);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo->handle);

    AssertMsg(num_attachments <= 6, "More color attachments not supported atm");
    fbo->num_attachments = num_attachments;
    memory_copy(&fbo->attachments, attachments, num_attachments*sizeof(frame_buffer_attachment));

    for (int n = 0; n < num_attachments; n++) {
        GLenum internal_format = 0;
        GLenum attach_type = 0;
        bool32 color_attachment = false;
        switch(fbo->attachments[n].texture_format) {
            case frame_buffer_texture_format::RED8:    { internal_format = GL_R8;      color_attachment = true; } break;
            case frame_buffer_texture_format::R32F:    { internal_format = GL_R32F;    color_attachment = true; } break;
            case frame_buffer_texture_format::RGBA8:   { internal_format = GL_RGBA8;   color_attachment = true; } break;
            case frame_buffer_texture_format::RGBA16F: { internal_format = GL_RGBA16F; color_attachment = true; } break;
            case frame_buffer_texture_format::RGB16F:  { internal_format = GL_RGB16F;  color_attachment = true; } break;
            case frame_buffer_texture_format::RG16F:   { internal_format = GL_RG16F;   color_attachment = true; } break;
            case frame_buffer_texture_format::RGBA32F: { internal_format = GL_RGBA32F; color_attachment = true; } break;
            case frame_buffer_texture_format::None:    { Assert(false); } break;

            case frame_buffer_texture_format::DEPTH24STENCIL8: {} break;
        }
        switch(fbo->attachments[n].texture_format) {
            case frame_buffer_texture_format::DEPTH24STENCIL8: { internal_format = GL_DEPTH24_STENCIL8; attach_type = GL_DEPTH_STENCIL_ATTACHMENT; } break;
            //case frame_buffer_texture_format::DEPTH32F:    { internal_format = GL_R32F;    color_attachment = true; } break;

            case frame_buffer_texture_format::RED8:    {  } break;
            case frame_buffer_texture_format::R32F:    {  } break;
            case frame_buffer_texture_format::RGBA8:   {  } break;
            case frame_buffer_texture_format::RGBA16F: {  } break;
            case frame_buffer_texture_format::RGB16F:  {  } break;
            case frame_buffer_texture_format::RG16F:   {  } break;
            case frame_buffer_texture_format::RGBA32F: {  } break;
            case frame_buffer_texture_format::None:    { Assert(false); } break;
        }
        if (color_attachment) {
            glGenTextures(1, &fbo->attachments[n].handle);
            glBindTexture(GL_TEXTURE_2D, fbo->attachments[n].handle);

            glTexImage2D(GL_TEXTURE_2D, 0, internal_format, 
                         fbo->width, fbo->height, 0, 
                         DataFormat(internal_format), DataType(internal_format), nullptr);

            //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, FilterType(info.min_filter));
            //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, FilterType(info.mag_filter));
            //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, WrapType(info.wrap_s));
            //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, WrapType(info.wrap_t));

            glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, info.border._data);

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + n, 
                                   GL_TEXTURE_2D, fbo->attachments[n].handle, 0);
        } else {
            glGenRenderbuffers(1, &fbo->attachments[n].handle);
            glBindRenderbuffer(GL_RENDERBUFFER, fbo->attachments[n].handle);

            glRenderbufferStorage(GL_RENDERBUFFER, internal_format, fbo->width, fbo->height);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, attach_type, GL_RENDERBUFFER, fbo->attachments[n].handle);
        }
    }

    GLenum buffers[5] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4 };
    glDrawBuffers(num_attachments-1, buffers);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return true;
}
bool32 OpenGL_api::recreate_framebuffer(frame_buffer* fbo) {
    // Assumes fbo is already created... might cause issues!

    // destroy the current fbo
    for (uint32 n = 0; n < fbo->num_attachments; n++) {
        if (fbo->attachments[n].texture_format == frame_buffer_texture_format::DEPTH24STENCIL8) {
            glDeleteRenderbuffers(1, &fbo->attachments[n].handle);
        } else {
            glDeleteTextures(1, &fbo->attachments[n].handle);
        }

        fbo->attachments[n].handle = 0;
    }

    glDeleteFramebuffers(1, &fbo->handle);
    fbo->handle = 0;

    // recreate the fbo using the same settings.
    glGenFramebuffers(1, &fbo->handle);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo->handle);

    for (uint32 n = 0; n < fbo->num_attachments; n++) {
        GLenum internal_format = 0;
        GLenum attach_type = 0;
        bool32 color_attachment = false;
        switch(fbo->attachments[n].texture_format) {
            case frame_buffer_texture_format::RED8:    { internal_format = GL_R8;      color_attachment = true; } break;
            case frame_buffer_texture_format::R32F:    { internal_format = GL_R32F;    color_attachment = true; } break;
            case frame_buffer_texture_format::RGBA8:   { internal_format = GL_RGBA8;   color_attachment = true; } break;
            case frame_buffer_texture_format::RGBA16F: { internal_format = GL_RGBA16F; color_attachment = true; } break;
            case frame_buffer_texture_format::RGB16F:  { internal_format = GL_RGB16F;  color_attachment = true; } break;
            case frame_buffer_texture_format::RG16F:   { internal_format = GL_RG16F;   color_attachment = true; } break;
            case frame_buffer_texture_format::RGBA32F: { internal_format = GL_RGBA32F; color_attachment = true; } break;
            case frame_buffer_texture_format::None:    { Assert(false); } break;

            case frame_buffer_texture_format::DEPTH24STENCIL8: {} break;
        }
        switch(fbo->attachments[n].texture_format) {
            case frame_buffer_texture_format::DEPTH24STENCIL8: { internal_format = GL_DEPTH24_STENCIL8; attach_type = GL_DEPTH_STENCIL_ATTACHMENT; } break;
                //case frame_buffer_texture_format::DEPTH32F:    { internal_format = GL_R32F;    color_attachment = true; } break;

            case frame_buffer_texture_format::RED8:    {  } break;
            case frame_buffer_texture_format::R32F:    {  } break;
            case frame_buffer_texture_format::RGBA8:   {  } break;
            case frame_buffer_texture_format::RGBA16F: {  } break;
            case frame_buffer_texture_format::RGB16F:  {  } break;
            case frame_buffer_texture_format::RG16F:   {  } break;
            case frame_buffer_texture_format::RGBA32F: {  } break;
            case frame_buffer_texture_format::None:    { Assert(false); } break;
        }
        if (color_attachment) {
            glGenTextures(1, &fbo->attachments[n].handle);
            glBindTexture(GL_TEXTURE_2D, fbo->attachments[n].handle);

            glTexImage2D(GL_TEXTURE_2D, 0, internal_format, 
                         fbo->width, fbo->height, 0, 
                         DataFormat(internal_format), DataType(internal_format), nullptr);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + n, 
                                   GL_TEXTURE_2D, fbo->attachments[n].handle, 0);
        } else {
            glGenRenderbuffers(1, &fbo->attachments[n].handle);
            glBindRenderbuffer(GL_RENDERBUFFER, fbo->attachments[n].handle);

            glRenderbufferStorage(GL_RENDERBUFFER, internal_format, fbo->width, fbo->height);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, attach_type, GL_RENDERBUFFER, fbo->attachments[n].handle);
        }
    }

    GLenum buffers[5] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4 };
    glDrawBuffers(fbo->num_attachments-1, buffers);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return true;
}
bool32 OpenGL_api::create_framebuffer_cube(frame_buffer* fbo, 
                                           int num_attachments, 
                                           const frame_buffer_attachment* attachments,
                                           bool32 generate_mipmaps) {

    glGenFramebuffers(1, &fbo->handle);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo->handle);

    AssertMsg(num_attachments <= 6, "More color attachments not supported atm");
    fbo->num_attachments = num_attachments;
    memory_copy(&fbo->attachments, attachments, num_attachments*sizeof(frame_buffer_attachment));

    for (int n = 0; n < num_attachments; n++) {
        GLenum internal_format = 0;
        GLenum attach_type = 0;
        bool32 color_attachment = false;
        switch(fbo->attachments[n].texture_format) {
            case frame_buffer_texture_format::RED8:    { internal_format = GL_R8;      color_attachment = true; } break;
            case frame_buffer_texture_format::R32F:    { internal_format = GL_R32F;    color_attachment = true; } break;
            case frame_buffer_texture_format::RGBA8:   { internal_format = GL_RGBA8;   color_attachment = true; } break;
            case frame_buffer_texture_format::RGBA16F: { internal_format = GL_RGBA16F; color_attachment = true; } break;
            case frame_buffer_texture_format::RGB16F:  { internal_format = GL_RGB16F;  color_attachment = true; } break;
            case frame_buffer_texture_format::RGBA32F: { internal_format = GL_RGBA32F; color_attachment = true; } break;
            case frame_buffer_texture_format::None:    { Assert(false); } break;

            case frame_buffer_texture_format::DEPTH24STENCIL8: {} break;
        }
        switch(fbo->attachments[n].texture_format) {
            case frame_buffer_texture_format::DEPTH24STENCIL8: { internal_format = GL_DEPTH24_STENCIL8; attach_type = GL_DEPTH_STENCIL_ATTACHMENT; } break;
            //case frame_buffer_texture_format::DEPTH32F:    { internal_format = GL_R32F;    color_attachment = true; } break;

            case frame_buffer_texture_format::RED8:    {  } break;
            case frame_buffer_texture_format::R32F:    {  } break;
            case frame_buffer_texture_format::RGBA8:   {  } break;
            case frame_buffer_texture_format::RGBA16F: {  } break;
            case frame_buffer_texture_format::RGB16F:  {  } break;
            case frame_buffer_texture_format::RGBA32F: {  } break;
            case frame_buffer_texture_format::None:    { Assert(false); } break;
        }
        if (color_attachment) {
            glGenTextures(1, &fbo->attachments[n].handle);
            glBindTexture(GL_TEXTURE_CUBE_MAP, fbo->attachments[n].handle);

            for (int i = 0; i < 6; i++) {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internal_format, 
                            fbo->width, fbo->height, 0, 
                            DataFormat(internal_format), DataType(internal_format), nullptr);
                // todo: do we need this?
                //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + n, 
                //                       GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, fbo->attachments[n].handle, 0);    
            }

            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            if (generate_mipmaps) {
                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
            } else {
                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            }
        } else {
            glGenRenderbuffers(1, &fbo->attachments[n].handle);
            glBindRenderbuffer(GL_RENDERBUFFER, fbo->attachments[n].handle);

            glRenderbufferStorage(GL_RENDERBUFFER, internal_format, fbo->width, fbo->height);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, attach_type, GL_RENDERBUFFER, fbo->attachments[n].handle);
        }
    }

    GLenum buffers[5] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4 };
    glDrawBuffers(num_attachments-1, buffers);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return true;
}
void OpenGL_api::destroy_framebuffer(frame_buffer* fbo) {

}
void OpenGL_api::set_framebuffer_cube_face(frame_buffer* fbuffer, uint32 attach_idx, uint32 slot, uint32 mip_level) {
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+attach_idx, 
                           GL_TEXTURE_CUBE_MAP_POSITIVE_X+slot, fbuffer->attachments[attach_idx].handle, mip_level);    
}
void OpenGL_api::resize_framebuffer_renderbuffer(frame_buffer* fbuffer, uint32 new_width, uint32 new_height) {
    // TODO: read actual FBO data and find its render target
    glBindRenderbuffer(GL_RENDERBUFFER, fbuffer->attachments[1].handle);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, new_width, new_height);
}
void OpenGL_api::copy_framebuffer_depthbuffer(frame_buffer * src, frame_buffer * dst) {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, src->handle);
    if (dst == nullptr) {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        // TODO: using src dimmensions for dst (default FBO)
        glBlitFramebuffer(0, 0, src->width, src->height, 0, 0, src->width, src->height, 
                          GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    } else {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dst->handle);
        glBlitFramebuffer(0, 0, src->width, src->height, 0, 0, dst->width, dst->height, 
                          GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    }
}
void OpenGL_api::copy_framebuffer_stencilbuffer(frame_buffer * src, frame_buffer * dst) {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, src->handle);
    if (dst == nullptr) {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        // TODO: using src dimmensions for dst (default FBO)
        glBlitFramebuffer(0, 0, src->width, src->height, 0, 0, src->width, src->height, 
                          GL_STENCIL_BUFFER_BIT, GL_NEAREST);
    } else {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dst->handle);
        glBlitFramebuffer(0, 0, src->width, src->height, 0, 0, dst->width, dst->height, 
                          GL_STENCIL_BUFFER_BIT, GL_NEAREST);
    }
}


void OpenGL_api::use_shader(shader* shader_prog) {
    glUseProgram(shader_prog->handle);
}
void OpenGL_api::use_framebuffer(frame_buffer* fbuffer) {
    if (fbuffer) {
        glBindFramebuffer(GL_FRAMEBUFFER, fbuffer->handle);
    } else { // if you pass in a nullptr, it binds the default fbo
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

void OpenGL_api::draw_geometry(render_geometry* geom) {
    glBindVertexArray(geom->handle);
    glDrawElements(GL_TRIANGLES, geom->num_inds, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
void OpenGL_api::draw_geometry(render_geometry* geom, uint32 start_idx, uint32 num_inds) {
    glBindVertexArray(geom->handle);
    //glDrawElements(GL_TRIANGLES, num_inds, GL_UNSIGNED_INT, (void*)(&start_idx));
    glDrawElements(GL_TRIANGLES, num_inds, GL_UNSIGNED_INT, (void*)(start_idx * sizeof(GLuint)));
}
void OpenGL_api::draw_geometry(render_geometry* geom, render_material* mat) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mat->DiffuseTexture.handle);

    glBindVertexArray(geom->handle);
    glDrawElements(GL_TRIANGLES, geom->num_inds, GL_UNSIGNED_INT, 0);
}
void OpenGL_api::draw_geometry_lines(render_geometry* geom) {
    glBindVertexArray(geom->handle);
    glDrawElements(GL_LINES, geom->num_inds, GL_UNSIGNED_INT, 0);
}
void OpenGL_api::draw_geometry_points(render_geometry* geom) {
    glBindVertexArray(geom->handle);
    glDrawElements(GL_POINTS, geom->num_inds, GL_UNSIGNED_INT, 0);
}

void OpenGL_api::bind_texture_2D(render_texture_2D texture, uint32 slot) {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, texture.handle);
}
void OpenGL_api::bind_texture_3D(render_texture_3D texture, uint32 slot) {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_3D, texture.handle);
}
void OpenGL_api::bind_texture_cube(render_texture_cube texture, uint32 slot) {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture.handle);
}


void OpenGL_api::bind_texture_2D(frame_buffer_attachment attachment, uint32 slot) {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, attachment.handle);
}
void OpenGL_api::bind_texture_3D(frame_buffer_attachment attachment, uint32 slot) {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_3D, attachment.handle);
}
void OpenGL_api::bind_texture_cube(frame_buffer_attachment attachment, uint32 slot) {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_CUBE_MAP, attachment.handle);
}



void OpenGL_api::set_viewport(uint32 x, uint32 y, uint32 width, uint32 height) {
    glViewport(x, y, width, height);
}
void OpenGL_api::clear_viewport(real32 r, real32 g, real32 b, real32 a) {
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}
void OpenGL_api::clear_viewport_only_color(real32 r, real32 g, real32 b, real32 a) {
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
void OpenGL_api::clear_framebuffer_attachment(frame_buffer_attachment *attach, real32 r, real32 b, real32 g, real32 a) {
    #if defined(RH_COMPILE_OPENGL_4_4)
        float color[] = {r, g, b, a};
        GLenum format = GL_RGBA, type = GL_UNSIGNED_BYTE;
        switch(attach->texture_format) {
            case frame_buffer_texture_format::RED8:    { format = GL_RED;  type = GL_UNSIGNED_BYTE; } break;
            case frame_buffer_texture_format::R32F:    { format = GL_RED;  type = GL_FLOAT; } break;
            case frame_buffer_texture_format::RGBA8:   { format = GL_RGBA; type = GL_UNSIGNED_BYTE; } break;
            case frame_buffer_texture_format::RGBA16F: { format = GL_RGBA; type = GL_FLOAT; } break;
            case frame_buffer_texture_format::RGB16F:  { format = GL_RGB;  type = GL_FLOAT; } break;
            case frame_buffer_texture_format::RG16F:   { format = GL_RG;   type = GL_FLOAT; } break;
            case frame_buffer_texture_format::RGBA32F: { format = GL_RGBA; type = GL_FLOAT; } break;

            case frame_buffer_texture_format::None:    { Assert(false); } break;
            case frame_buffer_texture_format::DEPTH24STENCIL8: { Assert(false); } break;
        }
        //glClearTexImage(attach->handle, 0, GL_RGBA, GL_FLOAT, color);
        glClearTexImage(attach->handle, 0, format, type, color);
    #else
        // not defined...
    #endif
}

void OpenGL_api::get_texture_data(render_texture_2D texture, void* data, int num_channels, bool is_hdr, uint32 mip) {
    GLenum Type = is_hdr ? GL_FLOAT : GL_UNSIGNED_BYTE;

    GLenum InternalFormat = 0;
    switch (num_channels) {
        case 1: {
            InternalFormat = GL_RED;
        } break;
        case 2: {
            InternalFormat = GL_RG;
        } break;
        case 3: {
            InternalFormat = GL_RGB;
        } break;
        case 4: {
            InternalFormat = GL_RGBA;
        } break;
        default:
            Assert(false);
    }

    glBindTexture(GL_TEXTURE_2D, texture.handle);
    glGetTexImage(GL_TEXTURE_2D, mip, InternalFormat, Type, data);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void OpenGL_api::get_cubemap_data(render_texture_cube texture, void* data, int num_channels, bool is_hdr, uint32 face, uint32 mip) {
    GLenum Type = is_hdr ? GL_FLOAT : GL_UNSIGNED_BYTE;

    GLenum InternalFormat = 0;
    switch (num_channels) {
        case 1: {
            InternalFormat = GL_RED;
        } break;
        case 2: {
            InternalFormat = GL_RG;
        } break;
        case 3: {
            InternalFormat = GL_RGB;
        } break;
        case 4: {
            InternalFormat = GL_RGBA;
        } break;
        default:
            Assert(false);
    }

    glBindTexture(GL_TEXTURE_CUBE_MAP, texture.handle);
    glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, mip, InternalFormat, Type, data);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void OpenGL_api::upload_uniform_float(   ShaderUniform_float uniform, real32  value) {
    glUniform1f(uniform.Location, value);
}
void OpenGL_api::upload_uniform_float2(  ShaderUniform_vec2 uniform, const laml::Vec2& values) {
    glUniform2fv(uniform.Location, 1, values._data);
}
void OpenGL_api::upload_uniform_float3(  ShaderUniform_vec3 uniform, const laml::Vec3& values) {
    glUniform3fv(uniform.Location, 1, values._data);
}
void OpenGL_api::upload_uniform_float4(  ShaderUniform_vec4 uniform, const laml::Vec4& values) {
    glUniform4fv(uniform.Location, 1, values._data);
}
void OpenGL_api::upload_uniform_float4x4(ShaderUniform_mat4 uniform, const laml::Mat4& values) {
    glUniformMatrix4fv(uniform.Location, 1, GL_FALSE, values._data);
}
void OpenGL_api::upload_uniform_int(     ShaderUniform_int  uniform, int32  value) {
    glUniform1i(uniform.Location, value);
}
void OpenGL_api::upload_uniform_int2(    ShaderUniform_ivec2 uniform, const laml::Vector<int32,2>& values) {
    glUniform2iv(uniform.Location, 1, values._data);
}
void OpenGL_api::upload_uniform_int3(    ShaderUniform_ivec3 uniform, const laml::Vector<int32,3>& values) {
    glUniform3iv(uniform.Location, 1, values._data);
}
void OpenGL_api::upload_uniform_int4(    ShaderUniform_ivec4 uniform, const laml::Vector<int32,4>& values) {
    glUniform4iv(uniform.Location, 1, values._data);
}