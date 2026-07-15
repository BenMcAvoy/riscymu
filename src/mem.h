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

    void write(std::uint64_t addr, const void *data, std::size_t size)
    {
        if (addr + size > Size)
        {
            throw std::out_of_range("Memory write out of bounds");
        }
        std::memcpy(data_.data() + addr, data, size);
    }

    template <typename T>
    void write(std::uint64_t addr, const T &data)
    {
        if (addr + sizeof(T) > Size)
        {
            throw std::out_of_range("Memory write out of bounds");
        }
        std::memcpy(data_.data() + addr, &data, sizeof(T));
    }

    template <typename T>
    T read(std::uint64_t addr) const
    {
        if (addr + sizeof(T) > Size)
        {
            throw std::out_of_range("Memory read out of bounds");
        }
        T value;
        std::memcpy(&value, data_.data() + addr, sizeof(T));
        return value;
    }
};