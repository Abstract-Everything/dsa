#include <dsa/heap.hpp>

#include <memory>

#include <gtest/gtest.h>

using Int_Heap = dsa::Heap<int>;

static_assert(std::is_same_v<Int_Heap::Value, int>);
