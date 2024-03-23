# Expert-fiesta
Project made for the Computer Graphics course competition, built upon the second OPENGL assignment. It is a basic procedural generated terrain visualizer having multiple shading modes. 

## Videos
Below you can find a short video of how each shading mode looks like.

https://github.com/Serbbi/expert-fiesta/assets/92867933/20d6a4e1-2ed5-4149-ad1f-3e221b89911d



https://github.com/Serbbi/expert-fiesta/assets/92867933/fa330f53-7511-4207-8bc0-c985d1c5ad60



https://github.com/Serbbi/expert-fiesta/assets/92867933/d3e8b29e-58d5-4076-8e0f-fce55c9a214b

## Images
Some higher quality images.
![image](https://github.com/Serbbi/expert-fiesta/assets/92867933/8e3b15aa-9956-42a8-8f29-dcc6bf17142a)

![image](https://github.com/Serbbi/expert-fiesta/assets/92867933/25792553-582c-4930-9f9f-b499537afa23)

![image](https://github.com/Serbbi/expert-fiesta/assets/92867933/b5243d45-4c4f-4721-8f9f-158e9ccde238)


## Technical details
The terrain consists of a big square mesh made up of little triangles. The mesh model has no height variation, it is just a flat surface. The height of the vertices is defined by the the value in a perlin noise image. How that works is basically we take the coordinates of the vertex and seek the rgb value in the image, and depending on how big is it, we modify the y coordinate of the respective vertex. To give the impression of flying over the terrain, we have an offset variable which is incremented every frame. When we take the rgb value from the image we just need to add this offset to one of the coordinates. By doing this, it seems like the terrain is moving towards us. The mesh is rendered using GL_LINE which draws only the lines of triangles and does not fill them. Since the terrain is quite big and has many vertices, I used indexed rendering to improve the efficiency of the program.

The background and the little rocket ship are just square meshes with textures on them. To not render the background of the rocket ship, we just check the alpha value of the image in the fragment shader and if it is below a certain threshold, we just discard the calculation. The rocket ship moves up and down based on a similiar process on how the terrain gets its height.

The **color changing shader** works like this: we have a hue value which is incremented each draw cycle and then it is fed through a hsv to rgb function to get the color. Then all the vertices are drawn with the that color.

The **rainbow layers shader** assigns a preset color from the vertex shader based on its height. That's it.

With the **custom shader** you can choose your own colors to change the apperaance of the terrain. You have 3 choices of colors, one of them will color the high y values of the mesh, one will color the low y values and middle slider will define the middle color, although it is not really that vissible, it can add a smoother gradient between the top and bottom color.

## Future work
- [ ] Figure out how to render the terrain using triangle strips in order to boost the frames.
- [ ] Refactor the code as it is very messy
- [x] Make dad proud

## Author
Me, Serban
