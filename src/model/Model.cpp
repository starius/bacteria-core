/*
 * bacteria-core, core for cellular automaton
 * Copyright (C) 2016 Pavel Dolgov
 *
 * See the LICENSE file for terms of use.
 */

#include "Model.hpp"
#include "random.hpp"

namespace Abstract {

Model::Model(
    int /*width*/,
    int /*height*/,
    int /*bacteria*/,
    int /*teams*/
) {
}

CellState Model::cellState(int x, int y) const {
    return cellState_impl(x, y);
}

int Model::getDirectionByCoordinates(int x, int y) const {
    return getDirectionByCoordinates_impl(x, y);
}

int Model::getMassByCoordinates(int x, int y) const {
    return getMassByCoordinates_impl(x, y);
}

int Model::getTeamByCoordinates(int x, int y) const {
    return getTeamByCoordinates_impl(x, y);
}

int Model::getWidth() const {
    return getWidth_impl();
}

int Model::getHeight() const {
    return getHeight_impl();
}

int Model::getBacteriaNumber(int team) const {
    return getBacteriaNumber_impl(team);
}

int Model::getInstruction(int team, int bacterium_index) const {
    return getInstruction_impl(team, bacterium_index);
}

void Model::changeMass(int team, int bacterium_index, int change) {
    return changeMass_impl(team, bacterium_index, change);
}

void Model::setInstruction(
    int team,
    int bacterium_index,
    int new_instruction
) {
    return setInstruction_impl(
        team,
        bacterium_index,
        new_instruction
    );
}

}

namespace Implementation {

static bool checkIndex(int index, int size) {
    return (index >= 0) && (index < size);
}

/* get global coordinate from horizontal and
   vertical coordinates
*/
static int getIndex(int x, int y, int width, int height) {
    bool less = ((x < 0) || (y < 0));
    bool greater = ((x >= width) || (y >= height));
    if (less || greater) {
        throw Exception("Model: index of cell in arguments "
                        "of some methods is out of range.");
    }
    int index = y * width + x;
    return index;
}

Unit::Unit(
    int mass,
    int direction,
    int team,
    int instruction
)
    : mass(mass)
    , direction(direction)
    , team(team)
    , instruction(instruction) {
}

Model::Model(
    int width,
    int height,
    int bacteria,
    int teams
)
    : Abstract::Model(width, height, bacteria, teams)
    , width_(width)
    , height_(height) {
    board_.resize(width * height);
    teams_.resize(teams);
    initializeBoard(bacteria, teams);
}

Abstract::CellState Model::cellState_impl(int x, int y) const {
    int index = getIndex(x, y, width_, height_);
    UnitPtr unit_ptr = board_[index];
    if (!unit_ptr.isNull()) {
        return Abstract::BACTERIUM;
    } else {
        return Abstract::EMPTY;
    }
}

int Model::getDirectionByCoordinates_impl(int x, int y) const {
    int index = getIndex(x, y, width_, height_);
    UnitPtr unit_ptr = board_[index];
    if (!unit_ptr.isNull()) {
        return unit_ptr->direction;
    } else {
        throw Exception("Error: Attempt to get direction of empty cell.");
    }
}

int Model::getMassByCoordinates_impl(int x, int y) const {
    int index = getIndex(x, y, width_, height_);
    UnitPtr unit_ptr = board_[index];
    if (!unit_ptr.isNull()) {
        return unit_ptr->mass;
    } else {
        throw Exception("Error: Attempt to get mass of empty cell.");
    }
}

int Model::getTeamByCoordinates_impl(int x, int y) const {
    int index = getIndex(x, y, width_, height_);
    UnitPtr unit_ptr = board_[index];
    if (!unit_ptr.isNull()) {
        return unit_ptr->team;
    } else {
        // no unit in the current cell
        throw Exception("Error: Attempt to get team of empty cell.");
    }
}

int Model::getWidth_impl() const {
    return width_;
}

int Model::getHeight_impl() const {
    return height_;
}

int Model::getBacteriaNumber_impl(int team) const {
    if (!checkIndex(team, teams_.size())) {
        throw Exception("Model: team argument of "
                        "getBacteriaNumber() method is out of "
                        "allowable range.");
    }
    return teams_[team].size();
}

int Model::getInstruction_impl(
    int team,
    int bacterium_index
) const {
    checkParams(team, bacterium_index, "getInstruction()");
    UnitPtr unit_ptr = teams_[team][bacterium_index];
    return unit_ptr->instruction;
}

void Model::changeMass_impl(
    int team,
    int bacterium_index,
    int change
) {
    if (!checkIndex(team, teams_.size())) {
        throw Exception("Model: team argument of changeMass() "
                        "method is out of allowable range.");
    }
    if (!checkIndex(bacterium_index, teams_[team].size())) {
        throw Exception("Model: bacterium_index argument of "
                        "changeMass() method is out of "
                        "allowable range.");
    }
    UnitPtr unit_ptr = teams_[team][bacterium_index];
    if (!unit_ptr.isNull()) {
        unit_ptr->mass += change;
    } else {
        throw Exception("Model: Attempt to change mass "
                        "of NULL ptr.");
    }
}

void Model::setInstruction_impl(
    int team,
    int bacterium_index,
    int new_instruction
) {
    if (!checkIndex(team, teams_.size())) {
        throw Exception("Model: team argument of setInstruction() "
                        "method is out of allowable range.");
    }
    if (!checkIndex(bacterium_index, teams_[team].size())) {
        throw Exception("Model: bacterium_index argument of "
                        "setInstruction() method is out of "
                        "allowable range.");
    }
    UnitPtr unit_ptr = teams_[team][bacterium_index];
    if (!unit_ptr.isNull()) {
        unit_ptr->instruction = new_instruction;
    } else {
        throw Exception("Model: Attempt to set instruction "
                        "for NULL ptr.");
    }
}

void Model::initializeBoard(int bacteria, int teams) {
    for (int team = 0; team < teams; team++) {
        for (int bacterium = 0; bacterium < bacteria; bacterium++) {
            tryToPlace(team);
        }
    }
}

void Model::tryToPlace(int team) {
    int x = random(width_);
    int y = random(height_);
    while (cellState(x, y) == Abstract::BACTERIUM) {
        x = random(width_);
        y = random(height_);
    }
    int direction = random(4);
    UnitPtr unit_ptr(new Unit(DEFAULT_MASS, direction, team, 0));
    teams_[team].push_back(unit_ptr);
    int index = getIndex(x, y, width_, height_);
    board_[index] = unit_ptr;
}

void Model::checkParams(
    int team,
    int bacterium_index,
    const std::string& method_name
) const {
    if (!checkIndex(team, teams_.size())) {
        throw Exception("Model: team argument of " + method_name +
                        " is out of allowable range.");
    }
    if (!checkIndex(bacterium_index, teams_[team].size())) {
        throw Exception("Model: bacterium_index argument"
                        " of " + method_name + " is out of"
                        " allowable range");
    }
    UnitPtr unit_ptr = teams_[team][bacterium_index];
    if (unit_ptr.isNull()) {
        throw Exception("Model: Attempt to call " + method_name +
                        " for NULL ptr.");
    }
}

}
