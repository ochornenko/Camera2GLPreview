# Camera2 Preview

Android camera preview application using Camera2 API. Capturing is implemented in Java but rendering
in C++ with OpenGL ES/Vulkan using NDK and JNI.

- Rendering video using GLSL Shaders with OpenGL ES/Vulkan. App starts with OpenGL ES renderer,
  swipe left initially to use Vulkan renderer.
- Realtime camera filters. Processing video frames in GLSL Shaders (OpenGL ES) to apply filters.
  Swipe right to change filter.
- Swipe up to change preview size.
- Double tap to switch camera.

<br />
<div class="centered">
<img src="/screenshots/camera-preview.gif?raw=true" width="400" alt="">
</div>

## License

Copyright © 2018, Oleg Chornenko

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
