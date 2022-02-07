#version 430
///------------------------------------------
//-------------INPUTS---------------
//-----------------------------------------
layout(local_size_x = 1, local_size_y = 1) in;
layout(rgba32f, binding = 0) uniform image2D img_output;
uniform sampler2D env_texture;
///------------------------------------------
//-------------UBO---------------
//-----------------------------------------
layout (std140) uniform camInfo
{
    vec4 cameraPosition;
    vec4 directionView;
    vec4 rightPerp;
};


///------------------------------------------
//-------------UNIFORMS---------------
//-----------------------------------------
uniform int state;
uniform float u_Accum;

///------------------------------------------

// constants
const float PI = 3.14159265359;
uint g_state = 0;


#define LAMBERTIAN  0
#define METAL  1
#define DIELECTRIC 2
#define LIGHT 3

#define LENS_RADIUS 0
#define K_samples 10
#define MAX_DEPTH 30


//-------------Help Functions---------------
//-----------------------------------------
uint wang_hash(uint seed)
{
    seed = (seed ^ 61) ^ (seed >> 16);
    seed *= 9;
    seed = seed ^ (seed >> 4);
    seed *= 0x27d4eb2d;
    seed = seed ^ (seed >> 15);
    return seed;
}
uint rand(inout uint state)
{
    state++;
    return wang_hash(state);
}


float random_float(inout uint state)
{
    return (rand(state) & 0xFFFFFF) / 16777216.0f;
}

vec3 random_in_unit_sphere(inout uint state)
{
    float z = random_float(state) * 2.0f - 1.0f;
    float t = random_float(state) * 2.0f * 3.1415926f;
    float r = sqrt(max(0.0, 1.0f - z * z));
    float x = r * cos(t);
    float y = r * sin(t);
    vec3 res = vec3(x, y, z);
    res *= pow(random_float(state), 1.0 / 3.0);
    return res;
}



vec3 random_in_unit_disk(inout uint state)
{
    float a = random_float(state) * 2.0f * 3.1415926f;
    vec2 xy = vec2(cos(a), sin(a));
    xy *= sqrt(random_float(state));
    return vec3(xy, 0);
}

struct Metal{
    float roughness;
};
struct Dielectric
{
    float ref_idx;
};


struct Material {
    vec3 col; // color
    int type; // metal, or lamb
    Metal metal;
    Dielectric dielectric;
};


struct Ray
{
    vec3 direction;
    vec3 origin;
};

struct Sphere
{
    vec3 center_pos;
    float R;
    int material_id;
};

layout(std430) buffer Spheres_data
{
    Sphere spheres_ssbo[];
};



layout(std430) buffer Materials_data
{
    Material materials_ssbo[];
};


uint spheres_number = spheres_ssbo.length();

struct HitRecord
{
    float t;
    vec3 position;
    vec3 normal;
    int material_id;
};

struct Camera{

    vec3 origin;
    vec3 look_at;
    float vfov;
    vec3 horizontal;
    vec3 vertical;
    vec3 lower_left_corner;
    vec3 offset;

};

struct Scene
{   
    int num_spheres;
    int num_materials;
    Sphere spheres[32];
    Material materials[32];
};

Camera createCamera(float aspect_ratio){
    Camera cam;

    // this block is temporary. later need to hook it up with keyboard and mouth moovement
    cam.origin  = cameraPosition.xyz;
    cam.vfov = 40.0 * PI/180.0;
    cam.look_at  = cam.origin + directionView.xyz;
    vec3 vup = vec3(0.0,1.0,0.0);
    /////

    float viewport_height = 2*tan(cam.vfov/2)* length(cam.look_at - cam.origin);
    float viewport_width = aspect_ratio * viewport_height;

    vec3 w = normalize(cam.origin - cam.look_at);
    vec3 u = normalize(cross(vup, w));
    vec3 v = cross(w, u);
    
    cam.horizontal = viewport_width * u;
    cam.vertical = viewport_height * v;
    cam.lower_left_corner = cam.origin - cam.horizontal/2 - cam.vertical/2 - w;

    
    vec3 dn = LENS_RADIUS*random_in_unit_disk(g_state);
    cam.offset = u*dn.x + v*dn.y;

    return cam;   
}

