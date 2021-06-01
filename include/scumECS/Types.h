#pragma once
#include <cstdint>
#include <tsl/robin_map.h>

namespace scum
{

// the entity ID type
using ID = uint32_t;
// the hash table type used for lookup
template<typename K, typename V>
using AssocContainer = tsl::robin_map<K,V>;
const ID Null = 0;

}
