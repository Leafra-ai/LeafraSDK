#pragma once

#include "types.h"

namespace leafra {

/**
 * @brief Mathematical utility class
 */
class LEAFRA_API MathUtils {
public:
    MathUtils();
    ~MathUtils();
    
    /**
     * @brief Calculate distance between two 2D points
     * @param p1 First point
     * @param p2 Second point
     * @return Distance between points
     */
    double calculate_distance_2d(const Point2D& p1, const Point2D& p2);
    
    /**
     * @brief Calculate distance between two 3D points
     * @param p1 First point
     * @param p2 Second point
     * @return Distance between points
     */
    double calculate_distance_3d(const Point3D& p1, const Point3D& p2);
    
    /**
     * @brief Multiply 3x3 matrices
     * @param a First matrix
     * @param b Second matrix
     * @param result Result matrix
     * @return ResultCode indicating success or failure
     */
    ResultCode multiply_matrix_3x3(const Matrix3x3& a, const Matrix3x3& b, Matrix3x3& result);
    
    /**
     * @brief Calculate determinant of 3x3 matrix
     * @param matrix Input matrix
     * @return Determinant value
     */
    double matrix_determinant(const Matrix3x3& matrix);
    
    /**
     * @brief Invert a 3x3 matrix
     * @param input Input matrix
     * @param result Result matrix
     * @return ResultCode indicating success or failure
     */
    ResultCode invert_matrix_3x3(const Matrix3x3& input, Matrix3x3& result);
    
    /**
     * @brief Rotate a 2D point by angle
     * @param point Point to rotate
     * @param angle_radians Rotation angle in radians
     * @return Rotated point
     */
    Point2D rotate_point_2d(const Point2D& point, double angle_radians);
    
    /**
     * @brief Linear interpolation between two 2D points
     * @param p1 First point
     * @param p2 Second point
     * @param t Interpolation factor (0.0 to 1.0)
     * @return Interpolated point
     */
    Point2D lerp_2d(const Point2D& p1, const Point2D& p2, double t);
    
    /**
     * @brief Linear interpolation between two 3D points
     * @param p1 First point
     * @param p2 Second point
     * @param t Interpolation factor (0.0 to 1.0)
     * @return Interpolated point
     */
    Point3D lerp_3d(const Point3D& p1, const Point3D& p2, double t);
    
    /**
     * @brief Clamp value between min and max
     * @param value Value to clamp
     * @param min_val Minimum value
     * @param max_val Maximum value
     * @return Clamped value
     */
    double clamp(double value, double min_val, double max_val);
    
    /**
     * @brief Convert degrees to radians
     * @param degrees Angle in degrees
     * @return Angle in radians
     */
    double degrees_to_radians(double degrees);
    
    /**
     * @brief Convert radians to degrees
     * @param radians Angle in radians
     * @return Angle in degrees
     */
    double radians_to_degrees(double radians);
};

} // namespace leafra 