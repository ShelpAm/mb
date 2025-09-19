#version 460 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
out vec3 LocalPos;
out vec3 FragPos;
out vec3 FragNormal;
out vec2 TexCoord;

uniform mat3 transposed_inverse_model;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    vec4 clipPos = projection * view * model * vec4(aPos, 1.0);
    gl_Position = clipPos;
    LocalPos = aPos;
    FragPos = vec3(model * vec4(aPos, 1)); // World pos
    FragNormal = transposed_inverse_model * aNormal;
    TexCoord = aTexCoord;
}
