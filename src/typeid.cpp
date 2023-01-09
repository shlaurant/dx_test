#include "typeid.h"

namespace fuse {
    uint32_t type_id_s::get_unique_typeid() {
        return type++;
    }
}