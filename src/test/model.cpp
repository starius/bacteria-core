/*
 * bacteria-core, core for cellular automaton
 * Copyright (C) 2016 Pavel Dolgov
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/test/unit_test.hpp>

#include "Model.hpp"

static Implementation::Model* createBaseModel(
    int bacteria = 0,
    int teams = 1
) {
    Implementation::Model* model =
        Abstract::makeModel<Implementation::Model>(
            MIN_WIDTH,
            MIN_HEIGHT,
            bacteria,
            teams
        );
    return model;
}

BOOST_AUTO_TEST_CASE (height_test) {
    Implementation::Model* model = createBaseModel();
    BOOST_REQUIRE(model->getHeight() == MIN_HEIGHT);
    delete model;
}

BOOST_AUTO_TEST_CASE (width_test) {
    Implementation::Model* model = createBaseModel();
    BOOST_REQUIRE(model->getWidth() == MIN_WIDTH);
    delete model;
}

BOOST_AUTO_TEST_CASE (kill_test) {
    Implementation::Model* model = createBaseModel();
    Abstract::Point coordinates(0, 0);
    model->createNewByCoordinates(
        coordinates,
        DEFAULT_MASS,
        0,
        0,
        0
    );
    Abstract::CellState state = model->cellState(coordinates);
    BOOST_REQUIRE(state == Abstract::BACTERIUM);
    model->killByCoordinates(coordinates);
    state = model->cellState(coordinates);
    BOOST_REQUIRE(state == Abstract::EMPTY);
    delete model;
}
