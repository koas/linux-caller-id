# Introduction #

When you launch the program for the first time it will pop up the config window, where you can set the values for the modem connection and commands. Once you've filled this info press the _Save and run_ button to save the config and connect to the modem. The config is saved in your user folder, in a folder named .LinuxCallerId and with the name config.

The program puts an icon in the system tray, you can click this icon to show or hide the config window. Once the config is saved the next time the program runs will not show the config window, and sit quietly in the tray.

When the modem detects an incoming call, a notification will appear showing the calling number. If the number is in the database, the description will also be shown. If the number is marked as _blocked_, the modem will answer and then hang up. If the number is not in the database or marked as _not blocked_ the program lets it ring.


# Configuration #

The configuration tab allows you to set up the modem connection options.

![http://s8.postimage.org/9e6fwyj9h/recent2.png](http://s8.postimage.org/9e6fwyj9h/recent2.png)

The port field may vary from my computer (/dev/ttyACM0) to yours. To find the port, first look for the name of the device. If it's a PCI modem, run command **lspci**, if it's USB run **lsusb**. My modem is USB, so I ran lsusb and here is the output:

```
Bus 001 Device 001: ID 1d6b:0002 Linux Foundation 2.0 root hub
Bus 002 Device 001: ID 1d6b:0001 Linux Foundation 1.1 root hub
Bus 001 Device 010: ID 05e3:0608 Genesys Logic, Inc. USB-2.0 4-Port HUB
Bus 001 Device 004: ID 2040:5500 Hauppauge Windham
Bus 001 Device 005: ID 058f:6362 Alcor Micro Corp. Flash Card Reader/Writer
Bus 002 Device 002: ID 1e7d:2ced ROCCAT Kone Mouse
Bus 001 Device 011: ID 05e3:0608 Genesys Logic, Inc. USB-2.0 4-Port HUB
Bus 001 Device 012: ID 18ec:3299 Arkmicro Technologies Inc. 
Bus 001 Device 013: ID 0572:1329 Conexant Systems (Rockwell), Inc. 
Bus 001 Device 009: ID 0fce:0171 Sony Ericsson Mobile Communications AB
```

The modem device is the Conexant Systems one, so I just ran a simple find command to look for it in the /dev folder

```
koas@home:/$ find /dev -name '*Conexant*'
/dev/serial/by-id/usb-Conexant_USB_Modem_24680246-if00
```

Once you get the file, just do a ls -l to find where it points to:

```
koas@home:/$ ls -l /dev/serial/by-id/usb-Conexant_USB_Modem_24680246-if00 
lrwxrwxrwx 1 root root 13 2012-09-19 10:12 /dev/serial/by-id/usb-Conexant_USB_Modem_24680246-if00 -> ../../ttyACM0
```

So now I know that my port is /dev/ttyACM0.

Once you found the port, the other parameters (baud rate, data bits, parity, stop bits and flow control) are pretty much standard I think.

If after doing this you still get the message "Could not open modem, please check config" maybe you don't have permissions to open the port. This can be fixed by adding your user to the group of the device. In my box, for example, /dev/ttyACM0 has the user root and the group dialout:

```
crw-rw----  1 root dialout   166,   0 ene 15 16:53 ttyACM0
```

so I add my user (koas) to that group by typing:

```
sudo usermod -a -G dialout koas
```

and by logging out and in again the problem is solved. Always remember to change ttyACM0 to the port of your modem.

- Activate Caller Id Command: I found this command in a reference manual from Conexant. If your modem uses another chip you'll have to look for the reference manual for this code, and be sure you set it to the value for formatted data. Please send it to me so I can add it to this wiki.

- Caller Id Variable and Separator: when the modem detects an incoming call it sends the caller Id to the program. In my modem it uses this format:

NMBR = XXXXXXXXX

Hence the variable is NMBR and the separator is =. This may change from modem to modem.

- Answer command: this is the command sent to the modem to answer the call. Again, it may be different for your modem.

- Hang up command: this is the command sent to hang up the call. The reference for my modem indicates the command is ATH0, but I noticed that just sending the first A hangs the call (another part of the reference indicates that sending any data to the modem will abort the last operation, so that's maybe the reason).

Again, if you have a different modem and manage to make the program work send me the details so I can add them here and save time for future users.

# Database #

Here you can keep a list of numbers with a description and a flag that tells the program to block it or not. The data is saved in a plain text file in .LinuxCallerId/db

![http://s8.postimage.org/g76st8a2t/database.png](http://s8.postimage.org/g76st8a2t/database.png)

# Recent calls #

All incoming calls since the program started are shown here. You can select any of the numbers and add it to your database.

![http://s11.postimage.org/ekq6bh5gz/recent_Calls.png](http://s11.postimage.org/ekq6bh5gz/recent_Calls.png)

# Log #

All communication with the modem is logged here. You can use it to get the format your modem uses to tell you the caller id for the configuration tab.

![http://s8.postimage.org/4gsvbuhad/log.png](http://s8.postimage.org/4gsvbuhad/log.png)

# Final words #

This is just a little program I wrote in one day since I didn't found anything usable in the net. There's a lot of room for improving, so feel free to download the source code and play with it.

Bug reports and suggestions are welcome!

This program uses the [QextSerialport library](http://code.google.com/p/qextserialport/). Icons by [Webdesigner Depot](http://www.webdesignerdepot.com/)
and [nuoveXT](http://nuovext.pwsp.net/)