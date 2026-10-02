#include "Simulation.h"

#include <algorithm>
#include <cmath>

namespace {

constexpr float pi = 3.14159265358979323846f;

} // namespace

const std::array<Simulation::Color, Simulation::maximumSpecies> Simulation::palette {{
    {0.20f, 0.82f, 1.00f},
    {1.00f, 0.30f, 0.38f},
    {1.00f, 0.82f, 0.18f},
    {0.28f, 1.00f, 0.52f},
    {0.70f, 0.36f, 1.00f},
    {1.00f, 0.48f, 0.16f},
    {0.28f, 0.46f, 1.00f},
    {1.00f, 0.30f, 0.76f}
}};

Simulation::Simulation()
    : agents(static_cast<std::size_t>(defaultAgentCount)),
      displayPixels(static_cast<std::size_t>(width * height), 0xFF000000u),
      randomEngine(std::random_device{}()),
      unitDistribution(0.0f, 1.0f),
      attractionEnabled(true),
      repulsionEnabled(false),
      movementSpeed(defaultMovementSpeed),
      diffusion(defaultDiffusion),
      evaporation(defaultEvaporation),
      activeSpeciesCount(1) {
    const std::size_t fieldSize = static_cast<std::size_t>(width * height);
    const Color white {1.0f, 1.0f, 1.0f};

    for (int species = 0; species < maximumSpecies; ++species) {
        trails[species].assign(fieldSize, 0.0f);
        nextTrails[species].assign(fieldSize, 0.0f);
        speciesColors[species] = white;
        targetSpeciesColors[species] = white;
    }

    reset();
}

void Simulation::update() {
    updateSpeciesColors();

    for (Agent& agent : agents) {
        steerAgent(agent);
        moveAgent(agent);
    }

    diffuseAndEvaporate();
}

void Simulation::reset() {
    for (int species = 0; species < maximumSpecies; ++species) {
        std::fill(trails[species].begin(), trails[species].end(), 0.0f);
        std::fill(nextTrails[species].begin(), nextTrails[species].end(), 0.0f);
    }

    std::fill(displayPixels.begin(), displayPixels.end(), 0xFF000000u);

    for (std::size_t index = 0; index < agents.size(); ++index) {
        initializeAgent(agents[index], index);
    }
}

bool& Simulation::attraction() {
    return attractionEnabled;
}

bool& Simulation::repulsion() {
    return repulsionEnabled;
}

float& Simulation::speed() {
    return movementSpeed;
}

float& Simulation::diffusionRate() {
    return diffusion;
}

float& Simulation::evaporationRate() {
    return evaporation;
}

int Simulation::agentCount() const {
    return static_cast<int>(agents.size());
}

void Simulation::setAgentCount(int count) {
    const int newCount = std::clamp(count, 1, maximumAgentCount);
    const std::size_t previousSize = agents.size();
    
    agents.resize(static_cast<std::size_t>(newCount));

    for (std::size_t index = previousSize; index < agents.size(); ++index) {
        initializeAgent(agents[index], index);
    }

    distributeSpecies();
}

int Simulation::speciesCount() const {
    return activeSpeciesCount;
}

void Simulation::setSpeciesCount(int count) {
    const int newCount = std::clamp(count, 1, maximumSpecies);

    if (newCount == activeSpeciesCount)
        return;

    const int previousCount = activeSpeciesCount;
    activeSpeciesCount = newCount;

    distributeSpecies();

    const Color white {1.0f, 1.0f, 1.0f};

    for (int species = 0; species < maximumSpecies; ++species) {
        if (species >= previousCount && species < activeSpeciesCount) {
            speciesColors[species] = white;
        }

        targetSpeciesColors[species] = activeSpeciesCount > 1
            && species < activeSpeciesCount
                ? palette[species]
                : white;

        if (species >= activeSpeciesCount) {
            std::fill(trails[species].begin(), trails[species].end(), 0.0f);
            std::fill(nextTrails[species].begin(), nextTrails[species].end(), 0.0f);
        }
    }
}

