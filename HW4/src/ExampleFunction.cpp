#include "ExampleFunction.h"
//#include "Wrapper.hpp"
#include <iostream>

ExampleFunction::ExampleFunction(wrapper::Placement &placement):_placement(placement)
{
    // calculate total area/ core area ratio
    const double corewidth = _placement.boundryRight() - _placement.boundryLeft();
    const double coreHeight  = _placement.boundryTop() - _placement.boundryBottom();
    const double core_area = corewidth*coreHeight;
    double total_area = 0;
    for(unsigned int i = 0 ;i < _placement.numModules();i++)
    {
        total_area += _placement.module(i).area();
    }
    // target bin density
    Tb = total_area/core_area;// must smaller than 1;
    Initialbin(corewidth,coreHeight);
    Gamma = (corewidth)/600;
    beta = 0;
    
}
// first step: make the core area to grid with bin;
// module is assumed as unit area;
void ExampleFunction::Initialbin(const double width, const double height)
{
    // set up a number of bins in height and width
    //unsigned int number_bins = 20;
    double yc = 0, xc = 0;
    unit_h_b = height / per_bins;
    unit_w_b = width / per_bins;

    for(unsigned int i = 0; i < per_bins; i++)
    {
        xc = _placement.boundryBottom() + 0.5*unit_w_b + i*unit_w_b;
        for(unsigned int j = 0; j < per_bins; j++)
        {
            yc = _placement.boundryLeft() + 0.5* unit_h_b + i * unit_h_b;
            bin[j][i].x = xc;
            bin[j][i].y = yc;
        }
    }
    return;

}

void ExampleFunction::evaluateFG(const vector<double> &x, double &f, vector<double> &g)
{
    //P.62
    g = vector<double>(g.size(), 0);
    // double temp = LSE_FG(x,g);
    // double temp1 = Bindensity_FG(x,g);
    f = LSE_FG(x,g) + Bindensity_FG(x,g);
    // cout << "LSE_FG: " << temp <<endl;
    // cout << "B_FG: " << temp1 << endl;
    return;
}

void ExampleFunction::evaluateF(const vector<double> &x, double &f)
{
    f = LSE_F(x)+ Bindensity_F(x);
    return;
}

unsigned ExampleFunction::dimension()
{
    return _placement.numModules()*2;
    // each two dimension represent the X and Y dimensions of each block
}

//LSE for evaluateFG
//因為HPWL無法微分，所以無法使用evaluateFG and evaluateF，而根據網路上的查詢LSE 相當接近HPWL的走勢 if gamma 很小的時候
double ExampleFunction::LSE_FG(const vector<double> &x, vector<double> &g)
{   
    double total_lse = 0;
    unsigned int number_module = _placement.numModules();
    vector<double> x_exp_table(number_module * 4);
    //build up log-sum-exponential
    for (unsigned int i = 0; i < number_module; i++) {
        x_exp_table[i * 4] = exp(x[i * 2]/Gamma);//x[i] = xk
        x_exp_table[i * 4 + 1] = exp(-x[i * 2]/Gamma);
        x_exp_table[i * 4 + 2] = exp(x[i * 2 + 1]/Gamma);
        x_exp_table[i * 4 + 3] = exp(-x[i * 2 + 1]/Gamma);
    }
    const unsigned int num_nets = _placement.numNets();
    for (unsigned int net_id = 0; net_id < num_nets; net_id++) {
        double sum_x = 0;
        double sum_nx = 0;
        double sum_y = 0;
        double sum_ny = 0;
        const unsigned int num_pins = _placement.net(net_id).numPins();
        
        for (unsigned int index = 0; index < num_pins; index++) {
            const unsigned int module_id =  _placement.net(net_id).pin(index).moduleId();
            sum_x += x_exp_table[module_id * 4];
            sum_nx += x_exp_table[module_id * 4 + 1];
            sum_y += x_exp_table[module_id * 4 + 2];
            sum_ny += x_exp_table[module_id * 4 + 3];
        }

        total_lse += (log(sum_x) + log(sum_nx) + log(sum_y) + log(sum_ny));
        for (unsigned int index = 0; index < num_pins; index++) {
            const unsigned int module_id = this->_placement.net(net_id).pin(index).moduleId();
            g[module_id * 2] += x_exp_table[module_id * 4] / sum_x - x_exp_table[module_id * 4 + 1] / sum_nx;
            g[module_id * 2 + 1] += x_exp_table[module_id * 4 + 2] / sum_y - x_exp_table[module_id * 4 + 3] / sum_ny;
        }
    }
    return Gamma* total_lse;
}

