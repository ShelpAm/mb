#version 460 core
layout (location = 0) in vec2 aPosition; // <vec2 pos, vec2 tex>
layout (location = 1) in vec2 aTexCoord; // <vec2 pos, vec2 tex>
out vec2 TexCoord;

uniform mat4 projection;

void main()
{
    gl_Position = projection * vec4(aPosition, 0.0, 1.0);
    TexCoord = aTexCoord;
}
