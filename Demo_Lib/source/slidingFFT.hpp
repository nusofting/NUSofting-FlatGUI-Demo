#include <complex>
#include <float.h>
#include "../../../DashLibrary/uFFT.hpp"

// Buffer size, make it a power of two if you want to improve fftw
static const int sizeFFT = 1 << 11;

typedef std::complex<double> complex;

class SlidingFFT
{
public:

	SlidingFFT() { 	init(); }

	// initialize all data buffers
	void init()
	{
		// clear data
		for (int i = 0; i < sizeFFT; ++i)
		{
			in[i] = 0;
		}
		// seed rand()
		init_coeffs();
		oldest_data = newest_data = 0.0;
		idx = 0;
	}

	void mainLoop(float** inputs, float** outputs, const int NSAMPS, const bool DO_SDFT)
	{	

		float* in1 = inputs[0];
		float* in2 = inputs[1];
		float* out1 = outputs[0];
		float* out2 = outputs[1];

		if(DO_SDFT)
		{
			// ------------------------------ SDFT ---------------------------------------------
			for (int i = 0; i < NSAMPS; ++i) 
			{
				oldest_data = in[idx];
				newest_data = in[idx] = complex(0.5f*((*in1++)+(*in2++)));

				sdft();
				// Mess about with freqs[] here
				isdft();
				
				*out2++ = *out1++ = outB[idx].real()/sizeFFT;

				if (++idx == sizeFFT) idx = 0; // bump global index				
			}

			//powr_spectrum(powr1);
		}else{

			// ------------------------------ FFTW ---------------------------------------------
			//plan_fwd = fftw::fftw_plan_dft_1d(sizeFFT, (fftw::fftw_complex *)in, (fftw::fftw_complex *)freqs, FFTW_FORWARD, FFTW_MEASURE);
			//plan_inv = fftw::fftw_plan_dft_1d(sizeFFT, (fftw::fftw_complex *)freqs, (fftw::fftw_complex *)out2, FFTW_BACKWARD, FFTW_MEASURE);


			fft_complex vector[sizeFFT];

			for (int i = 0; i < NSAMPS; ++i) 
			{
				oldest_data = in[idx];
				newest_data = in[idx] = complex(*in1++);

				for (size_t n = 0; n < sizeFFT; n++) 
				{
					vector[n].real = in[idx].real();
					vector[n].imag = in[idx].imag();
				}

				fft(vector, sizeFFT);	

				// mess about with freqs here

				ifft(vector, sizeFFT);

				//for (size_t n = 0; n < sizeFFT; n++) {
				//	out2[idx] = complex(vector[n].real,vector[n].imag);
				//}

				if (++idx == sizeFFT) idx = 0; // bump global index
			}
			//// normalize fftw's output 
			//for (int j = 0; j < sizeFFT; ++j) 
			//{
			//	out2[j] /= sizeFFT;
			//}

			//powr_spectrum(powr1);
		}
	}



private:

	// input signal 
	complex in[sizeFFT];

	// frequencies of input signal after ft
	// Size increased by one because the optimized sdft code writes data to freqs[sizeFFT]
	complex freqs[sizeFFT+1];

	// output signal after inverse ft of freqs
	complex outA[sizeFFT];
	complex outB[sizeFFT];

	// forward coeffs -2 PI e^iw -- normalized (divided by sizeFFT)
	complex coeffs[sizeFFT];
	// inverse coeffs 2 PI e^iw
	complex icoeffs[sizeFFT];

	double powr1[sizeFFT/2];

	// global index for input and output signals
	int idx;

	// these are just there to optimize (get rid of index lookups in sdft)
	complex oldest_data, newest_data;

	//initialize e-to-the-i-thetas for theta = 0..2PI in increments of 1/sizeFFT
	void init_coeffs()
	{
		for (int i = 0; i < sizeFFT; ++i) {
			double a = -2.0 * M_PI * i  / double(sizeFFT);
			coeffs[i] = complex(cos(a)/* / sizeFFT */, sin(a) /* / sizeFFT */);
		}
		for (int i = 0; i < sizeFFT; ++i) {
			double a = 2.0 * M_PI * i  / double(sizeFFT);
			icoeffs[i] = complex(cos(a),sin(a));
		}
	}

	// sliding dft
	void sdft()
	{
		complex delta = newest_data - oldest_data;
		int ci = 0;
		for (int i = 0; i < sizeFFT; ++i) {
			freqs[i] += delta * coeffs[ci];
			if ((ci += idx) >= sizeFFT)
				ci -= sizeFFT;
		}
	}

	// sliding inverse dft
	void isdft()
	{
		complex delta = newest_data - oldest_data;
		int ci = 0;
		for (int i = 0; i < sizeFFT; ++i) {
			freqs[i] += delta * icoeffs[ci];
			if ((ci += idx) >= sizeFFT)
				ci -= sizeFFT;
		}
	}

	// "textbook" slow dft, nested loops, O(sizeFFT*sizeFFT)
	void ft()
	{
		for (int i = 0; i < sizeFFT; ++i) {
			freqs[i] = 0.0;
			for (int j = 0; j < sizeFFT; ++j) {
				double a = -2.0 * M_PI * i * j / double(sizeFFT);
				freqs[i] += in[j] * complex(cos(a),sin(a));
			}
		}
	}

	double mag(complex& c)
	{
		return sqrt(c.real() * c.real() + c.imag() * c.imag());
	}

	void powr_spectrum(double *powr)
	{
		for (int i = 0; i < sizeFFT/2; ++i) {
			powr[i] = mag(freqs[i]);
		}
	}
};