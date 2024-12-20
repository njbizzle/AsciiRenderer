# Ascii Renderer (The Terminal Torus)

The whole goal of this project is to parametric surfaces. 

I had the idea after my [ray tracing project](https://github.com/njbizzle/RayTracingInOneWeekend), which followed a book, only ever ended up rendering spheres.

Modern GPUs are built around using triangles, and so are most nearly all graphics frameworks which makes them difficult to use for this. I also didn't want still images in a pixel map like the [ray tracing project](https://github.com/njbizzle/RayTracingInOneWeekend), since I wanted to so some real time rotations. The only way I knew how to truly make my own thing, was with the terminal and text art. So the final product would send out a ton of rays, test the distance, and then choose an appropiraly dense ascii character (not really lighting yet, more just a depth map).

When I first started thinking though the project I got stumpped on finding a way to programmatically solve for the intersection between some surface and a ray. But, when doing some optimization problems for higher dimensional functions in a calculus class, I had an idea.

## Solving for the Intersections

The setup is that I have a ray with a position and normailized direction, and then a parametric surface. So idea was to have a function taking in a ray distance and the inputs to the parametric surface, then returning a 3D vector representing the distance between the ray (given the distance) and the surface (given the coordinates), all of which is continuous, meaning that there are ways to estimate its zeros, which in this case corresponds to the ray intersecting with the surface.

I did some research on exisiting optimization algorimthms and read about some really interesting stuff like the math behind the fsolve algorithm, a method called the powell hybrid method. But I decided for my purposes, multidimensional version of newtons method would work well. So the program sends out a ray for each ascii character (right now theres no perspecitive, it all looks like an isometric drawing).

## Transformations

This drew on a part of my [Vulkan project](https://github.com/njbizzle/VulkanTutorial) that I really loved working with, which was using matricies to represent transofmrations. I just loved the idea of having all the information being stored so compactly in a matrix. I did a lot of the math and thinking around how to scale and translate (using homogenous coordinates since its a linear transformation) in that project, but I really wanted to understand the rotation portion.

## Quaternion Rotations

I read a little bit on quaterions before and understood them on a very high level, but when it came to implementing something, I was very limmited at first. 
