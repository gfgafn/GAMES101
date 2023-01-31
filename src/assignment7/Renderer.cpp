//
// Created by goksu on 2/25/20.
//

#include <fstream>
#include <omp.h>
#include "Scene.hpp"
#include "Renderer.hpp"

inline float deg2rad(const float &deg) { return deg * M_PI / 180.0; }

const float EPSILON = 0.00001;

// The main render function. This where we iterate over all pixels in the image,
// generate primary rays and cast these rays into the scene. The content of the
// framebuffer is saved to a file.
void Renderer::Render(const Scene &scene)
{
    std::vector<Vector3f> framebuffer(scene.width * scene.height);

    float scale = tan(deg2rad(scene.fov * 0.5));
    float imageAspectRatio = scene.width / (float)scene.height;
    Vector3f eye_pos(278, 273, -800);

    // change the spp value to change sample ammount
    int spp = 128;
    std::cout << "SPP: " << spp << "\n";

    std::cout << "omp_get_num_procs: " << omp_get_num_procs() << std::endl;

    for (uint32_t j = 0; j < scene.height; ++j)
    {
        size_t indexOfLineHead = j * scene.width;

#pragma omp parallel for num_threads(omp_get_num_procs()) shared(scene, framebuffer, imageAspectRatio, scale, spp, indexOfLineHead)
        for (uint32_t i = 0; i < scene.width; ++i)
        {
            for (int k = 0; k < spp; k++)
            {
                // generate primary ray direction
                float x = (2 * (i + get_random_float()) / (float)scene.width - 1) *
                          imageAspectRatio * scale;
                float y = (1 - 2 * (j + get_random_float()) / (float)scene.height) * scale;

                Vector3f dir = normalize(Vector3f(-x, y, 1));
                framebuffer[indexOfLineHead + i] += scene.castRay(Ray(eye_pos, dir), 0) / spp;
            }
        }
        UpdateProgress(j / (float)scene.height);
    }
    UpdateProgress(1.f);

    // save framebuffer to file
    FILE *fp = fopen("binary.ppm", "wb");
    (void)fprintf(fp, "P6\n%d %d\n255\n", scene.width, scene.height);
    for (auto i = 0; i < scene.height * scene.width; ++i)
    {
        static unsigned char color[3];
        color[0] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].x), 0.6f));
        color[1] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].y), 0.6f));
        color[2] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].z), 0.6f));
        fwrite(color, 1, 3, fp);
    }
    fclose(fp);
}
