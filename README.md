# Ferrite

## To Do
These tasks are (roughly) in order. They will be split between several PRs, and updated regularly.

* Styling
  * Add `.clang-format`
  * Format all files
  * Change namespaces
    * Add private namespaces to all main modules (`Ferrite::_MODULE`)
    * Export all important things (`Component`, `Object`, etc.) to the `Ferrite` namespace
* Optimization
  * Reduce the memory footprint of `Object`s
* Finish Math module
  * Ensure `Vector3` and `Vector2` math is implemented and correct
  * Implement `Matrix3x3`, `Matrix4x4`, and `Quaternion`
* Finish Rendering Module
  * Create window with GLFW
  * Create generic render API
  * Support OpenGL 4.6
  * Support OpenGL 4.1
  * Support OpenGL 3.3
  * Support Vulkan
* Long-term goals (may or may not make it into 1.0 release)
  * General
    * Support Python
    * Support C#
    * Support Java
  * Rendering
    * Support OpenGL ES
    * Support WebGL
    * Support Metal
    * Support DirectX11
    * Support DirectX12

## Contributors

- worksteve

## License

[BSD 3-Clause](LICENSE.md)
