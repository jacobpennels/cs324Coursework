#version 120

void main(void)
{
	// transform the vertex using the modelview and projection matrix
	gl_Position =  gl_ModelViewProjectionMatrix * gl_Vertex;
	vec4 n_colour = gl_Color;
	n_colour.a = 0.75f;
	gl_FrontColor = n_colour; // pass out the input colour to geom/frag shader
}
