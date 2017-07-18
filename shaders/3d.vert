#version 130

attribute vec4 position;
attribute vec3 vcolor;
out vec4 color;

/* uniform mat4 trans; */
uniform mat4 x;
uniform mat4 y;
uniform mat4 z;
uniform mat4 pos;
uniform mat4 size;


void main()
{
    /* gl_Position = position*trans; */
	gl_Position = size*pos*x*y*z*position;
    color = vec4( vcolor, 1.0 );
}
