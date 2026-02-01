cmake_minimum_required(VERSION 3.21)

project(stargrind VERSION 1.0.0)

# --- Configuration du standard C++ ---
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Configuration Geode ---
# On l'inclut tôt pour profiter des macros Geode
find_package(Geode REQUIRED)

# --- Configuration de CPM ---
set(CPM_DOWNLOAD_VERSION 0.40.0)
if(NOT EXISTS "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
    message(STATUS "Downloading CPM.cmake...")
    file(DOWNLOAD
        "https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake"
        "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake"
    )
endif()
include("${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")

# --- Ajout de ENet ---
# On utilise la version officielle. 
# Note : ENet sera compilé statiquement dans ton mod.
CPMAddPackage(
    NAME enet
    GITHUB_REPOSITORY lsalzman/enet
    GIT_TAG v1.3.17
)

# --- Sources ---
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)

# --- Création du Mod ---
add_library(${PROJECT_NAME} SHARED ${SOURCES})

# --- Liaison des dépendances ---
# Sur Windows, ENet a besoin des sockets système (Winsock)
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE enet ws2_32 winmm)
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE enet)
endif()

# Liaison avec l'API Geode
target_link_libraries(${PROJECT_NAME} PRIVATE geode-sdk)

# Configuration finale spécifique à Geode
setup_geode_mod(${PROJECT_NAME})