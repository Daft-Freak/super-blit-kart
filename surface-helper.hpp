#pragma once

#include <memory>

#include "graphics/surface.hpp"

// work around blit::Surface ownership issues
class OwnedSurface final {
public:
    OwnedSurface(blit::Surface *surf = nullptr) : surf(surf) {}
    OwnedSurface(const OwnedSurface &) = delete;
    OwnedSurface(OwnedSurface &&other) {
        *this = std::move(other);
    }

    ~OwnedSurface() {
        if(surf) {
            delete[] surf->data;
            delete[] surf->palette;
            delete surf;
        }
    }

    OwnedSurface &operator=(const OwnedSurface &) = delete;
    OwnedSurface &operator=(OwnedSurface &&other) {
        surf = other.surf;
        other.surf = nullptr;
        return *this;
    }

    // pretend to be a Surface *
    operator blit::Surface *() {
        return surf;
    }

    blit::Surface *operator->() {
        return surf;
    }

private:
    blit::Surface *surf;
};

static_assert(sizeof(OwnedSurface) == sizeof(blit::Surface *));