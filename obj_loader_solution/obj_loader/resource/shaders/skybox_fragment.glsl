#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;
uniform float lightStrength;

void main()
{   
    vec4 textureVal = texture(skybox, TexCoords);
    FragColor = textureVal * ((lightStrength / 100.0f) * 0.65f);
}