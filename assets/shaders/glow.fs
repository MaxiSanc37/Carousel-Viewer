#version 330 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D glowTex;

void main()
{
    vec4 tex = texture(glowTex, TexCoords);

    // Replace full-color texture output with warm-tinted alpha-based glow
    vec3 warmColor = vec3(1.0, 0.85, 0.6); // soft yellowish tone
    FragColor = vec4(warmColor * tex.a * 0.5, tex.a * 0.5); // scale brightness and transparency
}
