### BUG-001 — invalid colour name dereferences `unordered_map::end()`

`Colour(const std::string&)` detects a missing colour, initializes magenta, and then unconditionally executes `*this = result->second`. When the lookup fails, `result == end()`, so this dereferences an invalid iterator. This is undefined behaviour and can crash immediately.

Fix branch: `fix/colour-invalid-name`.
Regression test: `tests/test_material.cpp`.

### BUG-002 — OBJ importer accepts invalid indices and corrupt face data

Face indices were converted with `stoi()` and used without bounds checking. Invalid, zero, or out-of-range indices could later cause `mesh->vertices[mesh->indices[i]]` to access outside the vector. Malformed numeric strings also propagated exceptions, while malformed vertex lines could use uninitialized coordinates.

The old importer also stored every polygon index sequentially. A quad such as `f 1 2 3 4` was consequently rendered as triangles `(1,2,3)` and `(4,1,2)` rather than a triangulated quad.

The fix validates positive/negative OBJ indices, rejects zero/out-of-range indices and malformed vertex data, and triangulates polygons using a fan. A local `unique_ptr` also prevents the temporary mesh leaking if parsing throws.

Fix branch: `fix/obj-import-validation`.
Regression test: `tests/test_import3d.cpp`.

### BUG-003 — renderer objects leaked and resize left depth buffers stale

`cRenderer` allocated `Razor3D` and `Hairline` with `new` but did not delete them. `Game` similarly allocated `cRenderer` and did not delete it. The renderer's resize operation resized the colour buffer but did not resize `bufDepth` or `bufIsDrawn`. The depth path is currently disabled, but enabling it after a resize would make those buffers inconsistent with the viewport and can produce out-of-bounds access.

The renderer destructor now releases its owned rasterizers, `Game` releases the renderer, and resize keeps all raster buffers at the new dimensions. Resource pointers are also initialized to null so early SDL failures do not leave indeterminate pointers.

Fix branch: `fix/renderer-lifetime-resize`.

### BUG-004 — triangle visibility test misses triangles crossing the viewport

`isTriangleOnScreen()` only returned true if at least one vertex was inside an enlarged rectangle. A large triangle could have all three vertices outside while its edges/interior crossed the screen. Such triangles were incorrectly culled.

The replacement uses the triangle's screen-space AABB against the viewport. This is conservative: it can retain some triangles whose AABB intersects but whose actual triangle does not, but it does not incorrectly reject triangles solely because all vertices are outside.

Fix branch: `fix/triangle-screen-culling`.
Regression test: `tests/test_general2d.cpp`.

### BUG-005 — `Mesh::setRotationQuat()` did not update rotation state

The function computed a delta from `quatIdentity`, rotated the vertices, but never stored the target quaternion. A second call with the same target therefore applied the same rotation again. The orientation basis vectors also remained stale.

The fix normalizes the target, applies only the delta from the recorded orientation, stores the target, and recalculates the basis vectors.

Fix branch: `fix/mesh-rotation-state`.
Regression test: `tests/test_mesh_rotation.cpp`.

### BUG-006 — empty Stubble files crash in `import()`

The tokenizer can produce an empty vector for an empty file. The parser then called `tokens.front()` without checking `tokens.empty()`. A one-token file also reached `tokens[1]` without checking its size.

The fix explicitly rejects empty token streams and validates the minimum token count before indexing.

Fix branch: `fix/parser-input-validation`.
Regression test: `tests/test_stringy.cpp`.

### BUG-007 — SDL initialization failure leaves a partially usable renderer

The original constructor printed an error and used `system("pause")`, then continued. If SDL or window creation failed, later calls could dereference null SDL objects. The `Game::run()` guard also only checked whether the renderer object itself was null, which could not detect a partially constructed renderer.

The fix removes the shell `pause` path, initializes resource pointers safely, and makes `Game::run()` reject a renderer whose CPU rasterizer objects were not initialized. SDL texture creation itself is still not explicitly checked and should be addressed before treating this area as fully closed.

Fix branch: `fix/renderer-lifetime-resize`.

### BUG-008 — no clipping for triangles crossing the near plane