Ray CreateRay(in ivec2 pixel_coords){
    vec2 dims = imageSize(img_output); // fetch image dimensions
    float x = (float(pixel_coords.x ) / dims.x);
    float y = (float(pixel_coords.y ) / dims.y);
    float aspect = dims.x/dims.y;
    
    Camera cam = createCamera(aspect);

    
    Ray r;
    

    r.origin = cam.origin+ cam.offset;
    r.direction = normalize(cam.lower_left_corner + (x + (0.5 - random_float(g_state))*0.002) *cam.horizontal
     + (y + (0.5 - random_float(g_state))*0.002)*cam.vertical - cam.origin);
    return r;
}


bool sphere_hit(in Ray r, in Sphere s,float t_min, float t_max, out HitRecord rec){
    vec3 oc = r.origin - s.center_pos;
    float a = dot(r.direction, r.direction);
    float b = 2.0 * dot(oc, r.direction);
    float c = dot(oc, oc) - s.R*s.R;
    float discriminant = b*b - 4*a*c;

    if (discriminant > 0)
    { 
    float root = (-b -sqrt(discriminant))/(2*a);

    if (root > t_min && root < t_max)
        {
            rec.t = root;
            rec.position = r.origin + root*r.direction;
            rec.normal = normalize( (rec.position - s.center_pos) / s.R);
            rec.material_id = s.material_id;
            return true;
        }
    root = (-b +sqrt(discriminant))/(2*a);
    if (root > t_min && root < t_max)
        {
            rec.t = root;
            rec.position = r.origin + root*r.direction;
            rec.normal = (rec.position - s.center_pos) / s.R;
            rec.material_id = s.material_id;
            return true;
        }    

    }

    return false;
    
}


bool ray_scene_hit(in Ray r, in float t_min, in float t_max,  inout HitRecord rec){

float closest = t_max;
bool hit_anything = false;

    for (int i=0; i < spheres_number; i++)
    {
        Sphere s = spheres_ssbo[i];
        if (sphere_hit(r,s, t_min, closest, rec)) // if we shoot into sphere
        {
            closest = rec.t;
            hit_anything = true;
        }
    }    
return hit_anything;
}

bool trace_once(in Ray ray, out HitRecord rec)
{
    if (ray_scene_hit(ray, 0.001, 100000.0, rec))
        return true;
    else
        return false;
}


bool scatter_metal(in Ray in_ray,in Material mat, in HitRecord rec,out vec3 attenuation, out Ray out_ray)
{
    out_ray.direction = normalize( reflect(in_ray.direction, rec.normal) + mat.metal.roughness*random_in_unit_sphere(g_state));
    out_ray.origin = rec.position;
    attenuation = mat.col;
    return dot(out_ray.direction, rec.normal) > 0;
}

bool scatter_lambertian(in Ray in_ray,in Material mat, in HitRecord rec,out vec3 attenuation, out Ray out_ray)
{
    out_ray.direction = normalize(rec.position + rec.normal + random_in_unit_sphere(g_state) -rec.position );
    out_ray.origin = rec.position;
    attenuation = mat.col;
    return true;
}
bool scatter_light(in Ray in_ray,in Material mat, in HitRecord rec,out vec3 attenuation, out Ray out_ray)
{   
    attenuation = mat.col;
    return true;
}

bool refract(in vec3 v, in vec3 n, in float ni_over_nt, out vec3 refracted)
{
    vec3 uv = normalize(v);
    float dt = dot(uv, n);
    float discriminant = 1.0 - ni_over_nt * ni_over_nt * (1 - dt * dt);

    if (discriminant > 0)
    {
        refracted = ni_over_nt * (uv - n * dt) - n * sqrt(discriminant);
        return true;
    }
    else
        return false;
}

// ------------------------------------------------------------------

float schlick(float cosine, float ref_idx)
{
    float r0 = (1.0 - ref_idx) / (1.0 + ref_idx);
    r0 = r0 * r0;
    return r0 + (1.0 - r0) * pow((1.0 - cosine), 5);
}

// ------------------------------------------------------------------

