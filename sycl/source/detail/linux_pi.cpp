#include <dlfcn.h>
#include <string>
#include <iostream>

void *loadOsLibrary(const std::string &PluginPath) {
  // TODO: Check if the option RTLD_NOW is correct. Explore using
  // RTLD_DEEPBIND option when there are multiple plugins.
#if INTEL_CUSTOMIZATION
  void *so = dlopen(PluginPath.c_str(), RTLD_NOW);
  if (!so) {
    std::cerr << "dlopen(" << PluginPath << ") failed with <" <<
      dlerror() << ">" << std::endl;
  }
  return so;
#endif // INTEL_CUSTOMIZATION
}

void *getOsLibraryFuncAddress(void *Library, const std::string &FunctionName) {
  return dlsym(Library, FunctionName.c_str());
}
