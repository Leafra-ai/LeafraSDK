#include "leafra/math_utils.h"
#include <cmath>
#include <stdexcept>

namespace leafra {

MathUtils::MathUtils() = default;

MathUtils::~MathUtils() = default;

double MathUtils::calculate_distance_2d(const Point2D& p1, const Point2D& p2) {
    double dx = p2.x - p1.x;
    double dy = p2.y - p1.y;
    return std::sqrt(dx * dx + dy * dy);
}

double MathUtils::calculate_distance_3d(const Point3D& p1, const Point3D& p2) {
    double dx = p2.x - p1.x;
    double dy = p2.y - p1.y;
    double dz = p2.z - p1.z;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

ResultCode MathUtils::multiply_matrix_3x3(const Matrix3x3& a, const Matrix3x3& b, Matrix3x3& result) {
    try {
        // Initialize result matrix to zero
        for (int i = 0; i < 9; ++i) {
            result.data[i] = 0.0;
        }
        
        // Perform matrix multiplication
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                for (int k = 0; k < 3; ++k) {
                    result(i, j) += a(i, k) * b(k, j);
                }
            }
        }
        
        return ResultCode::SUCCESS;
        
    } catch (const std::exception&) {
        return ResultCode::ERROR_PROCESSING_FAILED;
    }
}

double MathUtils::matrix_determinant(const Matrix3x3& matrix) {
    // Calculate determinant of 3x3 matrix using the rule of Sarrus
    double det = matrix(0, 0) * (matrix(1, 1) * matrix(2, 2) - matrix(1, 2) * matrix(2, 1))
               - matrix(0, 1) * (matrix(1, 0) * matrix(2, 2) - matrix(1, 2) * matrix(2, 0))
               + matrix(0, 2) * (matrix(1, 0) * matrix(2, 1) - matrix(1, 1) * matrix(2, 0));
    
    return det;
}

ResultCode MathUtils::invert_matrix_3x3(const Matrix3x3& input, Matrix3x3& result) {
    try {
        double det = matrix_determinant(input);
        
        // Check if matrix is invertible
        if (std::abs(det) < 1e-10) {
            return ResultCode::ERROR_INVALID_PARAMETER; // Matrix is singular
        }
        
        double inv_det = 1.0 / det;
        
        // Calculate adjugate matrix and multiply by 1/determinant
        result(0, 0) = (input(1, 1) * input(2, 2) - input(1, 2) * input(2, 1)) * inv_det;
        result(0, 1) = (input(0, 2) * input(2, 1) - input(0, 1) * input(2, 2)) * inv_det;
        result(0, 2) = (input(0, 1) * input(1, 2) - input(0, 2) * input(1, 1)) * inv_det;
        
        result(1, 0) = (input(1, 2) * input(2, 0) - input(1, 0) * input(2, 2)) * inv_det;
        result(1, 1) = (input(0, 0) * input(2, 2) - input(0, 2) * input(2, 0)) * inv_det;
        result(1, 2) = (input(0, 2) * input(1, 0) - input(0, 0) * input(1, 2)) * inv_det;
        
        result(2, 0) = (input(1, 0) * input(2, 1) - input(1, 1) * input(2, 0)) * inv_det;
        result(2, 1) = (input(0, 1) * input(2, 0) - input(0, 0) * input(2, 1)) * inv_det;
        result(2, 2) = (input(0, 0) * input(1, 1) - input(0, 1) * input(1, 0)) * inv_det;
        
        return ResultCode::SUCCESS;
        
    } catch (const std::exception&) {
        return ResultCode::ERROR_PROCESSING_FAILED;
    }
}

Point2D MathUtils::rotate_point_2d(const Point2D& point, double angle_radians) {
    double cos_angle = std::cos(angle_radians);
    double sin_angle = std::sin(angle_radians);
    
    Point2D result;
    result.x = point.x * cos_angle - point.y * sin_angle;
    result.y = point.x * sin_angle + point.y * cos_angle;
    
    return result;
}

Point2D MathUtils::lerp_2d(const Point2D& p1, const Point2D& p2, double t) {
    // Clamp t to [0, 1]
    t = std::max(0.0, std::min(1.0, t));
    
    Point2D result;
    result.x = p1.x + t * (p2.x - p1.x);
    result.y = p1.y + t * (p2.y - p1.y);
    
    return result;
}

Point3D MathUtils::lerp_3d(const Point3D& p1, const Point3D& p2, double t) {
    // Clamp t to [0, 1]
    t = std::max(0.0, std::min(1.0, t));
    
    Point3D result;
    result.x = p1.x + t * (p2.x - p1.x);
    result.y = p1.y + t * (p2.y - p1.y);
    result.z = p1.z + t * (p2.z - p1.z);
    
    return result;
}

double MathUtils::clamp(double value, double min_val, double max_val) {
    return std::max(min_val, std::min(value, max_val));
}

double MathUtils::degrees_to_radians(double degrees) {
    return degrees * M_PI / 180.0;
}

double MathUtils::radians_to_degrees(double radians) {
    return radians * 180.0 / M_PI;
}

} // namespace leafra 