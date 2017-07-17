#version 130

attribute  vec3 position;
attribute  vec3 vcolor;
varying vec4 color;

void
main()
{
    gl_Position = vec4(position, 1.0);
    color = vec4( vcolor, 1.0 );
}
