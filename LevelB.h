#pragma once
// LevelB.h
#include "Scene.h"

class LevelB : public Scene {
public:
    int ENEMY_COUNT = 2;

    bool survive_level = false;

    ~LevelB();

    bool const get_survive_level() const { return survive_level; }


    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram* program) override;
};