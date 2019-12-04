#version 420 core

out vec4 FragColor;

in vec2 TexCoords;

void main()
{
	vec2 screenCoords = gl_FragCoord.xy;

	vec3 col = vec3(0);

    FragColor = vec4(col, 1.0);


} 