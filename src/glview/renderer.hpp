#pragma once

typedef unsigned int id_t;
typedef unsigned long long usize_t;

#define ID_INVALID static_cast<id_t>(~(0u))

struct renderer_t
{
    virtual void init() noexcept = 0;
    virtual id_t load_material(const char* name) noexcept = 0;
    virtual id_t load_mesh(const char* name) noexcept = 0;
    virtual void begin_frame() noexcept = 0;
    virtual void end_frame() noexcept = 0;
    virtual void draw(id_t material, id_t* mesh, usize_t count) noexcept = 0;
    virtual void destroy() noexcept = 0;
};

template <typename T>
class renderer_impl : public renderer_t
{
public:
    virtual void init() noexcept override;
    virtual id_t load_material(const char* name) noexcept override;
    virtual id_t load_mesh(const char* name) noexcept override;
    virtual void begin_frame() noexcept override;
    virtual void end_frame() noexcept override;
    virtual void draw(id_t material, id_t* mesh, usize_t count) noexcept override;
    virtual void destroy() noexcept override;
private:
    T state_;
};

extern renderer_t& get_renderer();
