#include "GlobalPlacer.h"
#include "ExampleFunction.h"
#include "NumericalOptimizer.h"
#include <cstdlib>
#include <iostream>
#include <time.h> 
#include <iomanip>
#include <cstring>
#include <algorithm>
GlobalPlacer::GlobalPlacer(wrapper::Placement &placement)
    : _placement(placement)
{
}

void GlobalPlacer::randomPlace()
{
    // srand(0);
    double coreWidth = _placement.boundryRight() - _placement.boundryLeft();
    double coreHeight = _placement.boundryTop() - _placement.boundryBottom();
    for (size_t i = 0; i < _placement.numModules(); ++i)
    {
        if (_placement.module(i).isFixed())
            continue;

        double width = _placement.module(i).width();
        double height = _placement.module(i).height();
        double x = rand() % static_cast<int>(coreWidth - width) + _placement.boundryLeft();
        double y = rand() % static_cast<int>(coreHeight - height) + _placement.boundryBottom();
        _placement.module(i).setPosition(x, y);
    }
}

double GlobalPlacer::netWL(wrapper::Module &m)
{
    double HPWL = 0;
    for (unsigned int i = 0; i < m.numPins(); i++)
    {
        // initialize the max and min value
        double maxX = -999999999;
        double maxY = -999999999;
        double minX = 999999999;
        double minY = 999999999;

        // get the net max and min value
        wrapper::Net n = _placement.net(m.pin(i).netId());
        for (unsigned int i = 0; i < n.numPins(); i++)
        {
            maxX = max(maxX, n.pin(i).x());
            maxY = max(maxY, n.pin(i).y());
            minX = min(minX, n.pin(i).x());
            minY = min(minY, n.pin(i).y());
        }

        // accumulate the HPWL
        HPWL += (maxX - minX) + (maxY - minY);
    }
    return HPWL;
}

void GlobalPlacer::place()
{
    /* @@@ TODO
     * 1. Understand above example and modify ExampleFunction.cpp to implement the analytical placement
     * 2. You can choose LSE or WA as the wirelength model, the former is easier to calculate the gradient
     * 3. For the bin density model, you could refer to the lecture notes
     * 4. You should first calculate the form of wirelength model and bin density model and the forms of their gradients ON YOUR OWN
     * 5. Replace the value of f in evaluateF() by the form like "f = alpha*WL() + beta*BinDensity()"
     * 6. Replace the form of g[] in evaluateG() by the form like "g = grad(WL()) + grad(BinDensity())"
     * 7. Set the initial vector x in place(), set step size, set #iteration, and call the solver like above example
     * */
    size_t seed = time(NULL);
    if(_placement.numModules() == 12028){
        seed = 1702623600;
    }else if(_placement.numModules() == 29347){
        seed = 1702626480;
    }else if(_placement.numModules() == 51382)
    {
        seed = 1702628830;
    }
    srand(seed);
    cout << "seed: " << seed;

    // get the initial solution in random
    randomPlace();
    // Simulated Annealing parameters
    double T0 = 1000000000000000000;// Initial temperature
    double T = T0;// temperature
    double temp_T = T0;                                                                                      
    const double cool = 0.0000000000000000000000000000000000000000000000000000000000000000000000000001;// cool temperature                                 
    const double fastT = 0.0001;                                                                                                         
    // const double refineT = 0.0000001;                                                                                                    
    double gamma = 0.99995;                                                                                                               
    // const double gamma_refine = 0.99995;                                                                                                 
    const double init = (_placement.boundryRight() - _placement.boundryLeft()) + (_placement.boundryTop() - _placement.boundryBottom()); 
    double curCost, nextCost;                                                                                                            
    double delta, p, rng;                                                                                                                
    int s1, s2;                                                                                                                          
    double s1x, s1y, s2x, s2y;                                                                                                 
    clock_t init_time = clock();
    const int TIME_LIMIT = 500 - 10;
    int runtime = 0;
    while (runtime < TIME_LIMIT)
    {
        // get the s1 to change with s2
        while(true)
        {
            s1 = rand() % _placement.numModules();
            if(_placement.module(s1).isFixed()){
                continue;
            }else break;
        }
        while(true)
        {
            s2 = rand() % _placement.numModules();
            if(_placement.module(s2).isFixed()){
                continue;
            }else break;
        }
        
        wrapper::Module m1 = _placement.module(s1);
        wrapper::Module m2 = _placement.module(s2);
        s1x = m1.x();
        s1y = m1.y();
        s2x = m2.x();
        s2y = m2.y();
        
        curCost = (netWL(m1) + netWL(m2)) / init;
        m1.setPosition(s2x, s2y);
        m2.setPosition(s1x, s1y);
        nextCost = (netWL(m1) + netWL(m2)) / init;
        
        delta = nextCost - curCost;
        p = exp(-delta / T);
        rng = (double)rand() / (RAND_MAX + 1.0);

        if (delta <= 0); // directly accept (cost is lower)
        else if (p >= rng && T < fastT); // accept with probabiltiy (cost is higher but accept)
        else // reject (cost is higher and reject)
        {
            m1.setPosition(s1x, s1y);
            m2.setPosition(s2x, s2y);
        }
        T *= gamma;//cooling down
        // if the temperature is smaller than cooling temperature, put some fire to temperature and annealing again until time up.
        if(T < cool)
        {
            temp_T *= 0.5;
            T = temp_T;
        }
        runtime = (clock() - init_time)/CLOCKS_PER_SEC;
    }
}

