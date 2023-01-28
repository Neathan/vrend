#include "uuid.h"

#include <random>


static std::random_device s_randomDevice;
static std::mt19937_64 s_engine(s_randomDevice());
static std::uniform_int_distribution<uint32_t> s_uniformDistribution;

UUID::UUID() : value(s_uniformDistribution(s_engine)) {}
