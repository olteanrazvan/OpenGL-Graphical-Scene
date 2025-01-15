#version 410 core

in vec3 fNormal;
in vec4 fPosEye;
in vec2 fTexCoords;
in vec4 fragPosLightSpace;

out vec4 fColor;

//lighting
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 sunDir;
uniform vec3 sunColor;
uniform bool lightOn;

// Fog parameters
uniform float fogDensity; // Densitatea cetii
uniform vec3 fogColor;   // Culoarea cetii

//texture
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;

vec3 ambient;
float ambientStrength = 0.5f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.2f;
float shininess = 32.0f;

float computeShadow() {
    // perform perspective divide
    vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    
    // Transform to [0,1] range
    normalizedCoords = normalizedCoords * 0.5 + 0.5;
    
    // Get closest depth value from light's perspective
    float closestDepth = texture(shadowMap, normalizedCoords.xy).r;
    
    // Get depth of current fragment from light's perspective
    float currentDepth = normalizedCoords.z;
    
    // Check whether current frag pos is in shadow
    float bias = 0.005f;
    float shadow = currentDepth - bias > closestDepth ? 1.0f : 0.0f;

    if (normalizedCoords.z > 1.0f)
        return 0.0f;

    return shadow;
}

float computeFog()
{
    float fragmentDistance = length(fPosEye);
    float fogFactor = exp(-pow(fogDensity * fragmentDistance, 2));
    
    return clamp(fogFactor, 0.0f, 1.0f);
}

void computeLightComponents()
{		
    vec3 cameraPosEye = vec3(0.0f);//in eye coordinates, the viewer is situated at the origin
    
    //transform normal
    vec3 normalEye = normalize(fNormal);	
    
    //compute light direction
    vec3 lightDirN = normalize(lightDir);
    vec3 sunDirN = normalize(sunDir);
    
    //compute view direction 
    vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);
    
    // First light source (only if turned on)
    if (lightOn) {
        //compute ambient light
        ambient = ambientStrength * lightColor;
        
        //compute diffuse light
        diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;
        
        //compute specular light
        vec3 reflection = reflect(-lightDirN, normalEye);
        float specCoeff = pow(max(dot(viewDirN, reflection), 0.0f), shininess);
        specular = specularStrength * specCoeff * lightColor;
    } else {
        ambient = vec3(0.0f);
        diffuse = vec3(0.0f);
        specular = vec3(0.0f);
    }
    
    // Sun light (always on)
    vec3 sunDiffuse = max(dot(normalEye, sunDirN), 0.0f) * sunColor;
    vec3 sunReflection = reflect(-sunDirN, normalEye);
    float sunSpecCoeff = pow(max(dot(viewDirN, sunReflection), 0.0f), shininess);
    vec3 sunSpecular = specularStrength * sunSpecCoeff * sunColor;
    
    // Add sun contribution
    diffuse += sunDiffuse;
    specular += sunSpecular;
}

void main() 
{
    computeLightComponents();
    
    vec3 baseColor = vec3(texture(diffuseTexture, fTexCoords));
    
    ambient *= texture(diffuseTexture, fTexCoords).rgb;
    diffuse *= texture(diffuseTexture, fTexCoords).rgb;
    specular *= texture(specularTexture, fTexCoords).rgb;

    float shadow = computeShadow();
    vec3 color = min((ambient + (1.0f - shadow)*diffuse) + (1.0f - shadow)*specular, 1.0f);
    
    // Apply fog
    float fogFactor = computeFog();
    vec3 fragmentColor = mix(fogColor, color, fogFactor);
    
    fColor = vec4(fragmentColor, 1.0f);
}