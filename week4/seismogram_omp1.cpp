/*
  Assignment: Make an OpenMP parallelised wave propagation
  model for computing the seismic repsonse for a wave
  propagating through a horizontally stratified medium
*/

#include <iostream>
#include <fstream>
#include <iomanip>
#include <random>
#include <chrono>
#include <thread>
#include <complex>
#include <cmath>
#include <omp.h>

// shorthand name for complex number type definition below
typedef std::complex<double> Complex;

// ======================================================
// The number of frequencies sets the cost of the problem
const long nfreq=16*1024*64; // frequencies in spectrum
// ======================================================

// Initialize Basic Constants
const double dT=0.001;     // sampling distance
const long nsamp=2*nfreq;   // samples in seismogram

// Frequency resolution (frequency sampling distance)
double dF = 1/(nsamp*dT);

// read data file with one number per line
std::vector<double> read_txt_file(std::string fname) {
    std::vector<double> data;  // vector of data points
    std::string line;          // string to read in each line    
    std::ifstream file(fname); // open file
    while (std::getline(file, line))     // loop over lines until end of file
        data.push_back(std::stod(line)); // convert string to double and store in array
    return data;
}

// Cooley–Tukey FFT (in-place computation)
void fft(std::vector<Complex>& x)
{
	const long N = x.size();
	if (N <= 1) return;

	// divide
	std::vector<Complex> even(N/2), odd(N/2);
	for (long i=0; i<N/2; i++) {
	    even[i] = x[2*i];
	    odd[i]  = x[2*i+1];
	}

	// conquer
	fft(even);
	fft(odd);

	// combine
	for (long k = 0; k < N/2; k++)
	{
		Complex t = std::polar(1.0, -2 * M_PI * k / N) * odd[k];
		x[k    ] = even[k] + t;
		x[k+N/2] = even[k] - t;
	}
}

// inverse fft (in-place)
void ifft(std::vector<Complex>& x)
{
    double inv_size = 1.0 / x.size();
    for (auto& xx: x) xx = std::conj(xx); // conjugate the input
	fft(x);  	   // forward fft
    for (auto& xx: x) 
        xx = std::conj(xx)  // conjugate the output
            * inv_size;     // scale the numbers
}

