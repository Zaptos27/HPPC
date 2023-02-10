#include <iostream>
#include <vector>
#include <functional> 
#include <tuple>
#include <math.h>

using namespace std;

vector<double> SIR_dx(vector<double> param,int N=1000, double beta = 1/5., double gamma = 1/10.){
    vector<double> vec(3);
    vec[0] = -beta*param[1]*param[0]/N;
    vec[1] = beta*param[1]*param[0]/N-gamma*param[1];
    vec[2] = gamma*param[1];
    return vec;
}

double inverse_abs(double f){
    return 1/pow(abs(f),0.2);
}

void vector_absinverse(vector<double> &v){
    transform(v.begin(), v.end(), v.begin(), inverse_abs);
}

vector<double> vector_multiply(vector<double> v, double mult){
    transform(v.begin(), v.end(), v.begin(), bind(multiplies<double>(), placeholders::_1, mult));
    return v;
}

vector<double> vector_sum(vector<double> a, vector<double> b){
    transform(a.begin(), a.end(), b.begin(), a.begin(), plus<double>());
    return a;
}

tuple<vector<double>, vector<vector<double>>> euler(double dt, double time_stop, vector<double> init_param){
    vector<vector<double>> SIR_out = {init_param};
    vector<double> time_step = {0};
    int reserve = time_stop/dt;
    SIR_out.reserve(reserve);
    time_step.reserve(reserve);
    for(int i = 0; i < reserve; i++){
        SIR_out.push_back(vector_sum(SIR_out[i],vector_multiply(SIR_dx(SIR_out[i]),dt)));
        time_step.push_back(time_step[i]+dt);
    }
    return make_tuple(time_step,SIR_out);
}



tuple<vector<double>, vector<vector<double>>> RK45(double dt, double time_stop, vector<double> init_param, double tolerance = 1e-4, int max_step = 100000){
    vector<vector<double>> SIR_out = {init_param};
    vector<double> time_step = {0};
    int reserve = 2*time_stop/dt;
    SIR_out.reserve(reserve);
    time_step.reserve(reserve);
    for(int i = 0; i<max_step; i++){
        vector<double> k1 = SIR_dx(SIR_out[i]);
        vector<double> k2 = SIR_dx(vector_sum(SIR_out[i],vector_multiply(k1,dt*0.5)));
        vector<double> k3 = SIR_dx(vector_sum(SIR_out[i],vector_sum(vector_multiply(k1,dt*0.25), vector_multiply(k2 ,dt*0.25))));
        vector<double> k4 = SIR_dx(vector_sum(SIR_out[i],vector_sum(vector_multiply(k2,dt), vector_multiply(k3 ,dt*2))));
        vector<double> k5 = SIR_dx(vector_sum(SIR_out[i],vector_sum(vector_sum(vector_multiply(k1,7./27*dt), vector_multiply(k2 ,10./27*dt)),vector_multiply(k4 ,1./27*dt))));
        vector<double> k6 = SIR_dx(vector_sum(SIR_out[i],vector_sum(vector_sum(vector_sum(vector_sum(vector_multiply(k1,28./625*dt), vector_multiply(k2 ,-1./5*dt)),vector_multiply(k3 ,546./625*dt)),vector_multiply(k4 ,54./625*dt)),vector_multiply(k5 ,-378./625*dt))));

        SIR_out.push_back(vector_sum(SIR_out[i],vector_sum(vector_sum(vector_sum(vector_multiply(k1,1./24*dt),vector_multiply(k4 ,5./48*dt)),vector_multiply(k5 ,27./56*dt)),vector_multiply(k6 ,125./336 *dt))));
        time_step.push_back(time_step[i]+dt);
        if(time_step[time_step.size()-1]>time_stop){
            return make_tuple(time_step,SIR_out);
        }
        vector<double> TE = vector_sum(vector_sum(vector_sum(vector_sum(vector_multiply(k1,-1./8),vector_multiply(k3,-2./3)),vector_multiply(k4,-1./16)),vector_multiply(k5,27./56)),vector_multiply(k6,125./336));
        vector_absinverse(TE);
        vector<double> temp = vector_multiply(TE,0.9*dt*pow(tolerance,0.2));
        dt = *min_element(temp.begin(),temp.end());
    }
    return make_tuple(time_step,SIR_out);
}






int main(){
    
    return 0;
}