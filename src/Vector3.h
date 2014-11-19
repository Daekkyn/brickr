#ifndef Vector3_H
#define Vector3_H

#include "math.h"
#include <qglobal.h>
#include <iostream>

struct Vector3
{
    float data_[3];

    Vector3()
    {
      data_[0] = 0;
      data_[1] = 0;
      data_[2] = 0;
    }

    float& x() { return data_[0]; }
    float& y() { return data_[1]; }
    float& z() { return data_[2]; }

    float x() const { return data_[0]; }
    float y() const { return data_[1]; }
    float z() const { return data_[2]; }

    Vector3(float x_, float y_, float z_)
    {
      data_[0] = x_;
      data_[1] = y_;
      data_[2] = z_;
    }

    Vector3 operator+(const Vector3 &p) const
    {
        return Vector3(*this) += p;
    }

    Vector3 operator-(const Vector3 &p) const
    {
        return Vector3(*this) -= p;
    }

    Vector3 operator*(float f) const
    {
        return Vector3(*this) *= f;
    }

    Vector3 operator/(float f) const
    {
        return Vector3(*this) /= f;
    }


    Vector3 &operator+=(const Vector3 &p)
    {
        x() += p.x();
        y() += p.y();
        z() += p.z();
        return *this;
    }

    Vector3 &operator-=(const Vector3 &p)
    {
        x() -= p.x();
        y() -= p.y();
        z() -= p.z();
        return *this;
    }

    Vector3 &operator*=(float f)
    {
        x() *= f;
        y() *= f;
        z() *= f;
        return *this;
    }

    Vector3 &operator/=(float f)
    {
        x() /= f;
        y() /= f;
        z() /= f;
        return *this;
    }

    float norm() const
    {
        return sqrt(x() * x() + y() * y() + z() * z());
    }

    float squaredNorm() const
    {
        return x() * x() + y() * y() + z() * z();
    }

    Vector3 normalize() const
    {
        float r = 1. /  norm();
        return Vector3(x() * r, y() * r, z() * r);
    }

    Vector3 min(Vector3 a)
    {
      return Vector3(std::min(x(),a.x()), std::min(y(),a.y()), std::min(z(),a.z()));
    }

    Vector3 max(Vector3 a)
    {
      return Vector3(std::max(x(),a.x()), std::max(y(),a.y()), std::max(z(),a.z()));
    }

    float *data() { return data_; }
    const float *data() const { return data_; }

    float &operator[](unsigned int index) {
        Q_ASSERT(index < 3);
        return data_[index];
    }

    const float &operator[](unsigned int index) const {
        Q_ASSERT(index < 3);
        return data_[index];
    }

    friend std::ostream& operator<< (std::ostream& stream, const Vector3& a)
    {
      stream << a.x() << " " << a.y() << " " << a.z();
      return stream;
    }
};

inline float dot(const Vector3 &a, const Vector3 &b)
{
    return a.x() * b.x() + a.y() * b.y() + a.z() * b.z();
}

inline Vector3 cross(const Vector3 &a, const Vector3 &b)
{
    return Vector3(a.y() * b.z() - a.z() * b.y(),
                   a.z() * b.x() - a.x() * b.z(),
                   a.x() * b.y() - a.y() * b.x());
}


#endif