const std::vector<std::uint32_t>& Simulation::pixels() {
    updateDisplayPixels();
    return displayPixels;
}

float Simulation::sense(const Agent& agent, float angleOffset) const {
    const float direction = agent.angle + angleOffset;

    const int centerX = static_cast<int>(
        agent.x + std::cos(direction) * sensorDistance
    );

    const int centerY = static_cast<int>(
        agent.y + std::sin(direction) * sensorDistance
    );

    float value = 0.0f;

    if (attractionEnabled) {
        value += sampleTrail(agent.species, centerX, centerY);
    }

    if (repulsionEnabled) {
        if (activeSpeciesCount == 1) {
            value -= sampleTrail(agent.species, centerX, centerY);
        } else {
            for (int species = 0; species < activeSpeciesCount; ++species) {
                if (species != agent.species) {
                    value -= sampleTrail(species, centerX, centerY);
                }
            }
        }
    }

    return value;
}

float Simulation::sampleTrail(int species, int centerX, int centerY) const {
    float value = 0.0f;

    for (int offsetY = -1; offsetY <= 1; ++offsetY) {
        for (int offsetX = -1; offsetX <= 1; ++offsetX) {
            const int x = centerX + offsetX;
            const int y = centerY + offsetY;

            if (isInside(x, y)) {
                value += trails[species][static_cast<std::size_t>(y * width + x)];
            }
        }
    }

    return value;
}

void Simulation::steerAgent(Agent& agent) {
    if (attractionEnabled || repulsionEnabled) {
        const float forward = sense(agent, 0.0f);
        const float left = sense(agent, -sensorAngle);
        const float right = sense(agent, sensorAngle);

        if (forward < left && forward < right) {
            const float direction = unitDistribution(randomEngine) < 0.5f ? -1.0f : 1.0f;
            agent.angle += direction * turnSpeed;
        } else if (left > right) {
            agent.angle -= turnSpeed;
        } else if (right > left) {
            agent.angle += turnSpeed;
        }
    }

    agent.angle += (unitDistribution(randomEngine) - 0.5f) * randomTurn;
    agent.angle = normalizeAngle(agent.angle);
}

void Simulation::moveAgent(Agent& agent) {
    const int stepCount = std::max(
        1,
        static_cast<int>(std::ceil(movementSpeed / maximumMovementStep))
    );

    const float stepDistance = movementSpeed / static_cast<float>(stepCount);

    for (int step = 0; step < stepCount; ++step) {
        float nextX = agent.x + std::cos(agent.angle) * stepDistance;

        if (nextX < 0.0f || nextX >= static_cast<float>(width)) {
            agent.angle = normalizeAngle(pi - agent.angle);
            nextX = agent.x + std::cos(agent.angle) * stepDistance;
        }

        float nextY = agent.y + std::sin(agent.angle) * stepDistance;

        if (nextY < 0.0f || nextY >= static_cast<float>(height)) {
            agent.angle = normalizeAngle(-agent.angle);
            nextY = agent.y + std::sin(agent.angle) * stepDistance;
        }

        agent.x = std::clamp(
            nextX,
            0.0f,
            std::nextafter(static_cast<float>(width), 0.0f)
        );

        agent.y = std::clamp(
            nextY,
            0.0f,
            std::nextafter(static_cast<float>(height), 0.0f)
        );

        depositTrail(agent);
    }
}

void Simulation::depositTrail(const Agent& agent) {
    const int x = static_cast<int>(agent.x);
    const int y = static_cast<int>(agent.y);

    float& trailValue = trails[agent.species][static_cast<std::size_t>(y * width + x)];
    
    trailValue = std::min(1.0f, trailValue + depositAmount);
}

