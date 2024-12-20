cmake_minimum_required(VERSION 3.10)
project(MyProject)

set(CMAKE_CXX_STANDARD 11)

# Добавление исполняемого файла
add_executable(MyExecutable main.cpp)

# Установка целевого каталога для установки
set(CMAKE_INSTALL_PREFIX "/usr/local")  # Установка по умолчанию

# Установка исполняемого файла
install(TARGETS MyExecutable DESTINATION bin)

# Установка конфигурации для пакета
if(APPLE)
    set(CPACK_GENERATOR "DMG")  # Для macOS используем DMG
    set(CPACK_PACKAGE_FILE_NAME "MyExecutable-macos")  # Имя пакета для macOS
elseif(UNIX)
    set(CPACK_GENERATOR "DEB")  # Для Linux используем DEB
    set(CPACK_PACKAGE_FILE_NAME "MyExecutable-linux")  # Имя пакета для Linux
else()
    set(CPACK_GENERATOR "ZIP")  # Для Windows используем ZIP
    set(CPACK_PACKAGE_FILE_NAME "MyExecutable-windows")  # Имя пакета для Windows
endif()

# Дополнительные параметры
set(CPACK_PACKAGE_VERSION "0.1.0")
set(CPACK_PACKAGE_DESCRIPTION "MyExecutable package description")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Your Name <your.email@example.com>")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.3.1-6)")
set(CPACK_PACKAGE_CONTACT "Your Name <your.email@example.com>")
set(CPACK_PACKAGE_HOMEPAGE "https://example.com")
set(CPACK_PACKAGE_LICENSE "MIT")

include(CPack)  # Включение CPack