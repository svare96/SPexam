#include <iostream>
#include <algorithm>
#include <cmath>
#include <future>

#include "./lib/lib.h"

//using namespace stochastic;

void circadian(const std::string &filePath);
void fig1Examples(int a, int b, int c, double endTime, const std::string& filePath);
int covid19SEIHR(int N, const std::string& filePath);
void covid19on20Threads (const std::string &filePath);


int main() {
    int NNJ = 589755;
    int NDK = 5822763;

    std::cout << "Hello, World!" << std::endl;
    /**
     * Req. 5 + 6
     * This is where we demonstrate the use of the library on the three examples
     * The plots are found in the report
     */
//    circadian("../graphs/circadian.csv");
//    fig1Examples(100, 0, 1, 2000, "../graphs/fig1A.csv");
//    fig1Examples(100, 0, 2, 1500, "../graphs/fig1B.csv");
//    fig1Examples(50, 50, 1, 2000, "../graphs/fig1C.csv");
//    covid19SEIHR(10000, "../graphs/covid10k.csv");
//    covid19SEIHR(NNJ, "../graphs/NorthJutland.csv");
//    covid19SEIHR(NNJ, "../graphs/Denmark.csv");

    covid19on20Threads ("../graphs/covid10k.csv");


    return 0;
}

/**
 * This function is used to perform circadian oscillation as seen in Example A
 *
 * @param filePath
 */
void circadian(const std::string &filePath) {
    auto alphaA = 50.0;
    auto alpha_A = 500.0;
    auto alphaR = 0.01;
    auto alpha_R = 50.0;
    auto betaA = 50.0;
    auto betaR = 5.0;
    auto gammaA = 1.0;
    auto gammaR = 1.0;
    auto gammaC = 2.0;
    auto deltaA = 1.0;
    auto deltaR = 0.2;
    auto deltaMA = 10.0;
    auto deltaMR = 0.5;
    auto thetaA = 50.0;
    auto thetaR = 100.0;

    std::vector<stochastic::Agent> agentVec;
    auto DA = stochastic::Agent("DA", 1);
    auto D_A = stochastic::Agent("D_A", 0);
    auto DR = stochastic::Agent("DR", 1);
    auto D_R = stochastic::Agent("D_R", 0);
    auto MA = stochastic::Agent("MA", 0);
    auto MR = stochastic::Agent("MR", 0);
    auto A = stochastic::Agent("A", 0);
    auto R = stochastic::Agent("R", 0);
    auto C = stochastic::Agent("C", 0);

    agentVec.push_back(DA);
    agentVec.push_back(D_A);
    agentVec.push_back(DR);
    agentVec.push_back(D_R);
    agentVec.push_back(MA);
    agentVec.push_back(MR);
    agentVec.push_back(A);
    agentVec.push_back(R);
    agentVec.push_back(C);

    std::vector<stochastic::Reaction> reactionVec;
    reactionVec.emplace_back(A + DA >>= D_A, gammaA);
    reactionVec.emplace_back(D_A >>= DA + A, thetaA);
    reactionVec.emplace_back(A + DR >>= D_R, gammaR);
    reactionVec.emplace_back(D_R >>= DR + A, thetaR);
    reactionVec.emplace_back(D_A >>= MA + D_A, alpha_A);
    reactionVec.emplace_back(DA >>= MA + DA, alphaA);
    reactionVec.emplace_back(D_R >>= MR + D_R, alpha_R);
    reactionVec.emplace_back(DR >>= MR + DR, alphaR);
    reactionVec.emplace_back(MA >>= MA + A, betaA);
    reactionVec.emplace_back(MR >>= MR + R, betaR);
    reactionVec.emplace_back(A + R >>= C, gammaC);
    reactionVec.emplace_back(C >>= R, deltaA);
    reactionVec.emplace_back(A >>= "env", deltaA);
    reactionVec.emplace_back(R >>= "env", deltaR);
    reactionVec.emplace_back(MA >>= "env", deltaMA);
    reactionVec.emplace_back(MR >>= "env", deltaMR);

    stochastic::Visualizer::generateNetworkGraph(reactionVec, "C:/Users/jonas/AAU/P8/sp/SPexam/Exam/graphs/circadianNetworkGraph.dot");
    stochastic::Visualizer::prettyPrintReactions(reactionVec);

    stochastic::Monitor monitor = stochastic::Algorithm::simulation(reactionVec, 100, agentVec, filePath);
}

/**
 * This function is used to demonstrate and plot the three plots in Figure 1
 * @param a
 * @param b
 * @param c
 * @param endTime
 * @param filePath
 */
void fig1Examples(int a, int b, int c, double endTime, const std::string& filePath) {
    std::vector<stochastic::Agent> agentVec;
    std::vector<stochastic::Reaction> reactionVec;

    auto A = stochastic::Agent("A", a);
    auto B = stochastic::Agent("B", b);
    auto C = stochastic::Agent("C", c);

    agentVec.push_back(A);
    agentVec.push_back(B);
    agentVec.push_back(C);

    reactionVec.emplace_back(A + C >>= B + C, 0.001);

    stochastic::Visualizer::generateNetworkGraph(reactionVec, "C:/Users/jonas/AAU/P8/sp/SPexam/Exam/graphs/fig1NetworkGraph.dot");

    stochastic::Monitor monitor = stochastic::Algorithm::simulation(reactionVec, endTime, agentVec, filePath);
}

