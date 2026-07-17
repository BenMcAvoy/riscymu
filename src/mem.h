#pragma once

#include <cstddef>
#include <array>
#include <stdexcept>

template <std::size_t Size>
struct Mem
{
    Mem()
    {
        data_.fill(0);
    }

    std::array<std::uint8_t, Size> data_;

    void *data()
    {
        return data_.data();
    }

    inline void write(std::uint64_t addr, const void *data, std::size_t size) noexcept
    {
        std::memcpy(data_.data() + addr, data, size);
    }

    template <typename T>
    inline void write(std::uint64_t addr, const T &data) noexcept
    {
        std::memcpy(data_.data() + addr, &data, sizeof(T));
    }

    template <typename T>
    inline T read(std::uint64_t addr) const noexcept
    {
        T value;
        std::memcpy(&value, data_.data() + addr, sizeof(T));
        return value;
    }
};