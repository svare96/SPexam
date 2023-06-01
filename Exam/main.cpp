#include <iostream>
#include <fstream>
#include <algorithm>
#include <cmath>
#include <utility>

#include "./lib/lib.h"
#include <omp.h>

#define THREAD_NUM 4

using namespace stochastic;

void circadian();
void generateNetworkGraph(const std::vector<Reaction>& reactionVec);
SymbolTable<Agent> generateSymbolTable(const std::vector<Agent>& agentVec);
void fig1Examples(int a, int b, int c, double endTime, std::string fileName);
void covid19SEIHR(int N, std::string fileName);

int main() {
    omp_set_num_threads(THREAD_NUM);
    int NNJ = 589755;
    int NDK = 5822763;
#pragma omp parallel
    {
        std::cout << "Hello, World!" << std::endl;
    }
//    circadian();
//    fig1Examples(100, 0, 1, 2000, "fig1A.csv");
//    fig1Examples(100, 0, 2, 1500, "fig1B.csv");
//    fig1Examples(50, 50, 1, 2000, "fig1C.csv");
//    covid19SEIHR(10000, "covid10k.csv");
//    covid19SEIHR(NNJ, "NorthJutland.csv");
//    covid19SEIHR(NNJ, "Denmark.csv");


    return 0;
}

void circadian() {
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

    std::vector<Agent> agentVec;
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

    std::vector<Reaction> reactionVec;
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

    SymbolTable<Agent> table = generateSymbolTable(agentVec);

//    generateNetworkGraph(reactionVec);

    std::vector<Monitor> monitorVec;
    Monitor monitorAvg;
    for (int i = 0; i < 100; ++i) {
        monitorVec.push_back(stochastic::Algorithm::simulation(reactionVec, 100, table));
    }
    for (auto &monitor : monitorVec) {
        //TODO: figure out how to compute average of A, C, R of 100 trajectories
    }
//    Monitor monitor = stochastic::Algorithm::simulation(reactionVec, 100, table);
//    monitor.fileStream("circadian.csv");
}

void fig1Examples(int a, int b, int c, double endTime, std::string fileName) {
    std::vector<Agent> agentVec;
    std::vector<Reaction> reactionVec;

    auto A = Agent("A", a);
    auto B = Agent("B", b);
    auto C = Agent("C", c);

    agentVec.push_back(A);
    agentVec.push_back(B);
    agentVec.push_back(C);

    reactionVec.emplace_back(A + C >>= B + C, 0.001);

    SymbolTable<Agent> table = generateSymbolTable(agentVec);
    Monitor monitor = stochastic::Algorithm::simulation(reactionVec, endTime, table);
    monitor.fileStream(std::move(fileName));
}

void covid19SEIHR(int N, std::string fileName) {
    std::vector<Agent> agentVec;
    std::vector<Reaction> reactionVec;

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

    auto S = Agent("S", S0);
    auto E = Agent("E", E0);
    auto I = Agent("I", I0);
    auto H = Agent("H", 0);
    auto R = Agent("R", 0);
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

    SymbolTable<Agent> table = generateSymbolTable(agentVec);
    Monitor monitor = stochastic::Algorithm::simulation(reactionVec, 100, table);
    monitor.estimatePeak();
    monitor.multiplyH();
    monitor.fileStream(std::move(fileName));
}

void generateNetworkGraph(const std::vector<Reaction>& reactionVec) {
    std::ofstream outfile ("D:/Jonas/AAU/P8/Exam/graphs/networkgraph.dot");
    //outfile.open("./graphs/networkgraph.dot", std::ofstream::out | std::ofstream::trunc);

    std::vector<std::string> nameVec;

    std::string numNode = "N";

    outfile << "digraph {" << std::endl;

    for (auto &reaction : reactionVec) {
        for (auto &lhs : reaction.leftHandSide) {
            if (!(std::find(nameVec.begin(), nameVec.end(), lhs.name) != nameVec.end())) {
                outfile << lhs.name + " [shape=box];" << std::endl;
                nameVec.push_back(lhs.name);
            }
            outfile <<  lhs.name + " -> " + numNode << std::endl;
        }
        for (auto &rhs : reaction.rightHandSide) {
            if (!(std::find(nameVec.begin(), nameVec.end(), rhs.name) != nameVec.end())) {
                outfile << rhs.name + " [shape=box];" << std::endl;
                nameVec.push_back(rhs.name);
            }
            outfile << numNode + " -> " + rhs.name << std::endl;
        }
        outfile << numNode + " [label=\"" + std::to_string(reaction.lambdaRate) + "\"];";
        numNode += "1";
    }

    outfile << "}" << std::endl;
    outfile.close();
}

SymbolTable<Agent> generateSymbolTable(const std::vector<Agent>& agentVec) {
    auto symbolTable = SymbolTable<Agent>();
    for (auto &agent : agentVec) {
        symbolTable.insert(agent.name, agent);
    }
    return symbolTable;
}
