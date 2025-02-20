# MRHelper

установка:
1. Клонируйте репозиторий
2. Перейдите с помощью консоли в скаченную папку
3. Выполните следующий код:
```console
mkdir build
cd build
cmake ..
cmake --build . --config Release --target install
```
4. В CMakeLists.txt проекта добавьте:
```cmake
find_package(MRHelper)
target_link_libraries(${PROJECT_NAME} PUBLIC MRHelper)
``` 