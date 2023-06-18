//
// Created by jonas on 31-05-2023.
//

#include <random>
#include <iostream>
#include <algorithm>
#include "lib.h"

//using namespace stochastic;
namespace stochastic {

    /** =================================
     *  AGENT CLASS
     *  =================================
     */

    /*
     * Req. 1
     * Operator overload (+)
     */
    std::vector<stochastic::Agent> stochastic::Agent::operator+(const stochastic::Agent &rhs) const {
        std::vector<stochastic::Agent> agents;
        agents.push_back(*this);
        agents.push_back(rhs);
        return agents;
    }

    /*
     * Req. 1
     * Operator overload (>>=)
     */
    stochastic::Reaction operator>>=(const Agent &lhs, const Agent &rhs) {
        auto reaction = stochastic::Reaction();
        std::string out;

        out += lhs.name;
        reaction.agentVec.push_back(lhs);
        reaction.actionVec.emplace_back("increment");

        out += " -> ";

        out += rhs.name;
        reaction.agentVec.push_back(rhs);
        reaction.actionVec.emplace_back("increment");

        reaction.leftHandSide = std::vector<Agent>{lhs};
        reaction.rightHandSide = std::vector<Agent>{rhs};
        reaction.out = out;
        return reaction;
    }

    /*
     * Req. 1
     * Operator overload (>>=)
     */
    stochastic::Reaction operator>>=(const Agent &lhs, const std::vector<Agent> &rhs) {
        auto reaction = stochastic::Reaction();
        std::string out;

        out += lhs.name;
        reaction.agentVec.push_back(lhs);
        reaction.actionVec.emplace_back("increment");

        out += " -> ";

        for (int i = 0; i < rhs.size(); ++i) {
            if (i > 0) {
                out += " + ";
            }
            out += rhs[i].name;
            reaction.agentVec.push_back(rhs[i]);
            reaction.actionVec.emplace_back("increment");
        }

        reaction.leftHandSide = std::vector<Agent>{lhs};
        reaction.rightHandSide = rhs;
        reaction.out = out;
        return reaction;
    }

    /*
     * Req. 1
     * Operator overload (>>=)
     */
    stochastic::Reaction operator>>=(const std::vector<Agent> &lhs, const Agent &rhs) {
        auto reaction = stochastic::Reaction();
        std::string out;
        for (int i = 0; i < lhs.size(); ++i) {
            if (i > 0) {
                out += " + ";
            }
            out += lhs[i].name;
            reaction.agentVec.push_back(lhs[i]);
            reaction.actionVec.emplace_back("decrement");
        }

        out += " -> ";

        out += rhs.name;
        reaction.agentVec.push_back(rhs);
        reaction.actionVec.emplace_back("increment");

        reaction.leftHandSide = lhs;
        reaction.rightHandSide = std::vector<Agent>{rhs};
        reaction.out = out;
        return reaction;
    }

    /*
     * Req. 1
     * Operator overload (>>=)
     */
    stochastic::Reaction operator>>=(const std::vector<Agent> &lhs, const std::vector<Agent> &rhs) {
        auto reaction = stochastic::Reaction();
        std::string out;
        for (int i = 0; i < lhs.size(); ++i) {
            if (i > 0) {
                out += " + ";
            }
            out += lhs[i].name;
            reaction.agentVec.push_back(lhs[i]);
            reaction.actionVec.emplace_back("decrement");
        }

        out += " -> ";

        for (int i = 0; i < rhs.size(); ++i) {
            if (i > 0) {
                out += " + ";
            }
            out += rhs[i].name;
            reaction.agentVec.push_back(rhs[i]);
            reaction.actionVec.emplace_back("increment");
        }

        reaction.leftHandSide = lhs;
        reaction.rightHandSide = rhs;
        reaction.out = out;
        return reaction;
    }

    /*
     * Req. 1
     * Operator overload (>>=)
     * Used for reactants that decay into environment
     */
    stochastic::Reaction operator>>=(const Agent &lhs, const std::string &env) {
        auto reaction = stochastic::Reaction();
        std::string out;

        out += lhs.name;
        reaction.agentVec.push_back(lhs);
        reaction.actionVec.emplace_back("increment");

        out += " -> " + env;

        reaction.leftHandSide = std::vector<Agent>{lhs};
        reaction.out = out;
        return reaction;
    }

    /** =================================
     *  MONITOR CLASS
     *  =================================
     */

    void stochastic::Monitor::insert (double &time, SymbolTable<Agent> &table) {
        auto it = monitorMap.find(time);

        if (it != monitorMap.end()) {
            auto tableVec = it -> second;
            tableVec.push_back(table);
            monitorMap[time] = tableVec;
        }
        else {
            monitorMap[time] = std::vector<SymbolTable<Agent>> {table};
        }
    }

