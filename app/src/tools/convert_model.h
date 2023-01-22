#pragma once

#include <string>
#include <memory>

#include "data/model_source.h"


std::unique_ptr<ModelSource> convertToModelSource(const std::string &path);
