#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <numeric>
#include <climits>

struct Fraction {
    long long num;
    long long den;
    
    Fraction(long long n = 0, long long d = 1) : num(n), den(d) {
        if (den < 0) {
            num = -num;
            den = -den;
        }
        simplify();
    }
    
    void simplify() {
        if (num == 0) {
            den = 1;
            return;
        }
        long long g = std::gcd(std::abs(num), std::abs(den));
        num /= g;
        den /= g;
    }
    
    Fraction operator-(const Fraction& other) const {
        return Fraction(num * other.den - other.num * den, den * other.den);
    }
    
    Fraction operator+(const Fraction& other) const {
        return Fraction(num * other.den + other.num * den, den * other.den);
    }
    
    Fraction operator*(const Fraction& other) const {
        return Fraction(num * other.num, den * other.den);
    }
    
    Fraction sqrt() const {
        // For exact rational square root, we need both num and den to be perfect squares
        // This is a simplified approach - we'll return an approximate fraction
        double val = static_cast<double>(num) / static_cast<double>(den);
        double sqrtVal = std::sqrt(val);
        // Convert back to fraction with limited precision to avoid huge numbers
        return doubleToFraction(sqrtVal, 10000);
    }
    
    static Fraction doubleToFraction(double value, int maxDenominator = 10000) {
        if (value == 0) return Fraction(0, 1);
        
        long long sign = (value < 0) ? -1 : 1;
        value = std::abs(value);
        
        long long intPart = static_cast<long long>(value);
        double fracPart = value - intPart;
        
        if (fracPart < 1e-6) return Fraction(sign * intPart, 1);
        
        // Continued fraction method
        long long n0 = 0, d0 = 1, n1 = 1, d1 = 0;
        double x = fracPart;
        
        for (int i = 0; i < 20 && d1 < maxDenominator; ++i) {
            long long a = static_cast<long long>(x);
            long long n2 = a * n1 + n0;
            long long d2 = a * d1 + d0;
            
            if (d2 > maxDenominator) break;
            
            n0 = n1; d0 = d1;
            n1 = n2; d1 = d2;
            
            if (std::abs(x - a) < 1e-6) break;
            x = 1.0 / (x - a);
        }
        
        return Fraction(sign * (intPart * d1 + n1), d1);
    }
};

struct City {
    int id;
    Fraction x;
    Fraction y;
};

class TSPDataGenerator {
private:
    std::vector<City> cities;
    std::vector<std::vector<long long>> distanceMatrix;
    long long scaleFactor;
    
    Fraction abs(const Fraction& f) const {
        return Fraction(std::abs(f.num), f.den);
    }
    
    Fraction manhattanDistance(const City& a, const City& b) const {
        Fraction dx = a.x - b.x;
        Fraction dy = a.y - b.y;
        return abs(dx) + abs(dy);
    }
    
    long long lcm(long long a, long long b) {
        if (a == 0 || b == 0) return 1;
        long long g = std::gcd(std::abs(a), std::abs(b));
        // Check for potential overflow
        if (a / g > LLONG_MAX / b) {
            return LLONG_MAX;  // Return max value if overflow would occur
        }
        return std::abs((a / g) * b);
    }
    
    void calculateDistanceMatrix() {
        int n = cities.size();
        std::vector<std::vector<Fraction>> fracMatrix(n, std::vector<Fraction>(n));
        
        // Calculate all distances as fractions (using Manhattan distance for exact rationals)
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                if (i == j) {
                    fracMatrix[i][j] = Fraction(0, 1);
                } else {
                    fracMatrix[i][j] = manhattanDistance(cities[i], cities[j]);
                }
            }
        }
        
        // Find LCM of all denominators with overflow protection
        scaleFactor = 1;
        for (int i = 0; i < n && scaleFactor < LLONG_MAX / 1000; ++i) {
            for (int j = 0; j < n && scaleFactor < LLONG_MAX / 1000; ++j) {
                if (i != j) {
                    long long newLcm = lcm(scaleFactor, fracMatrix[i][j].den);
                    if (newLcm < scaleFactor) break;  // Overflow detected
                    scaleFactor = newLcm;
                }
            }
        }
        
        // Convert to integers by multiplying by LCM
        distanceMatrix.resize(n, std::vector<long long>(n));
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                distanceMatrix[i][j] = (fracMatrix[i][j].num * scaleFactor) / fracMatrix[i][j].den;
            }
        }
        
        std::cout << "Scale factor (LCM of denominators): " << scaleFactor << std::endl;
    }
    
public:
    void generate(int numCities, int maxCoord = 100) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> disNum(0, maxCoord);
        std::uniform_int_distribution<> disDen(1, 10);
        
        cities.clear();
        
        for (int i = 0; i < numCities; ++i) {
            City city;
            city.id = i;
            city.x = Fraction(disNum(gen), disDen(gen));
            city.y = Fraction(disNum(gen), disDen(gen));
            cities.push_back(city);
        }
        
        calculateDistanceMatrix();
    }
    
    const std::vector<City>& getCities() const {
        return cities;
    }
    
    const std::vector<std::vector<long long>>& getDistanceMatrix() const {
        return distanceMatrix;
    }
    
    long long getDistance(int i, int j) const {
        return distanceMatrix[i][j];
    }
    
    void printDistanceMatrix() const {
        int n = cities.size();
        std::cout << "\nInteger Distance Matrix (scaled by LCM):\n";
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                std::cout << distanceMatrix[i][j] << "\t";
            }
            std::cout << "\n";
        }
    }
};

int main() {
    TSPDataGenerator generator;
    
    int numCities;
    std::cout << "Enter number of cities: ";
    std::cin >> numCities;
    
    generator.generate(numCities);
    
    std::cout << "Generated TSP data for " << numCities << " cities" << std::endl;

    generator.printDistanceMatrix();
    
    return 0;
}