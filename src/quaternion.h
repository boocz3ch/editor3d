#ifndef QUATERNION
#define QUATERNION

#include "vec3.h"

class quaternion {

public:

  double x, y, z, w;

  double length(quaternion quat) {
    return sqrt(quat.x * quat.x + quat.y * quat.y + quat.z * quat.z + quat.w * quat.w);
  }

  quaternion normalize(quaternion quat) {
    double L = length(quat);

    quat.x /= L;
    quat.y /= L;
    quat.z /= L;
    quat.w /= L;

    return quat;
  }

  static quaternion conjugate(quaternion quat) {
    quat.x = -quat.x;
    quat.y = -quat.y;
    quat.z = -quat.z;
    return quat;
  }

  static quaternion mult(quaternion A, quaternion B) {
    quaternion C;

    C.x = A.w*B.x + A.x*B.w + A.y*B.z - A.z*B.y;
    C.y = A.w*B.y - A.x*B.z + A.y*B.w + A.z*B.x;
    C.z = A.w*B.z + A.x*B.y - A.y*B.x + A.z*B.w;
    C.w = A.w*B.w - A.x*B.x - A.y*B.y - A.z*B.z;

    return C;
  }

  static Vec3 rotate(double Angle, double x_, double y_, double z_, Vec3 View) {
    quaternion temp, quat_view, result;

    double sin_angle_over_2 = sin(Angle/2);
    temp.x = x_ * sin_angle_over_2;
    temp.y = y_ * sin_angle_over_2;
    temp.z = z_ * sin_angle_over_2;
    temp.w = cos(Angle/2);

    quat_view.x = View.x;
    quat_view.y = View.y;
    quat_view.z = View.z;
    quat_view.w = 0;

    result = quaternion::mult(quaternion::mult(temp, quat_view), quaternion::conjugate(temp));

    return Vec3((float) result.x, (float) result.y, (float) result.z);
  }

};

#endif
