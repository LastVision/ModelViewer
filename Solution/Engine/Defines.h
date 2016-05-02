#pragma once

#define NUMBER_OF_DIRECTIONAL_LIGHTS 2
#define NUMBER_OF_POINT_LIGHTS 3
#define NUMBER_OF_SPOT_LIGHTS 3
#define MAX_NR_OF_BONES 64
#define OCTREE_WIDTH 10000.f

//If we get random crashes in .exes undefine USE_DGFX
//#define USE_DGFX

#ifdef USE_DGFX
//#define CONVERT_TO_DGFX_IN_RUNTIME
#endif

//DO NOT CHANGE
#define DGFX_VERSION 7
#define TERRAIN_VERSION 1

//#define ENABLE_DEBUG_TEXT
#ifndef DLL_EXPORT
#define THREADED_LOADING
//#define SCENE_USE_OCTREE
#endif