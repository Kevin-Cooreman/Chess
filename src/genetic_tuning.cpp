#include "game.hpp"
#include "engine.hpp"
#include "evaluation.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <algorithm>
#include <cmath>

using namespace std;

// Chromosome represents evaluation weights
struct Chromosome {
    double materialWeight;
    double positionWeight;
    double kingSafetyWeight;
    double pawnStructureWeight;
    double fitness;  // Win rate
    int wins;
    int losses;
    int draws;
    
    Chromosome() : fitness(0.0), wins(0), losses(0), draws(0) {}
    
    Chromosome(double m, double p, double ks, double ps) 
        : materialWeight(m), positionWeight(p), kingSafetyWeight(ks), 
          pawnStructureWeight(ps), fitness(0.0), wins(0), losses(0), draws(0) {}
};

// Custom evaluation class
class GeneticEvaluation : public Evaluation {
private:
    double materialWeight;
    double positionWeight;
    double kingSafetyWeight;
    double pawnStructureWeight;

public:
    GeneticEvaluation(const Chromosome& c) 
        : materialWeight(c.materialWeight), positionWeight(c.positionWeight), 
          kingSafetyWeight(c.kingSafetyWeight), pawnStructureWeight(c.pawnStructureWeight) {}

    double evaluate(const ChessGame& game) const {
        double evaluation = 0.0;
        evaluation += materialWeight * materialCount(game);
        evaluation += positionWeight * position(game);
        evaluation += kingSafetyWeight * kingsafety(game);
        evaluation += pawnStructureWeight * pawnStructure(game);
        return evaluation;
    }
};

// Play one game - use material count at end if no winner
string playGame(Chromosome& c1, Chromosome& c2, bool c1PlaysWhite, int depth, mt19937& gen) {
    ChessGame game;
    
    GeneticEvaluation eval1(c1);
    GeneticEvaluation eval2(c2);
    
    Engine engine1(eval1);
    Engine engine2(eval2);
    
    int moveCount = 0;
    int maxMoves = 80;  // Shorter games
    
    // Add slight depth randomization to create asymmetry
    uniform_int_distribution<> depthVariation(-1, 1);
    
    while (!game.isGameOver() && moveCount < maxMoves) {
        bool isWhiteTurn = game.isWhiteToMove();
        
        Engine* currentEngine = nullptr;
        if ((isWhiteTurn && c1PlaysWhite) || (!isWhiteTurn && !c1PlaysWhite)) {
            currentEngine = &engine1;
        } else {
            currentEngine = &engine2;
        }
        
        // Vary depth slightly each move to break determinism
        int moveDepth = max(1, depth + depthVariation(gen));
        Move bestMove = currentEngine->getBestMove(game, moveDepth);
        
        if (bestMove.startRow == -1) {
            break;
        }
        
        game.makeEngineMove(bestMove);
        moveCount++;
    }
    
    if (game.isGameOver()) {
        string result = game.getGameResult();
        
        if (result.find("White wins") != string::npos) {
            return c1PlaysWhite ? "c1" : "c2";
        } else if (result.find("Black wins") != string::npos) {
            return c1PlaysWhite ? "c2" : "c1";
        }
    }
    
    // Game timed out at 80 moves - use material count to decide winner
    double materialScore = eval1.materialCount(game);
    
    // Award win if material advantage is significant (3+ pawns)
    if (materialScore > 3.0) {  // White ahead
        return c1PlaysWhite ? "c1" : "c2";
    } else if (materialScore < -3.0) {  // Black ahead
        return c1PlaysWhite ? "c2" : "c1";
    }
    
    return "draw";
}

// Tournament to evaluate fitness
void evaluateFitness(vector<Chromosome>& population, int gamesPerMatchup, int depth, mt19937& gen) {
    // Reset fitness
    for (auto& c : population) {
        c.wins = 0;
        c.losses = 0;
        c.draws = 0;
        c.fitness = 0.0;
    }
    
    // Round-robin tournament
    for (size_t i = 0; i < population.size(); i++) {
        for (size_t j = i + 1; j < population.size(); j++) {
            for (int g = 0; g < gamesPerMatchup; g++) {
                // Game 1: i plays white
                string result1 = playGame(population[i], population[j], true, depth, gen);
                if (result1 == "c1") {
                    population[i].wins++;
                    population[j].losses++;
                } else if (result1 == "c2") {
                    population[j].wins++;
                    population[i].losses++;
                } else {
                    population[i].draws++;
                    population[j].draws++;
                }
                
                // Game 2: i plays black
                string result2 = playGame(population[i], population[j], false, depth, gen);
                if (result2 == "c1") {
                    population[i].wins++;
                    population[j].losses++;
                } else if (result2 == "c2") {
                    population[j].wins++;
                    population[i].losses++;
                } else {
                    population[i].draws++;
                    population[j].draws++;
                }
            }
        }
    }
    
    // Calculate fitness as win rate
    for (auto& c : population) {
        int totalGames = c.wins + c.losses + c.draws;
        if (totalGames > 0) {
            c.fitness = (c.wins + 0.5 * c.draws) / totalGames;
        }
    }
}

