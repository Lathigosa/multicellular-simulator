#ifndef MATH_INCLUDED_H
#define MATH_INCLUDED_H

#include <math.h>

namespace math
{

    template <typename precision>
    struct vec3
    {
        precision x;
        precision y;
        precision z;

        precision length();

        vec3<precision>& operator+=( const vec3<precision>& right_side )
        {
            x += right_side.x;
            y += right_side.y;
            z += right_side.z;
            return *this;
        }

        vec3<precision>& operator-=( const vec3<precision>& right_side )
        {
            x -= right_side.x;
            y -= right_side.y;
            z -= right_side.z;
            return *this;
        }
    };

    template <typename precision>
    precision vec3<precision>::length(){
        return sqrt(x * x + y * y + z * z);
    }

    template <typename precision>
    vec3<precision> normalize(const vec3<precision>& input){
        register vec3<precision> result;
        register precision length = sqrt(input.x * input.x + input.y * input.y + input.z * input.z);
        result.x = input.x/length;
        result.y = input.y/length;
        result.z = input.z/length;
        return result;
    }

    template <typename precision>
    vec3<precision> normalize(const vec3<precision>& input, const precision& length){
        register vec3<precision> result;
        result.x = input.x/length;
        result.y = input.y/length;
        result.z = input.z/length;
        return result;
    }

    template <typename precision>
    vec3<precision> cross_product(const vec3<precision>& inputA, const vec3<precision>& inputB){
        register vec3<precision> result;
        result.x = inputA.y * inputB.z - inputB.y * inputA.z;
        result.y = inputA.z * inputB.x - inputB.z * inputA.x;
        result.z = inputA.x * inputB.y - inputB.x * inputA.y;
        return result;
    }

    template <typename precision>
    precision dot_product(const vec3<precision>& inputA, const vec3<precision>& inputB){
        return inputA.x * inputB.x + inputA.y * inputB.y + inputA.z * inputB.z;
    }

    typedef vec3<float> vec3f;
    typedef vec3<double> vec3d;
    typedef vec3<long double> vec3l;
}

#endif // MATH_INCLUDED_H

