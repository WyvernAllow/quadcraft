#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

out vec4 finalColor;

vec4 texture_2d_aa(sampler2D tex, vec2 uv) {
    vec2 texsize = vec2(textureSize(tex, 0));
    vec2 uv_texspace = uv * texsize;
    vec2 seam = floor(uv_texspace + 0.5);
    uv_texspace = (uv_texspace - seam)/ fwidth(uv_texspace) + seam;
    uv_texspace = clamp(uv_texspace, seam - 0.5, seam + 0.5);
    return texture(tex, uv_texspace / texsize);
}

void main()
{
    vec4 texelColor = texture_2d_aa(texture0, fragTexCoord);
    finalColor = texelColor * colDiffuse;
}