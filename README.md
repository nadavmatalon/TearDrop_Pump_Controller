#TearDrop | Pump Controller

##Table of Contents

* [Screenshots](#screenshots)
* [General Description](#general-description)
* [License](#license)


##Screenshots

<table>
	<tr>
		<td align="center" width=25% >
			<a href="https://raw.githubusercontent.com/nadavmatalon/TearDrop_Pump_Controller/master/images/TDPC_Arduino.jpg">
				<img src="images/TDPC_Arduino.jpg" height="105px" />
			</a>
		</td>
		<td align="center" width=25% >
			<a href="https://raw.githubusercontent.com/nadavmatalon/TearDrop_Pump_Controller/master/images/TDPC_PCB.jpg">
				<img src="images/TDPC_PCB.jpg" height="105px" />
			</a>
		</td>
		<td align="center" width=25% >
			<a href="https://raw.githubusercontent.com/nadavmatalon/TearDrop_Pump_Controller/master/images/TearDrop_1.jpg">
				<img src="images/TearDrop_1.jpg" height="105px" />
			</a>
		</td>
		<td align="center" width=25% >
			<a href="https://raw.githubusercontent.com/nadavmatalon/TearDrop_Pump_Controller/master/images/TearDrop_2.jpg">
				<img src="images/TearDrop_2.jpg" height="105px" />
			</a>
		</td>
	</tr>
</table>


##General Description

This repo contains the following files:

* __arduino__

    The source code for the ATmega328P that runs the Pump Controller 

    The file can can be directly accessed by clicking this link:

    * [Pump Controller](arduino/TearDrop_Pump_Controller/TearDrop_Pump_Controller.ino)

	The source code requires three supporting open-source [Arduino](http://www.arduino.cc/) libraries.

	For convenience, these libraries are inlcuded here in their own sub-folder.

	Credit goes to:

	[PinChangeInt](http://playground.arduino.cc/Main/PinChangeInt)

	[OneWire](https://github.com/ntruchsess/arduino-OneWire/tree/master)

	[WSWire](https://github.com/steamfire/WSWireLib)


* __eagle__

    This folder contains the complete hardware schematics & PCB design of the Pump Controller.

    __Please do not click on the files in this sub-folder as they are in foramts that 
    are incompatible with Github's hosting server__.

    However, the files can be downloaded and run as usual with [CadSoft](http://www.cadsoftusa.com/?language=en)'s 
    free version of [Eagle](http://www.cadsoftusa.com/download-eagle/freeware/).


* __parts__

	A speadsheet with all information about the parts used for building the Pump Controller.

	__The file in this sub-folder is also in a foramt that's incompatible with Github's hosting server__.

    However, it can be downloaded and run as usual with any spreadsheet editing software.






##  License

<p>Released under the <a href="http://www.opensource.org/licenses/MIT">MIT license</a>.</p>

