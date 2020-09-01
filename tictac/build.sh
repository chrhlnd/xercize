#!/bin/bash

set -eu
set -o pipefail

g++ --std=c++2a ./tictac.cpp -o ./tictac
