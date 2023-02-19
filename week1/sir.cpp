#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
using namespace std;

vector<double> diff_eq(vector<double> SIR, double B = 0.2, double gamma = 0.1, int N=1000){
    double dS = -B * SIR[1]*SIR[0]/N;
    double dI = B*SIR[1]*SIR[0]/N - gamma*SIR[1];
    double dR = gamma * SIR[1];
    return {dS, dI, dR};
}

vector<double> vec_plus(vector<double> a, vector<double> b){
    for(int i=0; i<a.size(); i++){
        a[i] += b[i];
    }
    return a;
}

vector<double> vec_plus(vector<double> v, double k){
    for(int i=0; i<v.size(); i++){
        v[i] += k;
    }
    return v;
}

vector<double> vec_mult(vector<double> v, double k){
    for(int i=0; i<v.size(); i++){
        v[i] *= k;
    }
    return v;
}

void rk_step(vector<double> &SIR, double dt){
    vector<double> k1 = vec_mult(diff_eq(SIR), dt);
    vector<double> k2 = vec_mult(diff_eq(vec_plus(SIR, vec_mult(k1, 0.5))), dt);
    vector<double> k3 = vec_mult(diff_eq(vec_plus(SIR, vec_mult(k2, 0.5))), dt);
    vector<double> k4 = vec_mult(diff_eq(vec_plus(SIR, k3)), dt);
    vector<double> k = vec_plus(vec_plus(vec_plus(k1, vec_mult(k2, 2)), vec_mult(k3, 2)), k4);
    SIR = vec_plus(SIR, k);
}

vector<double> SIR = {699, 1, 300};
double dt = 0.1;

int main(){
    std::ofstream fw("SIR_data.txt");
    fw << "t" << "\t" << "S" << "\t" << "I" << "\t" << "R" << "\n";
    fw << 0 << "\t" << SIR[0] << "\t" << SIR[1] << "\t" << SIR[2] << "\n";    
    for(int i=0; i<1000; i++){
        rk_step(SIR, dt);
        fw << (i+1)*dt << "\t" << SIR[0] << "\t" << SIR[1] << "\t" << SIR[2] << "\n";
    }
    fw.close();
}