    void stochastic::Monitor::fileStream(const std::string& filePath) {
        std::ofstream outFile (filePath);

        if (outFile.is_open()) {
            outFile << "time,agentname,agentamount" << std::endl;

            for (auto &mapping : monitorMap) {
                for (auto &table : mapping.second) {
                    for (auto &agent : table.fetchTable()) {
                        outFile << std::to_string(mapping.first) + "," + agent.first + "," + std::to_string(agent.second.amount) << std::endl;
                    }
                }
            }
            outFile.close();
        }
    }

    int stochastic::Monitor::estimatePeak() {
        auto peak = 0;
        for (auto &mapping : monitorMap) {
            for (auto &table: mapping.second) {
                auto H = table.get("H");
                if (H.amount > peak) {
                    peak = H.amount;
                }
            }
        }

        std::cout << "Estimated peak of hospitalized agents: " + std::to_string(peak) << std::endl;
        return peak;
    }

    void stochastic::Monitor::multiplyH() {
        for (auto &mapping : monitorMap) {
            for (auto &table: mapping.second) {
                auto H = table.get("H");
                H.amount *= 1000;
                table.insert("H", H);
            }
        }
    }

    /** =================================
     *  ALGORITHM CLASS
     *  =================================
     */

    double stochastic::Algorithm::computeDelay(stochastic::Reaction &r, stochastic::SymbolTable<Agent> &table) {
        std::random_device rd;
        std::mt19937 gen(rd());

        double lambdaK = 1.0;
        for (auto &agent: r.leftHandSide) {
            lambdaK *= table.get(agent.name).amount;
        }
        lambdaK *= r.lambdaRate;

        std::exponential_distribution<double> expDist(lambdaK);
        return expDist(gen);
    }

    bool stochastic::Algorithm::amountChecker(const stochastic::Reaction &reaction, stochastic::SymbolTable<Agent> &table) {
        for (auto &agent: reaction.leftHandSide) {
            if (!(agent.amount <= table.get(agent.name).amount)) {
                return false;
            }
        }
        return true;
    }

    /**
     * Req. 4
     * This is the stochastic simulation
     * @param reactionVec Used as the set of reactions to compute
     * @param endTime Defines the end time
     * @param agentVec Used to create our symbol table for
     * @param filePath The path to where we want to save the state monitor
     * @return
     */
    stochastic::Monitor stochastic::Algorithm::simulation(
            std::vector<stochastic::Reaction> &reactionVec,
            double endTime,
            std::vector<stochastic::Agent> &agentVec,
            const std::string& filePath)
    {
        double t = 0.0;
        Monitor monitor;
        stochastic::SymbolTable<stochastic::Agent> table = stochastic::SymbolTable<stochastic::Agent>::generateSymbolTable(agentVec);

        while (t <= endTime) {
            stochastic::Reaction minDelayRec = stochastic::Reaction();
            for (auto &reaction: reactionVec) {
                reaction.delay = stochastic::Algorithm::computeDelay(reaction, table);

                if (reaction.delay < minDelayRec.delay) {
                    minDelayRec = reaction;
                }
            }

            t += minDelayRec.delay;

            if (all_of(minDelayRec.leftHandSide.begin(), minDelayRec.leftHandSide.end(),
                       [&table](const auto &agent) { return table.get(agent.name).amount > 0; })) {
                for (stochastic::Agent &r: minDelayRec.leftHandSide) {
                    auto a = table.get(r.name);
                    a.amount -= 1;
                    table.insert(r.name, a);
                }
                for (stochastic::Agent &p: minDelayRec.rightHandSide) {
                    auto a = table.get(p.name);
                    a.amount += 1;
                    table.insert(p.name, a);
                }
            }
            //TODO: print/save/monitor state
            monitor.insert(t, table);
        }

        monitor.fileStream(filePath);
        return monitor;
    }

    /** =================================
     *  VISUALIZER CLASS
     *  =================================
     */

    /**
     * Req. 2
     * This function pretty prints the reaction network
     * @param reactionVec
     */
    void stochastic::Visualizer::prettyPrintReactions(const std::vector<Reaction> &reactionVec) {
        std::cout << "=======================================" << std::endl;
        for (auto &reaction: reactionVec) {
            std::cout << "Reaction: " + reaction.out + " || Rate: " + std::to_string(reaction.lambdaRate) << std::endl;
        }
        std::cout << "=======================================" << std::endl;
    }

    /**
     * Req. 2
     * This function generates a network graph of any given set of reactions
     * @param reactionVec
     * @param filePath
     */
    void stochastic::Visualizer::generateNetworkGraph(const std::vector<Reaction> &reactionVec, const std::string &filePath) {
        std::ofstream outfile(filePath);

        std::vector<std::string> nameVec;

        std::string numNode = "N";

        outfile << "digraph {" << std::endl;

        for (auto &reaction: reactionVec) {
            for (auto &lhs: reaction.leftHandSide) {
                if (!(std::find(nameVec.begin(), nameVec.end(), lhs.name) != nameVec.end())) {
                    outfile << lhs.name + " [shape=box];" << std::endl;
                    nameVec.push_back(lhs.name);
                }
                outfile << lhs.name + " -> " + numNode << std::endl;
            }
            for (auto &rhs: reaction.rightHandSide) {
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
}