/**
 * @file model.hpp
 * @author Hasan Karaman (hk@hasankaraman.dev)
 * @brief
 * @version 0.1
 * @date 2024-02-13
 *
 * @copyright Copyright (c) 2024
 *
 */

#pragma once

#include "../mesh/multimesh_instance_3d.h"

#include <string>

namespace nle
{

    class model
    {
    public:
        model() {}
        virtual ~model() {}
        virtual ref<multimesh_instance_3d> create_instance() = 0;

        ref<class multimesh_3d> multimesh_3d() { return m_multimesh_3d; }

    protected:
        ref<class multimesh_3d> m_multimesh_3d;
        std::string m_name;

    private:
        virtual void load(const std::string &path) = 0;
    };

} // namespace nle
