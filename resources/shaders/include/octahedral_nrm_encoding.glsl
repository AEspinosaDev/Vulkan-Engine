vec2 encode_normal_oct(vec3 n) {
    n /= (abs(n.x) + abs(n.y) + abs(n.z)); // Project onto octahedron
    vec2 enc = n.xy;
    if (n.z < 0.0) {
        vec2 signNotZero = vec2(
            (n.x >= 0.0) ? 1.0 : -1.0,
            (n.y >= 0.0) ? 1.0 : -1.0
        );
        enc = (1.0 - abs(enc.yx)) * signNotZero;
    }
    enc = enc * 0.5 + 0.5; // Map from [-1,1] to [0,1]
    return enc;
}

vec3 decode_normal_oct(vec2 f) {
    f = f * 2.0 - 1.0; // Map from [0,1] to [-1,1]
    vec3 n = vec3(f.xy, 1.0 - abs(f.x) - abs(f.y));
    if (n.z < 0.0) {
        vec2 signNotZero = vec2(
            (n.x >= 0.0) ? 1.0 : -1.0,
            (n.y >= 0.0) ? 1.0 : -1.0
        );
        n.xy = (1.0 - abs(n.yx)) * signNotZero;
    }
    return normalize(n);
}
