#ifndef GLOBALPLACER_H
#define GLOBALPLACER_H

#include "Wrapper.hpp"
class GlobalPlacer
{
public:
    GlobalPlacer(wrapper::Placement &placement);

    void randomPlace(); // An example of random placement implemented by TA
    void place();
    double netWL(wrapper::Module &m);
private:
    wrapper::Placement &_placement;
};

#endif // GLOBALPLACER_H
