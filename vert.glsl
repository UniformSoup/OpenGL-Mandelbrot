#version 440 core
layout (location = 0) in vec2 aPos;
void main(void) { gl_Position = vec4(aPos, 0.0, 1.0); }