#version 130

attribute vec4 position;
attribute vec3 vcolor;
attribute vec3 normal;
out vec4 color;

uniform mat4 trans;
uniform vec4 lightpos;

vec3 vNormal = vec3(0.0, 0.0, 0.0);

float ambientStrength = 1.0;
float shininess = 1;
vec4 lightColor = vec4(1.0, 1.0, 1.0, 1.0);
vec4 lightPosition = vec4(0.0, 0.0, 0.0, 1.0);
vec4 screenVector = vec4(0.0, 0.0, 1.0, 1.0);

void main()
{
    vec4 l = normalize(lightPosition);
    vec4 n = normalize(vec4(normal, 1.0));

    vec4 v = normalize(screenVector);
    vec4 r = reflect(-l, n);

    vec4 ambient = ambientStrength * lightColor;
    float cosTheta = clamp(dot(n, l), 0, 1);
    vec4 diffuse = lightColor * cosTheta;
    vec4 materialColor = vec4(vcolor, 1.0);
    float specular = pow(clamp(dot(v, r), 0, 1), shininess);

    gl_Position = trans * position;
    /* color = specular * diffuse * ambient * materialColor; */
    color = diffuse * ambient * materialColor;
}
