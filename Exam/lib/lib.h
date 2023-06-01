//
// Created by jonas on 31-05-2023.
//

#ifndef EXAM_LIB_H
#define EXAM_LIB_H

#include <string>
#include <vector>
#include <map>
#include <cfloat>
#include <fstream>

namespace stochastic {
    class Reaction;

    class Agent {
    public:
        std::string name;
        int amount = 0;

        Agent() = default;

        Agent(const std::string& name, int amount) {
            this->name = name;
            this->amount = amount;
        }

        std::vector<Agent> operator+(const Agent &other) const;
        friend stochastic::Reaction operator >>= (const Agent &lhs, const Agent &rhs);
        friend stochastic::Reaction operator >>= (const std::vector<Agent> &lhs, const Agent &rhs);
        friend stochastic::Reaction operator >>= (const Agent &lhs, const std::vector<Agent> &rhs);
        friend stochastic::Reaction operator >>= (const std::vector<Agent> &lhs, const std::vector<Agent> &rhs);
        friend stochastic::Reaction operator>>=(const Agent &lhs,  const std::string &env);

    };

    class Reaction {
    public:
        std::vector<stochastic::Agent> agentVec;
        std::vector<std::string> actionVec;

        std::vector<Agent> leftHandSide;
        std::vector<Agent> rightHandSide;

        double lambdaRate;
        double delay = DBL_MAX;

        std::string out;

        Reaction() = default;

        Reaction(Reaction const &reaction, double lambdaRate) {
            *this = reaction;
            this->lambdaRate = lambdaRate;
        }
    };

    template <typename T>
    class SymbolTable {
        std::map<std::string, T> table;

    public:
        void insert (const std::string &key, const T &value) {
            table[key] = value;
        }

        void update (const std::string &key, const T &value) {
            auto previousValue = get(key);
            auto newValue = previousValue + value;
            table[key] = newValue;
        }

        T& get (const std::string &key) {
            auto it = table.find(key);
            if (it != table.end()) {
                return it->second;
            }
            else {
                throw std::invalid_argument("Does not exist");
            }
        }

        bool contains (const std::string &key) const {
            return table.count(key) != 0;
        }

        void remove (const std::string &key) {
            table.erase(key);
        }

        void PrintAll () {
            for (auto val : table) {
                std::cout << "Name: " << val.first << " Amount: " << val.second.amount << std::endl;
            }
            std::cout << "--------------------------" << std::endl;
        }

        std::map<std::string, T> fetchTable() {
            return table;
        }

    };

     class Monitor {
         std::map<double, std::vector<SymbolTable<Agent>>> monitorMap;
     public:
         Monitor() = default;

         void insert (double &time, SymbolTable<Agent> &table) {
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

         void fileStream(std::string fileName) {
             std::ofstream outFile ("D:/Jonas/AAU/P8/Exam/graphs/" + fileName);

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

         void estimatePeak() {
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
         }

         void multiplyH() {
             for (auto &mapping : monitorMap) {
                 for (auto &table: mapping.second) {
                     auto H = table.get("H");
                     H.amount *= 1000;
                     table.insert("H", H);
                 }
             }
         }
     };

    class Algorithm {
    public:
//        Algorithm() = default;
        static double computeDelay(stochastic::Reaction &r, stochastic::SymbolTable<Agent>& table);
        static bool amountChecker(const stochastic::Reaction& reaction, stochastic::SymbolTable<Agent>& table);
        static stochastic::Monitor simulation(std::vector<stochastic::Reaction> &reactionVec, double endTime, stochastic::SymbolTable<Agent> table);
    };

}

#endif //EXAM_LIB_H
