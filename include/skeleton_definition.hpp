#pragma once

#include <vector>
#include <array>
#include <string>
#include <unordered_map>
#include <cmath>

namespace pose {

/**
 * Joint indices for HPE (Human Pose Estimation) model
 * Custom 18-joint skeleton from HPE replay dataset
 */
enum class JointIndex : uint16_t {
    NOSE = 0,       // /NOS_
    NECK = 1,       // /NEC_
    LEFT_SHOULDER = 2,   // /SHOL
    RIGHT_SHOULDER = 3,  // /SHOR
    LEFT_ELBOW = 4,      // /ELBL
    RIGHT_ELBOW = 5,     // /ELBR
    LEFT_WRIST = 6,      // /WRIL
    RIGHT_WRIST = 7,     // /WRIR
    LEFT_HIP = 8,        // /HIPL
    RIGHT_HIP = 9,       // /HIPR
    LEFT_KNEE = 10,      // /KNEL
    RIGHT_KNEE = 11,     // /KNER
    LEFT_ANKLE = 12,     // /ANKL
    RIGHT_ANKLE = 13,    // /ANKR
    LEFT_EYE = 14,       // /EYEL
    RIGHT_EYE = 15,      // /EYER
    LEFT_EAR = 16,       // /EARL
    RIGHT_EAR = 17,      // /EARR
    COUNT = 18  // Total number of joints
};

/**
 * Defines a bone (connection) between two joints
 */
struct Bone {
    uint16_t start_joint;
    uint16_t end_joint;
    std::string name;
    
    Bone() = default;
    Bone(uint16_t start, uint16_t end, const std::string& bone_name)
        : start_joint(start), end_joint(end), name(bone_name) {}
};

/**
 * Container for skeleton structure definition
 * Defines which joints are connected by bones
 */
class SkeletonDefinition {
public:
    SkeletonDefinition() = default;
    
    /**
     * HPE 18-joint skeleton from HPE replay dataset
     * Includes all major body joints with facial landmarks
     */
    static SkeletonDefinition hpe_18() {
        SkeletonDefinition skeleton;
        skeleton.set_num_joints(18);
        
        // Head connections
        skeleton.add_bone(static_cast<uint16_t>(JointIndex::NOSE), 
                         static_cast<uint16_t>(JointIndex::LEFT_EYE), 
                         "nose_to_left_eye");
        skeleton.add_bone(static_cast<uint16_t>(JointIndex::NOSE), 
                         static_cast<uint16_t>(JointIndex::RIGHT_EYE), 
                         "nose_to_right_eye");
        skeleton.add_bone(static_cast<uint16_t>(JointIndex::LEFT_EYE), 
                         static_cast<uint16_t>(JointIndex::LEFT_EAR), 
                         "left_eye_to_left_ear");
        skeleton.add_bone(static_cast<uint16_t>(JointIndex::RIGHT_EYE), 
                         static_cast<uint16_t>(JointIndex::RIGHT_EAR), 
                         "right_eye_to_right_ear");
        
        // Nose to neck
        skeleton.add_bone(static_cast<uint16_t>(JointIndex::NOSE), 
                         static_cast<uint16_t>(JointIndex::NECK), 
                         "nose_to_neck");
        
        // Neck to shoulders
        skeleton.add_bone(static_cast<uint16_t>(JointIndex::NECK), 
                         static_cast<uint16_t>(JointIndex::LEFT_SHOULDER), 
                         "neck_to_left_shoulder");
        skeleton.add_bone(static_cast<uint16_t>(JointIndex::NECK), 
                         static_cast<uint16_t>(JointIndex::RIGHT_SHOULDER), 
                         "neck_to_right_shoulder");
        
        // Shoulder-to-shoulder
        skeleton.add_bone(static_cast<uint16_t>(JointIndex::LEFT_SHOULDER), 
                         static_cast<uint16_t>(JointIndex::RIGHT_SHOULDER), 
                         "left_shoulder_to_right_shoulder");
        
        // Left arm
        skeleton.add_bone(static_cast<uint16_t>(JointIndex::LEFT_SHOULDER), 
                         static_cast<uint16_t>(JointIndex::LEFT_ELBOW), 
                         "left_shoulder_to_left_elbow");
        skeleton.add_bone(static_cast<uint16_t>(JointIndex::LEFT_ELBOW), 
                         static_cast<uint16_t>(JointIndex::LEFT_WRIST), 
                         "left_elbow_to_left_wrist");
        
        // Right arm
        skeleton.add_bone(static_cast<uint16_t>(JointIndex::RIGHT_SHOULDER), 
                         static_cast<uint16_t>(JointIndex::RIGHT_ELBOW), 
                         "right_shoulder_to_right_elbow");
        skeleton.add_bone(static_cast<uint16_t>(JointIndex::RIGHT_ELBOW), 
                         static_cast<uint16_t>(JointIndex::RIGHT_WRIST), 
                         "right_elbow_to_right_wrist");
        
        // Torso
        skeleton.add_bone(static_cast<uint16_t>(JointIndex::LEFT_SHOULDER), 
                         static_cast<uint16_t>(JointIndex::LEFT_HIP), 
                         "left_shoulder_to_left_hip");
        skeleton.add_bone(static_cast<uint16_t>(JointIndex::RIGHT_SHOULDER), 
                         static_cast<uint16_t>(JointIndex::RIGHT_HIP), 
                         "right_shoulder_to_right_hip");
        skeleton.add_bone(static_cast<uint16_t>(JointIndex::LEFT_HIP), 
                         static_cast<uint16_t>(JointIndex::RIGHT_HIP), 
                         "left_hip_to_right_hip");
        
        // Left leg
        skeleton.add_bone(static_cast<uint16_t>(JointIndex::LEFT_HIP), 
                         static_cast<uint16_t>(JointIndex::LEFT_KNEE), 
                         "left_hip_to_left_knee");
        skeleton.add_bone(static_cast<uint16_t>(JointIndex::LEFT_KNEE), 
                         static_cast<uint16_t>(JointIndex::LEFT_ANKLE), 
                         "left_knee_to_left_ankle");
        
        // Right leg
        skeleton.add_bone(static_cast<uint16_t>(JointIndex::RIGHT_HIP), 
                         static_cast<uint16_t>(JointIndex::RIGHT_KNEE), 
                         "right_hip_to_right_knee");
        skeleton.add_bone(static_cast<uint16_t>(JointIndex::RIGHT_KNEE), 
                         static_cast<uint16_t>(JointIndex::RIGHT_ANKLE), 
                         "right_knee_to_right_ankle");
        
        return skeleton;
    }
    
