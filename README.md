# AmdZeroRpm

This application switches off the "zero RPM" mode when you open up a video file, or play a game.

This prevents the GPU fans from constantly spinning up and down.

Radeon profiles were too buggy for my taste, so I wrote an even buggier alternative. It has one thing going for it — it uses the most efficient process monitoring API available and was written to be as light on resources as my (very limited) C++ knowledge permitted.

## Usage

Install the MSVC build tools, build the project, and run it.

It will create a configuration file in your home directory and will open it in a text editor. Follow the instructions in the file.

