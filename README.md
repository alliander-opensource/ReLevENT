# ReLevENT, an integration project of DER-scheduling (OSS) with FledgePower (OSS)
ReLevENT stands for "**Re**silient **Lev**erage for **E**nergy-edge **N**ative **T**echnology".
The development goal is to supply a production grade general software stack to control edge devices (like EV-chargers, heat-pumps, Home-energy-management-systems, PV-solar) to market parties that enables to interact with more actors with their own communication protocols and needs. The implementation of usable-functionality takes place step by step where the 'production grade' label is key.

# Business goals
The business goals related to energy systems and controling in the edge are:
1. Enable market parties and grid users to use all different protocols to their need (unlock the “best protocol dilimma”).
2. Improve the resilience in the edge
3. Improve the quality, compliancy and go to market for hardware suppliers.

Image to business goal 1 explains the current situation where each market party (e.g. energy supplier, charging point operator, congestion service provider, ...) and each DSO or TSO like to see their own preferred communication protocol be deployed to all devices at end customers in the edge. Everybody optimized for their own use-case, but at customers this leads to silo-ed solutions. In the right part you see the to-be situation where we want to enable a proxy respecting all optimized protocols. The proxy doesn't realize the harmonization in the protocol, but in the functionality.
![image](images/ReLevENT_From-To.png)

For a more detailed understanding, also see [the Use-Cases section.](./usecases/)

## Related projects
The integration is based on the following repositories:
1. https://github.com/alliander-opensource/der-scheduling
2. https://github.com/fledge-power[https://github.com/fledge-power]

## Collaboration and contributing
This is a collaboration of [Alliander](alliander.com), [Fraunhofer ISE](https://www.ise.fraunhofer.de/) and [MZ Automation](https://www.mz-automation.de) and you are welcome to contribute with your expertise on raising issues, contributing to documentation, testing, making suggestions or writing code. The final acceptance of the code is done by a Technical Steering Committee with the members stated above and consulting the Technical Steering Committee of FledgePower.

The work is based upon MZ Automation's [IEC 61850 Protocol Library](https://www.mz-automation.de/communication-protocols/iec-61850-protocol-library/).

## Roadmap
### Scheduling as stand alone-service (realized!)
This has been realized till sofar at [DER-scheduling-software](https://github.com/alliander-opensource/der-scheduling). The main functionalities are to control a DERs regarding:
- absolute power output
- maximal power output
- turning them on or off

The configuration of how the software is configured is realized with the [SCL file](https://github.com/alliander-opensource/der-scheduling/blob/main/models/der_scheduler.cid) which can be managed and adopted by any SCL compliant tool, e.g. [CoMPAS](https://github.com/com-pas). Schedules themselves are now fed into the software with an API from the CLI.
With positive results, the solution is benchmarked against German commercial FNN-steuerboxes to have the same behaviour and to have at least the same performance.

### Scheduling integraded with official communication protol (active!)
We currently integrate the IEC scheduling as stand-service into [FLEDGE](https://www.lfedge.org/projects/fledge/) in close collaboration with [FLEDGE POWER](https://lfenergy.org/projects/fledgepower/) as they share the vision to build productive software for the edge. (Please remark that "scheduling" in Fledge is not the same as IEC-scheduling or DER-scheduling developed here). In this stage we expect to communicate with one protocol (WAN at IEC61850-MMS) to central systems (SCADA / Substation automation like) and with one protocol (LAN at MQTT) to local devices.

### Scheduling integrated with several exchangable communication protocols (stretched goal)
In this stage we expect to have the integration as such, that it is possible to use more protocols to central systems (WAN) and more protocols to local devices (LAN) independently and parallell from each other.

### Scheduling integrated with security framework for registration, monitoring etc.
In this stage we expect to have the right features and in place so that this solution can be used on broader scale securely. A basis (partly outated) in Dutch language can be found [here](https://alliander.gitbook.io/interfacespecificatie-elektriciteit-productie-eenh/bijlage_3_gemaakte_keuzes_en_toelichting). 

## Architecture
The architecture defines the possibilities and limitations of the delivery.
In the first phase, the integration with Fledge-Power will be solid, but limited to a selected number of services only. This will guarantee the core functionality when more advanced integration can evaluate with both architectures.
More information about the architectural choises, see section -Architecture-


# Building and running

## Fledge test docker
To start the ReLevENT framework in a convenient way, use [the docker compose file](deployments/n61850-smqtt-ubuntu2004/docker-compose.yml) to start the most relevant components. See [this manual](deployments/n61850-smqtt-ubuntu2004/README.md) for details.

## Hedera 61850 Gateway docker
In order to connect to HEDERA, you will need credentials and the DER schedulers ip inserted into [the configuration ini](hedera-61850-gateway/docker-image/hedera-interface.ini). See [the example](hedera-61850-gateway/docker-image/example-hedera-interface.ini) for the expected format.
After the credentials and ip/port of the DER have been inserted, you can build and run the docker image by calling [the build script](hedera-61850-gateway/create-docker-and-run.sh).
The script assumes another docker containing named 'der-scheduler' is running an will try to connect to its network. For testing, the [DER scheduler](https://github.com/alliander-opensource/der-scheduling/tree/docker#docker-build) can be used until the ReLevENT docker is ready.


## Manually building lib61850

To install the dependencies you can run the requirements.sh script.

If you want to install them manually :

To build IEC61850 C/C++ North plugin, you need to download lib61850 at:
https://github.com/mz-automation/libiec61850

```bash
$ git clone https://github.com/mz-automation/libiec61850.git
$ cd libiec61850
$ export LIB_IEC61850=`pwd`
```
As shown above, you need a $LIB_IEC61850 env var set to the source tree of the
library.

Then, you can build libiec61850 with:

```bash
$ cd libiec61850
$ cmake -DBUILD_TESTS=NO -DBUILD_EXAMPLES=NO ..
$ make
$ sudo make install
$ sudo ldconfig
```

If you want scheduling support also, you can add the der-scheduling library as follows:

```bash
$ mkdir -p dependencies
$ cd dependencies
$ echo Fetching iec61850 scheduler library
$ git clone -b develop_mz git@github.com:alliander-opensource/der-scheduling.git
$ mv alliander-ise-iec-61850-scheduler der-scheduling
$ cd der-scheduling
```

Build
-----


To build the iec61850 plugin, once you are in the plugin source tree you need to run:

To build a release:

```bash 
$ mkdir build
$ cd build
$ cmake -DCMAKE_BUILD_TYPE=Release ..
$ make
```

To build with unit tests and code coverage:

```bash
$ mkdir build
$ cd build
$ cmake -DCMAKE_BUILD_TYPE=Coverage ..
$ make
```

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
