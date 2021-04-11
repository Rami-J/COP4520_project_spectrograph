------------------------------------- Performance Analysis Code -------------------------------------

Build/Run Steps using QtCreator/command-line:

Windows:
   1. Open the project file in QtCreator (File -> Open File or Project)
      located in COP4520_project_spectrograph/experimental/experimental.pro

   2. Configure the project to build for Desktop Qt 5.15.2 MSVC2019 64bit

   3. You can change the output of the built files by clicking on the Projects wrench icon in
      QtCreator on the left. Then click Desktop Qt 5.15.2 MSVC2019 64bit -> Build -> Build directory.
      Repeat this for debug, release, and profile configurations if needed.

   4. Now, with the build settings correctly configured, click on the build hammer icon on the
      bottom-left corner.

   5. Click on the green run icon to execute the analysis code, with the output appearing in the
      terminal window.

Linux:
   1. Same steps as Windows above using QtCreator and make/qmake as the build tool instead of MSVC

	OR

   2. Using the command-line cd into COP4520_project_spectrograph/experimental and run:

	qmake && make
	./COP4520_project_spectrograph
	
All sample audio files (located in experimental/audio) were generated as WAV
files via Audacity's tone generator and are used to gauge the performance of the
sequential/parallel DFT/FFT algorithms.