double ExampleFunction::LSE_F(const vector<double> &x)// evaluateF
{
    double total_lse = 0;
    unsigned int number_module = x.size()/2;
    vector<double> x_exp_table(number_module * 4);
    for (unsigned int i = 0; i < number_module; i++) {
        x_exp_table[i * 4] = exp(x[i * 2]/Gamma);//x[i] = xk
        x_exp_table[i * 4 + 1] = exp(-x[i * 2]/Gamma);
        x_exp_table[i * 4 + 2] = exp(x[i * 2 + 1]/Gamma);
        x_exp_table[i * 4 + 3] = exp(-x[i * 2 + 1]/Gamma);
    }

    const unsigned int num_nets = this->_placement.numNets();
    for (unsigned int net_id = 0; net_id < num_nets; net_id++) {
        //wrapper::Net &net = this->_placement.net(net_id);

        double sum_x = 0;
        double sum_nx = 0;
        double sum_y = 0;
        double sum_ny = 0;
        const unsigned int num_pins = this->_placement.net(net_id).numPins();
        for (unsigned int index = 0; index < num_pins; index++) {
            const unsigned int module_id = this->_placement.net(net_id).pin(index).moduleId();
            sum_x += x_exp_table[module_id * 4];
            sum_nx += x_exp_table[module_id * 4 + 1];
            sum_y += x_exp_table[module_id * 4 + 2];
            sum_ny += x_exp_table[module_id * 4 + 3];
        }

        total_lse +=log(sum_x) + log(sum_nx) + log(sum_y) + log(sum_ny);
    }
    return Gamma* total_lse;
}



double ExampleFunction::Bindensity_F(const vector<double> &x){
    double bindenx = 0;
    double bindeny = 0;
    double Dbxy = 0;
    for(unsigned int i = 0; i < per_bins;i++)
    {
        for(unsigned int j = 0; j < per_bins;j++)
        {
            double per_Dbxy = 0;
            for(unsigned int id = 0; id < _placement.numModules();id++)
            {
                double dx = abs(x[id*2] - bin[j][i].x);
                if(dx <= unit_w_b/2 + _placement.module(id).width()/2)
                {
                    double a = 4/((unit_w_b + _placement.module(id).width()) * (2 * unit_w_b + _placement.module(id).width()));
                    bindenx = 1 - a * dx * dx;
                }else if(dx <= unit_w_b + _placement.module(id).width()/2)
                {
                    double b = 4/(unit_w_b*(2*unit_w_b + _placement.module(id).width()));
                    bindenx = b * (dx - unit_w_b - _placement.module(id).width()/2) *(dx - unit_w_b - _placement.module(id).width()/2) ;
                }else{
                    bindenx = 0;
                }
                double dy = abs(x[id*2 + 1] - bin[j][i].y);
                if(dy <= unit_h_b/2 + _placement.module(id).height()/2)
                {
                    double a = 4/((unit_h_b + _placement.module(id).height()) * (2 * unit_h_b + _placement.module(id).height()));
                    bindeny = 1 - a * dy * dy;
                }else if(dy <= unit_h_b + _placement.module(id).height()/2)
                {
                    double b = 4/(unit_h_b*(2*unit_h_b + _placement.module(id).height()));
                    bindeny = b*(dy - unit_h_b - _placement.module(id).height()/2) * (dy - unit_h_b - _placement.module(id).height()/2);
                }else{
                    bindeny = 0;
                }

                per_Dbxy += bindenx * bindeny;
            }
            Dbxy += (per_Dbxy-Tb) *(per_Dbxy - Tb);
        }
    }
    return beta * Dbxy;
}