    /**
     * Add a bone connection between two joints
     */
    void add_bone(uint16_t start, uint16_t end, const std::string& name) {
        if (start < _num_joints && end < _num_joints) {
            _bones.emplace_back(start, end, name);
        }
    }
    
    /**
     * Get all bones in this skeleton
     */
    const std::vector<Bone>& bones() const { 
        return _bones; 
    }
    
    /**
     * Get number of joints in this skeleton
     */
    size_t num_joints() const { 
        return _num_joints; 
    }
    
    /**
     * Set the number of joints
     */
    void set_num_joints(size_t n) { 
        _num_joints = n; 
    }
    
    /**
     * Clear all bones
     */
    void clear_bones() {
        _bones.clear();
    }
    
    /**
     * Get bone count
     */
    size_t bone_count() const {
        return _bones.size();
    }

private:
    std::vector<Bone> _bones;
    size_t _num_joints = 18;  // Default HPE 18-joint
};

/**
 * Utility functions for pose processing
 */
namespace utils {

/**
 * Calculate distance between two 3D points
 */
inline double distance_3d(const std::array<double, 3>& p1, 
                         const std::array<double, 3>& p2) {
    double dx = p1[0] - p2[0];
    double dy = p1[1] - p2[1];
    double dz = p1[2] - p2[2];
    return std::sqrt(dx*dx + dy*dy + dz*dz);
}

/**
 * Calculate angle between three joints (in radians)
 */
inline double joint_angle(const std::array<double, 3>& p1,
                         const std::array<double, 3>& center,
                         const std::array<double, 3>& p2) {
    // Vector from center to p1
    std::array<double, 3> v1 = {p1[0] - center[0], p1[1] - center[1], p1[2] - center[2]};
    // Vector from center to p2
    std::array<double, 3> v2 = {p2[0] - center[0], p2[1] - center[1], p2[2] - center[2]};
    
    // Dot product
    double dot = v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
    
    // Magnitudes
    double mag1 = std::sqrt(v1[0]*v1[0] + v1[1]*v1[1] + v1[2]*v1[2]);
    double mag2 = std::sqrt(v2[0]*v2[0] + v2[1]*v2[1] + v2[2]*v2[2]);
    
    if (mag1 < 1e-10 || mag2 < 1e-10) return 0.0;
    
    double cos_angle = dot / (mag1 * mag2);
    // Clamp to [-1, 1] to avoid numerical errors
    cos_angle = std::max(-1.0, std::min(1.0, cos_angle));
    return std::acos(cos_angle);
}

/**
 * Check if a joint has sufficient confidence
 */
inline bool is_joint_valid(double confidence, double min_confidence = 0.5) {
    return confidence >= min_confidence;
}

} // namespace utils

} // namespace pose
