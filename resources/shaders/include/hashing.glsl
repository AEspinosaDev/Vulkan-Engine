// Permutation-based hash a.k.a simplex noise
vec3 hash0(vec3 a,vec3 k, float scale)
{
    a = fract(a * scale);
    a += dot(a, a.yxz + k);
    return fract((a.xxy + a.yxx)*a.zyx);
}
vec3 hash31(float a, vec3 k, float scale)
{
   vec3 p3 = fract(vec3(a) * scale);
   p3 += dot(p3, p3.yzx+k);
   return fract((p3.xxy+p3.yzz)*p3.zyx); 
}