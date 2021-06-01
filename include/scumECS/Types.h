#pragma once
#include <cstdint>
#include <unordered_map>

namespace scum
{

// the entity ID type
using ID = uint32_t;
// the hash table type used for lookup
template<typename K, typename V>
using AssocContainer = std::unordered_map<K,V>; const ID Null = 0;

}
