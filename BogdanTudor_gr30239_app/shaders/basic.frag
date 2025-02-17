#version 410 core

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;

out vec4 fColor;

//matrices
uniform mat4 model;
uniform mat4 view;
uniform mat3 normalMatrix;
//lighting
uniform vec3 lightDir;
uniform vec3 lightColor;
// textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;

// Fog
uniform bool fogEnabled; // Activează sau dezactivează ceața
uniform float fogDensity; // Densitatea ceții
uniform vec3 fogColor; // Culoarea ceții

//components
vec3 ambient;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;

//pt lightcube
uniform vec3 pointLightPosition; // Poziția luminii punctuale
uniform vec3 pointLightColor;    // Culoarea luminii punctuale

// Atenuare (falloff)
uniform float constantAttenuation;
uniform float linearAttenuation;
uniform float quadraticAttenuation;

//activare cub lumina
uniform bool lightCubeActive; // Controlează activarea luminii punctuale


void computeDirLight()
{
    //compute eye space coordinates
    vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
    vec3 normalEye = normalize(normalMatrix * fNormal);

    //normalize light direction
    vec3 lightDirN = vec3(normalize(view * vec4(lightDir, 0.0f)));

    //compute view direction (in eye coordinates, the viewer is situated at the origin
    vec3 viewDir = normalize(- fPosEye.xyz);

    //compute ambient light
    ambient = ambientStrength * lightColor;

    //compute diffuse light
    diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;

    //compute specular light
    vec3 reflectDir = reflect(-lightDirN, normalEye);
    float specCoeff = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
    specular = specularStrength * specCoeff * lightColor;
}

// Iluminarea din sursa punctuală
void computePointLight(out vec3 ambient, out vec3 diffuse, out vec3 specular) {
    if (!lightCubeActive) {
        ambient = vec3(0.0);
        diffuse = vec3(0.0);
        specular = vec3(0.0);
        return; // Lumina punctuală este dezactivată
    }

    // Poziția fragmentului în coordonate ochi
    vec3 fragPos = (view * model * vec4(fPosition, 1.0)).xyz;
    vec3 lightPosView = (view * vec4(pointLightPosition, 1.0)).xyz;

    vec3 toLight = normalize(lightPosView - fragPos); // Direcția spre lumină
    vec3 normal = normalize(normalMatrix * fNormal); // Normala fragmentului

    // Calcul iluminare
    ambient = 0.2 * pointLightColor;
    float diff = max(dot(normal, toLight), 0.0);
    diffuse = diff * pointLightColor;

    vec3 viewDir = normalize(-fragPos);
    vec3 reflectDir = reflect(-toLight, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    specular = 0.5 * spec * pointLightColor;

    // Atenuare
    float distance = length(lightPosView - fragPos);
    float attenuation = 1.0 / (constantAttenuation + linearAttenuation * distance + quadraticAttenuation * (distance * distance));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
}


void main() 
{
    // Variabile pentru iluminare
    vec3 ambientDir, diffuseDir, specularDir; // Pentru lumina direcțională
    vec3 ambientPoint, diffusePoint, specularPoint; // Pentru lumina punctuală

    // Calculează lumina direcțională (dacă este necesar)
    computeDirLight();
    ambientDir = ambient;
    diffuseDir = diffuse;
    specularDir = specular;

    // Calculează lumina punctuală
    computePointLight(ambientPoint, diffusePoint, specularPoint);

    // Combinație finală de lumină
    vec3 ambientTotal = ambientDir + ambientPoint;
    vec3 diffuseTotal = diffuseDir + diffusePoint;
    vec3 specularTotal = specularDir + specularPoint;

    // Texturi și culoare finală
    vec3 baseColor = min((ambientTotal + diffuseTotal) * texture(diffuseTexture, fTexCoords).rgb + specularTotal * texture(specularTexture, fTexCoords).rgb, 1.0f);

    // Aplică ceață (dacă este activată)
    if (fogEnabled) {
        float distance = length((view * vec4(fPosition, 1.0)).xyz); // Distanța fragmentului la cameră
        float fogFactor = exp(-pow(fogDensity * distance, 2.0)); // Calcul ceață
        fogFactor = clamp(fogFactor, 0.2, 1.0);
        baseColor = mix(fogColor, baseColor, fogFactor); // Amestec culoare bază cu ceață
    }

    fColor = vec4(baseColor, 1.0f);
}


