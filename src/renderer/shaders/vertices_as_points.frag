#version 330 core

in vec4 f_color;

layout(location = 0) out vec4 FragColor;

void main() {
    vec2 coord = gl_PointCoord - vec2(0.5);
    float dist = length(coord);
    
    if(dist > 0.5)
        discard;
    
    FragColor = (1.0 - dist) * f_color;
}