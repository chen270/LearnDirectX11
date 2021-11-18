#ifndef __CAMERA_H__
#define __CAMERA_H__

//#include <windows.h>
#include <DirectXMath.h>

class Camera
{
public:
    Camera(/* args */) = default;
    ~Camera() = default;

public:
    DirectX::XMMATRIX GetMatrix() const noexcept; //通过参数计算矩阵
    void SpawnControlWindow() noexcept;     //使用 Imgui 控制
    void Reset() noexcept;

private:
    float r = 20.0f;     //离原点的距离

    //theta 和 phi 控制摄像机原点旋转的角度,类似经纬度
    float theta = 0.0f;
    float phi = 0.0f;

    //摄像机旋转
    float pitch = 0.0f;
    float yaw = 0.0f;
    float roll = 0.0f;

};


#endif // __CAMERA_H__