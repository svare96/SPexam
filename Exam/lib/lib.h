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

        //Req. 1
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

        double lambdaRate{};
        double delay = DBL_MAX;

        std::string out;

        Reaction() = default;

        Reaction(Reaction const &reaction, double lambdaRate) {
            *this = reaction;
            this->lambdaRate = lambdaRate;
        }
    };

    /**
     * Req. 3
     * This class allows us to store an instance of the reaction network, using Agent objects, in a symbol table
     * @tparam T
     */
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

        static SymbolTable<T> generateSymbolTable(const std::vector<T>& inputVec) {
            auto symbolTable = SymbolTable<T>();
            for (auto &input : inputVec) {
                symbolTable.insert(input.name, input);
            }
            return symbolTable;
        }

    };

    /**
     * Req. 7
     * This class lets us instantiate a state monitor consisting of several symbol tables.
     * This allows us to store our reactions over a timespan.
     */
     class Monitor {
         std::map<double, std::vector<SymbolTable<Agent>>> monitorMap;
     public:
         Monitor() = default;

         void insert (double &time, SymbolTable<Agent> &table);
         void fileStream(const std::string& fileName);
         int estimatePeak();
         void multiplyH();
     };

    class Algorithm {
    public:
        static double computeDelay(stochastic::Reaction &r, stochastic::SymbolTable<Agent>& table);
        static bool amountChecker(const stochastic::Reaction& reaction, stochastic::SymbolTable<Agent>& table);
        static stochastic::Monitor simulation(std::vector<stochastic::Reaction> &reactionVec, double endTime, std::vector<stochastic::Agent> &agentVec, const std::string& filePath);
    };

    class Visualizer {
    public:
        //Req. 2
        static void prettyPrintReactions(const std::vector<Reaction> &reactionVec);
        static void generateNetworkGraph(const std::vector<Reaction>& reactionVec, const std::string &filePath);
    };

}

#endif //EXAM_LIB_H