// Selection: tournament selection
Chromosome tournamentSelect(const vector<Chromosome>& population, mt19937& gen) {
    uniform_int_distribution<> dist(0, population.size() - 1);
    
    int idx1 = dist(gen);
    int idx2 = dist(gen);
    
    return (population[idx1].fitness > population[idx2].fitness) ? population[idx1] : population[idx2];
}

// Crossover: blend two chromosomes
Chromosome crossover(const Chromosome& parent1, const Chromosome& parent2, mt19937& gen) {
    uniform_real_distribution<> blend(0.0, 1.0);
    
    double alpha = blend(gen);
    
    Chromosome child;
    child.materialWeight = 1.0;  // Always keep material weight at 1.0 for reference
    child.positionWeight = alpha * parent1.positionWeight + (1 - alpha) * parent2.positionWeight;
    child.kingSafetyWeight = alpha * parent1.kingSafetyWeight + (1 - alpha) * parent2.kingSafetyWeight;
    child.pawnStructureWeight = alpha * parent1.pawnStructureWeight + (1 - alpha) * parent2.pawnStructureWeight;
    
    return child;
}

// Mutation: randomly adjust weights
void mutate(Chromosome& c, double mutationRate, mt19937& gen) {
    uniform_real_distribution<> prob(0.0, 1.0);
    normal_distribution<> adjustment(0.0, 1.0);  // Bigger mutations!
    
    // Material weight always stays at 1.0 for reference
    c.materialWeight = 1.0;
    
    if (prob(gen) < mutationRate) {
        c.positionWeight = max(0.1, c.positionWeight + adjustment(gen));
    }
    if (prob(gen) < mutationRate) {
        c.kingSafetyWeight = max(0.1, c.kingSafetyWeight + adjustment(gen));
    }
    if (prob(gen) < mutationRate) {
        c.pawnStructureWeight = max(0.01, c.pawnStructureWeight + adjustment(gen));
    }
}

