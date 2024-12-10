#pragma once
// LevelC.h
#include "Scene.h"

class LevelC : public Scene {
public:
    int ENEMY_COUNT = 3;

    bool survive_level = false;

    ~LevelC();

    bool const get_survive_level() const { return survive_level; }
    void set_survived() { survive_level = true; }

    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram* program) override;
};
