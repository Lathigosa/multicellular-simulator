#version 330 core

varying vec4 f_color;

void main(void) {
	vec2 p = gl_PointCoord * 2.0 - 1.0;
	float r2 = dot(p, p);
	if (r2 > 1.0) discard;
	vec3 normal = normalize(vec3(p, sqrt(1.0 - r2)));
	vec3 lightDir = normalize(vec3(0.0, 0.0, 1.0));
	float ambient = 0.4;
	float diffuse = max(dot(normal, lightDir), 0.0) * (1.0 - ambient) + ambient;
	gl_FragColor = vec4(f_color.rgb * diffuse, f_color.a);
}