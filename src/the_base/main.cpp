#include <napp/appmain.h>
#include <napp/callback.h>

class app_t
{
public:
    void tick()
    {
    }
    void run()
    {
        napp_set_context(this);
        napp_set_callback(NAPP_UPDATE_END, &napp_cb<app_t, &app_t::tick>);
        napp_run();
    }
};

static void napp_main()
{
    if (napp_initialize())
    {
        app_t my_app;
        my_app.run();
    }
}

/*
#include <Eigen/Geometry>
#include <par/par_shapes.h>
#include <rapidjson/document.h>

typedef Eigen::Vector2f vec2f_t;
typedef Eigen::Vector3f vec3f_t;
typedef Eigen::Vector4f vec4f_t;
typedef Eigen::Quaternionf quatf_t;
typedef Eigen::Matrix4f mat4f_t;
typedef Eigen::Transform<float, 3, Eigen::Affine> transform_t;
#define vec3_init(x,y,z) vec3f_t(x, y, z)
#define quat_identity(q) do { q.setIdentity(); } while(0)
#define transform_identity(t) quat_identity(t)
#define quat_axis_angle(q, an, ax) do { q = Eigen::AngleAxisf(deg_to_rad(an), ax); } while (0)
#define quat_append(q1, q2)  do { q1 = (q1 * q2); } while(0)
#define vec3_rotate(v3, q) do { q = (q * v3); } while(0)
#define deg_to_rad(d) (d*0.01745329251f) //d*M_PI/180

// left-right: rotate POSITION of the camera (around WORLD UP vector) before creating view matrix

struct scene_node_t
{
    transform_t transform;
};

int main(int argc, const char * argv[])
{
    quatf_t rotation, rotator;
    vec3f_t point(1.f, 0.f, 0.f);
    quat_identity(rotation);
    quat_axis_angle(rotator, 90.f, vec3_init(0.f, 0.f, 1.f));
    quat_append(rotation, rotator);
    
    transform_t transform;
    transform_identity(transform);
    transform.rotate(rotation);
    
    quat_append(rotation, rotator);
    quat_append(rotation, rotator);
    quat_append(rotation, rotator);
    return 0;
}*/
