# Spectrograph Visualizer (COP 4520)
https://github.com/znoble360/COP4520_project_spectrograph

## Build Requirements
<ul>
<li>For all systems:</li>

* To build this project you need to have Qt 5.15.2, which can be downloaded
for free via the open-source version at https://www.qt.io/download-open-source or via your package manager.

* You also need the Qt Charts library that is not included with the base Qt installation by default.

<li>Linux Packages:</li>

* qmake

* make

* qt5-charts (installed above)

* qt5-gstreamer

* qt5-multimedia

<li>Windows:</li>

* MSVC_2019 64-bit compiler (can be installed with Qt online installer)

* Visual Studio or QtCreator

</ul>

## Project Description
Project for COP 4520 to visualize the spectrum of audio files and their Fourier Transforms while evaluating the performance of using threading. The project uses the Qt C++ framework for the GUI elements of the program built upon one of the default examples Qt provides for their multimedia library.

## Building and Running
<ul>
<li>Linux</li>

To build and run on Linux using qmake and make build tools, navigate
to the project's root directory in your terminal and execute:
```console
qmake && make
./COP4520_project_spectrograph
```
Another option is to use QtCreator to build/run the project.


<li>Windows</li>

<ul>
<li>Visual Studio:</li>

1. To build and run on Windows using Visual Studio open the solution Spectrograph.sln and download the Qt VS Tools extension. 

2. Navigate to Extensions -> Qt VS Tools -> Qt Versions
and add the msvc2019_64 Qt version by browsing to its folder located in the Qt\5.15.2 folder.
(Example: C:\Qt\5.15.2\msvc2019_64)

3. Navigate to Extensions -> Qt VS Tools -> Qt Project Settings
and make sure that Qt Installation = msvc2019_64 and Qt Modules = charts;core;gui;multimedia;widgets

4. Then hit the green run button at the top to build the solution and run.

<li>QtCreator:</li>

1. Open the COP4520_project_spectrograph.pro file using QtCreator (File -> 
Open File or Project).

2. Configure the project to build for Desktop Qt 5.15.2 MSVC2019 64-bit in the Projects tab.

3. With the build settings configured, click on the build hammer icon on the
bottom-left corner.

4. Click on the green run icon on the bottom-left to run the program.
</ul>
</ul>