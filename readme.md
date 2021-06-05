# Cloud Tracer

## How to use

 1. Compile the cpp files in the `src` folder to an executable (the CMake config
    will work, but use whatever you want)
 2. Create an `output` folder inside of the `dist` folder (Sorry somehow I could
    not get the .gitkeep to work)
 3. Copy any `.xyz` file into the `dist` folder
 4. Create / Adapt a scene file (like `scene1.txt`)
 5. Execute the executable from the `dist` folder passing the name of the scene
    file as command line arg (like `cloud_tracer scene1.txt`)
 6. Open the blender file `viewer.blend` and go to the `scripting` tab
 7. Execute the `ReloadOutput` script in blender
 
## Open Questions
 - does the output look reasonable?
 - what is the exact xyz file interpretation?
 - where is "up" / right handed or left handed?

## Known issues
 - Sample3DTexture crashes for some Seeds or SamplesPerPixel
