#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <string>

class Route;  // Forward declaration

const int NUM_CITIES = 10;
const int POPULATION_SIZE = 90;
const int MAX_GENERATIONS = 100;
const double MUTATION_RATE = 0.1;
const double EVAPORATION_RATE = 0.5;
const double ALPHA = 1.0;  // Pheromone importance
const double BETA = 5.0;   // Distance importance
const double Q = 100.0;    // Pheromone contribution

std::random_device rd;
std::mt19937 gen(rd());

std::vector<std::vector<double>> distances(NUM_CITIES, std::vector<double>(NUM_CITIES));
std::vector<std::vector<double>> traffic(NUM_CITIES, std::vector<double>(NUM_CITIES));
std::vector<std::vector<double>> pheromones(NUM_CITIES, std::vector<double>(NUM_CITIES));

class Route {
private:
    std::vector<int> route;
    double distance;

    double calculateDistance() {
        double totalDistance = 0.0;
        for (int i = 0; i < NUM_CITIES - 1; i++) {
            totalDistance += distances[route[i]][route[i + 1]] * traffic[route[i]][route[i + 1]];
        }
        return totalDistance;
    }

public:
    Route(const std::vector<int>& r) : route(r) {
        distance = calculateDistance();
    }

    const std::vector<int>& getRoute() const { return route; }
    std::vector<int>& getRoute() { return route; }
    double getDistance() const { return distance; }

    double getDistanceWithoutTraffic() const {
        double totalDistance = 0.0;
        for (int i = 0; i < NUM_CITIES - 1; i++) {
            totalDistance += distances[route[i]][route[i + 1]];
        }
        return totalDistance;
    }

    void recalculateDistance() {
        distance = calculateDistance();
    }

    std::string toString() const {
        std::string result = "[";
        for (size_t i = 0; i < route.size(); ++i) {
            result += std::to_string(route[i]);
            if (i < route.size() - 1) result += ", ";
        }
        result += "]";
        return result;
    }
}; //Represents a Path Between Cities

void initialize() {
    std::vector<std::vector<double>> realDistances = {
        {0, 10, 15, 20, 25, 30, 35, 40, 45, 50},
        {10, 0, 35, 25, 30, 20, 15, 10, 30, 25},
        {15, 35, 0, 30, 20, 25, 30, 35, 40, 45},
        {20, 25, 30, 0, 15, 10, 20, 30, 35, 40},
        {25, 30, 20, 15, 0, 10, 15, 20, 30, 35},
        {30, 20, 25, 10, 10, 0, 20, 25, 30, 35},
        {35, 15, 30, 20, 15, 20, 0, 15, 25, 30},
        {40, 10, 35, 30, 20, 25, 15, 0, 15, 20},
        {45, 30, 40, 35, 30, 30, 25, 15, 0, 15},
        {50, 25, 45, 40, 35, 35, 30, 20, 15, 0}
    };

    // Initialize traffic levels
    std::vector<std::vector<double>> realTraffic = {
        {1.0, 1.2, 1.5, 1.0, 1.3, 1.1, 1.4, 1.2, 1.3, 1.0},
        {1.2, 1.0, 1.4, 1.1, 1.3, 1.2, 1.5, 1.3, 1.4, 1.1},
        {1.5, 1.4, 1.0, 1.3, 1.2, 1.4, 1.6, 1.4, 1.5, 1.2},
        {1.0, 1.1, 1.3, 1.0, 1.1, 1.2, 1.3, 1.1, 1.2, 1.3},
        {1.3, 1.3, 1.2, 1.1, 1.0, 1.2, 1.4, 1.3, 1.2, 1.1},
        {1.1, 1.2, 1.4, 1.2, 1.2, 1.0, 1.5, 1.3, 1.4, 1.2},
        {1.4, 1.5, 1.6, 1.3, 1.4, 1.5, 1.0, 1.4, 1.5, 1.4},
        {1.2, 1.3, 1.4, 1.1, 1.3, 1.3, 1.4, 1.0, 1.2, 1.3},
        {1.3, 1.4, 1.5, 1.2, 1.2, 1.4, 1.5, 1.2, 1.0, 1.2},
        {1.0, 1.1, 1.2, 1.3, 1.1, 1.2, 1.4, 1.3, 1.2, 1.0}
    };

    distances = realDistances;
    traffic = realTraffic;
    std::fill(pheromones.begin(), pheromones.end(), std::vector<double>(NUM_CITIES, 1.0));
    //sets each row of pheromones matrix to a vector filled with 0.1 
}

