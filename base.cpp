#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <random>
#include <fstream>
#include <thread>

using namespace std;

double cdf(double x) {
    return 0.5 * erfc(-x * M_SQRT1_2);
}

class CallOption{
public:
    CallOption(double strike, double r, double impliedVol, double T)
        : strike(strike), r(r), impliedVol(impliedVol), T(T) {}

    double d1(double S, double t) {
        return (log(S/strike) + (r + 0.5*impliedVol*impliedVol)*(T-t))/(impliedVol*sqrt(T-t));
    }

    double d2(double S, double t) {
        return d1(S, t) - impliedVol*sqrt(T-t);
    }

    double delta(double S, double t) {
        return cdf(d1(S, t));
    }

    double getStrike(){
        return strike;
    }

private:
    double strike;
    double r;
    double impliedVol;
    double T; // Time to maturity in years
};

void simulateAndHedge(int iter, CallOption& option, int numTimePeriods, double T, double sigma, vector<double>& cashFlow, vector<double>& noise, vector<double>& S, double initCallDelta){
    // Assuing r = 0, thus Monte-Carlo equation for underlying becomes
    // S(t+dt) = S(t) * (1 + sigma*sqrt(dt)*Z) where Z is noise
    vector<double> callDeltaValues(numTimePeriods, initCallDelta);
    double dt = T/numTimePeriods;
    for(int j = 1; j < numTimePeriods+1; j++){

        //Monte Carlo Evolution of underlying
        S[j] = S[j-1]*(1 + sigma*sqrt(dt)*noise[j-1]);
        if(j != numTimePeriods){ // from first time period end till (numTimePeriods-1)th end
            callDeltaValues[j] = option.delta(S[j], j*dt);
            cashFlow[iter] += (callDeltaValues[j] - callDeltaValues[j-1]) * S[j];
            // Short (callDeltaValues[j] - callDeltaValues[j-1]) more shares of underlying
            // to Hedge the change in delta
        }
        else{ // j = numTimePeriods   at end of last time period ie at Expiry
            // Portfolio is ( 1 call option Long & callDeltaValues[i][numTimePeriods-1] shares of underlying Short )
            double callOptionPrice_atExpiry = max(0.0, S[numTimePeriods] - option.getStrike());
            // Closing long 1 call option position
            cashFlow[iter] += callOptionPrice_atExpiry; 
            // Closing short callDeltaValues[numTimePeriods-1] shares of underlying position
            cashFlow[iter] -= callDeltaValues[numTimePeriods-1]*S[numTimePeriods]; 
        }
    }

}

int main(int argc, char* argv[]){
    if(argc != 8){
        cout << "Usage: ./base numTimePeriods numSimulations T initPrice sigma impliedVol seed" << endl;
        return 1;
    }
    int numTimePeriods = atoi(argv[1]), numSimulations = atoi(argv[2]);
    double T = atoi(argv[3]); //in years
    double S0 = atoi(argv[4]); //Initial underlying price
    double sigma = atoi(argv[5])/100.0; //Volatility 

    double impliedVol = atoi(argv[6])/100.0;
    double strike = S0;
    CallOption option(strike, 0, impliedVol, T); // risk free interest rate fixed to 0

    double initCallPrice = S0*cdf(option.d1(S0, 0)) - strike*cdf(option.d2(S0, 0));
    double initCallDelta = option.delta(S0, 0);
    // Delta Hedged Portfolio ( 1 call option Long & initCallDelta shares of underlying Short )
    double initCashFlow = -initCallPrice+initCallDelta*S0; 
    vector<double> cashFlow(numSimulations, initCashFlow);

    vector<vector<double>> noise(numSimulations, vector<double>(numTimePeriods));
    default_random_engine generator(atoi(argv[7]));
    normal_distribution<double> dist(0.0, 1.0);
    for(int i = 0; i < numSimulations; i++){
        for(int j = 0; j < numTimePeriods; j++){
            noise[i][j] = dist(generator);
        }
    }
    
    vector<vector<double>> S(numSimulations, vector<double>(numTimePeriods+1, S0));
    vector<thread> threads;
    for (int i = 0; i < numSimulations; ++i) {
        threads.emplace_back(simulateAndHedge, i, ref(option), numTimePeriods, T, sigma, ref(cashFlow), ref(noise[i]), ref(S[i]), initCallDelta);
    }
    for (auto& thread : threads) {
        thread.join();
    }

    // Writing into output file
    string filename = "./simulations/sim_" + to_string(numTimePeriods) + "_IV_" + argv[6] + "_RealVol_" + argv[5] + ".csv";
    ofstream file(filename);

    double temp = 0;
    for(int i = 0; i < numSimulations; i++){
        temp += cashFlow[i];
        for(int j = 0; j < numTimePeriods; j++){
            file << S[i][j];
            if(j != numTimePeriods) file << ",";
        }
        file << endl;
    }

    cout << "Average Cash Flow across simulations: " << temp/numSimulations << endl;
}