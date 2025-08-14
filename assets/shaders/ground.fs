#version 330 core
#define MAX_POINT_LIGHTS 64

in vec2 TexCoords;
in vec3 FragPos;
out vec4 FragColor;

uniform vec3 viewPos;
uniform sampler2D diffuseMap;

struct PointLight {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float constant;
    float linear;
    float quadratic;
};

uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform int numPointLights;

void main()
{
    vec3 texColor = texture(diffuseMap, TexCoords).rgb;
    vec3 normal = vec3(0.0, 1.0, 0.0); // simple upward normal
    vec3 baseTint = texColor * vec3(0.0025, 0.0025, 0.0025); // warm shadow tone
    vec3 result = baseTint; // add subtle warm tint as base

    for (int i = 0; i < numPointLights; ++i) {
        vec3 lightDir = normalize(pointLights[i].position - FragPos);
        float diff = max(dot(normal, lightDir), 0.0);

        float dist = length(pointLights[i].position - FragPos);
        float attenuation = 1.0 / (pointLights[i].constant +
                                   pointLights[i].linear * dist +
                                   pointLights[i].quadratic * dist * dist);

        vec3 ambient = pointLights[i].ambient * texColor;
        vec3 diffuse = pointLights[i].diffuse * diff * texColor;

        result += attenuation * (ambient + diffuse);
    }

    // Gamma correction
    result = pow(result, vec3(1.0 / 2.2));
    FragColor = vec4(clamp(result, 0.0, 1.0), 1.0);
}
