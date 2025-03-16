#version 450

layout(location = 0) in vec3 position;
layout(location = 1) uniform float angle;

layout(location = 3) uniform float model_scale;
layout(location = 4) uniform vec3 model_offset;

layout(location = 5) in vec3 aColor;
layout(location = 6) in int aId;
layout(location = 7) in int aDepth;


vec3 DCENTERS[8] = {
    vec3(1, 1, 1),
    vec3(-1, 1, 1),
    vec3(-1, -1, 1),
    vec3(1, -1, 1), // upper
    vec3(1, 1, -1),
    vec3(-1, 1, -1),
    vec3(-1, -1, -1),
    vec3(1, -1, -1)
};

out vec4 vertColor;

vec3 compute_octant_offset(int id, int depth)
{
    vec3 offset = vec3(0, 0, 0);
    if (depth == 0) return offset;
    float n = 0.25;
    for (int i = 0; i < depth; i++)
    {
        offset += n * DCENTERS[id % 8];
        n /= 2;
        id /= 8;
    }

    return offset;
}

mat4 rotation3d(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;

    return mat4(
    oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
    oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
    oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
    0.0,                                0.0,                                0.0,                                1.0
    );
}

void main() {
    //    vec3 lamp = vec3(0, 20, -100);
    vec3 lamp = vec3(0, 1, -1);
    vec3 intensity = vec3(1.2);
    vec3 posi = position * pow(0.5, aDepth + 1) + compute_octant_offset(aId, aDepth);
    vec4 pos4 = vec4(posi, 1.);
    pos4 = rotation3d(vec3(0.0, 1.0, -0.2), angle) * pos4;
    //    pos4 = rotation3d(vec3(1.0, 0.0, 0.0), 3.14 / 5) * pos4;
    gl_Position = pos4;
    vec3 lamp_dir = normalize(lamp - pos4.xyz);
    //    vec3 strength = intensity * dot(lamp_dir, pos4.xyz) / distance(lamp, pos4.xyz);
    vec3 strength = intensity / (distance(lamp, pos4.xyz) * distance(lamp, pos4.xyz));
    vertColor = vec4(strength * aColor, 1.0);
}
