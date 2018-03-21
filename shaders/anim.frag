#version 410 core

flat in uint texture_id;
flat in vec3 frag_normal;
flat in vec3 frag_tangent;
flat in vec3 frag_bitangent;
flat in uint frag_side;
in vec3 frag_pos;

out vec4 frag_color;

uniform sampler2DArray texture_array;

void main() {
	vec3 light_dir = normalize(vec3(0.1, -1.0, 0.0));
	vec3 light_color = vec3(1.0, 1.0, 1.0);
	float kdiff = max(0.0, dot(frag_normal, -light_dir));

	vec2 tileUV = vec2(dot(frag_tangent, frag_pos), 
			dot(frag_bitangent, frag_pos));
	vec2 uv = fract(tileUV);
	vec3 ambient_color = vec4(texture(texture_array, vec3(uv, float(texture_id)))).rgb;
	if (texture_id >= 640){
		if (texture_id == 641) {ambient_color = vec3(0.0, 0.0, 0.9);}
		else if (texture_id == 643) {ambient_color = vec3(0.05, 0.75, 0.5);}
		else {ambient_color = vec3(0.0, 0x0, 0.9);}
	}

	if ((texture_id == 0 && frag_side == 5) || (texture_id == 52)) {
		vec3 biome_color = vec3(0.2, 0.5, 0.05);
		ambient_color = mix(ambient_color, biome_color, 0.5); 
	}
	vec3 color = 0.2 * (kdiff * light_color) + 0.6 * ambient_color;
	frag_color = vec4(color, 1.0);
}