void Simulation::diffuseAndEvaporate() {
    for (int species = 0; species < activeSpeciesCount; ++species) {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                float sum = 0.0f;
                int sampleCount = 0;

                for (int offsetY = -1; offsetY <= 1; ++offsetY) {
                    for (int offsetX = -1; offsetX <= 1; ++offsetX) {
                        const int sampleX = x + offsetX;
                        const int sampleY = y + offsetY;

                        if (!isInside(sampleX, sampleY))
                            continue;

                        sum += trails[species][static_cast<std::size_t>(
                            sampleY * width + sampleX
                        )];

                        ++sampleCount;
                    }
                }

                const std::size_t index = static_cast<std::size_t>(y * width + x);
                const float blurred = sum / static_cast<float>(sampleCount);
                const float mixed = trails[species][index] + (blurred - trails[species][index]) * diffusion;

                nextTrails[species][index] = std::max(0.0f, mixed - evaporation);
            }
        }

        trails[species].swap(nextTrails[species]);
    }
}

void Simulation::updateSpeciesColors() {
    for (int species = 0; species < maximumSpecies; ++species) {
        speciesColors[species].red = approach(
            speciesColors[species].red,
            targetSpeciesColors[species].red
        );

        speciesColors[species].green = approach(
            speciesColors[species].green,
            targetSpeciesColors[species].green
        );

        speciesColors[species].blue = approach(
            speciesColors[species].blue,
            targetSpeciesColors[species].blue
        );
    }
}

void Simulation::updateDisplayPixels() {
    for (std::size_t index = 0; index < displayPixels.size(); ++index) {
        float red = 0.0f;
        float green = 0.0f;
        float blue = 0.0f;

        for (int species = 0; species < activeSpeciesCount; ++species) {
            const float intensity = std::pow(
                std::clamp(trails[species][index], 0.0f, 1.0f),
                0.65f
            );
            red += intensity * speciesColors[species].red;
            green += intensity * speciesColors[species].green;
            blue += intensity * speciesColors[species].blue;
        }

        const auto redChannel = static_cast<std::uint32_t>(
            std::clamp(red, 0.0f, 1.0f) * 255.0f
        );

        const auto greenChannel = static_cast<std::uint32_t>(
            std::clamp(green, 0.0f, 1.0f) * 255.0f
        );
        
        const auto blueChannel = static_cast<std::uint32_t>(
            std::clamp(blue, 0.0f, 1.0f) * 255.0f
        );

        displayPixels[index] = 0xFF000000u
            | (redChannel << 16u)
            | (greenChannel << 8u)
            | blueChannel;
    }

    for (const Agent& agent : agents) {
        const int x = static_cast<int>(agent.x);
        const int y = static_cast<int>(agent.y);
        const Color& color = speciesColors[agent.species];
        const auto red = static_cast<std::uint32_t>(color.red * 255.0f);
        const auto green = static_cast<std::uint32_t>(color.green * 255.0f);
        const auto blue = static_cast<std::uint32_t>(color.blue * 255.0f);
        displayPixels[static_cast<std::size_t>(y * width + x)] = 0xFF000000u
            | (red << 16u)
            | (green << 8u)
            | blue;
    }
}

void Simulation::distributeSpecies() {
    for (std::size_t index = 0; index < agents.size(); ++index) {
        agents[index].species = static_cast<int>(index) % activeSpeciesCount;
    }
}

void Simulation::initializeAgent(Agent& agent, std::size_t index) {
    agent.x = unitDistribution(randomEngine) * std::nextafter(
        static_cast<float>(width),
        0.0f
    );
    agent.y = unitDistribution(randomEngine) * std::nextafter(
        static_cast<float>(height),
        0.0f
    );
    agent.angle = unitDistribution(randomEngine) * 2.0f * pi;
    agent.species = static_cast<int>(index) % activeSpeciesCount;
}

bool Simulation::isInside(int x, int y) {
    return x >= 0 && x < width && y >= 0 && y < height;
}

float Simulation::normalizeAngle(float angle) {
    angle = std::fmod(angle, 2.0f * pi);

    if (angle < 0.0f) {
        angle += 2.0f * pi;
    }

    return angle;
}

float Simulation::approach(float current, float target) {
    return current + (target - current) * colorTransitionSpeed;
}
