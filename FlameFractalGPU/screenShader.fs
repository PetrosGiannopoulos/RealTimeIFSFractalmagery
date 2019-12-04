#version 420 core

out vec4 FragColor;

in vec2 TexCoords;
uniform sampler2D displayScreenTexture;

void main()
{
	vec2 screenCoords = gl_FragCoord.xy;

	vec3 col = vec3(0);

	col = texture(displayScreenTexture, TexCoords).rgb;

	//TODO apply tone mapping 

	//col = (col)/(col+1);
	
	

    FragColor = vec4(col, 1.0);

	FragColor = pow(FragColor, vec4(1./2.2));
} 