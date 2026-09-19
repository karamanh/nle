/**
 * @file vertex.h
 * @author Hasan Karaman (hk@hasankaraman.dev)
 * @brief 
 * @version 0.1
 * @date 2024-02-12
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once

#include <glm/glm.hpp>

namespace nle
{
    struct vertex
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec3 color;
        glm::vec2 uv;

        /**
         * Skinning influences: up to four joint indices and their weights.
         *
         * The indices are floats rather than integers so the whole vertex stays
         * one homogeneous float array and needs no separate integer attribute
         * pointer. A float holds every integer up to 2^24 exactly, which is far
         * more than MAX_JOINTS.
         *
         * All-zero weights mean the vertex is not skinned.
         */
        glm::vec4 joints = glm::vec4(0.0f);
        glm::vec4 weights = glm::vec4(0.0f);
    };
    
} // namespace nle
