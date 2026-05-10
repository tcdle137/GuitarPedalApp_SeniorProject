How to install:

  Make sure you have the following downloaded on your computer:
    Visual Studio (make sure to select "Desktop Development with C++" during install)
      Download: https://visualstudio.microsoft.com/downloads/
    CMake (in a cmd window, paste the following command to install. Allow the installation.)
      winget install kitware.cmake

  Download my repository 
    1. Navigate to "https://github.com/tcdle137/GuitarPedalApp_SeniorProject"
    2. Click the green "<> Code" button and click "Download ZIP".
    3. Navigate to wherever you downloaded the repository and unzip the folder. (NOTE: Do NOT forget where these files are!)

  JUCE and cmake-includes Submodules (these files are linked submodules from other repositories, so you will have to download them separately)
    1. Navigate to the /modules/ folder in my repository.
    2. Open the "JUCE @ [random numbers and letters]" folder in a new tab.
    3. Click the green "<> Code" button and click "Download ZIP".
    4. Navigate to wherever you downloaded the repository and unzip the folder.
    5. Navigate into the folder you just downloaded, until you reach the level with the "CMakeLists.txt" file for JUCE.
        *** This should be the first level with more than one folder in it.
        *** For cmake-includes, navigate in until there are no more folders to go into.
    6. Go back one folder and rename the folder to "JUCE" or "cmake-includes" depending on which you are downloading. Then, copy the folder. 
    7. Navigate to wherever you downloaded my original GitHub repository and navigate to the /modules/ folder.
        *** The file path should be [wherever you saved the repo]/GuitarPedalApp_SeniorProject-main/modules/ or look very similar.
    8. Within the /modules/ folder, paste the folder that you have copied to your clipboard from step 6.
        *** You should see 2 folder with the same names, but there shouldn't be any files in it. 
              When pasting the folder ALLOW the pasted f4older to replace the current one.
    9. Repeat steps 1-8; however, for step 2, open the "cmake-includes @ [random numbers and letters]" folder instead.
    10. You can delete all downloaded files EXCEPT for the main GuitarPedalApp_SeniorProject project folder, as they should be copied into this folder now.

  Once everything in installed:
    1. Copy the path of the parent folder directory of my repository.
        *** You can do so by navigating to my repository on your computer, right clicking the project folder and selecting "Copy as path".
    2. Open a new cmd window (press the windows key, then search "cmd").
    3. type in "cd " then paste the file path you just copied. Hit enter to run the command. 
        *** the command should look similar to "cd C\[file path to where you saved my project]\GuitarPedalApp_SeniorProject"
    4. Now run the following commands one at a time to build the project:
        cmake -B build
        cmake --build build --config release
    5. Now go back to the main project folder. You should now see a new folder named "build".
    6. Navigate through the following path to find the executable file:
        /build/AudioPluginExample_artefacts/release/Standalone/
    7. You should see a file called "Le FX Guitar Pedal Simulator.exe". Double click this file to run the project.
    8. SUGGESTED: right click the .exe file and select "Create shortcut" (may be under "Show more options"), then move the shortcut to your desktop. 

If everything works, when you run the program you should have a function Guitar Pedal Simulator.
You may need to double check your Audio I/O setting by going to the top left, clicking "Options" then "Audio/MIDI Settings..."
    *** Under Audio Device type, Windows Audio (Exclusive Mode) or Windows Audio (Low Latency) are recommended.
        Audio interface options may be more ideal, depending on access to given hardware.



Uninstall:
  1. Delete the "GuitarPedalApp_SeniorProject" folder (with all the files in it).
  2. Navigate to the Windows "Add or remove programs" in settings (or searchable by pressing the Windows key).
  3. Search and uninstall the following:
       CMake
       Visual Studio (You may opt to keep this as it is just Microsoft's IDE for coding)
