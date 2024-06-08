#version 330 core
out vec4 FragColor;
in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

uniform bool moveRange = false;
uniform bool abilityRange = false;
uniform bool factionHighlight = false;
uniform bool hoveredHex = false;
uniform bool unitRender = false;
uniform int factionID;
uniform vec3 factionColors[10];

uniform sampler2D texture1;

uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

float ambientStrength = 0.5;


void main()
{
    if(factionHighlight){
        FragColor = vec4(vec3(1.0), 1.0);
    }
    else{

        vec3 color = texture(texture1, TexCoords).rgb;
        vec3 normal = normalize(Normal);
        vec3 lightColor = vec3(0.3);

        vec3 ambient = ambientStrength * lightColor;
        vec3 lightDir = normalize(lightPos - FragPos);
        float diff = max(dot(lightDir, normal), 0.0);
        vec3 diffuse = diff * lightColor;

        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, normal);
        float spec = 1.0;
        vec3 halfwayDir = normalize(lightDir + viewDir);
        spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
        vec3 specular = spec * lightColor;

        if(moveRange){
            diffuse *= 0.5;
            specular *= 0.5;
        }

        if(hoveredHex){
            diffuse *= 0.5;
            specular *= 0.5;
        }

        if(abilityRange){
            diffuse *= 0.5;
            specular *= 0.5;
            color *= vec3(1,0,0);
        }

        //vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;
        vec3 lighting = (ambient + (diffuse + specular)) * color;
        if(unitRender)
            FragColor = vec4(lighting + (factionColors[factionID] * 0.5), 1.0);
        else
            FragColor = vec4(lighting, 1.0);
        //FragColor = vec4(vec3(shadow), 1.0);
    }
}
