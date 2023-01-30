//
// Created by Göksu Güvendiren on 2019-05-14.
//

#include "Scene.hpp"

void Scene::buildBVH()
{
    printf(" - Generating BVH...\n\n");
    this->bvh = new BVHAccel(objects, 1, BVHAccel::SplitMethod::NAIVE);
}

Intersection Scene::intersect(const Ray &ray) const
{
    return this->bvh->Intersect(ray);
}

void Scene::sampleLight(Intersection &pos, float &pdf) const
{
    float emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k)
    {
        if (objects[k]->hasEmit())
        {
            emit_area_sum += objects[k]->getArea();
        }
    }
    float p = get_random_float() * emit_area_sum;
    emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k)
    {
        if (objects[k]->hasEmit())
        {
            emit_area_sum += objects[k]->getArea();
            if (p <= emit_area_sum)
            {
                objects[k]->Sample(pos, pdf);
                break;
            }
        }
    }
}

bool Scene::trace(
    const Ray &ray,
    const std::vector<Object *> &objects,
    float &tNear, uint32_t &index, Object **hitObject)
{
    *hitObject = nullptr;
    for (uint32_t k = 0; k < objects.size(); ++k)
    {
        float tNearK = kInfinity;
        uint32_t indexK;
        Vector2f uvK;
        if (objects[k]->intersect(ray, tNearK, indexK) && tNearK < tNear)
        {
            *hitObject = objects[k];
            tNear = tNearK;
            index = indexK;
        }
    }

    return (*hitObject != nullptr);
}

// Implementation of Path Tracing
Vector3f Scene::castRay(const Ray &ray, int depth) const
{
    // TODO Implement Path Tracing Algorithm here

    static const float EPSILON = 0.0001;

    // 光线与场景中的物体是否相交
    Intersection interWithObject = Scene::intersect(ray);
    if (!interWithObject.happened) // 光线与场景中的物体都不相交，返回背景颜色
        return Scene::backgroundColor;
    if (interWithObject.m->hasEmission()) // 打到光源，返回光源颜色
        return interWithObject.m->getEmission();

    // 直接光照
    Vector3f l_dir(0, 0, 0);
    Intersection intersectWithEmittingObj;
    float pdfLight = 0.0f;
    sampleLight(intersectWithEmittingObj, pdfLight); // 采样得到当前着色点发出的光线与自发光物体（光源）的相交信息与 pdf
    Vector3f objToLight = intersectWithEmittingObj.coords - interWithObject.coords;
    Vector3f objToLightDir = objToLight.normalized();
    // 当前着色点与照亮当前着色点的自发光物体（光源）间距离的平方
    float distanceSquared = dotProduct(objToLight, objToLight);
    // 当前着色点处的法向量
    Vector3f n = interWithObject.normal.normalized();
    // 照亮当前着色点的自发光物体（光源）的法向量
    Vector3f nn = intersectWithEmittingObj.normal.normalized();

    // 物体上的点与光源间有无遮挡? 无遮挡，计算光源的直接光照贡献；有遮挡，跳过
    Intersection check = Scene::intersect(Ray(interWithObject.coords, objToLightDir));
    if (EPSILON > std::sqrt(distanceSquared) - check.distance) // 物体上的点与光源间无遮挡
    {
        l_dir = intersectWithEmittingObj.emit *
                interWithObject.m->eval(ray.direction, objToLightDir, n) *
                std::max(0.0f, dotProduct(objToLightDir, n)) *
                std::max(0.0f, dotProduct(-objToLightDir, nn)) / distanceSquared / pdfLight;
    }

    // 间接光照
    Vector3f l_indir(0, 0, 0);
    // 以 Scene::RussianRoulette 的概率计算其他非光源物体间接光照对当前着色点的着色贡献，
    // 以 1 - Scene::RussianRoulette 的概率不计算该贡献, 解决无限递归问题
    if (get_random_float() > Scene::RussianRoulette)
        return l_dir;

    Vector3f objToObjDir = interWithObject.m->sample(ray.direction, n).normalized();
    Ray objToNonEmittingObjRay(interWithObject.coords, objToObjDir);
    Intersection intersectWithOtherObj = Scene::intersect(objToNonEmittingObjRay);
    if (intersectWithOtherObj.happened && !intersectWithOtherObj.m->hasEmission()) // 击中了不是光源的物体
    {
        float pdf = interWithObject.m->pdf(ray.direction, objToObjDir, n);
        if (pdf > EPSILON) // 解决 pdf 接近于 0 使得下面的 l_indir 为无穷大在图像上表现为白色像素点的问题
        {
            l_indir = castRay(objToNonEmittingObjRay, depth + 1) *
                      interWithObject.m->eval(ray.direction, objToObjDir, n) *
                      std::max(0.0f, dotProduct(objToObjDir, n)) / pdf / Scene::RussianRoulette;
        }
    }

    return l_dir + l_indir;
}