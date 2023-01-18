#pragma once

namespace directx_renderer {
    class type_id_s {
    public:
        static type_id_s *instance() {
            static type_id_s i;
            return &i;
        }

        template<typename T>
        uint32_t type_id(){
            static const uint32_t t = get_unique_typeid();
            return t;
        }

    private:
        uint32_t type = 1u;

        type_id_s() = default;

        uint32_t get_unique_typeid();
    };

    template<typename T>
    const uint32_t type_id() {
        return type_id_s::instance()->type_id<T>();
    }
}