std::vector<Route> initializePopulation() {
    std::vector<Route> population;
    for (int i = 0; i < POPULATION_SIZE; i++) {
        std::vector<int> route(NUM_CITIES);
        std::iota(route.begin(), route.end(), 0);  // Fill with 0 to NUM_CITIES-1
        std::shuffle(route.begin(), route.end(), gen);
        population.emplace_back(route);
    }
    return population;
} 

Route selectParent(const std::vector<Route>& population) {
    std::uniform_int_distribution<> dis(0, POPULATION_SIZE - 1);
    return population[dis(gen)];
}

Route crossover(const Route& parent1, const Route& parent2) {
    std::vector<int> childRoute(NUM_CITIES, -1);
    std::uniform_int_distribution<> dis(0, NUM_CITIES - 1);
    int start = dis(gen);//rendomly role the dices
    int end = dis(gen);
    if (start > end) std::swap(start, end);

    // Copy segment from parent1
    for (int i = start; i <= end; i++) {
        childRoute[i] = parent1.getRoute()[i];
    }

    // Fill remaining positions from parent2
    int currentIndex = 0;
    for (int i = 0; i < NUM_CITIES; i++) {
        int currentGene = parent2.getRoute()[i];
        if (std::find(childRoute.begin(), childRoute.end(), currentGene) == childRoute.end()) {
            while (childRoute[currentIndex] != -1) {
                currentIndex++;
            }
            childRoute[currentIndex] = currentGene;
        }
    }

    return Route(childRoute);
}

void mutate(Route& route) {//here random swapsill happens
    std::uniform_real_distribution<> dis(0.0, 1.0);
    if (dis(gen) < MUTATION_RATE) {
        std::uniform_int_distribution<> posDis(0, NUM_CITIES - 1);
        int index1 = posDis(gen);
        int index2 = posDis(gen);
        std::swap(route.getRoute()[index1], route.getRoute()[index2]);
        route.recalculateDistance();
    }
}

void refineWithACO(Route& route) {
    std::uniform_real_distribution<> dis(0.0, 1.0);
    for (int i = 0; i < NUM_CITIES - 1; i++) {
        int currentCity = route.getRoute()[i];
        int nextCity = route.getRoute()[i + 1];

        double probability = std::pow(pheromones[currentCity][nextCity], ALPHA) *
            std::pow(1.0 / (distances[currentCity][nextCity] * traffic[currentCity][nextCity]), BETA);

        if (dis(gen) < probability) {
            for (int j = i + 1; j < NUM_CITIES; j++) {
                int potentialNextCity = route.getRoute()[j];
                if (pheromones[currentCity][potentialNextCity] > pheromones[currentCity][nextCity]) {
                    std::swap(route.getRoute()[i + 1], route.getRoute()[j]);//The Probability Check: It looks at the path 1 → 2. It calculates the cost using the Current Traffic (from your sliders) and the Pheromones (past memory).
                    nextCity = potentialNextCity;
                }
            }
        }
    }
    route.recalculateDistance();
}

void updatePheromones(const std::vector<Route>& population) {
    // Evaporation
    for (auto& row : pheromones) {
        for (double& p : row) {
            p *= (1 - EVAPORATION_RATE);
        }
    }

    // Add new pheromones
    for (const auto& route : population) {
        double contribution = Q / route.getDistance();
        const auto& routeVec = route.getRoute();
        for (int i = 0; i < NUM_CITIES - 1; i++) {
            int currentCity = routeVec[i];
            int nextCity = routeVec[i + 1];
            pheromones[currentCity][nextCity] += contribution;
        }
    }
}

Route findBestRoute(const std::vector<Route>& population) {
    return *std::min_element(population.begin(), population.end(),
        [](const Route& a, const Route& b) { return a.getDistance() < b.getDistance(); });
}

