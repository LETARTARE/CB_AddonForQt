# AddOnForQt-3.4.7 April 24, 2022 (Win32-64, Linux-64)

Plugin to compile QT applications with 'Code::Blocks', but without 'Code:Block' patch.

**Versions for all 'Code::Block, sdk >= 2.4.0, svn >= 12142'**

**Last: sdk = 2.17.0, svn = 12782 -> ...**

It uses a log 'Prebuild log' for all posts plugin.

1- Overview

    It is a plugin to compile C++ Qt5 applications using 'Code::Blocks', in a 
	very similar way as projects 'wxWidgets'.
    For now it works on 'Win32, Win64 and Linux-64'.

2- Features

    It does not use the 'Makefile'

    This generates additional files the project by the Qt works
	'moc (.exe)', 'uic (.exe)', 'rcc (.exe)'  in a single directory in the 
	active target :'adding/targetname'

    1- it detects meta-objects 'Q_OBJECT' and 'QGADGETS' in files
        '*.h, *.hpp, *.cpp' and generates files 'moc_xxxxxx.cpp'

    2- it detected in file '*.cpp' files included by
         '#include "moc_xxxx.cpp' or '#include" yyyyyy.moc'
      to mark those files included 'no compile' and 'no link'

    3- detects forms of files '*.ui' to generate files 'ui_filename.h'
      to mark those files included 'no compile' and 'no link'

    4- detects resource files '*.qrc' to generate files 'qrc_filename.cpp'

3- 'AddOnForQt' contains :

	1- a plugin project for Win32 : 'AddOnForQt-wx31.cbp' using the directory 'src'
	2- a plugin project for Win64 : 'AddOnForQt-wx31-64.cbp' using the directory 'src'
	3- a plugin project for Linux : 'AddOnForQt-wx30-unix.cbp' using the directory 'src'
	4- a plugin project for Linux : 'AddOnForQt-wx31-unix.cbp' using the directory 'src'
	5- a directory 'CodeBlocks' containing wizards and templates files 'cbqt5'


4- Installation to 'Code::Blocks 12800' (sdk-2.17.0)

	1- first install wizards 'cbqt5',
	2- then with 'C::B-12800' compile and install the plugin 'AddOnForQt' 
        : change the local variable 'cb = $(#sdk2170)' in agreement with your 
        global 'sdk2170' which should identify sources of 'Code::Blocks-sdk2170' (>= C:B-12782 )

5- Using

    1- create a new project by 'cbqt5',

    2- compile and run (if executable).


Menu for Qt

![Menu](https://github.com/LETARTARE/CB_AddonForQt/blob/Images/3.5/MenuAddOn.png "MenuAddonForQt")

Popup menu on project

![Popup](https://github.com/LETARTARE/CB_AddonForQt/blob/Images/3.5/PopUpAddOn.png "PopUpAddonForQt")

Window log 

![LogAddonForQt](https://github.com/LETARTARE/CB_AddonForQt/blob/Images/3.5/LogAddon.png "LogAddonForQt")
		
