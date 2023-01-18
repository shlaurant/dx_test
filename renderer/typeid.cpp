#include "typeid.h"

namespace directx_renderer {
    uint32_t type_id_s::get_unique_typeid() {
        return type++;
    }
}