#version 410 core

layout(location=0) in vec3 vPosition;
layout(location=1) in vec3 vNormal;
layout(location=2) in vec2 vTexCoords;

out vec3 fPosition;  // Poziția în spațiul lumii
out vec3 fNormal;    // Normala în spațiul lumii
out vec2 fTexCoords; // Coordonatele texturii

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() 
{
    // Calculează poziția în spațiul ecranului
    gl_Position = projection * view * model * vec4(vPosition, 1.0f);

    // Transformă poziția în spațiul lumii
    fPosition = vec3(model * vec4(vPosition, 1.0f));

    // Transformă normala în spațiul lumii
    fNormal = mat3(transpose(inverse(model))) * vNormal;

    // Transmite coordonatele texturii
    fTexCoords = vTexCoords;
}