`Position3d::project()` returns a sentinel for `z <= 0.00001`. `Razor3D::drawTri()` then rejects a triangle if any projected point is the sentinel. A triangle with two vertices in front of the camera and one behind therefore disappears rather than being clipped against the near plane.

This is a deterministic rendering correctness bug. The correct fix is to clip the triangle in camera space against a positive near plane before projection, producing zero, one, or two triangles.

Status: open.

### BUG-009 — camera-space calculations depend on the global `currentScene`

`Position3d::cameraspace()` ignores the scene/camera represented by the caller and reads `currentScene->currentCam`. `renderScene(Scene&)` likewise receives an explicit scene but several downstream calculations still use the global scene. This makes rendering a scene other than the globally active scene incorrect and makes calls before `currentScene` is initialized unsafe.

Status: open. Recommended fix: make camera-space conversion explicitly take a `Camera`/camera transform and remove the global dependency from rendering code.

### BUG-010 — `mixbyfac()` allocates on every gradient update and the caller never frees it

`mixbyfac()` returns `new Colour(...)`. `clearGrad()` dereferences the returned pointer into a local colour but never deletes the allocation. When the gradient factors differ from their endpoints this creates one or two heap allocations leaked per frame.

This is particularly undesirable in a renderer because it creates unbounded process growth over long runs and adds allocator traffic to the frame loop.

Status: open. Recommended fix: return `Colour` by value (the natural C++20 interface) and remove the heap allocation. This changes the function signature, so treat it as an API change under the project's versioning policy.

### BUG-011 — mesh ownership is unclear and imported meshes are leaked

`importObj()` creates a `Mesh` with `new`, stores it in a raw pointer in `Object3D`, and `Scene` stores `Object3D` by value. Neither `Object3D` nor `Scene` owns/deletes the mesh. The normal import path therefore leaks every imported mesh for the lifetime of the process.

The importer fix only prevents a leak while an import is failing; it does not solve successful-import ownership.

Status: open. Recommended fix: make `Object3D::mesh` a `std::unique_ptr<Mesh>` and update construction/access accordingly, or introduce an explicit owning resource container.

### BUG-012 — `Scene::objectByName()` returns a pointer vulnerable to vector reallocation

The function returns a pointer into `Scene::objects`. Any subsequent `emplace_back()` that reallocates the vector invalidates that pointer. Callers retaining the pointer across scene mutation can then use a dangling pointer.

Status: open. Recommended fix: return an index/handle, use stable storage, or document and enforce pointer lifetime rules.

### BUG-013 — colour conversion does not clamp component ranges

`Colour::raw()` converts floating-point components directly to 8-bit values. Callers can construct colours outside `[0,1]`, and shading/multiplication can also generate out-of-range values. Conversion to `uint32_t` then produces implementation-sensitive or wrapped channel values rather than a defined clamp.

Status: open. Recommended fix: clamp each component to `[0,1]` before conversion and define the invariant in the `Colour` API.

### BUG-014 — documented Linux build is not genuinely platform-independent

The GCC presets use `g++` and `windres`, while the main CMake file unconditionally sets the SDL root to the repository's vendored SDL directory, links `external/SDL2/lib/x64`, and copies `external/SDL2/lib/x64/SDL2.dll`. The executable source list also always contains the Windows `.rc` resource.

This conflicts with the README's Linux build instructions and makes the build configuration strongly Windows-specific. It is an integration/build-system bug rather than a runtime security issue.

CMake also contains `set(CMAKE_CXX_COMPILER_FORCED TRUE)`, which is an internal-style override and is unnecessary for a normal native build. CMake's documented compiler configuration is to let compiler detection run or set `CMAKE_CXX_COMPILER` through a preset/toolchain. The CMake documentation describes compiler-forcing mechanisms as deprecated for their original use case.

Status: open.

### BUG-015 — `Position3d` relational operators are not ordinary component-wise ordering

`operator<` and `operator>` require all three components to satisfy the comparison. This is not a conventional lexicographical ordering and does not form a useful strict weak ordering for standard ordered algorithms. They are currently not observed in a critical standard-library ordering path, so this is low severity.

Status: open. Recommended fix: remove them unless component-wise predicates are genuinely intended, or provide explicitly named functions such as `allLess()`.
