///@file VPcmdDefs.h
/// Necessary definitions for declaring and using VPcmdIF functions..
#pragma once
#include <stdint.h>

#if defined(_WIN32) || defined(__CYGWIN__) || defined(_WIN32_WCE)
#include <windows.h>
#else
#define CALLBACK
#endif

#ifndef VPCMD_STATIC
///////////////////////////////////////////////////////////
#ifdef VPCMDIF_EXPORTS
#define VPCMD_API __declspec(dllexport)
#else
#define VPCMD_API __declspec(dllimport)
#endif
///////////////////////////////////////////////////////////
#ifdef POLHEMUS_EXPORTS
#define POLHEMUS_API __declspec(dllexport)
#else
#define POLHEMUS_API __declspec(dllimport)
#endif
///////////////////////////////////////////////////////////
#else
#define VPCMD_API 
#define POLHEMUS_API 
#endif

