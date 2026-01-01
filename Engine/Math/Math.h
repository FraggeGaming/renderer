#ifndef MATH_H
#define MATH_H

#include <string>
#include <cmath>

static constexpr float EPSILON = 1e-8f;

struct Vec3 {
    float x, y, z;

    Vec3();
    Vec3(float x, float y, float z);

    float Magnitude() const;

    Vec3 operator+(const Vec3& v) const;
    Vec3 operator-(const Vec3& v) const;
    Vec3 operator*(float scalar) const;
    Vec3 operator/(float scalar) const;

    Vec3& operator+=(const Vec3& v);
    Vec3& operator-=(const Vec3& v);
    Vec3& operator*=(float scalar);

    void Normalize();
    float Distance(const Vec3& v) const;
    float Dot(const Vec3& v) const;
    Vec3 Cross(const Vec3& v) const;

    std::string Print() const;
};

struct Mat4 {
    float mat[16];

    Mat4();
    explicit Mat4(float value);

    Mat4 operator*(const Mat4& m) const;
    Mat4& operator*=(const Mat4& m);

    static Mat4 Identity();

    void Translate(float x, float y, float z);
    void Scale(float sx, float sy, float sz);
    void RotateX(float a);
    void RotateY(float a);
    void RotateZ(float a);
    std::string Print();
};

#endif // MATH_H