bool scatter_dielectric(in Ray in_ray, in Material mat,in HitRecord rec, out vec3 attenuation, out Ray scattered_ray)
{
    vec3 outward_normal;
    vec3 reflected = reflect(in_ray.direction, rec.normal);
    float ni_over_nt;
    attenuation = vec3(1.0, 1.0, 1.0);
    vec3 refracted;
    float reflect_prob;
    float cosine;

    if (dot(in_ray.direction, rec.normal) > 0)
    {
        outward_normal = -rec.normal;
        ni_over_nt = mat.dielectric.ref_idx;
        cosine = mat.dielectric.ref_idx * dot(in_ray.direction, rec.normal) / length(in_ray.direction);
    }
    else
    {
        outward_normal = rec.normal;
        ni_over_nt = 1.0 / mat.dielectric.ref_idx;
        cosine = -dot(in_ray.direction, rec.normal) / length(in_ray.direction);
    }

    if (refract(in_ray.direction, outward_normal, ni_over_nt, refracted))
        reflect_prob = schlick(cosine, mat.dielectric.ref_idx);
    else
        reflect_prob = 1.0;

    if (random_float(g_state) < reflect_prob)
    {
        scattered_ray.origin = rec.position;
        scattered_ray.direction = reflected;
    }
    else
    {
        scattered_ray.origin = rec.position;
        scattered_ray.direction = refracted;
    }
    return true;
}




vec3 calculate_color(in Ray r)
{   vec3 color = vec3(1.0);
    int depth = 0;
    vec3 attenuation = vec3(0.0);

    HitRecord rec;
    Ray new_ray = r;
    while (depth < MAX_DEPTH)
    {
        if (trace_once(new_ray, rec))
        {
            Ray scattered_ray;
            Material mat = materials_ssbo[rec.material_id];

            if (mat.type == METAL)
            {
            
                if(scatter_metal(new_ray,mat,rec,attenuation,scattered_ray))
                {
                    color *= attenuation*0.7;
                    new_ray = scattered_ray;
                }
            }
            else if (mat.type == LAMBERTIAN)
            {
                if(scatter_lambertian(new_ray,mat,rec,attenuation,scattered_ray))
                {
                    color *= attenuation*0.7;
                    new_ray = scattered_ray;
                }
                else 
                {
                    attenuation = vec3(0.0);
                    color *= attenuation;
                    break;
                }
            }
            else if (mat.type == DIELECTRIC)
            {
                if(scatter_dielectric(new_ray,mat,rec,attenuation,scattered_ray))
                {
                    color *= attenuation;
                    new_ray = scattered_ray;
                }
                else
                {
                    attenuation = vec3(0.0,0.0,0.0);
                    color *= attenuation;
                    break;
                }
            }
            else if(mat.type == LIGHT)
            {
                if (scatter_light(new_ray,mat,rec,attenuation,scattered_ray))
                {
                    color *= attenuation;
                    break;
                }
            }
        }

        else 
        {
            float t = 0.5 * (r.direction.y + 1.0);
            //vec3 sky_color =vec3(0.2);//(1.0 - t) * vec3(1.0) + t * vec3(0.5, 0.7, 1.0);

            vec3 sky_color = texture(env_texture,0.3 + vec2(1+new_ray.direction.x/2.0,new_ray.direction.y )).rgb;
            color *= sky_color *0.9;
            break;
        }

        depth ++;
    }
    return color;

}




///------------------------------------------
//MAIN 
//-----------------------------------------
void main()
{


Scene scene;



vec3 pixel_color = vec3(0.0);

ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
//g_state = gl_GlobalInvocationID.x * 1973 + gl_GlobalInvocationID.y * 9277 + uint(state) * 2699 | 1;
g_state = gl_GlobalInvocationID.x * 1973 + gl_GlobalInvocationID.y * 9277 + uint(state) * 2699 ;
/////// spheres



for (int i = 0; i < K_samples; i++)
{
    Ray ray = CreateRay(pixel_coords);
    pixel_color += calculate_color(ray);
}
pixel_color /= K_samples;

vec3 prev_color = pow(imageLoad(img_output, pixel_coords).rgb,vec3(2));
vec3 final = mix(pixel_color, prev_color, u_Accum);


imageStore(img_output, pixel_coords, sqrt(vec4(final,1.0)));

}