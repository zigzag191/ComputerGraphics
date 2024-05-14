#version 330

layout(location = 0) in vec4 in_worldSpacePosition;
layout(location = 1) in vec3 in_worldSpaceNormal;
layout(location = 2) in vec2 in_textureCoordinates;

uniform mat4 worldToCameraMatrix;
uniform mat4 projectionMatrix;

out vec3 worldSpaceNormal;
out vec2 textureCoordinates;

void main()
{
	worldSpaceNormal = in_worldSpaceNormal;
	textureCoordinates = in_textureCoordinates;
	gl_Position = projectionMatrix * worldToCameraMatrix * in_worldSpacePosition;
}
