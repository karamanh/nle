#pragma once

#include <memory>

namespace nle
{
    template <typename ref_type>
    using ref = std::shared_ptr<ref_type>;

    template <typename weak_ref_type>
    using weak_ref = std::weak_ptr<weak_ref_type>;

    template <typename ref_type, typename ... args_type>
    ref<ref_type> make_ref(args_type&&... args)
    {
        #ifdef _USE_BOOST_
            return boost::make_shared<ref_type>(args);
        #else
            return std::make_shared<ref_type>(std::forward<args_type>(args)...);
        #endif
    } 
} // namespace nle
