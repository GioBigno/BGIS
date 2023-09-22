# BGIS
A cross-platform app for viewing shapefiles made with Qt.

![output](https://github.com/GioBigno/BGIS/assets/90970339/e3c339c7-2a3e-4bfd-8a65-877fe5496a02)

## Features
- visualization and selection of shapes
- different fill colors
- hole support
- Fast rendering via GPU
- cross-platform

## Build and install
### dependencies
to build BGIS you must have installed:
- [Qt 6.5](https://doc.qt.io/qt-6/get-and-install-qt.html)
- [geos](https://github.com/libgeos/geos)
- [shapelib](https://github.com/OSGeo/shapelib)
### build with cmake
```
mkdir build && cd build
cmake ..
cmake --build .
cmake --install .
```
if you have installed the dependency libraries in a non-standard path you must specify the path before building with:
```
cmake .. -DQT_PATH:PATH=<your_path_to_qt>
cmake .. -DGEOS_PATH:PATH=<your_path_to_geos>
cmake .. -DSHAPELIB_PATH:PATH=<your_path_to_shapelib>
```
if you want to specify the installation path:
```
cmake .. -DCMAKE_INSTALL_PREFIX:PATH=<BGIS_install_location>
```
