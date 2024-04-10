
in vec3 position;
in vec2 texture_coord;

out vec2 frag_texture_coord;

uniform mat4 transform;

void main() {
	frag_texture_coord = texture_coord;
	gl_Position = vec4(position, 1.0);
}
