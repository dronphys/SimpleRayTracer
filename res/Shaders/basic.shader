#shader vertex
#version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;
out vec2 PS_IN_TexCoord;
void main()
{
	gl_Position = vec4(aPos.x,aPos.y,aPos.z, 1.0);
	PS_IN_TexCoord = aTexCoord;
}
#shader fragment
#version 430 core
in vec2 PS_IN_TexCoord;
out vec4 PS_OUT_FragColor;
uniform sampler2D tex_output;
void main()
{
    vec3 color = texture(tex_output, PS_IN_TexCoord).rgb;
    PS_OUT_FragColor = vec4(color,1.0f);
}