int main() {
    cout << "Genetic Algorithm for Chess Engine Tuning\n";
    cout << "==========================================\n\n";
    
    // Parameters (small trial)
    int populationSize = 10;
    int generations = 10;
    int gamesPerMatchup = 2;  // Games per matchup (both colors)
    int depth = 2;  // Lower depth = more mistakes = more decisive games
    double mutationRate = 0.1;  // Keep mutation moderate for the trial
    
    cout << "Parameters:\n";
    cout << "  Population size: " << populationSize << "\n";
    cout << "  Generations: " << generations << "\n";
    cout << "  Games per matchup: " << gamesPerMatchup << " (x2 for both colors)\n";
    cout << "  Search depth: " << depth << "\n";
    cout << "  Mutation rate: " << mutationRate << "\n\n";
    
    // Calculate total games
    int gamesPerGen = (populationSize * (populationSize - 1) / 2) * gamesPerMatchup * 2;
    int totalGames = gamesPerGen * generations;
    // REALISTIC estimate: Each game has ~40 moves, each move takes time based on depth
    // depth 2: ~2s per game (40 moves × 0.05s), depth 3: ~4s per game (40 moves × 0.1s)
    // depth 4: ~20s per game (40 moves × 0.5s)
    double secondsPerGame = (depth == 2) ? 2.0 : (depth == 3) ? 4.0 : 20.0;
    double estimatedSeconds = totalGames * secondsPerGame;
    
    cout << "  Games per generation: " << gamesPerGen << "\n";
    cout << "  Total games: " << totalGames << "\n";
    cout << "  Estimated time: ~" << (int)(estimatedSeconds / 3600) << " hours " 
         << ((int)(estimatedSeconds / 60) % 60) << " min (at depth " << depth << ")\n\n";
    
    cout << "Press Enter to start...";
    cin.get();
    
    random_device rd;
    mt19937 gen(rd());
    
    // Initialize population with WIDE range of weights
    vector<Chromosome> population;
    uniform_real_distribution<> posDist(0.001, 1.0);  // Much wider!
    uniform_real_distribution<> ksDist(0.001, 1.0);   // Wider!
    uniform_real_distribution<> psDist(0.001, 1.0);  // Wider!
    
    // Add some extreme starting points for diversity (material always 1.0)
    population.push_back(Chromosome(1.0, 0.01, 0.01, 0.01));   // Balanced
    population.push_back(Chromosome(1.0, 0.05, 0.05, 0.05));   // Minimal weights
    population.push_back(Chromosome(1.0, 0.1, 0.01, 0.01));   // Position focused
    population.push_back(Chromosome(1.0, 0.01, 0.1, 0.01));   // King safety focused
    
    // Fill rest with random (material always 1.0)
    for (int i = 4; i < populationSize; i++) {
        population.push_back(Chromosome(
            1.0,  // Material weight always 1.0
            posDist(gen),
            ksDist(gen),
            psDist(gen)
        ));
    }
    
    // Evolution loop
    for (int gen_num = 0; gen_num < generations; gen_num++) {
        cout << "\n===========================================\n";
        cout << "Generation " << (gen_num + 1) << "/" << generations << "\n";
        cout << "===========================================\n";
        
        // Evaluate fitness
        cout << "Running tournament (" << gamesPerGen << " games)...\n";
        evaluateFitness(population, gamesPerMatchup, depth, gen);
        
        // Sort by fitness
        sort(population.begin(), population.end(), 
             [](const Chromosome& a, const Chromosome& b) {
                 return a.fitness > b.fitness;
             });
        
        // Display top performers
        cout << "\nTop 3 performers:\n";
        for (int i = 0; i < min(3, (int)population.size()); i++) {
            cout << (i + 1) << ". Fitness: " << (population[i].fitness * 100) << "% "
                 << "(" << population[i].wins << "W-" << population[i].losses << "L-" 
                 << population[i].draws << "D)\n";
            cout << "   Weights: M=" << population[i].materialWeight 
                 << ", P=" << population[i].positionWeight
                 << ", KS=" << population[i].kingSafetyWeight
                 << ", PS=" << population[i].pawnStructureWeight << "\n";
        }
        
        // Create next generation
        vector<Chromosome> newPopulation;
        
        // Elitism: keep top 2
        newPopulation.push_back(population[0]);
        newPopulation.push_back(population[1]);
        newPopulation.push_back(population[2]);
        newPopulation.push_back(population[3]);
        newPopulation.push_back(population[4]);
        newPopulation.push_back(population[5]);
        
        // Generate offspring
        while (newPopulation.size() < (size_t)populationSize) {
            Chromosome parent1 = tournamentSelect(population, gen);
            Chromosome parent2 = tournamentSelect(population, gen);
            
            Chromosome child = crossover(parent1, parent2, gen);
            mutate(child, mutationRate, gen);
            
            newPopulation.push_back(child);
        }
        
        population = newPopulation;
    }
    
    // Final evaluation
    cout << "\n\n===========================================\n";
    cout << "FINAL EVALUATION\n";
    cout << "===========================================\n";
    evaluateFitness(population, 2, depth, gen);  // More games for final eval
    
    sort(population.begin(), population.end(), 
         [](const Chromosome& a, const Chromosome& b) {
             return a.fitness > b.fitness;
         });
    
    cout << "\nBEST EVOLVED WEIGHTS:\n";
    cout << "---------------------\n";
    cout << "Fitness: " << (population[0].fitness * 100) << "%\n";
    cout << "Record: " << population[0].wins << "W-" << population[0].losses << "L-" 
         << population[0].draws << "D\n\n";
    cout << "Weights:\n";
    cout << "  materialWeight = " << population[0].materialWeight << "\n";
    cout << "  positionWeight = " << population[0].positionWeight << "\n";
    cout << "  kingSafetyWeight = " << population[0].kingSafetyWeight << "\n";
    cout << "  pawnStructureWeight = " << population[0].pawnStructureWeight << "\n\n";
    
    cout << "Copy these values into evaluation.cpp:\n";
    cout << "evaluation += " << population[0].materialWeight << "*materialCount(game);\n";
    cout << "evaluation += " << population[0].positionWeight << "*position(game);\n";
    cout << "evaluation += " << population[0].kingSafetyWeight << "*kingsafety(game);\n";
    cout << "evaluation += " << population[0].pawnStructureWeight << "*pawnStructure(game);\n";
    
    return 0;
}
