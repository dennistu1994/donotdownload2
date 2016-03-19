#pragma once
#include <random>
#include <time.h>
using namespace std;

class RNG{
	public:
		RNG(){
			srand(time(NULL));
			this->generator = new default_random_engine(random_device{}()+rand());
			this->distribution = new uniform_real_distribution<double>(0.0, 1.0);
		};

		RNG(default_random_engine* generator, uniform_real_distribution<double>* distribution){
			this->generator = generator;
			this->distribution = distribution;
		};

		double get_exp_from_uni(double x, double lambda){
			return -log(1l-x)/lambda;
		};

		double get_next(){
			return (*this->distribution)(*(this->generator));
		}

	private:
		default_random_engine* generator;
		uniform_real_distribution<double>* distribution;
};
