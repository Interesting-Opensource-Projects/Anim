#include "bone.h"
#include "../util/utility.h"
#include <string>
#include <util/log.h>

namespace anim
{
    Bone::Bone() = default;
    Bone::Bone(const std::string &name, const aiNodeAnim *channel, const glm::mat4 &inverse_binding_pose)
        : name_(name),
          local_transform_(1.0f)
    {
        bindpose_ = glm::inverse(inverse_binding_pose);
        int num_positions = channel->mNumPositionKeys;
        int num_rotations = channel->mNumRotationKeys;
        int num_scales = channel->mNumScalingKeys;
        // push position
        for (int pos_idx = 0; pos_idx < num_positions; ++pos_idx)
        {
            aiVector3D aiPosition = channel->mPositionKeys[pos_idx].mValue;
            float time = channel->mPositionKeys[pos_idx].mTime;
            push_position(AiVecToGlmVec(aiPosition), time);
        }

        // push rotation
        for (int rot_idx = 0; rot_idx < num_rotations; ++rot_idx)
        {
            aiQuaternion aiOrientation = channel->mRotationKeys[rot_idx].mValue;
            float time = channel->mRotationKeys[rot_idx].mTime;
            push_rotation(AiQuatToGlmQuat(aiOrientation), time);
        }

        // push scaling
        for (int scale_idx = 0; scale_idx < num_scales; ++scale_idx)
        {
            aiVector3D scale = channel->mScalingKeys[scale_idx].mValue;
            float time = channel->mScalingKeys[scale_idx].mTime;
            push_scale(AiVecToGlmVec(scale), time);
        }

        // unbind bind pose
        for (auto time : time_set_)
        {
            auto t = positions_.find(time);
            auto r = rotations_.find(time);
            auto s = scales_.find(time);
            glm::vec3 tt = (t != positions_.end()) ? t->second.position : glm::vec3(0.0f, 0.0f, 0.0f);
            glm::quat rr = (r != rotations_.end()) ? r->second.orientation : glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
            glm::vec3 ss = (s != scales_.end()) ? s->second.scale : glm::vec3(1.0f, 1.0f, 1.0f);
            glm::mat4 transformation = glm::translate(glm::mat4(1.0f), tt) * glm::toMat4(glm::normalize(rr)) * glm::scale(glm::mat4(1.0f), ss);
            transformation = inverse_binding_pose * transformation;

            auto [translation, rotation, scale] = DecomposeTransform(transformation);
            if (t != positions_.end())
            {
                t->second.position = translation;
            }
            if (r != rotations_.end())
            {
                r->second.orientation = rotation;
            }
            if (s != scales_.end())
            {
                s->second.scale = scale;
            }
        }
    }

    void Bone::update(float animation_time, float factor)
    {
        factor_ = factor;
        animation_time /= factor;
        glm::mat4 translation = interpolate_position(animation_time);
        glm::mat4 rotation = interpolate_rotation(animation_time);
        glm::mat4 scale = interpolate_scaling(animation_time);
        local_transform_ = translation * rotation * scale;
    }

    glm::mat4 &Bone::get_local_transform(float animation_time, float factor)
    {
        update(animation_time, factor);
        return local_transform_;
    }

    const std::set<float> &Bone::get_time_set() const
    {
        return time_set_;
    }

    const std::string &Bone::get_name() const { return name_; }

