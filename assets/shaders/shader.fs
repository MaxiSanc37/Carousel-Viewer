#version 330 core
#define MAX_POINT_LIGHTS 64

in vec2 TexCoords;
in vec3 FragPos;
in mat3 TBN;

out vec4 FragColor;

uniform vec3 viewPos;

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;

uniform int forceBulbColor; // 0 = normal material, 1 = emissive override
uniform float time;

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
    // Fallback: show bright red if nothing works
    //vec3 texColor = vec3(1.0, 0.8, 0.3); // override textures
    
    vec3 result = vec3(0.0);

    vec3 texColor = texture(diffuseMap, TexCoords).rgb;
    if (forceBulbColor == 1) {
        float flicker = 0.85 + 0.15 * sin(time * 8.0 + FragPos.x * 5.0); // unique light flickering per bulb
        vec3 glow = vec3(1.0, 0.85, 0.4) * flicker * 1.5;
        glow = pow(glow, vec3(1.0 / 2.2));
        FragColor = vec4(clamp(glow, 0.0, 1.0), 1.0);
        return;
    }



    vec3 sampledNormal = texture(normalMap, TexCoords).rgb;
    vec3 normal = normalize(TBN * (sampledNormal * 2.0 - 1.0));

    vec3 viewDir = normalize(viewPos - FragPos);

    for (int i = 0; i < numPointLights; ++i) {
        vec3 lightDir = normalize(pointLights[i].position - FragPos);
        float diff = max(dot(normal, lightDir), 0.0);
        vec3 reflectDir = reflect(-lightDir, normal);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
        float dist = length(pointLights[i].position - FragPos);
        float attenuation = 1.0 / (pointLights[i].constant +
                                   pointLights[i].linear * dist +
                                   pointLights[i].quadratic * dist * dist);

        vec3 ambient = pointLights[i].ambient * texColor;
        vec3 diffuse = pointLights[i].diffuse * diff * texColor;
        vec3 specular = pointLights[i].specular * spec;

        result += attenuation * (ambient + diffuse + specular);
    }

    // Boost brightness slightly before gamma
    result *= 1.8; // or 2.0 if still a bit dim

    // Apply gamma correction
    result = pow(result, vec3(1.0 / 2.2));
    result = clamp(result, 0.0, 1.0);
    FragColor = vec4(result, 1.0);
}