Route findShortestRoute(const std::vector<Route>& population) {
    return *std::min_element(population.begin(), population.end(),
        [](const Route& a, const Route& b) { return a.getDistanceWithoutTraffic() < b.getDistanceWithoutTraffic(); });
}

void printTable(const Route& bestRoute, const Route& shortestRoute, const Route& bestRouteWithTraffic) {
    const int columnWidth = 30;
    
    std::cout << std::left 
              << std::setw(columnWidth) << "Metric"
              << std::setw(columnWidth) << "Value"
              << std::setw(columnWidth) << "Details" << std::endl;
    
    std::cout << std::setw(columnWidth) << "------"
              << std::setw(columnWidth) << "------"
              << std::setw(columnWidth) << "-------" << std::endl;
    
    std::cout << std::setw(columnWidth) << "Best Distance Route"
              << std::setw(columnWidth) << std::fixed << std::setprecision(2) << bestRoute.getDistance()
              << std::setw(columnWidth) << bestRoute.toString() << std::endl;
              
    std::cout << std::setw(columnWidth) << "Shortest Route"
              << std::setw(columnWidth) << shortestRoute.getDistanceWithoutTraffic()
              << std::setw(columnWidth) << shortestRoute.toString() << std::endl;
              
    std::cout << std::setw(columnWidth) << "Best Route with Traffic"
              << std::setw(columnWidth) << bestRouteWithTraffic.getDistance()
              << std::setw(columnWidth) << bestRouteWithTraffic.toString() << std::endl;
}

int main() {
    initialize();
    auto population = initializePopulation();///intialise and auto like array of class

    Route bestRoute = population[0];
    Route shortestRoute = population[0];
    Route bestRouteWithTraffic = population[0];
    for (int generation = 0; generation < MAX_GENERATIONS; generation++) {
        std::vector<Route> newPopulation;
        newPopulation.reserve(POPULATION_SIZE);// it will reseervse the size of arry of class
        for (int i = 0; i < POPULATION_SIZE; i++) {

            Route parent1 = selectParent(population);
            Route parent2 = selectParent(population);

            Route child = crossover(parent1, parent2);

            mutate(child);
            

            refineWithACO(child);// this is where ACO is in use

            newPopulation.push_back(child);// now we get one optimum path push _back it into the newpoplation 
        }

        population = std::move(newPopulation);//What it does: It takes the 90 brand-new children you just created (newPopulation) and makes them the current population. The old parents are completely removed from memory.
        updatePheromones(population);//This is the Ant Colony Optimization (ACO) part. This is how the "ants" tell the next generation which roads were the fastest.


        Route currentBestRoute = findBestRoute(population);
        Route currentShortestRoute = findShortestRoute(population);
        Route currentBestRouteWithTraffic = findBestRoute(population);

        if (currentBestRoute.getDistance() < bestRoute.getDistance()) {
            bestRoute = currentBestRoute;
        }

        if (currentShortestRoute.getDistanceWithoutTraffic() < shortestRoute.getDistanceWithoutTraffic()) {
            shortestRoute = currentShortestRoute;
        }

        if (currentBestRouteWithTraffic.getDistance() < bestRouteWithTraffic.getDistance()) {
            bestRouteWithTraffic = currentBestRouteWithTraffic;
        }

        std::cout << "Generation " << generation << ":" << std::endl;
        printTable(bestRoute, shortestRoute, bestRouteWithTraffic);
    }

    std::cout << "-----------------------------------------------------------------------" << std::endl;
    std::cout << "Final Results:" << std::endl;
    printTable(bestRoute, shortestRoute, bestRouteWithTraffic);

    return 0;
}
//ALPHA (1.0) is the "History factor" (how much we trust the pheromones/ants). BETA (5.0) is the "Heuristic factor" (how much we trust the actual distance and traffic). Since BETA is higher, my ants are more "Greedy"—they prioritize current traffic over past memory.
//Dijkstra's Algorithm is a "Greedy" algorithm. It works great for finding the shortest path from Point A to Point B on a fixed map.