// Main routine: propgate wave through layers and compute seismogram
std::vector<double> propagator(std::vector<double> wave,
                               std::vector<double> density,
                               std::vector<double> velocity) {
    const long nlayers = density.size();
    std::vector<double> imp(nlayers);      // impedance
    std::vector<double> ref(nlayers-1);    // reflection coefficient
    std::vector<Complex> half_filter(nfreq/2+1,1); // half filter
    std::vector<Complex> filter(nfreq+1);  // full filter
    std::vector<double> half_wave(nfreq+1,0); // half wave
    std::vector<Complex> wave_spectral(nsamp); // FFT(wave)
    std::vector<Complex> U(nfreq+1,0);     // Upgoing waves
    std::vector<Complex> Upad(nsamp,0);    // FFT(seismogram)
    std::vector<double> seismogram(nsamp); // final seismogram
    long n_wave = wave.size();             // size of wave array
    long lc = std::lround(std::floor(nfreq*0.01)); // low-cut indices
    double mean_wave = 0.;                 // wave zero point

    std::chrono::time_point<std::chrono::system_clock> tstart1,tstart2,tend1,tend2;
    #pragma omp parallel
    {
    #pragma omp single nowait
    {
    #pragma omp task
    {
    for (long i=0; i < n_wave/2; i++)
        half_wave[i] = wave[n_wave/2-1+i];

    for (long i=0; i < 2*nfreq; i++) {
        if (i < nfreq) {
            wave_spectral[i] = half_wave[i];
        } else {
            wave_spectral[i] = half_wave[2*nfreq-i];
        }
        mean_wave += std::real(wave_spectral[i]);
    }

    mean_wave = mean_wave / nsamp;

    for (long i=0.; i < 2*nfreq; i++)
        wave_spectral[i] -= mean_wave;

    // Fourier transform waveform to frequency domain
    tstart1 = std::chrono::high_resolution_clock::now(); // start time (nano-seconds)
    // check if fft can optimise prob can't because of reference
    fft(wave_spectral);
    tend1 = std::chrono::high_resolution_clock::now(); // end time (nano-seconds)
    }
    #pragma omp task
    {
    // Compute seismic impedance
    for (long i=0; i < nlayers; i++)
        imp[i] = density[i] * velocity[i];
    
    // Reflection coefficients at the base of the layers :
    for (long i=0; i < nlayers-1; i++)
        ref[i] = (imp[i+1] - imp[i])/(imp[i+1] + imp[i]);
    }

    #pragma omp task
    {
    // Spectral window (both low- and high cut)
    for (long i=0; i < lc+1; i++)
        half_filter[i]= (sin(M_PI*(2*i-lc)/(2*lc)))/2+0.5;

    for (long i=0; i < nfreq/2+1; i++)
        filter[i] = half_filter[i];

    filter[nfreq/2+1] = 1;

    for (long i=nfreq/2+2; i < nfreq+1; i++)
        filter[i] = half_filter[nfreq+1-i];

    }
    
    // spectrum U of upgoing waves just below the surface.
    // See eq. (43) and (44) in Ganley (1981).

    }
    }
    //end pragma
    #pragma omp parallel for
    for (long i=0; i < nfreq+1; i++) {
        Complex omega{0, 2*M_PI*i*dF};
        Complex exp_omega = exp( - dT * omega);
        Complex Y = 0;
        for (long n=nlayers-2; n > -1; n--)
            Y = exp_omega * (ref[n] + Y) / (1.0 + ref[n]*Y);
        U[i] = Y;
    }

    // needs stuff from filter and ref
    // Compute seismogram
    for (long i=0; i < nfreq+1; i++) {
        U[i] *= filter[i];
        Upad[i] = U[i];
    }

    for (long i=nfreq+1; i < nsamp; i++)
        Upad[i] = std::conj(Upad[nsamp - i]);

    for (long i=0; i < nsamp; i++)
        Upad[i] *= wave_spectral[i];
    
    // Fourier transform back again
    tstart2 = std::chrono::high_resolution_clock::now(); // start time (nano-seconds)
    ifft(Upad);
    tend2 = std::chrono::high_resolution_clock::now(); // end time (nano-seconds)

    for (long i=0; i < nsamp; i++)
        seismogram[i] = std::real(Upad[i]);

    // end pragma before here
    std::cout <<  "Wave zero-point        : "  << std::setw(9) << std::setprecision(5) 
              << mean_wave<< "\n";    
    std::cout <<  "Seismogram first coeff : "  << std::setw(9) << std::setprecision(5) 
              << seismogram[0] << ", " << seismogram[1] << ", " << seismogram[2] << ", " << seismogram[3] <<"\n";    
    std::cout <<  "Elapsed time for FFTs  :" << std::setw(9) << std::setprecision(4)
              << ((tend1 - tstart1).count() + (tend2 - tstart2).count())*1e-9 << "\n";
    
    return seismogram;
}

//======================================================================================================
//======================== Main function ===============================================================
//======================================================================================================
int main(int argc, char* argv[]){    
    // Load the wave profile and the density and velocity structure of the rock from text files
    std::vector<double> wave = read_txt_file("wave_data.txt");         // input impulse wave in medium
    std::vector<double> density = read_txt_file("density_data.txt");   // density as a function of depth
    std::vector<double> velocity = read_txt_file("velocity_data.txt"); // seismic wave velocity as a function of depth

    auto tstart = std::chrono::high_resolution_clock::now(); // start time (nano-seconds)
    
    // Propagate wave
    std::vector<double> seismogram = propagator(wave,density,velocity);

    auto tend = std::chrono::high_resolution_clock::now(); // end time (nano-seconds)
    
    // write output and make checksum
    double checksum=0;
    std::ofstream file("seismogram.txt"); // open file
    for (long i=0; i < nsamp; i++) {
        file << seismogram[i] << '\n';
        checksum += abs(seismogram[i]);
    }

    std::cout <<  "Elapsed time:" << std::setw(9) << std::setprecision(4)
              << (tend - tstart).count()*1e-9 << "\n";
    std::cout <<  "Checksum    :" << std::setw(20) << std::setprecision(15)
              << checksum << "\n";
}
