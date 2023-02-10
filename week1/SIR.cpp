#include <iostream>
#include <vector>
#include <functional> 
#include <math.h>
#include <fstream>
#include <string>


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
    transform (a.begin(), a.end(), b.begin(), a.begin(), plus<double>());
    return a;
}


void euler(double dt, double time_stop, vector<double> param, string filename="out.txt"){
    ofstream outfile(filename);
    outfile << "t\tS\tI\tR\n";
    outfile << 0 <<"\t"<< param[0] << "\t"<< param[1]  << "\t"<< param[2] <<"\n";
    for(int i = 1; i<(int)ceil(time_stop/dt)+1; i++){
        param = vector_sum(param,vector_multiply(SIR_dx(param),dt));
        outfile << i*dt << "\t"<< param[0] << "\t"<< param[1]  << "\t"<< param[2] <<"\n";
    }
    outfile.close();
}

void RK45(double dt, double time_stop, vector<double> param, string filename="out.txt", double tolerance = 1e-4, int max_step = 100000){
    ofstream outfile(filename);
    outfile << "t\tS\tI\tR\n";
    outfile << 0 <<"\t"<< param[0] << "\t"<< param[1]  << "\t"<< param[2] <<"\n";
    double current_time = 0;
    for(int i = 0; i<max_step; i++){
        current_time += dt;
        vector<double> k1 = SIR_dx(param);
        vector<double> k2 = SIR_dx(vector_sum(param,vector_multiply(k1,dt*0.5)));
        vector<double> k3 = SIR_dx(vector_sum(param,vector_sum(vector_multiply(k1,dt*0.25), vector_multiply(k2 ,dt*0.25))));
        vector<double> k4 = SIR_dx(vector_sum(param,vector_sum(vector_multiply(k2,dt), vector_multiply(k3 ,dt*2))));
        vector<double> k5 = SIR_dx(vector_sum(param,vector_sum(vector_sum(vector_multiply(k1,7./27*dt), vector_multiply(k2 ,10./27*dt)),vector_multiply(k4 ,1./27*dt))));
        vector<double> k6 = SIR_dx(vector_sum(param,vector_sum(vector_sum(vector_sum(vector_sum(vector_multiply(k1,28./625*dt), vector_multiply(k2 ,-1./5*dt)),vector_multiply(k3 ,546./625*dt)),vector_multiply(k4 ,54./625*dt)),vector_multiply(k5 ,-378./625*dt))));

        param = vector_sum(param,vector_sum(vector_sum(vector_sum(vector_multiply(k1,1./24*dt),vector_multiply(k4 ,5./48*dt)),vector_multiply(k5 ,27./56*dt)),vector_multiply(k6 ,125./336 *dt)));
        outfile << current_time << "\t"<< param[0] << "\t"<< param[1]  << "\t"<< param[2] <<"\n";
        if(current_time>time_stop){
            break;
        }
        vector<double> TE = vector_sum(vector_sum(vector_sum(vector_sum(vector_multiply(k1,-1./8),vector_multiply(k3,-2./3)),vector_multiply(k4,-1./16)),vector_multiply(k5,27./56)),vector_multiply(k6,125./336));
        vector_absinverse(TE);
        vector<double> temp = vector_multiply(TE,0.9*dt*pow(tolerance,0.2));
        dt = *min_element(temp.begin(),temp.end());
    }

}


int main(){
    euler(0.01, 500, {999,1,0}, "SIR_out_euler.txt");
    RK45(0.01, 500, {999,1,0}, "SIR_out_RK45.txt");
    return 0;
}
