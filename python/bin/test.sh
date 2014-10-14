#!/bin/sh

SLURP_API_DIR=$(dirname $0)/..
PYTHONPATH=$PYTHONPATH:$SLURP_API_DIR

nosetests -s --with-spec --spec-color $SLURP_API_DIR/test
