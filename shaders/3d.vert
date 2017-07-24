#version 130

attribute vec4 position;
attribute vec3 vcolor;
attribute vec3 normal;
out vec4 color;

uniform mat4 trans;

vec3 vNormal = vec3(1.0, 1.0, 1.0);

float ambientStrength = 1.0;
float shininess = 10000;
vec4 lightColor = vec4(1.0, 1.0, 1.0, 1.0);
vec4 lightPosition = vec4(1.0, 2.0, 3.0, 1.0);
vec4 screenVector = vec4(1.0, 1.0, 1.0, 1.0);

void main()
{
    vec4 l = normalize(lightPosition);
    vec4 n = normalize(vec4(vNormal, 1.0));

    vec4 v = normalize(screenVector);
    vec4 r = reflect(-l, n);

    vec4 ambient = ambientStrength * lightColor;
    float cosTheta = clamp(dot(n, l), 0, 1);
    vec4 diffuse = lightColor * cosTheta;
    vec4 materialColor = vec4(vcolor, 1.0);
    float specular = clamp(dot(v, r), 0, 1);

    gl_Position = trans * position;
    color = specular * diffuse * ambient * materialColor;
}
