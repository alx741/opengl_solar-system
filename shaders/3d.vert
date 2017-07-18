#version 130

attribute vec4 position;
attribute vec3 vcolor;
out vec4 color;

uniform mat4 trans;

void
main()
{
    /* gl_Position = position; */
    gl_Position = position*trans;
    color = vec4( vcolor, 1.0 );
}