    float Bone::get_factor()
    {
        return factor_;
    }
    void Bone::get_ai_node(aiNodeAnim *channel, const aiMatrix4x4 &binding_pose_transform, float factor, bool is_interpolated)
    {
        channel->mNodeName = aiString(name_);

        std::vector<aiVectorKey> pos_keys;
        std::vector<aiQuatKey> rot_keys;
        std::vector<aiVectorKey> scl_keys;

        std::map<float, glm::mat4> transform_map;
        transform_map[0.0f] = glm::mat4(1.0f);
        for (auto time : time_set_)
        {
            float timestamp = float(int(time * factor));
            transform_map[timestamp] = get_local_transform(timestamp, factor); // transformation;
        }

        if (is_interpolated)
        {
            float duration = *time_set_.rbegin();
            float anim_time = 0.0f;
            std::map<float, glm::mat4> new_transform_map;
            auto before_transform = glm::mat4(1.0f);
            for (auto &t_mp : transform_map)
            {
                for (; anim_time < t_mp.first; anim_time += 1.0f)
                {
                    new_transform_map[anim_time] = get_local_transform(anim_time, factor);
                }
                before_transform = new_transform_map[t_mp.first] = t_mp.second;
                anim_time = t_mp.first + 1.0f;
            }
            transform_map = new_transform_map;
        }

        for (auto &transform : transform_map)
        {
            auto transformation = AiMatToGlmMat(binding_pose_transform) * transform.second;
            auto [t, r, s] = DecomposeTransform(transformation);

            pos_keys.push_back(aiVectorKey(transform.first, GlmVecToAiVec(t)));
            rot_keys.push_back(aiQuatKey(transform.first, GlmQuatToAiQuat(r)));
            scl_keys.push_back(aiVectorKey(transform.first, GlmVecToAiVec(s)));
        }
        channel->mNumPositionKeys = pos_keys.size(); // positions_.size();
        channel->mNumRotationKeys = rot_keys.size(); // rotations_.size();
        channel->mNumScalingKeys = scl_keys.size();  // scales_.size();

        channel->mPositionKeys = new aiVectorKey[channel->mNumPositionKeys];
        channel->mRotationKeys = new aiQuatKey[channel->mNumRotationKeys];
        channel->mScalingKeys = new aiVectorKey[channel->mNumScalingKeys];

        for (int i = 0; i < pos_keys.size(); i++)
        {
            channel->mPositionKeys[i].mValue = pos_keys[i].mValue;
            channel->mPositionKeys[i].mTime = pos_keys[i].mTime;
            channel->mRotationKeys[i].mValue = rot_keys[i].mValue;
            channel->mRotationKeys[i].mTime = rot_keys[i].mTime;
            channel->mScalingKeys[i].mValue = scl_keys[i].mValue;
            channel->mScalingKeys[i].mTime = scl_keys[i].mTime;
        }
    }

    void Bone::set_name(const std::string &name)
    {
        name_ = name;
    }
    void Bone::push_position(const glm::vec3 &pos, float time)
    {
        if (floor(time) != time)
        {
            return;
        }
        positions_[time] = {pos, time};
        time_set_.insert(time);
    }
    void Bone::push_rotation(const glm::quat &quat, float time)
    {
        if (floor(time) != time)
        {
            return;
        }
        rotations_[time] = {quat, time};
        time_set_.insert(time);
    }
    void Bone::push_scale(const glm::vec3 &scale, float time)
    {
        if (floor(time) != time)
        {
            return;
        }
        scales_[time] = {scale, time};
        time_set_.insert(time);
    }

    float Bone::get_scale_factor(float last_time_stamp, float next_time_stamp, float animation_time)
    {
        float scaleFactor = 0.0f;
        float midWayLength = animation_time - last_time_stamp;
        float framesDiff = next_time_stamp - last_time_stamp;
        if (last_time_stamp == next_time_stamp || framesDiff < 0.0f)
        {
            return 1.0;
        }
        scaleFactor = midWayLength / framesDiff;
        return scaleFactor;
    }

    glm::mat4 Bone::interpolate_position(float animation_time)
    {
        // const KeyPosition &p1Index = get_end<KeyPosition>(positions_, animation_time, {glm::vec3(0.0f, 0.0f, 0.0f), animation_time});
        // const KeyPosition &p0Index = get_start<KeyPosition>(positions_, animation_time, {glm::vec3(0.0f, 0.0f, 0.0f), 0.0f});
        // int idx = get_start_vec<KeyPosition>(pos_, animation_time);
        // const KeyPosition &p0Index = pos_[idx];
        // const KeyPosition &p1Index = pos_[(idx == pos_.size() - 1) ? idx : idx + 1];
        const auto &[p0Index, p1Index] = get_start_end<KeyPosition>(positions_, animation_time, {glm::vec3(0.0f, 0.0f, 0.0f), 0.0f});
        float scaleFactor = get_scale_factor(p0Index.get_time(),
                                             p1Index.get_time(), animation_time);
        return glm::translate(glm::mat4(1.0f), glm::mix(p0Index.position, p1Index.position, scaleFactor));
    }

