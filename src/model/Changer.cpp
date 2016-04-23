/*
 * bacteria-core, core for cellular automaton
 * Copyright (C) 2016 Pavel Dolgov
 *
 * See the LICENSE file for terms of use.
 */

#include "Changer.hpp"

namespace Abstract {

Params::Params(
    int p1,
    int p2,
    bool spec
)
    : p1(p1)
    , p2(p2)
    , spec(spec)
{
}

void Changer::clearAfterMove() {
    return clearAfterMove_impl();
}

bool Changer::endOfMove(int bacterium_index) const {
    return endOfMove_impl(bacterium_index);
}

int Changer::getBacteriaNumber() const {
    return getBacteriaNumber_impl();
}

int Changer::getTeam() const {
    return getTeam_impl();
}

int Changer::getInstruction(int bacterium_index) const {
    return getInstruction_impl(bacterium_index);
}

void Changer::eat(const Params* params, int bacterium_index) {
    return eat_impl(params, bacterium_index);
}

void Changer::go(const Params* params, int bacterium_index) {
    return go_impl(params, bacterium_index);
}

void Changer::clon(const Params* params, int bacterium_index) {
    return clon_impl(params, bacterium_index);
}

Changer::Changer(
    Model& /*model*/,
    int /*team*/,
    int /*move_number*/,
    int /*instructions*/
) {
}

}

