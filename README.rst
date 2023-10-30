===================
fledge-iec61850-north-scheduler
===================

Fledge North Plugin that acts as an IEC 61850 server (slave)

This is a proof of concept IEC 61850 server that can be run in the sending
process of Fledge to make Fledge appear as an IEC 61850 server. It also supports iec61850 scheduling using the der-scheduling library.

Building lib61850
-----------------
To install the dependencies you can run the requirements.sh script.

If you want to install them manually :

To build IEC61850 C/C++ North plugin, you need to download lib61850 at:
https://github.com/mz-automation/libiec61850

.. code-block:: console

  $ git clone https://github.com/mz-automation/libiec61850.git
  $ cd libiec61850
  $ export LIB_IEC61850=`pwd`

As shown above, you need a $LIB_IEC61850 env var set to the source tree of the
library.

Then, you can build libiec61850 with:

.. code-block:: console

  $ cd libiec61850
  $ cmake -DBUILD_TESTS=NO -DBUILD_EXAMPLES=NO ..
  $ make
  $ sudo make install
  $ sudo ldconfig

If you want scheduling support also, you can add the der-scheduling library as follows:

.. code-block:: console

  $ mkdir -p dependencies
  $ cd dependencies
  $ echo Fetching iec61850 scheduler library
  $ git clone -b develop_mz git@bitbucket.org:mz-automation/alliander-ise-iec-61850-scheduler.git
  $ mv alliander-ise-iec-61850-scheduler der-scheduling
  $ cd der-scheduling

Build
-----


To build the iec61850 plugin, once you are in the plugin source tree you need to run:

To build a release:

.. code-block:: console

  $ mkdir build
  $ cd build
  $ cmake -DCMAKE_BUILD_TYPE=Release ..
  $ make

To build with unit tests and code coverage:

.. code-block:: console

  $ mkdir build
  $ cd build
  $ cmake -DCMAKE_BUILD_TYPE=Coverage ..
  $ make

- By default the Fledge develop package header files and libraries
  are expected to be located in /usr/include/fledge and /usr/lib/fledge
- If **FLEDGE_ROOT** env var is set and no -D options are set,
  the header files and libraries paths are pulled from the ones under the
  FLEDGE_ROOT directory.
  Please note that you must first run 'make' in the FLEDGE_ROOT directory.

You may also pass one or more of the following options to cmake to override
this default behaviour:

- **FLEDGE_SRC** sets the path of a Fledge source tree
- **FLEDGE_INCLUDE** sets the path to Fledge header files
- **FLEDGE_LIB sets** the path to Fledge libraries
- **FLEDGE_INSTALL** sets the installation path of Random plugin

NOTE:
 - The **FLEDGE_INCLUDE** option should point to a location where all the Fledge
   header files have been installed in a single directory.
 - The **FLEDGE_LIB** option should point to a location where all the Fledge
   libraries have been installed in a single directory.
 - 'make install' target is defined only when **FLEDGE_INSTALL** is set

Examples:

- no options

  $ cmake ..

- no options and FLEDGE_ROOT set

  $ export FLEDGE_ROOT=/some_fledge_setup

  $ cmake ..

- set FLEDGE_SRC

  $ cmake -DFLEDGE_SRC=/home/source/develop/Fledge  ..

- set FLEDGE_INCLUDE

  $ cmake -DFLEDGE_INCLUDE=/dev-package/include ..
- set FLEDGE_LIB

  $ cmake -DFLEDGE_LIB=/home/dev/package/lib ..
- set FLEDGE_INSTALL

  $ cmake -DFLEDGE_INSTALL=/home/source/develop/Fledge ..

  $ cmake -DFLEDGE_INSTALL=/usr/local/fledge ..


Using the plugin
----------------

As described in the Fledge documentation, you can use the plugin by adding
a service from a terminal, or from the web API.C

1 - Add the service from a terminal:

.. code-block:: console

  $ curl -sX POST http://localhost:8081/fledge/scheduled/task -d '{"name": "iec61850","plugin": "iec104","type": "north","schedule_type": 3,"schedule_day": 0,"schedule_time": 0,"schedule_repeat": 30,"schedule_enabled": true}' ; echo

Or

2) Add the service from the web GUI:

 - On the web GUI, go to the North tab
 - Click on "Add +"
 - Select iec104 and give it a name, then click on "Next"
 - Change the default settings to your settings, then click on "Next"
 - Let the "Enabled" option checked, then click on "Done"