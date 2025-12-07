#version 330 core

varying vec4 f_color;
varying vec4 v1;
varying vec4 division_axis;

void main(void) {
	if(length(gl_PointCoord - vec2(0.5, 0.5)) > 0.5)
		discard;

	//"vec3 division_axis = vec3(0.0f, 0.0f, 1.0f);

	float polarity_angle_size = length(v1.xyz);
	if(polarity_angle_size == 0.0f)
		discard;
	vec3 normalized_v1 = normalize(v1.xyz);

	vec2 center_coord = 2.0f*vec2(gl_PointCoord.x - 0.5, - gl_PointCoord.y + 0.5);

	vec3 fragment_coord = vec3(center_coord, sqrt(1.0 - center_coord.x*center_coord.x - center_coord.y*center_coord.y));
	float polarity_intensity = min(dot(fragment_coord, normalized_v1), 1.0f);
	float polarity_intensity_back = min(dot(vec3(fragment_coord.xy, -fragment_coord.z), normalized_v1), 1.0f);
	if(polarity_intensity < cos(polarity_angle_size) && polarity_intensity_back < cos(polarity_angle_size) && abs(dot(division_axis.xyz, fragment_coord)) > 0.01f)
		discard;

	float modifier = 1.0f;

	if(polarity_intensity < cos(polarity_angle_size))
		modifier = 0.5f;

	if(abs(dot(division_axis.xyz, fragment_coord)) <= 0.01f)
		modifier = 20.0f;
	
    gl_FragColor = (1.0f - 1.0f*length(gl_PointCoord - vec2(0.5, 0.5))) * modifier * f_color;
}