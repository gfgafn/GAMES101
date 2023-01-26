# 作业3

## 作业框架描述

相比上次实验，我们对框架进行了如下修改：

1. 我们引入了一个第三方 `.obj` 文件加载库来读取更加复杂的模型文件，这部分库文件在 `OBJ_Loader.h` file. 你无需详细理解它的工作原理，只需知道这个库将会传递给我们一个被命名被 `TriangleList` 的 Vector，其中每个三角形都有对应的点法向量与纹理坐标。此外，与模型相关的纹理也将被一同加载。
**注意：如果你想尝试加载其他模型，你目前只能手动修改模型路径。**
2. 我们引入了一个新的 `Texture` 类以从图片生成纹理，并且提供了查找纹理颜色的接口： `Vector3f getColor(float u, float v)`
3. 我们创建了 `Shader.hpp` 头文件并定义了 `fragment_shader_payload`，其中包括了 Fragment Shader 可能用到的参数。目前 `main.cpp` 中有三个 Fragment Shader，其中 fragment_shader 是按照法向量上色的样例 Shader，其余两个将由你来实现。
4. 主渲染流水线开始于 `rasterizer::draw(std::vector<Triangle> &TriangleList)`.我们再次进行一系列变换，这些变换一般由 Vertex Shader 完成。在此之后，我们调用函数 `rasterize_triangle`.
5. `rasterize_triangle` 函数与你在作业 2 中实现的内容相似。不同之处在于被设定的数值将不再是常数，而是按照 Barycentric Coordinates 对法向量、颜色、纹理颜色与底纹颜色 (Shading Colors) 进行插值。回忆我们上次为了计算 z value 而提供的 [alpha, beta, gamma]，这次你将需要将其应用在其他参数的插值上。你需要做的是计算插值后的颜色，并将 Fragment Shader 计算得到的颜色写入 framebuffer，这要求你首先使用插值得到的结果设置 fragment shader payload，并调用 fragment shader 得到计算结果。
