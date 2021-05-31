#pragma once
#include <cstdint>
#include <unordered_map>

namespace scum
{

// the entity ID type
using EntID = uint32_t;
// the hash table used for lookup
template<typename K, typename V>
using AssocContainer = std::unordered_map<K,V>; const EntID Null = 0;

}
