SHELL := /bin/sh

PRESET ?= desktop
LANGUAGE ?= en
SCREEN ?= WelcomeScreen
ARGS ?=

ifeq ($(shell uname -s),Darwin)
APP := build/$(PRESET)/bin/OpenCaddie.app/Contents/MacOS/OpenCaddie
else
APP := build/$(PRESET)/bin/OpenCaddie
endif

.DEFAULT_GOAL := help

.PHONY: help configure build simulation simulated-round screen sim demo test lint

help:
	@echo "OpenCaddie development commands"
	@echo ""
	@echo "  make simulation       Build and open the simulator"
	@echo "  make simulated-round  Build and open a simulated live round"
	@echo "  make screen SCREEN=StatsScreen  Open a specific simulator screen"
	@echo "  make build            Configure and build the desktop app"
	@echo "  make test             Run the desktop tests"
	@echo "  make lint             Run QML lint"
	@echo ""
	@echo "Options:"
	@echo "  LANGUAGE=nb           Run the UI in Norwegian"
	@echo "  ARGS='...'            Pass additional OpenCaddie arguments"

configure:
	cmake --preset $(PRESET)

build: configure
	cmake --build --preset $(PRESET) --parallel

simulation: build
	$(APP) --simulate --windowed --language $(LANGUAGE) $(ARGS)

simulated-round: build
	$(APP) --simulate --windowed --demo-round --language $(LANGUAGE) $(ARGS)

screen: build
	$(APP) --simulate --windowed --screen $(SCREEN) --language $(LANGUAGE) $(ARGS)

sim: simulation

demo: simulated-round

test: build
	ctest --preset $(PRESET) --output-on-failure

lint: build
	cmake --build build/$(PRESET) --target opencaddie_qmllint
