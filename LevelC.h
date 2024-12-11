#pragma once
// LevelC.h
#include "Scene.h"

class LevelC : public Scene {
public:
    int ENEMY_COUNT = 2;

    bool survive_level = false;
    bool got_diddled = false;

    ~LevelC();

    bool const get_survive_level() const { return survive_level; }
    bool const get_got_diddled() const { return got_diddled; }

    void set_survived() { survive_level = true; }

    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram* program) override;
};
