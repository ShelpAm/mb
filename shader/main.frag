#version 460 core

in vec3 LocalPos;
in vec3 FragPos;
in vec3 FragNormal;
in vec2 TexCoord;
out vec4 FragColor;

uniform float ambientStrength;
uniform float diffuseStrength;
uniform float specularStrength;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 cameraPos;

uniform sampler2D uTexture;

void main() {
    float ambient = ambientStrength;

    vec3 toLight = normalize(lightPos - FragPos);
    float diffuse = diffuseStrength * max(0, dot(toLight, normalize(FragNormal)));

    vec3 fromLight = -toLight;
    vec3 toCamera = normalize(cameraPos - FragPos);
    float specular = specularStrength * pow(max(0, dot(normalize(reflect(fromLight, FragNormal)), toCamera)), 128);

    float lightIntensity = 0;
    lightIntensity += ambient;
    lightIntensity += diffuse;
    lightIntensity += specular;

    // vec3 objColor = abs(sin(LocalPos));
    vec3 objColor = texture(uTexture, TexCoord).rgb;
    vec3 result = lightIntensity * lightColor * objColor;

    FragColor = vec4(result, 1.0);
}
