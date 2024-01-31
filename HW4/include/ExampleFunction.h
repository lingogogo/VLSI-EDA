#ifndef EXAMPLEFUNCTION_H
#define EXAMPLEFUNCTION_H

#include "NumericalOptimizerInterface.h"
#include "Wrapper.hpp"

#include <cmath>

class ExampleFunction : public NumericalOptimizerInterface
{
public:
    ExampleFunction(wrapper::Placement &placement);
    double LSE_FG(const vector<double> &x, vector<double> &g);
    double LSE_F(const vector<double> &x);
    void evaluateFG(const vector<double> &x, double &f, vector<double> &g);
    void evaluateF(const vector<double> &x, double &f);
    unsigned dimension();
    double Bindensity_F(const vector<double>&x);
    double Bindensity_FG(const vector<double>&x, vector<double>&g);

    void Initialbin(const double , const double);
    wrapper::Placement &_placement;
    struct Bin
    {
        double x = 0;
        double y = 0;
    };
    static const unsigned int per_bins = 16;
    Bin bin[per_bins][per_bins];
    double unit_h_b;
    double unit_w_b;
    double beta;
    double Tb;
    double Gamma;
};

#endif // EXAMPLEFUNCTION_H
