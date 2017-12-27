attribute vec2 coord2d;
varying vec4 f_color;
uniform float offset_x;
uniform float scale_x;
uniform float cutoff;

void main(void) {
    if(coord2d.x > cutoff/scale_x)
        gl_Position = vec4((coord2d.x + offset_x) * scale_x, coord2d.y, -2, 1);
    else
        gl_Position = vec4((coord2d.x + offset_x) * scale_x, coord2d.y, 0, 1);
    f_color = vec4(-coord2d.y- 0.2, coord2d.y + .85, coord2d.y + 1.15, 1);
}