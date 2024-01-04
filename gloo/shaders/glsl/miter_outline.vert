// Modified from https://stackoverflow.com/a/59688394/20791863.
// TODO: try modified version with geometry shader shown here: https://blog.scottlogic.com/2019/11/18/drawing-lines-with-webgl.html
#version 330 core

layout (std140) uniform TVertex {
    vec4 vertex[1024];
};

uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 model_view_project_matrix;
uniform mat4 projection_matrix;
uniform vec2  u_resolution;
uniform float u_thickness;

void main()
{
    mat4 u_mvp = projection_matrix * view_matrix * model_matrix;
    int line_i = gl_VertexID / 6;
    int tri_i  = gl_VertexID % 6;

    vec4 va[4];
    for (int i=0; i<4; ++i)
    {
        vec3 world_position = vec3(model_matrix * vertex[line_i + i]);
        va[i] = projection_matrix * view_matrix * vec4(world_position, 1.0);

        va[i].xyz /= va[i].w;
        va[i].xy = (va[i].xy + 1.0) * 0.5 * u_resolution;
    }

    vec2 v_line  = normalize(va[2].xy - va[1].xy);
    vec2 nv_line = vec2(-v_line.y, v_line.x);

    vec4 pos;
    if (tri_i == 0 || tri_i == 1 || tri_i == 3)
    {
        vec2 v_pred  = normalize(va[1].xy - va[0].xy);
        vec2 v_miter = normalize(nv_line + vec2(-v_pred.y, v_pred.x));

        pos = va[1];
        pos.xy += v_miter * u_thickness * (tri_i == 1 ? -0.5 : 0.5) / max(dot(v_miter, nv_line), 1);
    }
    else
    {
        vec2 v_succ  = normalize(va[3].xy - va[2].xy);
        vec2 v_miter = normalize(nv_line + vec2(-v_succ.y, v_succ.x));

        pos = va[2];
        pos.xy += v_miter * u_thickness * (tri_i == 5 ? 0.5 : -0.5) / max(dot(v_miter, nv_line), 1);
    }

    pos.xy = pos.xy / u_resolution * 2.0 - 1.0;
    pos.xyz *= pos.w;
    gl_Position = pos;
}