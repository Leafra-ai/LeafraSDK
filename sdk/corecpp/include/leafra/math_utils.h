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
     * @brief Initialize the math utils
     * @return ResultCode indicating success or failure
     */
    ResultCode initialize();
    
    /**
     * @brief Calculate distance between two 2D points
     * @param p1 First point
     * @param p2 Second point
     * @return Distance between points
     */
    double distance(const Point2D& p1, const Point2D& p2);
    
    /**
     * @brief Calculate distance between two 3D points
     * @param p1 First point
     * @param p2 Second point
     * @return Distance between points
     */
    double distance(const Point3D& p1, const Point3D& p2);
    
    /**
     * @brief Multiply 3x3 matrices
     * @param a First matrix
     * @param b Second matrix
     * @param result Result matrix
     * @return ResultCode indicating success or failure
     */
    ResultCode multiply(const Matrix3x3& a, const Matrix3x3& b, Matrix3x3& result);
    
    /**
     * @brief Calculate determinant of 3x3 matrix
     * @param matrix Input matrix
     * @return Determinant value
     */
    double determinant(const Matrix3x3& matrix);
    
private:
    class Impl;
    unique_ptr<Impl> pImpl;
};

} // namespace leafra 