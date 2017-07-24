#version 130

attribute vec4 position;
attribute vec3 vcolor;
out vec4 color;

uniform mat4 trans;

vec3 vNormal = vec3(1.0, 1.0, 1.0);

float ambientStrength = 0.8;
float shininess = 10000;
vec4 lightColor = vec4(1.0, 1.0, 1.0, 1.0);
vec4 ambient = ambientStrength * lightColor;
vec4 vSpecular = vec4(0.0,0.0,0.0,1.0);

vec4 lightPosition = vec4(1.0, 2.0, 3.0, 1.0);


void main()
{
    vec4 L = normalize(lightPosition);
    vec4 N = normalize(vec4(vNormal,0.0));

    float cosTheta = clamp(dot(N, L), 0, 1);
    vec4 specular = lightColor * cosTheta;
    vec4 materialColor = vec4(vcolor, 1.0);

    gl_Position = trans * position;
    color = specular * ambient * materialColor;
}
