#ifndef SIMULATION_H
#define SIMULATION_H

#include <array>
#include <cstdint>
#include <random>
#include <vector>

class Simulation {
public:
    static constexpr int width = 960;
    static constexpr int height = 540;
    static constexpr int defaultAgentCount = 5000;
    static constexpr int maximumAgentCount = 10000000;
    static constexpr int maximumSpecies = 8;

    Simulation();

    void update();
    void reset();

    bool& attraction();
    bool& repulsion();
    float& speed();
    float& diffusionRate();
    float& evaporationRate();
    int agentCount() const;
    void setAgentCount(int count);
    int speciesCount() const;
    void setSpeciesCount(int count);

    const std::vector<std::uint32_t>& pixels();

private:
    struct Agent {
        float x;
        float y;
        float angle;
        int species;
    };

    struct Color {
        float red;
        float green;
        float blue;
    };

    static constexpr float sensorDistance = 9.0f;
    static constexpr float sensorAngle = 0.55f;
    static constexpr float defaultMovementSpeed = 1.5f;
    static constexpr float maximumMovementStep = 0.5f;
    static constexpr float turnSpeed = 0.35f;
    static constexpr float randomTurn = 0.06f;
    static constexpr float defaultDiffusion = 0.1f;
    static constexpr float defaultEvaporation = 0.05f;
    static constexpr float depositAmount = 0.75f;
    static constexpr float colorTransitionSpeed = 0.045f;
    static const std::array<Color, maximumSpecies> palette;

    std::vector<Agent> agents;
    std::array<std::vector<float>, maximumSpecies> trails;
    std::array<std::vector<float>, maximumSpecies> nextTrails;
    std::array<Color, maximumSpecies> speciesColors;
    std::array<Color, maximumSpecies> targetSpeciesColors;
    std::vector<std::uint32_t> displayPixels;
    std::mt19937 randomEngine;
    std::uniform_real_distribution<float> unitDistribution;
    bool attractionEnabled;
    bool repulsionEnabled;
    float movementSpeed;
    float diffusion;
    float evaporation;
    int activeSpeciesCount;

    float sense(const Agent& agent, float angleOffset) const;
    float sampleTrail(int species, int centerX, int centerY) const;
    void steerAgent(Agent& agent);
    void moveAgent(Agent& agent);
    void depositTrail(const Agent& agent);
    void diffuseAndEvaporate();
    void updateSpeciesColors();
    void updateDisplayPixels();
    void distributeSpecies();
    void initializeAgent(Agent& agent, std::size_t index);

    static bool isInside(int x, int y);
    static float normalizeAngle(float angle);
    static float approach(float current, float target);
};

#endif
