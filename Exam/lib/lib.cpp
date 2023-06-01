//
// Created by jonas on 31-05-2023.
//

#include <random>
#include <iostream>
#include <algorithm>
#include "lib.h"

using namespace stochastic;

std::vector<stochastic::Agent> stochastic::Agent::operator+ (const stochastic::Agent &rhs) const {
    std::vector<stochastic::Agent> agents;
    agents.push_back(*this);
    agents.push_back(rhs);
    return agents;
}

stochastic::Reaction stochastic::operator>>=(const Agent &lhs, const Agent &rhs) {
    auto reaction = stochastic::Reaction();
    std::string out;

    out += lhs.name;
//    reaction.rightHandSide.push_back(lhs);
    reaction.agentVec.push_back(lhs);
    reaction.actionVec.emplace_back("increment");

    out += " -> ";

    out += rhs.name;
//    reaction.rightHandSide.push_back(rhs);
    reaction.agentVec.push_back(rhs);
    reaction.actionVec.emplace_back("increment");

    reaction.leftHandSide = std::vector<Agent> {lhs};
    reaction.rightHandSide = std::vector<Agent> {rhs};
    reaction.out = out;
    return reaction;
}

stochastic::Reaction stochastic::operator>>=(const Agent &lhs, const std::vector<Agent> &rhs) {
    auto reaction = stochastic::Reaction();
    std::string out;

    out += lhs.name;
//    reaction.rightHandSide.push_back(lhs);
    reaction.agentVec.push_back(lhs);
    reaction.actionVec.emplace_back("increment");

    out += " -> ";

    for (int i = 0; i < rhs.size(); ++i) {
        if (i > 0) {
            out +=  " + ";
        }
        out += rhs[i].name;
//        reaction.rightHandSide.push_back(rhs[i]);
        reaction.agentVec.push_back(rhs[i]);
        reaction.actionVec.emplace_back("increment");
    }

    reaction.leftHandSide = std::vector<Agent> {lhs};
    reaction.rightHandSide = rhs;
    reaction.out = out;
    return reaction;
}

stochastic::Reaction stochastic::operator>>=(const std::vector<Agent> &lhs, const Agent &rhs) {
    auto reaction = stochastic::Reaction();
    std::string out;
    for (int i = 0; i < lhs.size(); ++i) {
        if (i > 0) {
            out +=  " + ";
        }
        out += lhs[i].name;
        //reaction.leftHandSide.push_back(lhs[i]);
        reaction.agentVec.push_back(lhs[i]);
        reaction.actionVec.emplace_back("decrement");
    }

    out += " -> ";

    out += rhs.name;
//    reaction.rightHandSide.push_back(rhs);
    reaction.agentVec.push_back(rhs);
    reaction.actionVec.emplace_back("increment");

    reaction.leftHandSide = lhs;
    reaction.rightHandSide = std::vector<Agent> {rhs};
    reaction.out = out;
    return reaction;
}

stochastic::Reaction stochastic::operator>>=(const std::vector<Agent> &lhs, const std::vector<Agent> &rhs) {
    auto reaction = stochastic::Reaction();
    std::string out;
    for (int i = 0; i < lhs.size(); ++i) {
        if (i > 0) {
            out +=  " + ";
        }
        out += lhs[i].name;
        //reaction.leftHandSide.push_back(lhs[i]);
        reaction.agentVec.push_back(lhs[i]);
        reaction.actionVec.emplace_back("decrement");
    }

    out += " -> ";

    for (int i = 0; i < rhs.size(); ++i) {
        if (i > 0) {
            out +=  " + ";
        }
        out += rhs[i].name;
        //reaction.rightHandSide.push_back(rhs[i]);
        reaction.agentVec.push_back(rhs[i]);
        reaction.actionVec.emplace_back("increment");
    }

    reaction.leftHandSide = lhs;
    reaction.rightHandSide = rhs;
    reaction.out = out;
    return reaction;
}
stochastic::Reaction stochastic::operator>>=(const Agent &lhs, const std::string &env) {
    auto reaction = stochastic::Reaction();
    std::string out;

    out += lhs.name;
    reaction.agentVec.push_back(lhs);
    reaction.actionVec.emplace_back("increment");

    out += " -> " + env;

    reaction.leftHandSide = std::vector<Agent> {lhs};
    reaction.out = out;
    return reaction;
}

double stochastic::Algorithm::computeDelay(stochastic::Reaction &r, stochastic::SymbolTable<Agent>& table) {
    std::random_device rd;
    std::mt19937 gen(rd());

    double lambdaK = 1.0;
    for (auto &agent : r.leftHandSide) {
        lambdaK *= table.get(agent.name).amount;
    }
    lambdaK *= r.lambdaRate;

    std::exponential_distribution<double> expDist(lambdaK);
    return expDist(gen);
}

bool stochastic::Algorithm::amountChecker(const stochastic::Reaction& reaction, stochastic::SymbolTable<Agent>& table) {
    for (auto &agent : reaction.leftHandSide) {
        if (!(agent.amount <= table.get(agent.name).amount)) {
            return false;
        }
    }
    return true;
}

stochastic::Monitor stochastic::Algorithm::simulation(
        std::vector<stochastic::Reaction> &reactionVec,
        double endTime,
        stochastic::SymbolTable<Agent> table) {
    double t = 0.0;
    Monitor monitor;

    while (t <= endTime) {
        //table.PrintAll();
        stochastic::Reaction minDelayRec = stochastic::Reaction();
        for (auto &reaction: reactionVec) {
            reaction.delay = stochastic::Algorithm::computeDelay(reaction, table);

            if (reaction.delay < minDelayRec.delay) {
                minDelayRec = reaction;
            }
        }
        t += minDelayRec.delay;
//        std::cout << "--------------------------" << std::endl;
        if (all_of(minDelayRec.leftHandSide.begin(), minDelayRec.leftHandSide.end(),
                        [&table](const auto &agent) {return table.get(agent.name).amount > 0;})) {
            for (stochastic::Agent& r : minDelayRec.leftHandSide) {
                auto a = table.get(r.name);
                a.amount -= 1;
//                r.amount -= 1;
                table.insert(r.name, a);
            }
            for (stochastic::Agent& p : minDelayRec.rightHandSide) {
                auto a = table.get(p.name);
                a.amount += 1;
//                p.amount += 1;
                table.insert(p.name, a);
            }
        }
        //TODO: print/save/monitor state
        monitor.insert(t, table);

//        std::cout << "=======================" << std::endl;
//        std::cout << "Reaction: " + minDelayRec.out + " // Delay: " + std::to_string(minDelayRec.delay) + " t: " + std::to_string(t) << std::endl;
//        std::cout << "=======================" << std::endl;
    }

    return monitor;
}