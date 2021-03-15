#type vertex
#version 430 core

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec3 a_Tangent;
layout (location = 3) in vec3 a_Binormal;
layout (location = 4) in vec2 a_TexCoord;

// can combine these two into ModelView matrix
uniform mat4 r_Transform;
uniform mat4 r_View;

uniform mat4 r_Projection;

out VertexOutput { // all in view-space
    vec3 Position;
    vec3 Normal;
    vec2 TexCoord;
    mat3 ViewNormalMatrix;
} vs_Output;

void main() {
    mat4 model2view = r_View * r_Transform;
    mat4 normalMatrix = transpose(inverse(model2view));
    vs_Output.Position = vec3(model2view * vec4(a_Position, 1.0));
    vs_Output.Normal = vec3(normalMatrix * vec4(a_Normal, 0));
    vs_Output.TexCoord = vec2(a_TexCoord.x, 1.0 - a_TexCoord.y);
    vs_Output.ViewNormalMatrix = mat3(normalMatrix) * mat3(a_Tangent, a_Binormal, a_Normal);

    gl_Position = r_Projection * model2view * vec4(a_Position, 1.0);
}

#type fragment
#version 430 core

layout (location = 0) out vec4 out_Albedo; //RGBA8
layout (location = 1) out vec4 out_Normal; //RGBA16F
layout (location = 2) out vec4 out_AMR;    //RGBA8
layout (location = 3) out vec4 out_Depth;  //R32F

// from vertex shader
in VertexOutput {
    vec3 Position;
    vec3 Normal;
    vec2 TexCoord;
    mat3 ViewNormalMatrix;
} vs_Input;

// PBR Textures
uniform sampler2D u_AlbedoTexture;
uniform sampler2D u_NormalTexture;
uniform sampler2D u_MetalnessTexture;
uniform sampler2D u_RoughnessTexture;
uniform sampler2D u_AmbientTexture;

// Material properties
uniform vec3 u_AlbedoColor;
uniform float u_Metalness;
uniform float u_Roughness;

// Toggles
uniform float r_AlbedoTexToggle;
uniform float r_NormalTexToggle;
uniform float r_MetalnessTexToggle;
uniform float r_RoughnessTexToggle;
uniform float r_AmbientTexToggle;

float linearize_depth(float d,float zNear,float zFar)
{
    float z_n = 2.0 * d - 1.0;
    return 2.0 * zNear * zFar / (zFar + zNear - z_n * (zFar - zNear));
}

void main()
{
	// Standard PBR inputs
	vec3 Albedo = r_AlbedoTexToggle > 0.5 ? texture(u_AlbedoTexture, vs_Input.TexCoord).rgb : u_AlbedoColor; 
	float Metalness = r_MetalnessTexToggle > 0.5 ? texture(u_MetalnessTexture, vs_Input.TexCoord).r : u_Metalness;
	float Roughness = r_RoughnessTexToggle > 0.5 ?  texture(u_RoughnessTexture, vs_Input.TexCoord).r : u_Roughness;
    float Ambient = r_AmbientTexToggle > 0.5 ? texture(u_AmbientTexture, vs_Input.TexCoord).r : 1;
    Roughness = max(Roughness, 0.05); // Minimum roughness of 0.05 to keep specular highlight

	// Normals (either from vertex or map)
	vec3 Normal = normalize(vs_Input.Normal);
	if (r_NormalTexToggle > 0.5)
	{
		Normal = normalize(2.0 * texture(u_NormalTexture, vs_Input.TexCoord).rgb - 1.0);
		Normal = normalize(vs_Input.ViewNormalMatrix * Normal);
	}

    float farClipDistance = 100.0f;

    // write to render targets
    out_Albedo = vec4(Albedo, 1);
	out_Normal = vec4(Normal, 1);
    out_AMR = vec4(Ambient, Metalness, Roughness, 1);
    //out_Depth = vec4(vs_Input.Position.z / farClipDistance, 0, 0, 1);//linearize_depth(gl_FragCoord.z, .01, 100);
    out_Depth = vec4(length(vs_Input.Position.xyz), 0, 0, 1);//linearize_depth(gl_FragCoord.z, .01, 100);
}