    glm::mat4 Bone::interpolate_rotation(float animation_time)
    {
        const auto &[p0Index, p1Index] = get_start_end<KeyRotation>(rotations_, animation_time, {glm::quat(1.0f, 0.0f, 0.0f, 0.0f), 0.0f});
        float scaleFactor = get_scale_factor(p0Index.get_time(),
                                             p1Index.get_time(), animation_time);
        glm::quat finalRotation = glm::slerp(p0Index.orientation, p1Index.orientation, (float)scaleFactor);
        return glm::toMat4(glm::normalize(finalRotation));
    }

    glm::mat4 Bone::interpolate_scaling(float animation_time)
    {
        const auto &[p0Index, p1Index] = get_start_end<KeyScale>(scales_, animation_time, {glm::vec3(1.0f, 1.0f, 1.0f), 0.0f});
        float scaleFactor = get_scale_factor(p0Index.get_time(),
                                             p1Index.get_time(), animation_time);
        return glm::scale(glm::mat4(1.0f), glm::mix(p0Index.scale, p1Index.scale, scaleFactor));
    }
    void Bone::replace_or_add_keyframe(const glm::mat4 &transform, float time)
    {
        // auto [t, r, s] = DecomposeTransform(transform);
        // float time_stamp = floorf(time) / factor_;
        // positions_[time_stamp] = {t, time_stamp};
        // scales_[time_stamp] = {s, time_stamp};
        // rotations_[time_stamp] = {r, time_stamp};
        // time_set_.insert(time_stamp);
        float time_stamp = floorf(time) / factor_;
        auto [lt, lr, ls] = DecomposeTransform(get_local_transform(time, factor_));
        auto [t, r, s] = DecomposeTransform(transform);

        bool is_t_changed = !(lt.x - 1e-3 < t.x && t.x < lt.x + 1e-3 && lt.y - 1e-3 < t.y && t.y < lt.y + 1e-3 && lt.z - 1e-3 < t.z && t.z < lt.z + 1e-3);
        bool is_r_changed = !(lr.x - 1e-3 < r.x && r.x < lr.x + 1e-3 && lr.y - 1e-3 < r.y && r.y < lr.y + 1e-3 && lr.z - 1e-3 < r.z && r.z < lr.z + 1e-3);
        bool is_s_changed = !(ls.x - 1e-3 < s.x && s.x < ls.x + 1e-3 && ls.y - 1e-3 < s.y && s.y < ls.y + 1e-3 && ls.z - 1e-3 < s.z && s.z < ls.z + 1e-3);

        // replace
        if (positions_.find(time_stamp) != positions_.end() || is_t_changed || positions_.size() == 0)
        {
            LOG("Change POS =====");
            std::cout << t.x << " " << t.y << " " << t.z << "\n";
            std::cout << lt.x << " " << lt.y << " " << lt.z << "\n";
            positions_[time_stamp] = {t, time_stamp};
        }
        if (rotations_.find(time_stamp) != rotations_.end() || is_r_changed || rotations_.size() == 0)
        {
            LOG("Change ROT");
            std::cout << r.x << " " << r.y << " " << r.z << "\n";
            std::cout << lr.x << " " << lr.y << " " << lr.z << "\n";
            rotations_[time_stamp] = {r, time_stamp};
        }
        if (scales_.find(time_stamp) != scales_.end() || is_s_changed || scales_.size() == 0)
        {
            LOG("Change SCL");
            std::cout << ls.x << " " << ls.y << " " << ls.z << "\n";
            std::cout << s.x << " " << s.y << " " << s.z << "\n";
            scales_[time_stamp] = {s, time_stamp};
        }

        time_set_.insert(time_stamp);
    }
    // TODO: 여러 변환이 적용되었을 경우 revoke 잘 동작하지 않음
    void Bone::replace_or_sub_keyframe(const glm::mat4 &transform, float time)
    {
        float time_stamp = floorf(time) / factor_;
        auto [lt, lr, ls] = DecomposeTransform(get_local_transform(time, factor_));
        auto [t, r, s] = DecomposeTransform(transform);

        bool is_t_changed = !(lt.x - 1e-2 < t.x && t.x < lt.x + 1e-2 && lt.y - 1e-2 < t.y && t.y < lt.y + 1e-2 && lt.z - 1e-2 < t.z && t.z < lt.z + 1e-2);
        bool is_r_changed = !(lr.x - 1e-2 < r.x && r.x < lr.x + 1e-2 && lr.y - 1e-2 < r.y && r.y < lr.y + 1e-2 && lr.z - 1e-2 < r.z && r.z < lr.z + 1e-2);
        bool is_s_changed = !(ls.x - 1e-2 < s.x && s.x < ls.x + 1e-2 && ls.y - 1e-2 < s.y && s.y < ls.y + 1e-2 && ls.z - 1e-2 < s.z && s.z < ls.z + 1e-2);
        auto [ip, ir, is] = std::tuple{positions_.find(time_stamp),
                                       rotations_.find(time_stamp),
                                       scales_.find(time_stamp)};
        // positions
        if (ip != positions_.end() && !is_t_changed)
        {
            LOG(" - - Erase position");
            positions_.erase(ip);
        }
        else if (ip != positions_.end())
        {
            ip->second.position = t;
        }
        LOG("==================");
        std::cout << t.x << " " << t.y << " " << t.z << "\n";
        std::cout << lt.x << " " << lt.y << " " << lt.z << "\n";
        LOG("------------------");

        // rotations
        if (ir != rotations_.end() && !is_r_changed)
        {
            LOG(" - - Erase rotation");
            rotations_.erase(ir);
        }
        else if (ir != rotations_.end())
        {
            ir->second.orientation = r;
        }
        std::cout << r.x << " " << r.y << " " << r.z << "\n";
        std::cout << lr.x << " " << lr.y << " " << lr.z << "\n";
        LOG("------------------");

        // scales
        if (is != scales_.end() && !is_s_changed)
        {
            LOG(" - - Erase scale");
            scales_.erase(is);
        }
        else if (is != scales_.end())
        {
            is->second.scale = s;
        }
        std::cout << s.x << " " << s.y << " " << s.z << "\n";
        std::cout << ls.x << " " << ls.y << " " << ls.z << "\n";
        LOG("==================");

        auto [iip, iir, iis] = std::tuple{positions_.find(time_stamp),
                                          rotations_.find(time_stamp),
                                          scales_.find(time_stamp)};
        glm::vec3 temp_t, temp_s;
        glm::quat tmp_q;
        if (iip != positions_.end())
        {
            temp_t = iip->second.position;
            positions_.erase(iip);
        }
        if (iir != rotations_.end())
        {
            tmp_q = iir->second.orientation;
            rotations_.erase(iir);
        }
        if (iis != scales_.end())
        {
            temp_s = iis->second.scale;
            scales_.erase(iis);
        }
        auto temp_transform = get_local_transform(time, factor_);
        auto [t_t, t_r, t_s] = DecomposeTransform(temp_transform);

        bool is_t_changed2 = !(t_t.x - 1e-2 < t.x && t.x < t_t.x + 1e-2 && t_t.y - 1e-2 < t.y && t.y < t_t.y + 1e-2 && t_t.z - 1e-2 < t.z && t.z < t_t.z + 1e-2);
        bool is_r_changed2 = !(t_r.x - 1e-2 < r.x && r.x < t_r.x + 1e-2 && t_r.y - 1e-2 < r.y && r.y < t_r.y + 1e-2 && t_r.z - 1e-2 < r.z && r.z < t_r.z + 1e-2);
        bool is_s_changed2 = !(t_s.x - 1e-2 < s.x && s.x < t_s.x + 1e-2 && t_s.y - 1e-2 < s.y && s.y < t_s.y + 1e-2 && t_s.z - 1e-2 < s.z && s.z < t_s.z + 1e-2);

        if (is_t_changed2)
        {
            push_position(temp_t, time_stamp);
        }
        if (is_r_changed2)
        {
            push_rotation(tmp_q, time_stamp);
        }
        if (is_s_changed2)
        {
            push_scale(temp_s, time_stamp);
        }

        auto [iiip, iiir, iiis] = std::tuple{positions_.find(time_stamp),
                                             rotations_.find(time_stamp),
                                             scales_.find(time_stamp)};
        if (time_set_.find(time_stamp) != time_set_.end() && iiip == positions_.end() && iiir == rotations_.end() && iiis == scales_.end())
        {
            time_set_.erase(time_stamp);
        }
    }
}