/**
 * @file model_obj.h
 * @author Hasan Karaman (hk@hasankaraman.dev)
 * @brief
 * @version 0.1
 * @date 2024-02-13
 *
 * @copyright Copyright (c) 2024
 *
 */

#pragma once

#include "model.hpp"

namespace nle
{

    class model_obj : public model
    {
    public:
        model_obj(const std::string& path);
        virtual ~model_obj();

        ref<multimesh_instance_3d> create_instance();

        std::string name() const;
    private:
        void load(const std::string &path);
    };

} // namespace nle
