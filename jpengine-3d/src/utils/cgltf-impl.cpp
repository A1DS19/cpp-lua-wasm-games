// Single translation unit that emits the cgltf function bodies. Every other
// .cpp can `#include <cgltf.h>` normally — only this file defines the
// implementation macro. Defining it in more than one TU produces multiple-
// symbol linker errors.

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>
