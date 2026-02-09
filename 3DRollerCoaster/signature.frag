#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D uDiffMap;

void main()
{
    vec4 tex = texture(uDiffMap, TexCoord);
    if(tex.a < 0.01) discard;
    FragColor = tex;
}
