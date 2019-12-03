#version 420 core
#define MAX_BOXES 100

out vec4 FragColor;

in vec2 TexCoords;

uniform float width;
uniform float height;

//TODO Random functions

void main()
{
	vec2 screenCoords = gl_FragCoord.xy;

	vec3 col = vec3(1,0,0);

	

	

    FragColor = vec4(col, 1.0);
} 