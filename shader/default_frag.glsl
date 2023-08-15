#version 330

in vec4 vertexColor;
in vec2 textureCoordinates;
in vec3 normal;
in vec3 fragPos;

out vec4 color;

struct DirectionalLight
{
    vec3 color;
    float ambientIntensity;
    vec3 direction;
    float diffuseIntensity;
};

struct Material
{
    float specularIntensity;
    float shininess;
};

uniform int lightingEnabled = 1;
uniform int textureEnabled = 1;
uniform int fogEnabled = 1;
uniform sampler2D texture0;
uniform DirectionalLight directionalLight;
uniform Material material;

uniform vec3 eyePosition;

void main() {

    vec4 lightFactor = vec4(1.f);

    if(lightingEnabled == 1)
    {
        vec4 ambientColor = vec4(directionalLight.color, 1.f) * directionalLight.ambientIntensity;
        float diffuseFactor = max(dot(normalize(normal), normalize(directionalLight.direction)), 0.f);
        vec4 diffuseColor = vec4(directionalLight.color, 1.f) * directionalLight.diffuseIntensity * diffuseFactor * 1.f;
        lightFactor = ambientColor + diffuseColor;

        vec4 specularColor = vec4(0.0f);
        if(diffuseFactor > 0.0f)
        {
            vec3 fragToEye = normalize(eyePosition -  fragPos);
            vec3 reflectedVertex = normalize(reflect(directionalLight.direction, normalize(normal)));

            float specularFactor = dot(fragToEye, reflectedVertex);
            if(specularFactor > 0.0f)
            {
                specularFactor = pow(specularFactor, material.shininess);
                specularColor = vec4(directionalLight.color * material.specularIntensity * specularFactor, 1.0f);
                lightFactor += specularColor;
            }
        }
    }

    // color = texture(texture0, textureCoordinates) * lightFactor;
    if(textureEnabled == 1)
    {
        color = texture(texture0, textureCoordinates) * lightFactor;
    }
    else
    {
        color = vertexColor * lightFactor;
    }
}