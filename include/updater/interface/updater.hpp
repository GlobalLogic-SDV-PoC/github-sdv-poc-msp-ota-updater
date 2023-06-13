#pragma once

#include <string_view>

class IUpdater
{
public:
    virtual void update(std::string_view filepath) = 0;
    virtual ~IUpdater() = default;
};
