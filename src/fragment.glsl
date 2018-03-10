in ivec2 gl_FragCoord;
out vec3 colour;

void main() {
    colour = image[int(gl_FragCoord.x) + int(gl_FragCoord.y) * int(screen.x)].rgb;
}
