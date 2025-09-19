#version 460 core

in vec3 LocalPos;
in vec3 FragPos;
in vec3 FragNormal;
in vec2 TexCoord;
out vec4 FragColor;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float     shininess;
};

struct Light {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct Directional_light {
    Light light;
    vec3  dir;
};

struct Point_light {
    Light light;
    vec3  position;
    float constant;
    float linear;
    float quadratic;
};

uniform Material material;
uniform Directional_light dlight;
uniform Point_light plight;
uniform vec3 cameraPos;

uniform sampler2D uTexture;

vec3 calc_dlights() {
    vec3 toLight = normalize(-dlight.dir);
    vec3 fromLight = -toLight;
    vec3 toCamera = normalize(cameraPos - FragPos);

    // Calculated greatly based on above
    float diff = max(0, dot(toLight, normalize(FragNormal)));
    float spec = pow(max(0, dot(normalize(reflect(fromLight, FragNormal)), toCamera)), material.shininess);

    vec3 diffuseMap  = vec3(texture(material.diffuse, TexCoord));
    vec3 specularMap = vec3(texture(material.specular, TexCoord));
    vec3 ambient  = dlight.light.ambient * diffuseMap;
    vec3 diffuse  = dlight.light.diffuse * (diff * diffuseMap);
    vec3 specular = dlight.light.specular * (spec * specularMap); 

    vec3 resultColor = vec3(0);
    resultColor += ambient;
    resultColor += diffuse;
    resultColor += specular;

    return resultColor;
}

vec3 calc_plights() {
    vec3 toLight = normalize(plight.position - FragPos);
    vec3 fromLight = -toLight;
    vec3 toCamera = normalize(cameraPos - FragPos);

    // Calculated greatly based on above
    float diff = max(0, dot(toLight, normalize(FragNormal)));
    float spec = pow(max(0, dot(normalize(reflect(fromLight, FragNormal)), toCamera)), material.shininess);

    vec3 diffuseMap  = vec3(texture(material.diffuse, TexCoord));
    vec3 specularMap = vec3(texture(material.specular, TexCoord));
    vec3 ambient  = plight.light.ambient * diffuseMap;
    vec3 diffuse  = plight.light.diffuse * (diff * diffuseMap);
    vec3 specular = plight.light.specular * (spec * specularMap); 

    // Specificly for Point_light
    float dist = length(plight.position - FragPos);
    float attenuation = 1.0 / (plight.constant + plight.linear * dist + plight.quadratic * (dist * dist));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    vec3 resultColor = vec3(0);
    resultColor += ambient;
    resultColor += diffuse;
    resultColor += specular;

    return resultColor;
}

void main() {
    vec3 resultColor = vec3(0);
    resultColor += calc_dlights();
    // resultColor += calc_plights();

    FragColor = vec4(resultColor, 1.0);
}
