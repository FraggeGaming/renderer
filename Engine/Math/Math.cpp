#include <string>
#include <cmath>
#include "Math.h"


Vec3::Vec3(){
    x = 0;
    y = 0;
    z = 0;
}

Vec3::Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

float Vec3::Magnitude() const {
    float square_sum = x*x + y*y + z*z;
    if (square_sum > EPSILON)
        return std::sqrt(square_sum);
    return 0;
}


Vec3 Vec3::operator+(const Vec3& v) const{
    return Vec3(x+v.x, y+v.y, z+v.z);
}

Vec3 Vec3::operator-(const Vec3& v) const{
    return Vec3(x-v.x, y-v.y, z-v.z);
}

Vec3 Vec3::operator*(float scalar) const{
    return Vec3(x*scalar, y*scalar, z*scalar);
}

Vec3 Vec3::operator/(float scalar) const{
    return Vec3(x/scalar, y/scalar, z/scalar);
}

Vec3& Vec3::operator+=(const Vec3& v) {
    x += v.x;
    y += v.y;
    z += v.z;

    return *this;
}

Vec3& Vec3::operator-=(const Vec3& v) {
    x -= v.x;
    y -= v.y;
    z -= v.z;

    return *this;
}

Vec3& Vec3::operator*=(float scalar) {
    x *= scalar;
    y *= scalar;
    z *= scalar;

    return *this;
}

void Vec3::Normalize() {
    float mag = Magnitude();

    if(mag > EPSILON){
        float invMag = 1.0f / mag;
        (*this) *= invMag;
    }
    
}

float Vec3::Distance(const Vec3& v) const{
    return std::sqrt((x-v.x)*(x-v.x) + (y-v.y)*(y-v.y) +(z-v.z)*(z-v.z));
}

float Vec3::Dot(const Vec3& v) const{
    return x*v.x + y*v.y + z*v.z;
}

Vec3 Vec3::Cross(const Vec3& v) const{
    return Vec3(y*v.z - v.y*z, z*v.x - v.z*x, x*v.y - v.x*y);
}

std::string Vec3::Print() const{
    return "X: " + std::to_string(x) + ", Y: " + std::to_string(y) + ", Z: " + std::to_string(z);
}





Mat4::Mat4() : mat{0.0f} {}


Mat4::Mat4(float value){
    for (int i = 0; i < 16; i++){
        mat[i] = value;
    }
}

Mat4 Mat4::operator*(const Mat4& m) const{
    Mat4 res;
    for (int col = 0; col < 4; ++col){
        
        
        for (int row = 0; row < 4; ++row){
            //column by row dot product added together
            
            float cross_sum =
            mat[0 * 4 + row] * m.mat[col * 4 + 0] + 
            mat[1 * 4 + row] * m.mat[col * 4 + 1] + 
            mat[2 * 4 + row] * m.mat[col * 4 + 2] + 
            mat[3 * 4 + row] * m.mat[col * 4 + 3];

            res.mat[col*4 + row] = cross_sum;
        }
    }

    return res;
}

Mat4& Mat4::operator*=(const Mat4& m){
    *this = *this*m;
    return *this;
}

Mat4 Mat4::Identity(){
    Mat4 res;
    res.mat[0] = 1.0f;
    res.mat[5] = 1.0f;
    res.mat[10] = 1.0f;
    res.mat[15] = 1.0f;
    return res;
}

void Mat4::Translate(float x, float y, float z){
    Mat4 I = Mat4::Identity();

    I.mat[12] = x;
    I.mat[13] = y;
    I.mat[14] = z;

    *this  = I * (*this);
}

void Mat4::Scale(float sx, float sy, float sz){
    Mat4 I = Mat4::Identity();

    I.mat[0] = sx;
    I.mat[5] = sy;
    I.mat[10] = sz;

    *this *= I;
}

void Mat4::RotateZ(float a){
    Mat4 I = Mat4::Identity();

    I.mat[0] = std::cos(a);
    I.mat[1] = std::sin(a);
    I.mat[4] = -std::sin(a);
    I.mat[5] = std::cos(a);

    *this*= I;
}

void Mat4::RotateY(float a){
    Mat4 I = Mat4::Identity();

    I.mat[0] = std::cos(a);
    I.mat[2] = std::sin(a);
    I.mat[8] = -std::sin(a);
    I.mat[10] = std::cos(a);
    
    *this*= I;
}

void Mat4::RotateX(float a){
    Mat4 I = Mat4::Identity();

    I.mat[5] = std::cos(a);
    I.mat[6] = std::sin(a);
    I.mat[9] = -std::sin(a);
    I.mat[10] = std::cos(a);
    
    *this*= I;
}

std::string Mat4::Print(){
    std::string s;
    for(int i = 0; i < 4; i++){
        s.append(std::to_string(mat[0*4+i]) + ", ");
        s.append(std::to_string(mat[1*4+i]) + ", ");
        s.append(std::to_string(mat[2*4+i]) + ", ");
        s.append(std::to_string(mat[3*4+i]) + "\n");
    }

    return s;
}