int covid19SEIHR(int N, const std::string& filePath) {
    std::vector<stochastic::Agent> agentVec;
    std::vector<stochastic::Reaction> reactionVec;

    const auto eps = 0.0009; // initial fraction of infectious
    const auto I0 = size_t(std::round(eps*N)); // initial infectious
    const auto E0 = size_t(std::round(eps*N*15)); // initial exposed
    const auto S0 = N-I0-E0; // initial susceptible
    const auto R0 = 2.4; // basic reproductive number (initial, without lockdown etc)
    const auto alpha = 1.0 / 5.1; // incubation rate (E -> I) ~5.1 days
    const auto gamma = 1.0 / 3.1; // recovery rate (I -> R) ~3.1 days
    const auto beta = R0 * gamma; // infection/generation rate (S+I -> E+I)
    const auto P_H = 0.9e-3; // probability of hospitalization
    const auto kappa = gamma * P_H*(1.0-P_H); // hospitalization rate (I -> H)
    const auto tau = 1.0/10.12; // recovery/death rate in hospital (H -> R) ~10.12 days

    auto S = stochastic::Agent("S", S0);
    auto E = stochastic::Agent("E", E0);
    auto I = stochastic::Agent("I", I0);
    auto H = stochastic::Agent("H", 0);
    auto R = stochastic::Agent("R", 0);
    agentVec.push_back(S); //susceptible
    agentVec.push_back(E); //exposed
    agentVec.push_back(I); //infectious
    agentVec.push_back(H); //hospitalized
    agentVec.push_back(R); //removed/immune (recovered + dead)

    reactionVec.emplace_back(S+I >>= E+I, beta/N); // susceptible becomes exposed through infectious
    reactionVec.emplace_back(E >>= I, alpha); // exposed becomes infectious
    reactionVec.emplace_back(I >>= R, gamma); // infectious becomes removed
    reactionVec.emplace_back(I >>= H, kappa); // infectious becomes hospitalized
    reactionVec.emplace_back(H >>= R, tau); // hospitalized becomes removed

    stochastic::Visualizer::generateNetworkGraph(reactionVec, "C:/Users/jonas/AAU/P8/sp/SPexam/Exam/graphs/covid19NetworkGraph.dot");

    stochastic::Monitor monitor = stochastic::Algorithm::simulation(reactionVec, 100, agentVec, filePath);
    int peak = monitor.estimatePeak();

    monitor.multiplyH();
    monitor.fileStream(filePath);

    return peak;
}

/**
 * Req. 8
 * The library supports multithreaded runs of its functions, by utilizing futures and async.
 * In this function, we run the Covid 19 example on 20 threads, and calculate the the average peak of
 * hospitalized agents
 */
void covid19on20Threads (const std::string &filePath) {
    double peakMean = 0;

    std::future<int> cov1 = std::async(covid19SEIHR, 10000, filePath);
    std::future<int> cov2 = std::async(covid19SEIHR, 10000, filePath);
    std::future<int> cov3 = std::async(covid19SEIHR, 10000, filePath);
    std::future<int> cov4 = std::async(covid19SEIHR, 10000, filePath);
    std::future<int> cov5 = std::async(covid19SEIHR, 10000, filePath);
    std::future<int> cov6 = std::async(covid19SEIHR, 10000, filePath);
    std::future<int> cov7 = std::async(covid19SEIHR, 10000, filePath);
    std::future<int> cov8 = std::async(covid19SEIHR, 10000, filePath);
    std::future<int> cov9 = std::async(covid19SEIHR, 10000, filePath);
    std::future<int> cov10 = std::async(covid19SEIHR, 10000, filePath);
    std::future<int> cov11 = std::async(covid19SEIHR, 10000, filePath);
    std::future<int> cov12 = std::async(covid19SEIHR, 10000, filePath);
    std::future<int> cov13 = std::async(covid19SEIHR, 10000, filePath);
    std::future<int> cov14 = std::async(covid19SEIHR, 10000, filePath);
    std::future<int> cov15 = std::async(covid19SEIHR, 10000, filePath);
    std::future<int> cov16 = std::async(covid19SEIHR, 10000, filePath);
    std::future<int> cov17 = std::async(covid19SEIHR, 10000, filePath);
    std::future<int> cov18 = std::async(covid19SEIHR, 10000, filePath);
    std::future<int> cov19 = std::async(covid19SEIHR, 10000, filePath);
    std::future<int> cov20 = std::async(covid19SEIHR, 10000, filePath);

    peakMean += cov1.get() + cov2.get() + cov3.get() + cov4.get() + cov5.get() +
            cov6.get() + cov7.get() + cov8.get() + cov9.get() + cov10.get() +
            cov11.get() + cov12.get() + cov13.get() + cov14.get() + cov15.get() +
            cov16.get() + cov17.get() + cov18.get() + cov19.get() + cov20.get();
    std::cout << "Average estimated peak of 20 covid19 simulations: " + std::to_string(peakMean/20) << std::endl;
}