namespace Implementation {

LogicalChanger::LogicalChanger(
    Abstract::Model& model,
    int team,
    int move_number
)
    : model_(model)
    , team_(team)
    , move_number_(move_number)
{
}

void LogicalChanger::eat(int bacterium_index) {
    model_.changeMass(team_, bacterium_index, EAT_MASS);
}

void LogicalChanger::go(int bacterium_index) {
    model_.changeMass(team_, bacterium_index, GO_MASS);
    int mass = model_.getMass(team_, bacterium_index);
    if (mass <= 0) {
        model_.kill(team_, bacterium_index);
    } else {
        Abstract::Point coordinates = nextCoordinates(bacterium_index);
        Abstract::CellState state = model_.cellState(coordinates);
        if (state == Abstract::EMPTY) {
            model_.setCoordinates(team_, bacterium_index, coordinates);
        }
    }
}

void LogicalChanger::clon(int bacterium_index) {
    model_.changeMass(team_, bacterium_index, CLON_MASS);
    int mass = model_.getMass(team_, bacterium_index);
    if (mass < 0) {
        model_.kill(team_, bacterium_index);
    } else if (mass == 0) {
        model_.kill(team_, bacterium_index);
        clonLogic(bacterium_index);
    } else {
        clonLogic(bacterium_index);
    }
}

void LogicalChanger::clonLogic(int bacterium_index) {
    Abstract::Point coordinates = nextCoordinates(bacterium_index);
    Abstract::CellState state = model_.cellState(coordinates);
    if (state == Abstract::EMPTY) {
        model_.createNewByCoordinates(
            coordinates,
            DEFAULT_CLON_MASS,
            random(4),
            team_,
            0
        );
    } else {
        model_.changeMassByCoordinates(coordinates, DEFAULT_CLON_MASS);
    }
}

Abstract::Point LogicalChanger::nextCoordinates(
    int bacterium_index
) const {
    int direction = model_.getDirection(team_, bacterium_index);
    Abstract::Point coordinates = model_.getCoordinates(
        team_,
        bacterium_index
    );
    int max_width = model_.getWidth() - 1;
    int max_height = model_.getHeight() - 1;
    if ((direction == Abstract::LEFT) &&
        (coordinates.x > 0)) {
        coordinates.x--;
    } else if ((direction == Abstract::RIGHT) &&
               (coordinates.x < max_width)) {
        coordinates.x++;
    } else if ((direction == Abstract::BACKWARD) &&
               (coordinates.y > 0)) {
        coordinates.y--;
    } else if ((direction == Abstract::FORWARD) &&
               (coordinates.y < max_height)) {
        coordinates.y++;
    }
    return coordinates;
}

RepeaterParams::RepeaterParams(
    int bacterium_index,
    int commands,
    Ints& remaining_commands_vect,
    LogicalMethod logic_function
)
    : bacterium_index(bacterium_index)
    , commands(commands)
    , remaining_commands_vect(remaining_commands_vect)
    , logic_function(logic_function)
{
}

Changer::Changer(
    Abstract::Model& model,
    int team,
    int move_number,
    int instructions
)
    : Abstract::Changer(model, team, move_number, instructions)
    , model_(model)
    , team_(team)
    , move_number_(move_number)
    , instructions_(instructions)
    , logical_changer_(model_, team_, move_number_) {
    int bacteria = model_.getBacteriaNumber(team_);
    remaining_actions_.resize(bacteria, MAX_ACTIONS);
    remaining_pseudo_actions_.resize(bacteria, MAX_PSEUDO_ACTIONS);
    completed_commands_.resize(bacteria, 0);
}

void Changer::clearAfterMove_impl() {
    for (int i = 0; i < getBacteriaNumber_impl(); i++) {
        remaining_actions_[i] = MAX_ACTIONS;
        remaining_pseudo_actions_[i] = MAX_PSEUDO_ACTIONS;
    }
}

bool Changer::endOfMove_impl(int bacterium_index) const {
    bool actions = remaining_actions_[bacterium_index] > 0;
    bool pseudo_actions = remaining_pseudo_actions_[bacterium_index] > 0;
    return !(actions && pseudo_actions);
}

int Changer::getBacteriaNumber_impl() const {
    return model_.getBacteriaNumber(team_);
}

int Changer::getTeam_impl() const {
    return team_;
}

int Changer::getInstruction_impl(int bacterium_index) const {
    return model_.getInstruction(team_, bacterium_index);
}

void Changer::eat_impl(
    const Abstract::Params* params,
    int bacterium_index
) {
    int n = 1;
    if (params->spec) {
        n = random(RANDOM_MAX_ACTIONS);
    } else if (params->p1 != -1) {
        n = checkCommandsNumber(params->p1);
    }
    RepeaterParams rp(
        bacterium_index,
        n,
        remaining_actions_,
        &LogicalChanger::eat
    );
    repeater(&rp);
}

void Changer::go_impl(
    const Abstract::Params* params,
    int bacterium_index
) {
    int n = 1;
    if (params->spec) {
        n = random(RANDOM_MAX_ACTIONS);
    } else if (params->p1 != -1) {
        n = checkCommandsNumber(params->p1);
    }
    RepeaterParams rp(
        bacterium_index,
        n,
        remaining_actions_,
        &LogicalChanger::go
    );
    repeater(&rp);
}

void Changer::clon_impl(
    const Abstract::Params* params,
    int bacterium_index
) {
    int n = 1;
    RepeaterParams rp(
        bacterium_index,
        n,
        remaining_actions_,
        &LogicalChanger::clon
    );
    repeater(&rp);
}

bool Changer::remainingActionsDecrement(
    Ints& actions_vect,
    int bacterium_index
) {
    bool less = bacterium_index < 0;
    bool greater = bacterium_index >= actions_vect.size();
    if (less || greater) {
        throw Exception("Changer: invalid bacterium index.");
    }
    actions_vect[bacterium_index]--;
    if (actions_vect[bacterium_index] < 0) {
        throw Exception("Changer: too many commands for one move.");
    }
    if (actions_vect[bacterium_index] == 0) {
        return false;
    }
    return true;
}

void Changer::updateInstruction(int index) {
    completed_commands_[index] = 0;
    int instruction = model_.getInstruction(team_, index);
    if ((instruction + 1) < instructions_) {
        model_.setInstruction(team_, index, instruction + 1);
    } else {
        model_.setInstruction(team_, index, 0);
    }
}

int Changer::checkCommandsNumber(int number) const {
    bool greater = number > MIN_COMMANDS_PER_INSTRUCTION;
    bool less = number < MAX_COMMANDS_PER_INSTRUCTION;
    if (greater && less) {
        return number;
    } else {
        throw Exception("Changer: invalid commands number.");
    }
}

void Changer::repeater(RepeaterParams* params) {
    int index = params->bacterium_index;
    int total_commands = params->commands;
    bool finished = false;
    while ((completed_commands_[index] < total_commands) &&
           !finished) {
        finished = !remainingActionsDecrement(
            params->remaining_commands_vect,
            index
        );
        completed_commands_[index]++;
        LogicalMethod method = params->logic_function;
        (logical_changer_.*method)(index);
    }
    if (completed_commands_[index] == (total_commands)) {
        updateInstruction(index);
    }
}

}
