#version 400 core

in vec4 f_color;      // interpolated color from vertex shader
uniform vec4 grid_color;
//uniform float near_fade; // distance at which fading starts near the camera
//uniform float far_fade;  // distance at which fading starts far from the camera

void main() {

    const float near_fade = 0.8;
    const float far_fade = 500.0;

    // Get the fragment depth in view space (positive forward)
    float frag_z = gl_FragCoord.z / gl_FragCoord.w;  // linearized depth if needed

    // Compute fade factor (1.0 = fully visible, 0.0 = fully transparent)
    float fade = 1.0;

    // Fade out when too close
    if(frag_z < near_fade) {
        fade = clamp(frag_z / near_fade, 0.0, 1.0);
    }
    // Fade out when too far
    else if(frag_z > far_fade) {
        fade = 1.0 - clamp((far_fade - frag_z) / (1.0-far_fade), 0.0, 1.0);
    }

    float alpha = pow(smoothstep(0.0, 1.0, fade), 2.2);

    gl_FragColor = vec4(grid_color.rgb * fade, grid_color.a * fade);
}