double ExampleFunction::Bindensity_FG(const vector<double> &x, vector<double> &g){
    double bindenx = 0;
    double bindeny = 0;
    double Dbxy = 0;
    for(unsigned int i = 0; i < per_bins;i++)
    {
        for(unsigned int j = 0; j < per_bins;j++)
        {
            double per_Dbxy = 0;
            vector<double> g_temp(g.size(), 0);
            for(unsigned int id = 0; id < _placement.numModules();id++)
            {
                double dx = abs(x[id*2] - bin[j][i].x);
                double bin_density_x_g = 0;
                double bin_density_y_g = 0;                 
                if(dx <= unit_w_b/2 + _placement.module(id).width()/2)
                {
                    double a = 4/((unit_w_b + _placement.module(id).width()) * (2 * unit_w_b + _placement.module(id).width()));
                    bindenx = 1 - a * dx * dx;
                    if(x[id*2] >= bin[j][i].x)
                    {
                        bin_density_x_g = -a * 2 * dx;
                    }else{
                        bin_density_x_g = a * 2 * dx;
                    }
                }else if(dx <= unit_w_b + _placement.module(id).width()/2)
                {
                    double b = 4/(unit_w_b*(2*unit_w_b + _placement.module(id).width()));
                    bindenx = b * (dx - unit_w_b - _placement.module(id).width()/2)*(dx - unit_w_b - _placement.module(id).width()/2);
                    if(x[id*2] >= bin[j][i].x)
                    {
                        bin_density_x_g = b*2*(dx - unit_w_b - _placement.module(id).width()/2);
                    }else{
                        bin_density_x_g = b*2*(dx - unit_w_b - _placement.module(id).width()/2)*(-1);
                    }
                }else{
                    bindenx = 0;
                }

                double dy = abs(x[id*2 + 1] - bin[j][i].y);
                if(dy <= unit_h_b/2 + _placement.module(id).height()/2)
                {
                    double a = 4/((unit_h_b + _placement.module(id).height()) * (2 * unit_h_b + _placement.module(id).height()));
                    bindeny = 1 - a * dy*dy;
                    if(x[id*2 + 1] >= bin[j][i].y)
                    {
                        bin_density_y_g = -a * 2 * dy;
                    }else{
                        bin_density_y_g = a * 2 * dy;
                    }
                }else if(dy <= unit_h_b + _placement.module(id).height()/2)
                {
                    double b = 4/(unit_h_b*(2*unit_h_b + _placement.module(id).height()));
                    bindeny = b*(dy - unit_h_b - _placement.module(id).height()/2)*(dy - unit_h_b - _placement.module(id).height()/2);
                    if(x[id*2 + 1] >= bin[j][i].y)
                    {
                        bin_density_y_g = b*2*(dy - unit_h_b - _placement.module(id).height()/2);
                    }else{
                        bin_density_y_g = b*2*(dy - unit_h_b - _placement.module(id).height()/2)*(-1);
                    }
                }else{
                    bindeny = 0;
                }

                per_Dbxy += bindenx * bindeny;
                g_temp[id * 2] = bin_density_x_g * bindeny;
                g_temp[id * 2 + 1] = bindenx * bin_density_y_g;
            }
            Dbxy += (per_Dbxy - Tb)*(per_Dbxy - Tb);
            for (unsigned int module_id = 0; module_id < _placement.numModules(); module_id++) {
                g[module_id * 2] += beta * 2 * (per_Dbxy - Tb)* g_temp[module_id * 2];
                g[module_id * 2 + 1] += beta * 2 * (per_Dbxy - Tb)* g_temp[module_id * 2 + 1];
            }
        }
    }
    return beta * Dbxy;
}