#version 330

uniform sampler2D colorTexture;
uniform vec3 lightDirection;

in vec3 worldSpaceNormal;
in vec2 textureCoordinates;

out vec4 color;

void main()
{
	vec4 baseColor = texture(colorTexture, textureCoordinates);
	float cos = dot(normalize(worldSpaceNormal), -normalize(lightDirection));
	cos = clamp(cos, 0.0, 1.0);
	color = (0.95 * cos * baseColor) + (0.05 * baseColor);
}
