# Spectrograph Visualizer (COP 4520)

## Prerequisites
To build this project you need to have Qt 5.15.2, which can be downloaded
for free via the open-source version at https://www.qt.io/download-open-source or via your package manager.

## Project Description
Project for COP 4520 to visualize the spectrum of audio files and their Fourier Transforms while evaluating the performance of using threading. The project uses the Qt C++ framework for the GUI elements of the program.

## Building and Running
### Linux
To build and run on Linux using qmake and make build tools, navigate
to the project's root directory and execute:
```console
qmake && make
./COP4520_project_spectrograph
```

### Windows
To build and run on Windows using Visual Studio open the solution Spectrograph.sln and download the Qt VS Tools extension. Then hit the green run button at the top to build and run.

The project can also be built and run using QtCreator just by opening the COP4520_project_spectrograph.pro file and configuring the build settings.