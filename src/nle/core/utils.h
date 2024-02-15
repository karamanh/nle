#pragma once

#include <random>
#include <string>
#include <fstream>
#include <sstream>
#include <cstring>
#include <iostream>
#include <algorithm>

namespace nle::utils
{

    extern std::ostream& output_stream;

    template <typename arg_type, typename... args_type>
    inline void print(arg_type &&arg, args_type &&...args)
    {
        output_stream << "\x1b[32m" << std::forward<arg_type>(arg);
        ((output_stream << ',' << std::forward<args_type>(args)), ...);
        output_stream << "\x1b[0m\n";
    }

    template <typename arg_type, typename... args_type>
    inline void prdebug(arg_type &&arg, args_type &&...args)
    {
        output_stream << "\x1b[33m" << std::forward<arg_type>(arg);
        ((output_stream << ',' << std::forward<args_type>(args)), ...);
        output_stream << "\x1b[0m\n";
    }

    template <typename arg_type, typename... args_type>
    inline void prerror(arg_type &&arg, args_type &&...args)
    {
        output_stream << "\x1b[31m" << std::forward<arg_type>(arg);
        ((output_stream << ',' << std::forward<args_type>(args)), ...);
        output_stream << "\x1b[0m\n";
    }

    const constexpr unsigned int UID_LEN = 16;

    template <typename T = std::mt19937>
    inline auto random_generator() -> T
    {
        auto constexpr seed_bytes = sizeof(typename T::result_type) * T::state_size;
        auto constexpr seed_len = seed_bytes / sizeof(std::seed_seq::result_type);
        auto seed = std::array<std::seed_seq::result_type, seed_len>();
        auto dev = std::random_device();
        std::generate_n(begin(seed), seed_len, std::ref(dev));
        auto seed_seq = std::seed_seq(begin(seed), end(seed));
        return T{seed_seq};
    }

    inline std::string generate_random_alphanumeric_string(std::size_t len = UID_LEN)
    {
        static constexpr auto chars =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";
        thread_local auto rng = random_generator<>();
        auto dist = std::uniform_int_distribution{{}, std::strlen(chars) - 1};
        auto result = std::string(len, '\0');
        std::generate_n(begin(result), len, [&]()
                        { return chars[dist(rng)]; });
        return result;
    }

    inline std::string load_file(const std::string& path)
    {
        std::ifstream ifs(path);
        std::stringstream iss;
        if(ifs.is_open())
        {
            iss << ifs.rdbuf();
        }
        return iss.str();
    }

} // namespace nle
