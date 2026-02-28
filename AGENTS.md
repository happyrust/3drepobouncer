# AGENTS.md

## Cursor Cloud specific instructions

### Project Overview

3D Repo Bouncer is a C++ library (with a Node.js worker service) for 3D scene graph management, model import/conversion, and MongoDB database access. It has two main components:

1. **C++ Library + Client** (`bouncer/`, `client/`): CMake-based C++ project
2. **Bouncer Worker** (`tools/bouncer_worker/`): Node.js service that consumes RabbitMQ jobs and spawns the C++ client

### Bouncer Worker (Node.js)

- **Location**: `tools/bouncer_worker/`
- **Package manager**: Yarn (lockfile: `yarn.lock`)
- **Node version**: 22.x (specified in `package.json` engines)
- **Lint**: `yarn lint` (ESLint with airbnb-base config)
- **Run**: `yarn run-worker` (requires RabbitMQ + config.json)
- **Dependencies**: `cd tools/bouncer_worker && yarn install`

### C++ Build

- **Build system**: CMake >= 3.11, C++20
- **Required dependencies**: Boost, MongoDB CXX Driver (v3.11+, must be built with `-DBSONCXX_POLY_USE_STD=ON`), ASSIMP, IfcOpenShell, OpenCASCADE, Eigen, GMP, MPFR
- **Before cmake**: Run `python3 updateSources.py` from repo root to regenerate `CMakeLists.txt` in subdirectories
- **Configure**: `mkdir build && cd build && CC=gcc CXX=g++ cmake -DREPO_ASSET_GENERATOR_SUPPORT=OFF -DREPO_SVG_EXPORT_SUPPORT=OFF -DODA_SUPPORT=OFF -DSYNCHRO_SUPPORT=OFF -DLICENSE_CHECK=OFF ..`
- **Build**: `make -j$(nproc)`
- **Install**: `make install` (set `-DCMAKE_INSTALL_PREFIX=<path>`)
- **Tests**: Add `-DREPO_BUILD_TESTS=ON` to cmake and run `./3drepobouncerTest` after build

### Important Caveats

- The default C++ compiler (`/usr/bin/c++`) may point to clang, which can fail to find standard headers. Always set `CC=gcc CXX=g++` when running cmake.
- The `AssetGenerator` and `SVGExporter` git submodules are **private repos** — clone will fail without GitHub credentials. Disable with `-DREPO_ASSET_GENERATOR_SUPPORT=OFF -DREPO_SVG_EXPORT_SUPPORT=OFF`.
- **MongoDB CXX Driver must be built with `-DBSONCXX_POLY_USE_STD=ON`**. The bouncer code uses `std::optional::has_value()` and `std::string_view` comparisons that are incompatible with the default mnmlstc polyfill. The mnmlstc `core` namespace also conflicts with `repo::core`.
- **IfcOpenShell compatibility**: The public IfcOpenShell v0.8.0 changed `IfcEntityInstanceData` and `IfcGeom::Iterator` APIs. Two compatibility patches are needed in the installed headers:
  1. `Iterator.h`: Add string-based constructors that call `ifcopenshell::geometry::kernels::construct()` (include `hybrid_kernel.h`)
  2. `IfcEntityInstanceData.h`: Add `get_attribute_value(size_t index)` and `toString(ostream&, bool)` convenience overloads
- CI configuration is in `.travis.yml`. The linter job is: `cd tools/bouncer_worker && yarn install && yarn lint`.
- C++ integration tests require MongoDB 8.0 with test data from the private `3drepo/tests` repo.
- To run the bouncer client, set `LD_LIBRARY_PATH` to include the install lib dir and `/usr/local/lib`.
