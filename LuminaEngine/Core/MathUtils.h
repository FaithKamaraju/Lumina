//
// Created by Faith Kamaraju on 2026-01-31.
//

#pragma once

#include <glm/glm.hpp>
#include <glm/detail/type_quat.hpp>
#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/math.hpp>

#include "glm/gtc/type_ptr.hpp"


namespace LE::Math {

    inline void toGLM(glm::mat2& glm_mat ,const fastgltf::math::fmat2x2& fast_mat) {
        memcpy(glm::value_ptr(glm_mat), fast_mat.data(), sizeof(glm::mat2));
    }
    inline void toGLM(glm::mat3& glm_mat ,const fastgltf::math::fmat3x3& fast_mat) {
        memcpy(glm::value_ptr(glm_mat), fast_mat.data(), sizeof(glm::mat3));
    }
    inline void toGLM(glm::mat4& glm_mat ,const fastgltf::math::fmat4x4& fast_mat) {
        memcpy(glm::value_ptr(glm_mat), fast_mat.data(), sizeof(glm::mat4));
    }

    inline glm::vec2 toGLM(const fastgltf::math::fvec2& fast_vec) {
        return {fast_vec.x(), fast_vec.y()};
    }
    inline glm::vec3 toGLM(const fastgltf::math::fvec3& fast_vec) {
        return {fast_vec.x(), fast_vec.y(), fast_vec.z()};
    }
    inline glm::vec4 toGLM(const fastgltf::math::fvec4& fast_vec) {
        return {fast_vec.x(), fast_vec.y(), fast_vec.z(), fast_vec.w()};
    }

    inline glm::ivec2 toGLM(const fastgltf::math::ivec2& fast_vec) {
        return {fast_vec.x(), fast_vec.y()};
    }
    inline glm::ivec3 toGLM(const fastgltf::math::ivec3& fast_vec) {
        return {fast_vec.x(), fast_vec.y(), fast_vec.z()};
    }
    inline glm::ivec4 toGLM(const fastgltf::math::ivec4& fast_vec) {
        return {fast_vec.x(), fast_vec.y(), fast_vec.z(), fast_vec.w()};
    }

    inline glm::quat toGLM(const fastgltf::math::fquat& fast_quat) {
        return {fast_quat.w(), fast_quat.x(), fast_quat.y(), fast_quat.z()};
    }
    inline glm::mat4 toGLM(const fastgltf::TRS& trs)
    {
        glm::vec3 t = toGLM(trs.translation);
        glm::quat r = toGLM(trs.rotation);
        glm::vec3 s = toGLM(trs.scale);

        auto M = glm::mat4(1.0f);
        M = glm::translate(M, t);
        M *= glm::mat4_cast(r);
        M = glm::scale(M, s);

        return M;
    }


}

