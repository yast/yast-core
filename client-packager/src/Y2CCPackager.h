#ifndef Y2CCPackager_h
#define Y2CCPackager_h

#include <Y2.h>
#include "Y2PackagerComponent.h"

class Y2CCPackager : public Y2ComponentCreator
{
public:
  // Create a packager component creator and register it
  Y2CCPackager() : Y2ComponentCreator(Y2ComponentBroker::BUILTIN)
    {}

  // The packager component is a client
  bool isServerCreator() const { return false; }

  // Create a new packager component if name is our name
  Y2Component *create(const char *name) const
    {
      if (strcmp(name, Y2PackagerComponent::component_name().c_str()) == 0)
	return new Y2PackagerComponent();
      else
	return 0;
    }
};

#endif // Y2CCPackager_h
